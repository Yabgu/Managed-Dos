/*
 *  Copyright (C) 2002-2010  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

 /* $Id: cache.h,v 1.20 2009-07-12 20:13:05 c2woody Exp $ */

class CacheBlock {
public:
	void Clear(void);
	void LinkTo(unsigned index, CacheBlock * toblock) {
		assert(toblock);
		link[index].to = toblock;
		link[index].next = toblock->link[index].from;
		toblock->link[index].from = this;
	}
	struct {
		System::UInt16 start, end;				//Where the page is the original code
		CodePageHandler * handler;		//Page containing this code
	} page;
	struct {
		System::Byte * start;					//Where in the cache are we
		unsigned size;
		CacheBlock * next;
		System::Byte * wmapmask;
		System::UInt16 maskstart;
		System::UInt16 masklen;
	} cache;
	struct {
		unsigned index;
		CacheBlock * next;
	} hash;
	struct {
		CacheBlock * to;
		CacheBlock * next;
		CacheBlock * from;
	} link[2];
	CacheBlock * crossblock;
};

static struct {
	struct {
		CacheBlock * first;
		CacheBlock * active;
		CacheBlock * free;
		CacheBlock * running;
	} block;
	System::Byte * pos;
	CodePageHandler * free_pages;
	CodePageHandler * used_pages;
	CodePageHandler * last_page;
} cache;

static CacheBlock link_blocks[2];

class CodePageHandler : public PageHandler {
public:
	CodePageHandler() {
		invalidation_map = NULL;
	}
	void SetupAt(unsigned _phys_page, PageHandler * _old_pagehandler) {
		phys_page = _phys_page;
		old_pagehandler = _old_pagehandler;
		flags = old_pagehandler->flags | PFLAG_HASCODE;
		flags &= ~PFLAG_WRITEABLE;
		active_blocks = 0;
		active_count = 16;
		memset(&hash_map, 0, sizeof(hash_map));
		memset(&write_map, 0, sizeof(write_map));
		if (invalidation_map != NULL) {
			free(invalidation_map);
			invalidation_map = NULL;
		}
	}
	bool InvalidateRange(unsigned start, unsigned end) {
		int index = 1 + (end >> DYN_HASH_SHIFT);
		bool is_current_block = false;
		System::UInt32 ip_point = SegPhys(cs) + reg_eip;
		ip_point = (PAGING_GetPhysicalPage(ip_point) - (phys_page << 12)) + (ip_point & 0xfff);
		while (index >= 0) {
			unsigned map = 0;
			for (unsigned count = start; count <= end; count++) map += write_map[count];
			if (!map) return is_current_block;
			CacheBlock * block = hash_map[index];
			while (block) {
				CacheBlock * nextblock = block->hash.next;
				if (start <= block->page.end && end >= block->page.start) {
					if (ip_point <= block->page.end && ip_point >= block->page.start) is_current_block = true;
					block->Clear();
				}
				block = nextblock;
			}
			index--;
		}
		return is_current_block;
	}
	void writeb(PhysPt addr, unsigned val) {
		if (GCC_UNLIKELY(old_pagehandler->flags&PFLAG_HASROM)) return;
		if (GCC_UNLIKELY((old_pagehandler->flags&PFLAG_READABLE) != PFLAG_READABLE)) {
			E_Exit("wb:non-readable code page found that is no ROM page");
		}
		addr &= 4095;
		if (host_readb(hostmem + addr) == (System::Byte)val) return;
		host_writeb(hostmem + addr, val);
		if (!*(System::Byte*)&write_map[addr]) {
			if (active_blocks) return;
			active_count--;
			if (!active_count) Release();
			return;
		}
		else if (!invalidation_map) {
			invalidation_map = (System::Byte*)malloc(4096);
			memset(invalidation_map, 0, 4096);
		}
		invalidation_map[addr]++;
		InvalidateRange(addr, addr);
	}
	void writew(PhysPt addr, unsigned val) {
		if (GCC_UNLIKELY(old_pagehandler->flags&PFLAG_HASROM)) return;
		if (GCC_UNLIKELY((old_pagehandler->flags&PFLAG_READABLE) != PFLAG_READABLE)) {
			E_Exit("ww:non-readable code page found that is no ROM page");
		}
		addr &= 4095;
		if (host_readw(hostmem + addr) == (System::UInt16)val) return;
		host_writew(hostmem + addr, val);
		if (!*(System::UInt16*)&write_map[addr]) {
			if (active_blocks) return;
			active_count--;
			if (!active_count) Release();
			return;
		}
		else if (!invalidation_map) {
			invalidation_map = (System::Byte*)malloc(4096);
			memset(invalidation_map, 0, 4096);
		}
		(*(System::UInt16*)&invalidation_map[addr]) += 0x101;
		InvalidateRange(addr, addr + 1);
	}
	void writed(PhysPt addr, unsigned val) {
		if (GCC_UNLIKELY(old_pagehandler->flags&PFLAG_HASROM)) return;
		if (GCC_UNLIKELY((old_pagehandler->flags&PFLAG_READABLE) != PFLAG_READABLE)) {
			E_Exit("wd:non-readable code page found that is no ROM page");
		}
		addr &= 4095;
		if (host_readd(hostmem + addr) == (System::UInt32)val) return;
		host_writed(hostmem + addr, val);
		if (!*(System::UInt32*)&write_map[addr]) {
			if (active_blocks) return;
			active_count--;
			if (!active_count) Release();
			return;
		}
		else if (!invalidation_map) {
			invalidation_map = (System::Byte*)malloc(4096);
			memset(invalidation_map, 0, 4096);
		}
		(*(System::UInt32*)&invalidation_map[addr]) += 0x1010101;
		InvalidateRange(addr, addr + 3);
	}
	bool writeb_checked(PhysPt addr, unsigned val) {
		if (GCC_UNLIKELY(old_pagehandler->flags&PFLAG_HASROM)) return false;
		if (GCC_UNLIKELY((old_pagehandler->flags&PFLAG_READABLE) != PFLAG_READABLE)) {
			E_Exit("cb:non-readable code page found that is no ROM page");
		}
		addr &= 4095;
		if (host_readb(hostmem + addr) == (System::Byte)val) return false;
		if (!*(System::Byte*)&write_map[addr]) {
			if (!active_blocks) {
				active_count--;
				if (!active_count) Release();
			}
		}
		else {
			if (!invalidation_map) {
				invalidation_map = (System::Byte*)malloc(4096);
				memset(invalidation_map, 0, 4096);
			}
			invalidation_map[addr]++;
			if (InvalidateRange(addr, addr)) {
				cpu.exception.which = SMC_CURRENT_BLOCK;
				return true;
			}
		}
		host_writeb(hostmem + addr, val);
		return false;
	}
	bool writew_checked(PhysPt addr, unsigned val) {
		if (GCC_UNLIKELY(old_pagehandler->flags&PFLAG_HASROM)) return false;
		if (GCC_UNLIKELY((old_pagehandler->flags&PFLAG_READABLE) != PFLAG_READABLE)) {
			E_Exit("cw:non-readable code page found that is no ROM page");
		}
		addr &= 4095;
		if (host_readw(hostmem + addr) == (System::UInt16)val) return false;
		if (!*(System::UInt16*)&write_map[addr]) {
			if (!active_blocks) {
				active_count--;
				if (!active_count) Release();
			}
		}
		else {
			if (!invalidation_map) {
				invalidation_map = (System::Byte*)malloc(4096);
				memset(invalidation_map, 0, 4096);
			}
			(*(System::UInt16*)&invalidation_map[addr]) += 0x101;
			if (InvalidateRange(addr, addr + 1)) {
				cpu.exception.which = SMC_CURRENT_BLOCK;
				return true;
			}
		}
		host_writew(hostmem + addr, val);
		return false;
	}
	bool writed_checked(PhysPt addr, unsigned val) {
		if (GCC_UNLIKELY(old_pagehandler->flags&PFLAG_HASROM)) return false;
		if (GCC_UNLIKELY((old_pagehandler->flags&PFLAG_READABLE) != PFLAG_READABLE)) {
			E_Exit("cd:non-readable code page found that is no ROM page");
		}
		addr &= 4095;
		if (host_readd(hostmem + addr) == (System::UInt32)val) return false;
		if (!*(System::UInt32*)&write_map[addr]) {
			if (!active_blocks) {
				active_count--;
				if (!active_count) Release();
			}
		}
		else {
			if (!invalidation_map) {
				invalidation_map = (System::Byte*)malloc(4096);
				memset(invalidation_map, 0, 4096);
			}
			(*(System::UInt32*)&invalidation_map[addr]) += 0x1010101;
			if (InvalidateRange(addr, addr + 3)) {
				cpu.exception.which = SMC_CURRENT_BLOCK;
				return true;
			}
		}
		host_writed(hostmem + addr, val);
		return false;
	}
	void AddCacheBlock(CacheBlock * block) {
		unsigned index = 1 + (block->page.start >> DYN_HASH_SHIFT);
		block->hash.next = hash_map[index];
		block->hash.index = index;
		hash_map[index] = block;
		block->page.handler = this;
		active_blocks++;
	}
	void AddCrossBlock(CacheBlock * block) {
		block->hash.next = hash_map[0];
		block->hash.index = 0;
		hash_map[0] = block;
		block->page.handler = this;
		active_blocks++;
	}
	void DelCacheBlock(CacheBlock * block) {
		active_blocks--;
		active_count = 16;
		CacheBlock * * where = &hash_map[block->hash.index];
		while (*where != block) {
			where = &((*where)->hash.next);
			//Will crash if a block isn't found, which should never happen.
		}
		*where = block->hash.next;
		if (GCC_UNLIKELY(block->cache.wmapmask != NULL)) {
			for (unsigned i = block->page.start; i < block->cache.maskstart; i++) {
				if (write_map[i]) write_map[i]--;
			}
			unsigned maskct = 0;
			for (unsigned i = block->cache.maskstart; i <= block->page.end; i++, maskct++) {
				if (write_map[i]) {
					if ((maskct >= block->cache.masklen) || (!block->cache.wmapmask[maskct])) write_map[i]--;
				}
			}
			free(block->cache.wmapmask);
			block->cache.wmapmask = NULL;
		}
		else {
			for (unsigned i = block->page.start; i <= block->page.end; i++) {
				if (write_map[i]) write_map[i]--;
			}
		}
	}
	void Release(void) {
		MEM_SetPageHandler(phys_page, 1, old_pagehandler);
		PAGING_ClearTLB();
		if (prev) prev->next = next;
		else cache.used_pages = next;
		if (next) next->prev = prev;
		else cache.last_page = prev;
		next = cache.free_pages;
		cache.free_pages = this;
		prev = 0;
	}
	void ClearRelease(void) {
		for (unsigned index = 0; index < (1 + DYN_PAGE_HASH); index++) {
			CacheBlock * block = hash_map[index];
			while (block) {
				CacheBlock * nextblock = block->hash.next;
				block->page.handler = 0;			//No need, full clear
				block->Clear();
				block = nextblock;
			}
		}
		Release();
	}
	CacheBlock * FindCacheBlock(unsigned start) {
		CacheBlock * block = hash_map[1 + (start >> DYN_HASH_SHIFT)];
		while (block) {
			if (block->page.start == start) return block;
			block = block->hash.next;
		}
		return 0;
	}
	HostPt GetHostReadPt(unsigned phys_page) {
		hostmem = old_pagehandler->GetHostReadPt(phys_page);
		return hostmem;
	}
	HostPt GetHostWritePt(unsigned phys_page) {
		return GetHostReadPt(phys_page);
	}
public:
	System::Byte write_map[4096];
	System::Byte * invalidation_map;
	CodePageHandler * next, *prev;
private:
	PageHandler * old_pagehandler;
	CacheBlock * hash_map[1 + DYN_PAGE_HASH];
	unsigned active_blocks;
	unsigned active_count;
	HostPt hostmem;
	unsigned phys_page;
};


static INLINE void cache_addunsedblock(CacheBlock * block) {
	block->cache.next = cache.block.free;
	cache.block.free = block;
}

static CacheBlock * cache_getblock(void) {
	CacheBlock * ret = cache.block.free;
	if (!ret) E_Exit("Ran out of CacheBlocks");
	cache.block.free = ret->cache.next;
	ret->cache.next = 0;
	return ret;
}

void CacheBlock::Clear(void) {
	/* Check if this is not a cross page block */
	if (hash.index) for (unsigned ind = 0; ind < 2; ind++) {
		CacheBlock * fromlink = link[ind].from;
		link[ind].from = 0;
		while (fromlink) {
			CacheBlock * nextlink = fromlink->link[ind].next;
			fromlink->link[ind].next = 0;
			fromlink->link[ind].to = &link_blocks[ind];
			fromlink = nextlink;
		}
		if (link[ind].to != &link_blocks[ind]) {
			CacheBlock * * wherelink = &link[ind].to->link[ind].from;
			while (*wherelink != this && *wherelink) {
				wherelink = &(*wherelink)->link[ind].next;
			}
			if (*wherelink)
				*wherelink = (*wherelink)->link[ind].next;
			else
				LOG(LOG_CPU, LOG_ERROR)("Cache anomaly. please investigate");
		}
	}
	else
		cache_addunsedblock(this);
	if (crossblock) {
		crossblock->crossblock = 0;
		crossblock->Clear();
		crossblock = 0;
	}
	if (page.handler) {
		page.handler->DelCacheBlock(this);
		page.handler = 0;
	}
	if (cache.wmapmask) {
		free(cache.wmapmask);
		cache.wmapmask = NULL;
	}
}


static CacheBlock * cache_openblock(void) {
	CacheBlock * block = cache.block.active;
	/* check for enough space in this block */
	unsigned size = block->cache.size;
	CacheBlock * nextblock = block->cache.next;
	if (block->page.handler)
		block->Clear();
	while (size < CACHE_MAXSIZE) {
		if (!nextblock)
			goto skipresize;
		size += nextblock->cache.size;
		CacheBlock * tempblock = nextblock->cache.next;
		if (nextblock->page.handler)
			nextblock->Clear();
		cache_addunsedblock(nextblock);
		nextblock = tempblock;
	}
skipresize:
	block->cache.size = size;
	block->cache.next = nextblock;
	cache.pos = block->cache.start;
	return block;
}

static void cache_closeblock(void) {
	CacheBlock * block = cache.block.active;
	block->link[0].to = &link_blocks[0];
	block->link[1].to = &link_blocks[1];
	block->link[0].from = 0;
	block->link[1].from = 0;
	block->link[0].next = 0;
	block->link[1].next = 0;
	/* Close the block with correct alignments */
	unsigned written = cache.pos - block->cache.start;
	if (written > block->cache.size) {
		if (!block->cache.next) {
			if (written > block->cache.size + CACHE_MAXSIZE) E_Exit("CacheBlock overrun 1 %d", written - block->cache.size);
		}
		else E_Exit("CacheBlock overrun 2 written %d size %d", written, block->cache.size);
	}
	else {
		unsigned left = block->cache.size - written;
		/* Smaller than cache align then don't bother to resize */
		if (left > CACHE_ALIGN) {
			unsigned new_size = ((written - 1) | (CACHE_ALIGN - 1)) + 1;
			CacheBlock * newblock = cache_getblock();
			newblock->cache.start = block->cache.start + new_size;
			newblock->cache.size = block->cache.size - new_size;
			newblock->cache.next = block->cache.next;
			block->cache.next = newblock;
			block->cache.size = new_size;
		}
	}
	/* Advance the active block pointer */
	if (!block->cache.next) {
		//		LOG_MSG("Cache full restarting");
		cache.block.active = cache.block.first;
	}
	else {
		cache.block.active = block->cache.next;
	}
}

static INLINE void cache_addb(intptr_t val) {
	*cache.pos++ = val;
}

static INLINE void cache_addw(intptr_t val) {
	*(System::UInt16*)cache.pos = val;
	cache.pos += 2;
}

static INLINE void cache_addd(intptr_t val) {
	*((System::UInt32*)cache.pos) = val;
	cache.pos += 4;
}


static void gen_return(BlockReturn retcode);

static System::Byte * cache_code_start_ptr = NULL;
static System::Byte * cache_code = NULL;
static System::Byte * cache_code_link_blocks = NULL;
static CacheBlock * cache_blocks = NULL;

/* Define temporary pagesize so the MPROTECT case and the regular case share as much code as possible */
#if (C_HAVE_MPROTECT)
#define PAGESIZE_TEMP PAGESIZE
#else 
#define PAGESIZE_TEMP 4096
#endif

static bool cache_initialized = false;

static void cache_init(bool enable) {
	if (enable) {
		if (cache_initialized) return;
		cache_initialized = true;
		if (cache_blocks == NULL) {
			cache_blocks = static_cast<CacheBlock*>(malloc(CACHE_BLOCKS * sizeof(CacheBlock)));
			if (!cache_blocks) E_Exit("Allocating cache_blocks has failed");
			memset(cache_blocks, 0, sizeof(CacheBlock)*CACHE_BLOCKS);
			cache.block.free = &cache_blocks[0];
			for (int i = 0; i < CACHE_BLOCKS - 1; i++) {
				cache_blocks[i].link[0].to = (CacheBlock *)1;
				cache_blocks[i].link[1].to = (CacheBlock *)1;
				cache_blocks[i].cache.next = &cache_blocks[i + 1];
			}
		}
		if (cache_code_start_ptr == NULL) {
#if defined (WIN32)
			cache_code_start_ptr = (System::Byte*)VirtualAlloc(0, CACHE_TOTAL + CACHE_MAXSIZE + PAGESIZE_TEMP - 1 + PAGESIZE_TEMP,
				MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (!cache_code_start_ptr)
				cache_code_start_ptr = (System::Byte*)malloc(CACHE_TOTAL + CACHE_MAXSIZE + PAGESIZE_TEMP - 1 + PAGESIZE_TEMP);
#else
			cache_code_start_ptr = (System::Byte*)malloc(CACHE_TOTAL + CACHE_MAXSIZE + PAGESIZE_TEMP - 1 + PAGESIZE_TEMP);
#endif
			if (!cache_code_start_ptr) E_Exit("Allocating dynamic core cache memory failed");

			cache_code = (System::Byte*)(((int)cache_code_start_ptr + PAGESIZE_TEMP - 1) & ~(PAGESIZE_TEMP - 1)); //MEM LEAK. store old pointer if you want to free it.

			cache_code_link_blocks = cache_code;
			cache_code += PAGESIZE_TEMP;

#if (C_HAVE_MPROTECT)
			if (mprotect(cache_code_link_blocks, CACHE_TOTAL + CACHE_MAXSIZE + PAGESIZE_TEMP, PROT_WRITE | PROT_READ | PROT_EXEC))
				LOG_MSG("Setting excute permission on the code cache has failed!");
#endif
			CacheBlock * block = cache_getblock();
			cache.block.first = block;
			cache.block.active = block;
			block->cache.start = &cache_code[0];
			block->cache.size = CACHE_TOTAL;
			block->cache.next = 0;								//Last block in the list
		}
		/* Setup the default blocks for block linkage returns */
		cache.pos = &cache_code_link_blocks[0];
		link_blocks[0].cache.start = cache.pos;
		gen_return(BR_Link1);
		cache.pos = &cache_code_link_blocks[32];
		link_blocks[1].cache.start = cache.pos;
		gen_return(BR_Link2);
		cache.free_pages = 0;
		cache.last_page = 0;
		cache.used_pages = 0;
		/* Setup the code pages */
		for (int i = 0; i < CACHE_PAGES; i++) {
			CodePageHandler * newpage = new CodePageHandler();
			newpage->next = cache.free_pages;
			cache.free_pages = newpage;
		}
	}
}

static void cache_close(void) {
	/*	for (;;) {
			if (cache.used_pages) {
				CodePageHandler * cpage=cache.used_pages;
				CodePageHandler * npage=cache.used_pages->next;
				cpage->ClearRelease();
				delete cpage;
				cache.used_pages=npage;
			} else break;
		}
		if (cache_blocks != NULL) {
			free(cache_blocks);
			cache_blocks = NULL;
		}
		if (cache_code_start_ptr != NULL) {
			### care: under windows VirtualFree() has to be used if
			###       VirtualAlloc was used for memory allocation
			free(cache_code_start_ptr);
			cache_code_start_ptr = NULL;
		}
		cache_code = NULL;
		cache_code_link_blocks = NULL;
		cache_initialized = false; */
}
