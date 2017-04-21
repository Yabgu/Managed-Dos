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

#ifndef DOSBOX_MEM_H
#define DOSBOX_MEM_H

#ifndef DOSBOX_DOSBOX_H
#include "dosbox.h"
#endif
#include <cstdint>

typedef uint32_t PhysPt;
typedef uint8_t * HostPt;
typedef uint32_t RealPt;

typedef System::Int32 MemHandle;

#define MEM_PAGESIZE 4096

extern HostPt MemBase;
HostPt GetMemBase(void);

bool MEM_A20_Enabled(void);
void MEM_A20_Enable(bool enable);

/* Memory management / EMS mapping */
HostPt MEM_GetBlockPage(void);
unsigned MEM_FreeTotal(void);			//Free 4 kb pages
unsigned MEM_FreeLargest(void);			//Largest free 4 kb pages block
unsigned MEM_TotalPages(void);			//Total amount of 4 kb pages
unsigned MEM_AllocatedPages(MemHandle handle); // amount of allocated pages of handle
MemHandle MEM_AllocatePages(unsigned pages,bool sequence);
MemHandle MEM_GetNextFreePage(void);
PhysPt MEM_AllocatePage(void);
void MEM_ReleasePages(MemHandle handle);
bool MEM_ReAllocatePages(MemHandle & handle,unsigned pages,bool sequence);

MemHandle MEM_NextHandle(MemHandle handle);
MemHandle MEM_NextHandleAt(MemHandle handle,unsigned where);

/* 
	The folowing six functions are used everywhere in the end so these should be changed for
	Working on big or little endian machines 
*/

#if defined(WORDS_BIGENDIAN) || !defined(C_UNALIGNED_MEMORY)

static INLINE System::Byte host_readb(HostPt off) {
	return off[0];
}
static INLINE System::UInt16 host_readw(HostPt off) {
	return off[0] | (off[1] << 8);
}
static INLINE System::UInt32 host_readd(HostPt off) {
	return off[0] | (off[1] << 8) | (off[2] << 16) | (off[3] << 24);
}
static INLINE void host_writeb(HostPt off,System::Byte val) {
	off[0]=val;
}
static INLINE void host_writew(HostPt off,System::UInt16 val) {
	off[0]=(System::Byte)(val);
	off[1]=(System::Byte)(val >> 8);
}
static INLINE void host_writed(HostPt off,System::UInt32 val) {
	off[0]=(System::Byte)(val);
	off[1]=(System::Byte)(val >> 8);
	off[2]=(System::Byte)(val >> 16);
	off[3]=(System::Byte)(val >> 24);
}

#else

static INLINE System::Byte host_readb(HostPt off) {
	return *(System::Byte *)off;
}
static INLINE System::UInt16 host_readw(HostPt off) {
	return *(System::UInt16 *)off;
}
static INLINE System::UInt32 host_readd(HostPt off) {
	return *(System::UInt32 *)off;
}
static INLINE void host_writeb(HostPt off,System::Byte val) {
	*(System::Byte *)(off)=val;
}
static INLINE void host_writew(HostPt off,System::UInt16 val) {
	*(System::UInt16 *)(off)=val;
}
static INLINE void host_writed(HostPt off,System::UInt32 val) {
	*(System::UInt32 *)(off)=val;
}

#endif


static INLINE void var_write(System::Byte * var, System::Byte val) {
	host_writeb((HostPt)var, val);
}

static INLINE void var_write(System::UInt16 * var, System::UInt16 val) {
	host_writew((HostPt)var, val);
}

static INLINE void var_write(System::UInt32 * var, System::UInt32 val) {
	host_writed((HostPt)var, val);
}

/* The Folowing six functions are slower but they recognize the paged memory system */

System::Byte  mem_readb(PhysPt pt);
System::UInt16 mem_readw(PhysPt pt);
System::UInt32 mem_readd(PhysPt pt);

void mem_writeb(PhysPt pt,System::Byte val);
void mem_writew(PhysPt pt,System::UInt16 val);
void mem_writed(PhysPt pt,System::UInt32 val);

static INLINE void phys_writeb(PhysPt addr,System::Byte val) {
	host_writeb(MemBase+addr,val);
}
static INLINE void phys_writew(PhysPt addr,System::UInt16 val){
	host_writew(MemBase+addr,val);
}
static INLINE void phys_writed(PhysPt addr,System::UInt32 val){
	host_writed(MemBase+addr,val);
}

static INLINE System::Byte phys_readb(PhysPt addr) {
	return host_readb(MemBase+addr);
}
static INLINE System::UInt16 phys_readw(PhysPt addr){
	return host_readw(MemBase+addr);
}
static INLINE System::UInt32 phys_readd(PhysPt addr){
	return host_readd(MemBase+addr);
}

/* These don't check for alignment, better be sure it's correct */

void MEM_BlockWrite(PhysPt pt,void const * const data,unsigned size);
void MEM_BlockRead(PhysPt pt,void * data,unsigned size);
void MEM_BlockCopy(PhysPt dest,PhysPt src,unsigned size);
void MEM_StrCopy(PhysPt pt,char * data,unsigned size);

void mem_memcpy(PhysPt dest,PhysPt src,unsigned size);
unsigned mem_strlen(PhysPt pt);
void mem_strcpy(PhysPt dest,PhysPt src);

/* The folowing functions are all shortcuts to the above functions using physical addressing */

static INLINE System::Byte real_readb(System::UInt16 seg,System::UInt16 off) {
	return mem_readb((seg<<4)+off);
}
static INLINE System::UInt16 real_readw(System::UInt16 seg,System::UInt16 off) {
	return mem_readw((seg<<4)+off);
}
static INLINE System::UInt32 real_readd(System::UInt16 seg,System::UInt16 off) {
	return mem_readd((seg<<4)+off);
}

static INLINE void real_writeb(System::UInt16 seg,System::UInt16 off,System::Byte val) {
	mem_writeb(((seg<<4)+off),val);
}
static INLINE void real_writew(System::UInt16 seg,System::UInt16 off,System::UInt16 val) {
	mem_writew(((seg<<4)+off),val);
}
static INLINE void real_writed(System::UInt16 seg,System::UInt16 off,System::UInt32 val) {
	mem_writed(((seg<<4)+off),val);
}


static INLINE System::UInt16 RealSeg(RealPt pt) {
	return (System::UInt16)(pt>>16);
}

static INLINE System::UInt16 RealOff(RealPt pt) {
	return (System::UInt16)(pt&0xffff);
}

static INLINE PhysPt Real2Phys(RealPt pt) {
	return (RealSeg(pt)<<4) +RealOff(pt);
}

static INLINE PhysPt PhysMake(System::UInt16 seg,System::UInt16 off) {
	return (seg<<4)+off;
}

static INLINE RealPt RealMake(System::UInt16 seg,System::UInt16 off) {
	return (seg<<16)+off;
}

static INLINE void RealSetVec(System::Byte vec,RealPt pt) {
	mem_writed(vec<<2,pt);
}

static INLINE void RealSetVec(System::Byte vec,RealPt pt,RealPt &old) {
	old = mem_readd(vec<<2);
	mem_writed(vec<<2,pt);
}

static INLINE RealPt RealGetVec(System::Byte vec) {
	return mem_readd(vec<<2);
}	

#endif

