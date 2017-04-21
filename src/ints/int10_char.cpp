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

/* $Id: int10_char.cpp,v 1.60 2009-10-15 20:36:56 c2woody Exp $ */

/* Character displaying moving functions */

#include "dosbox.h"
#include "bios.h"
#include "mem.h"
#include "inout.h"
#include "int10.h"

static void CGA2_CopyRow(System::Byte cleft,System::Byte cright,System::Byte rold,System::Byte rnew,PhysPt base) {
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+((CurMode->twidth*rnew)*(cheight/2)+cleft);
	PhysPt src=base+((CurMode->twidth*rold)*(cheight/2)+cleft);
	unsigned copy=(cright-cleft);
	unsigned nextline=CurMode->twidth;
	for (unsigned i=0;i<cheight/2U;i++) {
		MEM_BlockCopy(dest,src,copy);
		MEM_BlockCopy(dest+8*1024,src+8*1024,copy);
		dest+=nextline;src+=nextline;
	}
}

static void CGA4_CopyRow(System::Byte cleft,System::Byte cright,System::Byte rold,System::Byte rnew,PhysPt base) {
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+((CurMode->twidth*rnew)*(cheight/2)+cleft)*2;
	PhysPt src=base+((CurMode->twidth*rold)*(cheight/2)+cleft)*2;	
	unsigned copy=(cright-cleft)*2;unsigned nextline=CurMode->twidth*2;
	for (unsigned i=0;i<cheight/2U;i++) {
		MEM_BlockCopy(dest,src,copy);
		MEM_BlockCopy(dest+8*1024,src+8*1024,copy);
		dest+=nextline;src+=nextline;
	}
}

static void TANDY16_CopyRow(System::Byte cleft,System::Byte cright,System::Byte rold,System::Byte rnew,PhysPt base) {
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+((CurMode->twidth*rnew)*(cheight/4)+cleft)*4;
	PhysPt src=base+((CurMode->twidth*rold)*(cheight/4)+cleft)*4;	
	unsigned copy=(cright-cleft)*4;unsigned nextline=CurMode->twidth*4;
	for (unsigned i=0;i<cheight/4U;i++) {
		MEM_BlockCopy(dest,src,copy);
		MEM_BlockCopy(dest+8*1024,src+8*1024,copy);
		MEM_BlockCopy(dest+16*1024,src+16*1024,copy);
		MEM_BlockCopy(dest+24*1024,src+24*1024,copy);
		dest+=nextline;src+=nextline;
	}
}

static void EGA16_CopyRow(System::Byte cleft,System::Byte cright,System::Byte rold,System::Byte rnew,PhysPt base) {
	PhysPt src,dest;unsigned copy;
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	dest=base+(CurMode->twidth*rnew)*cheight+cleft;
	src=base+(CurMode->twidth*rold)*cheight+cleft;
	unsigned nextline=CurMode->twidth;
	/* Setup registers correctly */
	IO_Write(0x3ce,5);IO_Write(0x3cf,1);		/* Memory transfer mode */
	IO_Write(0x3c4,2);IO_Write(0x3c5,0xf);		/* Enable all Write planes */
	/* Do some copying */
	unsigned rowsize=(cright-cleft);
	copy=cheight;
	for (;copy>0;copy--) {
		for (unsigned x=0;x<rowsize;x++) mem_writeb(dest+x,mem_readb(src+x));
		dest+=nextline;src+=nextline;
	}
	/* Restore registers */
	IO_Write(0x3ce,5);IO_Write(0x3cf,0);		/* Normal transfer mode */
}

static void VGA_CopyRow(System::Byte cleft,System::Byte cright,System::Byte rold,System::Byte rnew,PhysPt base) {
	PhysPt src,dest;unsigned copy;
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	dest=base+8*((CurMode->twidth*rnew)*cheight+cleft);
	src=base+8*((CurMode->twidth*rold)*cheight+cleft);
	unsigned nextline=8*CurMode->twidth;
	unsigned rowsize=8*(cright-cleft);
	copy=cheight;
	for (;copy>0;copy--) {
		for (unsigned x=0;x<rowsize;x++) mem_writeb(dest+x,mem_readb(src+x));
		dest+=nextline;src+=nextline;
	}
}

static void TEXT_CopyRow(System::Byte cleft,System::Byte cright,System::Byte rold,System::Byte rnew,PhysPt base) {
	PhysPt src,dest;
	src=base+(rold*CurMode->twidth+cleft)*2;
	dest=base+(rnew*CurMode->twidth+cleft)*2;
	MEM_BlockCopy(dest,src,(cright-cleft)*2);
}

static void CGA2_FillRow(System::Byte cleft,System::Byte cright,System::Byte row,PhysPt base,System::Byte attr) {
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+((CurMode->twidth*row)*(cheight/2)+cleft);
	unsigned copy=(cright-cleft);
	unsigned nextline=CurMode->twidth;
	attr=(attr & 0x3) | ((attr & 0x3) << 2) | ((attr & 0x3) << 4) | ((attr & 0x3) << 6);
	for (unsigned i=0;i<cheight/2U;i++) {
		for (unsigned x=0;x<copy;x++) {
			mem_writeb(dest+x,attr);
			mem_writeb(dest+8*1024+x,attr);
		}
		dest+=nextline;
	}
}

static void CGA4_FillRow(System::Byte cleft,System::Byte cright,System::Byte row,PhysPt base,System::Byte attr) {
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+((CurMode->twidth*row)*(cheight/2)+cleft)*2;
	unsigned copy=(cright-cleft)*2;unsigned nextline=CurMode->twidth*2;
	attr=(attr & 0x3) | ((attr & 0x3) << 2) | ((attr & 0x3) << 4) | ((attr & 0x3) << 6);
	for (unsigned i=0;i<cheight/2U;i++) {
		for (unsigned x=0;x<copy;x++) {
			mem_writeb(dest+x,attr);
			mem_writeb(dest+8*1024+x,attr);
		}
		dest+=nextline;
	}
}

static void TANDY16_FillRow(System::Byte cleft,System::Byte cright,System::Byte row,PhysPt base,System::Byte attr) {
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+((CurMode->twidth*row)*(cheight/4)+cleft)*4;
	unsigned copy=(cright-cleft)*4;unsigned nextline=CurMode->twidth*4;
	attr=(attr & 0xf) | (attr & 0xf) << 4;
	for (unsigned i=0;i<cheight/4U;i++) {
		for (unsigned x=0;x<copy;x++) {
			mem_writeb(dest+x,attr);
			mem_writeb(dest+8*1024+x,attr);
			mem_writeb(dest+16*1024+x,attr);
			mem_writeb(dest+24*1024+x,attr);
		}
		dest+=nextline;
	}
}

static void EGA16_FillRow(System::Byte cleft,System::Byte cright,System::Byte row,PhysPt base,System::Byte attr) {
	/* Set Bitmask / Color / Full Set Reset */
	IO_Write(0x3ce,0x8);IO_Write(0x3cf,0xff);
	IO_Write(0x3ce,0x0);IO_Write(0x3cf,attr);
	IO_Write(0x3ce,0x1);IO_Write(0x3cf,0xf);
	/* Write some bytes */
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+(CurMode->twidth*row)*cheight+cleft;	
	unsigned nextline=CurMode->twidth;
	unsigned copy = cheight;unsigned rowsize=(cright-cleft);
	for (;copy>0;copy--) {
		for (unsigned x=0;x<rowsize;x++) mem_writeb(dest+x,0xff);
		dest+=nextline;
	}
	IO_Write(0x3cf,0);
}

static void VGA_FillRow(System::Byte cleft,System::Byte cright,System::Byte row,PhysPt base,System::Byte attr) {
	/* Write some bytes */
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	PhysPt dest=base+8*((CurMode->twidth*row)*cheight+cleft);
	unsigned nextline=8*CurMode->twidth;
	unsigned copy = cheight;unsigned rowsize=8*(cright-cleft);
	for (;copy>0;copy--) {
		for (unsigned x=0;x<rowsize;x++) mem_writeb(dest+x,attr);
		dest+=nextline;
	}
}

static void TEXT_FillRow(System::Byte cleft,System::Byte cright,System::Byte row,PhysPt base,System::Byte attr) {
	/* Do some filing */
	PhysPt dest;
	dest=base+(row*CurMode->twidth+cleft)*2;
	System::UInt16 fill=(attr<<8)+' ';
	for (System::Byte x=0;x<(cright-cleft);x++) {
		mem_writew(dest,fill);
		dest+=2;
	}
}


void INT10_ScrollWindow(System::Byte rul,System::Byte cul,System::Byte rlr,System::Byte clr,System::SByte nlines,System::Byte attr,System::Byte page) {
/* Do some range checking */
	if (CurMode->type!=M_TEXT) page=0xff;
	BIOS_NCOLS;BIOS_NROWS;
	if(rul>rlr) return;
	if(cul>clr) return;
	if(rlr>=nrows) rlr=(System::Byte)nrows-1;
	if(clr>=ncols) clr=(System::Byte)ncols-1;
	clr++;

	/* Get the correct page */
	if(page==0xFF) page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
	PhysPt base=CurMode->pstart+page*real_readw(BIOSMEM_SEG,BIOSMEM_PAGE_SIZE);

	/* See how much lines need to be copied */
	System::Byte start,end;int next;
	/* Copy some lines */
	if (nlines>0) {
		start=rlr-nlines+1;
		end=rul;
		next=-1;
	} else if (nlines<0) {
		start=rul-nlines-1;
		end=rlr;
		next=1;
	} else {
		nlines=rlr-rul+1;
		goto filling;
	}
	while (start!=end) {
		start+=next;
		switch (CurMode->type) {
		case M_TEXT:
			TEXT_CopyRow(cul,clr,start,start+nlines,base);break;
		case M_CGA2:
			CGA2_CopyRow(cul,clr,start,start+nlines,base);break;
		case M_CGA4:
			CGA4_CopyRow(cul,clr,start,start+nlines,base);break;
		case M_TANDY16:
			TANDY16_CopyRow(cul,clr,start,start+nlines,base);break;
		case M_EGA:		
			EGA16_CopyRow(cul,clr,start,start+nlines,base);break;
		case M_VGA:		
			VGA_CopyRow(cul,clr,start,start+nlines,base);break;
		case M_LIN4:
			if ((machine==MCH_VGA) && (svgaCard==SVGA_TsengET4K) &&
					(CurMode->swidth<=800)) {
				// the ET4000 BIOS supports text output in 800x600 SVGA
				EGA16_CopyRow(cul,clr,start,start+nlines,base);break;
			}
			// fall-through
		default:
			LOG(LOG_INT10,LOG_ERROR)("Unhandled mode %d for scroll",CurMode->type);
		}	
	} 
	/* Fill some lines */
filling:
	if (nlines>0) {
		start=rul;
	} else {
		nlines=-nlines;
		start=rlr-nlines+1;
	}
	for (;nlines>0;nlines--) {
		switch (CurMode->type) {
		case M_TEXT:
			TEXT_FillRow(cul,clr,start,base,attr);break;
		case M_CGA2:
			CGA2_FillRow(cul,clr,start,base,attr);break;
		case M_CGA4:
			CGA4_FillRow(cul,clr,start,base,attr);break;
		case M_TANDY16:		
			TANDY16_FillRow(cul,clr,start,base,attr);break;
		case M_EGA:		
			EGA16_FillRow(cul,clr,start,base,attr);break;
		case M_VGA:		
			VGA_FillRow(cul,clr,start,base,attr);break;
		case M_LIN4:
			if ((machine==MCH_VGA) && (svgaCard==SVGA_TsengET4K) &&
					(CurMode->swidth<=800)) {
				EGA16_FillRow(cul,clr,start,base,attr);break;
			}
			// fall-through
		default:
			LOG(LOG_INT10,LOG_ERROR)("Unhandled mode %d for scroll",CurMode->type);
		}	
		start++;
	} 
}

void INT10_SetActivePage(System::Byte page) {
	System::UInt16 mem_address;
	if (page>7) LOG(LOG_INT10,LOG_ERROR)("INT10_SetActivePage page %d",page);

	if (IS_EGAVGA_ARCH && (svgaCard==SVGA_S3Trio)) page &= 7;

	mem_address=page*real_readw(BIOSMEM_SEG,BIOSMEM_PAGE_SIZE);
	/* Write the new page start */
	real_writew(BIOSMEM_SEG,BIOSMEM_CURRENT_START,mem_address);
	if (IS_EGAVGA_ARCH) {
		if (CurMode->mode<8) mem_address>>=1;
		// rare alternative: if (CurMode->type==M_TEXT)  mem_address>>=1;
	} else {
		mem_address>>=1;
	}
	/* Write the new start address in vgahardware */
	System::UInt16 base=real_readw(BIOSMEM_SEG,BIOSMEM_CRTC_ADDRESS);
	IO_Write(base,0x0c);
	IO_Write(base+1,(System::Byte)(mem_address>>8));
	IO_Write(base,0x0d);
	IO_Write(base+1,(System::Byte)mem_address);

	// And change the BIOS page
	real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE,page);
	System::Byte cur_row=CURSOR_POS_ROW(page);
	System::Byte cur_col=CURSOR_POS_COL(page);
	// Display the cursor, now the page is active
	INT10_SetCursorPos(cur_row,cur_col,page);
}

void INT10_SetCursorShape(System::Byte first,System::Byte last) {
	real_writew(BIOSMEM_SEG,BIOSMEM_CURSOR_TYPE,last|(first<<8));
	if (machine==MCH_CGA) goto dowrite;
	if (IS_TANDY_ARCH) goto dowrite;
	/* Skip CGA cursor emulation if EGA/VGA system is active */
	if (!(real_readb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL) & 0x8)) {
		/* Check for CGA type 01, invisible */
		if ((first & 0x60) == 0x20) {
			first=0x1e;
			last=0x00;
			goto dowrite;
		}
		/* Check if we need to convert CGA Bios cursor values */
		if (!(real_readb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL) & 0x1)) { // set by int10 fun12 sub34
//			if (CurMode->mode>0x3) goto dowrite;	//Only mode 0-3 are text modes on cga
			if ((first & 0xe0) || (last & 0xe0)) goto dowrite;
			System::Byte cheight=real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT)-1;
			/* Creative routine i based of the original ibmvga bios */

			if (last<first) {
				if (!last) goto dowrite;
				first=last;
				last=cheight;
			/* Test if this might be a cga style cursor set, if not don't do anything */
			} else if (((first | last)>=cheight) || !(last==(cheight-1)) || !(first==cheight) ) {
				if (last<=3) goto dowrite;
				if (first+2<last) {
					if (first>2) {
						first=(cheight+1)/2;
						last=cheight;
					} else {
						last=cheight;
					}
				} else {
					first=(first-last)+cheight;
					last=cheight;

					if (cheight>0xc) { // vgatest sets 15 15 2x where only one should be decremented to 14 14
						first--;     // implementing int10 fun12 sub34 fixed this.
						last--;
					}
				}
			}

		}
	}
dowrite:
	System::UInt16 base=real_readw(BIOSMEM_SEG,BIOSMEM_CRTC_ADDRESS);
	IO_Write(base,0xa);IO_Write(base+1,first);
	IO_Write(base,0xb);IO_Write(base+1,last);
}

void INT10_SetCursorPos(System::Byte row,System::Byte col,System::Byte page) {
	System::UInt16 address;

	if (page>7) LOG(LOG_INT10,LOG_ERROR)("INT10_SetCursorPos page %d",page);
	// Bios cursor pos
	real_writeb(BIOSMEM_SEG,BIOSMEM_CURSOR_POS+page*2,col);
	real_writeb(BIOSMEM_SEG,BIOSMEM_CURSOR_POS+page*2+1,row);
	// Set the hardware cursor
	System::Byte current=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
	if(page==current) {
		// Get the dimensions
		BIOS_NCOLS;
		// Calculate the address knowing nbcols nbrows and page num
		// NOTE: BIOSMEM_CURRENT_START counts in colour/flag pairs
		address=(ncols*row)+col+real_readw(BIOSMEM_SEG,BIOSMEM_CURRENT_START)/2;
		// CRTC regs 0x0e and 0x0f
		System::UInt16 base=real_readw(BIOSMEM_SEG,BIOSMEM_CRTC_ADDRESS);
		IO_Write(base,0x0e);
		IO_Write(base+1,(System::Byte)(address>>8));
		IO_Write(base,0x0f);
		IO_Write(base+1,(System::Byte)address);
	}
}

void ReadCharAttr(System::UInt16 col,System::UInt16 row,System::Byte page,System::UInt16 * result) {
	/* Externally used by the mouse routine */
	PhysPt fontdata;
	unsigned x,y;
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	bool split_chr = false;
	switch (CurMode->type) {
	case M_TEXT:
		{	
			// Compute the address  
			System::UInt16 address=page*real_readw(BIOSMEM_SEG,BIOSMEM_PAGE_SIZE);
			address+=(row*real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS)+col)*2;
			// read the char 
			PhysPt where = CurMode->pstart+address;
			*result=mem_readw(where);
		}
		return;
	case M_CGA4:
	case M_CGA2:
	case M_TANDY16:
		split_chr = true;
		/* Fallthrough */
	default:		/* EGA/VGA don't have a split font-table */
		for(System::UInt16 chr=0;chr <= 255 ;chr++) {
			if (!split_chr || (chr<128)) fontdata = Real2Phys(RealGetVec(0x43))+chr*cheight;
			else fontdata = Real2Phys(RealGetVec(0x1F))+(chr-128)*cheight;

			x=8*col;
			y=cheight*row;
			bool error=false;
			for (System::Byte h=0;h<cheight;h++) {
				System::Byte intel=128;
				System::Byte bitline=mem_readb(fontdata++);
				System::Byte res=0;
				System::Byte vidline=0;
				System::UInt16 tx=(System::UInt16)x;
				while (intel) {
					//Construct bitline in memory
					INT10_GetPixel(tx,(System::UInt16)y,page,&res);
					if(res) vidline|=intel;
					tx++;
					intel>>=1;
				}
				y++;
				if(bitline != vidline){
					/* It's not character 'chr', move on to the next */
					error = true;
					break;
				}
			}
			if(!error){
				/* We found it */
				*result = chr;
				return;
			}
		}
		LOG(LOG_INT10,LOG_ERROR)("ReadChar didn't find character");
		*result = 0;
		break;
	}
}
void INT10_ReadCharAttr(System::UInt16 * result,System::Byte page) {
	if(page==0xFF) page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
	System::Byte cur_row=CURSOR_POS_ROW(page);
	System::Byte cur_col=CURSOR_POS_COL(page);
	ReadCharAttr(cur_col,cur_row,page,result);
}
void WriteChar(System::UInt16 col,System::UInt16 row,System::Byte page,System::Byte chr,System::Byte attr,bool useattr) {
	/* Externally used by the mouse routine */
	RealPt fontdata;
	unsigned x,y;
	System::Byte cheight = real_readb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT);
	switch (CurMode->type) {
	case M_TEXT:
		{	
			// Compute the address  
			System::UInt16 address=page*real_readw(BIOSMEM_SEG,BIOSMEM_PAGE_SIZE);
			address+=(row*real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS)+col)*2;
			// Write the char 
			PhysPt where = CurMode->pstart+address;
			mem_writeb(where,chr);
			if (useattr) {
				mem_writeb(where+1,attr);
			}
		}
		return;
	case M_CGA4:
	case M_CGA2:
	case M_TANDY16:
		if (chr<128) 
			fontdata=RealGetVec(0x43);
		else {
			chr-=128;
			fontdata=RealGetVec(0x1f);
		}
		fontdata=RealMake(RealSeg(fontdata), RealOff(fontdata) + chr*cheight);
		break;
	default:
		fontdata=RealGetVec(0x43);
		fontdata=RealMake(RealSeg(fontdata), RealOff(fontdata) + chr*cheight);
		break;
	}

	if(GCC_UNLIKELY(!useattr)) { //Set attribute(color) to a sensible value
		static bool warned_use = false;
		if(GCC_UNLIKELY(!warned_use)){ 
			LOG(LOG_INT10,LOG_ERROR)("writechar used without attribute in non-textmode %c %X",chr,chr);
			warned_use = true;
		}
		switch(CurMode->type) {
		case M_CGA4:
			attr = 0x3;
			break;
		case M_CGA2:
			attr = 0x1;
			break;
		case M_TANDY16:
		case M_EGA:
		default:
			attr = 0xf;
			break;
		}
	}

	//Some weird behavior of mode 6 (and 11) 
	if ((CurMode->mode == 0x6)/* || (CurMode->mode==0x11)*/) attr = (attr&0x80)|1;
	//(same fix for 11 fixes vgatest2, but it's not entirely correct according to wd)

	x=8*col;
	y=cheight*row;System::Byte xor_mask=(CurMode->type == M_VGA) ? 0x0 : 0x80;
	//TODO Check for out of bounds
	if (CurMode->type==M_EGA) {
		/* enable all planes for EGA modes (Ultima 1 colour bug) */
		/* might be put into INT10_PutPixel but different vga bios
		   implementations have different opinions about this */
		IO_Write(0x3c4,0x2);IO_Write(0x3c5,0xf);
	}
	for (System::Byte h=0;h<cheight;h++) {
		System::Byte intel=128;
		System::Byte bitline = mem_readb(Real2Phys( fontdata ));
		fontdata = RealMake( RealSeg( fontdata ), RealOff( fontdata ) + 1);
		System::UInt16 tx=(System::UInt16)x;
		while (intel) {
			if (bitline&intel) INT10_PutPixel(tx,(System::UInt16)y,page,attr);
			else INT10_PutPixel(tx,(System::UInt16)y,page,attr & xor_mask);
			tx++;
			intel>>=1;
		}
		y++;
	}
}

void INT10_WriteChar(System::Byte chr,System::Byte attr,System::Byte page,System::UInt16 count,bool showattr) {
	if (CurMode->type!=M_TEXT) {
		showattr=true; //Use attr in graphics mode always
		switch (machine) {
			case EGAVGA_ARCH_CASE:
				page%=CurMode->ptotal;
				break;
			case MCH_CGA:
			case MCH_PCJR:
				page=0;
				break;
		}
	}

	System::Byte cur_row=CURSOR_POS_ROW(page);
	System::Byte cur_col=CURSOR_POS_COL(page);
	BIOS_NCOLS;
	while (count>0) {
		WriteChar(cur_col,cur_row,page,chr,attr,showattr);
		count--;
		cur_col++;
		if(cur_col==ncols) {
			cur_col=0;
			cur_row++;
		}
	}
}

static void INT10_TeletypeOutputAttr(System::Byte chr,System::Byte attr,bool useattr,System::Byte page) {
	BIOS_NCOLS;BIOS_NROWS;
	System::Byte cur_row=CURSOR_POS_ROW(page);
	System::Byte cur_col=CURSOR_POS_COL(page);
	switch (chr) {
	case 7:
	//TODO BEEP
	break;
	case 8:
		if(cur_col>0) cur_col--;
		break;
	case '\r':
		cur_col=0;
		break;
	case '\n':
//		cur_col=0; //Seems to break an old chess game
		cur_row++;
		break;
	case '\t':
		do {
			INT10_TeletypeOutputAttr(' ',attr,useattr,page);
			cur_row=CURSOR_POS_ROW(page);
			cur_col=CURSOR_POS_COL(page);
		} while(cur_col%8);
		break;
	default:
		/* Draw the actual Character */
		WriteChar(cur_col,cur_row,page,chr,attr,useattr);
		cur_col++;
	}
	if(cur_col==ncols) {
		cur_col=0;
		cur_row++;
	}
	// Do we need to scroll ?
	if(cur_row==nrows) {
		//Fill with black on non-text modes and with 0x7 on textmode
		System::Byte fill = (CurMode->type == M_TEXT)?0x7:0;
		INT10_ScrollWindow(0,0,(System::Byte)(nrows-1),(System::Byte)(ncols-1),-1,fill,page);
		cur_row--;
	}
	// Set the cursor for the page
	INT10_SetCursorPos(cur_row,cur_col,page);
}

void INT10_TeletypeOutputAttr(System::Byte chr,System::Byte attr,bool useattr) {
	INT10_TeletypeOutputAttr(chr,attr,useattr,real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE));
}

void INT10_TeletypeOutput(System::Byte chr,System::Byte attr) {
	INT10_TeletypeOutputAttr(chr,attr,CurMode->type!=M_TEXT);
}

void INT10_WriteString(System::Byte row,System::Byte col,System::Byte flag,System::Byte attr,PhysPt string,System::UInt16 count,System::Byte page) {
	System::Byte cur_row=CURSOR_POS_ROW(page);
	System::Byte cur_col=CURSOR_POS_COL(page);
	
	// if row=0xff special case : use current cursor position
	if (row==0xff) {
		row=cur_row;
		col=cur_col;
	}
	INT10_SetCursorPos(row,col,page);
	while (count>0) {
		System::Byte chr=mem_readb(string);
		string++;
		if (flag&2) {
			attr=mem_readb(string);
			string++;
		};
		INT10_TeletypeOutputAttr(chr,attr,true,page);
		count--;
	}
	if (!(flag&1)) {
		INT10_SetCursorPos(cur_row,cur_col,page);
	}
}
