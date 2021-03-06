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

/* $Id: vga_xga.cpp,v 1.17 2009-05-27 09:15:41 qbix79 Exp $ */

#include <string.h>
#include "dosbox.h"
#include "inout.h"
#include "vga.h"
#include <math.h>
#include <stdio.h>
#include "callback.h"
#include "cpu.h"		// for 0x3da delay

#define XGA_SCREEN_WIDTH	vga.s3.xga_screen_width
#define XGA_COLOR_MODE		vga.s3.xga_color_mode

#define XGA_SHOW_COMMAND_TRACE 0

struct XGAStatus {
	struct scissorreg {
		System::UInt16 x1, y1, x2, y2;
	} scissors;

	System::UInt32 readmask;
	System::UInt32 writemask;

	System::UInt32 forecolor;
	System::UInt32 backcolor;

	unsigned curcommand;

	System::UInt16 foremix;
	System::UInt16 backmix;

	System::UInt16 curx, cury;
	System::UInt16 destx, desty;

	System::UInt16 ErrTerm;
	System::UInt16 MIPcount;
	System::UInt16 MAPcount;

	System::UInt16 pix_cntl;
	System::UInt16 control1;
	System::UInt16 control2;
	System::UInt16 read_sel;

	struct XGA_WaitCmd {
		bool newline;
		bool wait;
		System::UInt16 cmd;
		System::UInt16 curx, cury;
		System::UInt16 x1, y1, x2, y2, sizex, sizey;
		System::UInt32 data; /* transient data passed by multiple calls */
		unsigned datasize;
		unsigned buswidth;
	} waitcmd;

} xga;

void XGA_Write_Multifunc(unsigned val, unsigned len) {
	unsigned regselect = val >> 12;
	unsigned dataval = val & 0xfff;
	switch(regselect) {
		case 0: // minor axis pixel count
			xga.MIPcount = dataval;
			break;
		case 1: // top scissors
			xga.scissors.y1 = dataval;
			break;
		case 2: // left
			xga.scissors.x1 = dataval;
			break;
		case 3: // bottom
			xga.scissors.y2 = dataval;
			break;
		case 4: // right
			xga.scissors.x2 = dataval;
			break;
		case 0xa: // data manip control
			xga.pix_cntl = dataval;
			break;
		case 0xd: // misc 2
			xga.control2 = dataval;
			break;
		case 0xe:
			xga.control1 = dataval;
			break;
		case 0xf:
			xga.read_sel = dataval;
			break;
		default:
			LOG_MSG("XGA: Unhandled multifunction command %x", regselect);
			break;
	}
}

unsigned XGA_Read_Multifunc() {
	switch(xga.read_sel++) {
		case 0: return xga.MIPcount;
		case 1: return xga.scissors.y1;
		case 2: return xga.scissors.x1;
		case 3: return xga.scissors.y2;
		case 4: return xga.scissors.x2;
		case 5: return xga.pix_cntl;
		case 6: return xga.control1;
		case 7: return 0; // TODO
		case 8: return 0; // TODO
		case 9: return 0; // TODO
		case 10: return xga.control2;
		default: return 0;
	}
}


void XGA_DrawPoint(unsigned x, unsigned y, unsigned c) {
	if(!(xga.curcommand & 0x1)) return;
	if(!(xga.curcommand & 0x10)) return;

	if(x < xga.scissors.x1) return;
	if(x > xga.scissors.x2) return;
	if(y < xga.scissors.y1) return;
	if(y > xga.scissors.y2) return;

	System::UInt32 memaddr = (y * XGA_SCREEN_WIDTH) + x;
	/* Need to zero out all unused int in modes that have any (15-bit or "32"-bit -- the last
	   one is actually 24-bit. Without this step there may be some graphics corruption (mainly,
	   during windows dragging. */
	switch(XGA_COLOR_MODE) {
		case M_LIN8:
			if (GCC_UNLIKELY(memaddr >= vga.vmemsize)) break;
			vga.mem.linear[memaddr] = c;
			break;
		case M_LIN15:
			if (GCC_UNLIKELY(memaddr*2 >= vga.vmemsize)) break;
			((System::UInt16*)(vga.mem.linear))[memaddr] = (System::UInt16)(c&0x7fff);
			break;
		case M_LIN16:
			if (GCC_UNLIKELY(memaddr*2 >= vga.vmemsize)) break;
			((System::UInt16*)(vga.mem.linear))[memaddr] = (System::UInt16)(c&0xffff);
			break;
		case M_LIN32:
			if (GCC_UNLIKELY(memaddr*4 >= vga.vmemsize)) break;
			((System::UInt32*)(vga.mem.linear))[memaddr] = c;
			break;
		default:
			break;
	}

}

unsigned XGA_GetPoint(unsigned x, unsigned y) {
	System::UInt32 memaddr = (y * XGA_SCREEN_WIDTH) + x;

	switch(XGA_COLOR_MODE) {
	case M_LIN8:
		if (GCC_UNLIKELY(memaddr >= vga.vmemsize)) break;
		return vga.mem.linear[memaddr];
	case M_LIN15:
	case M_LIN16:
		if (GCC_UNLIKELY(memaddr*2 >= vga.vmemsize)) break;
		return ((System::UInt16*)(vga.mem.linear))[memaddr];
	case M_LIN32:
		if (GCC_UNLIKELY(memaddr*4 >= vga.vmemsize)) break;
		return ((System::UInt32*)(vga.mem.linear))[memaddr];
	default:
		break;
	}
	return 0;
}


unsigned XGA_GetMixResult(unsigned mixmode, unsigned srcval, unsigned dstdata) {
	unsigned destval = 0;
	switch(mixmode &  0xf) {
		case 0x00: /* not DST */
			destval = ~dstdata;
			break;
		case 0x01: /* 0 (false) */
			destval = 0;
			break;
		case 0x02: /* 1 (true) */
			destval = 0xffffffff;
			break;
		case 0x03: /* 2 DST */
			destval = dstdata;
			break;
		case 0x04: /* not SRC */
			destval = ~srcval;
			break;
		case 0x05: /* SRC xor DST */
			destval = srcval ^ dstdata;
			break;
		case 0x06: /* not (SRC xor DST) */
			destval = ~(srcval ^ dstdata);
			break;
		case 0x07: /* SRC */
			destval = srcval;
			break;
		case 0x08: /* not (SRC and DST) */
			destval = ~(srcval & dstdata);
			break;
		case 0x09: /* (not SRC) or DST */
			destval = (~srcval) | dstdata;
			break;
		case 0x0a: /* SRC or (not DST) */
			destval = srcval | (~dstdata);
			break;
		case 0x0b: /* SRC or DST */
			destval = srcval | dstdata;
			break;
		case 0x0c: /* SRC and DST */
			destval = srcval & dstdata;
			break;
		case 0x0d: /* SRC and (not DST) */
			destval = srcval & (~dstdata);
			break;
		case 0x0e: /* (not SRC) and DST */
			destval = (~srcval) & dstdata;
			break;
		case 0x0f: /* not (SRC or DST) */
			destval = ~(srcval | dstdata);
			break;
		default:
			LOG_MSG("XGA: GetMixResult: Unknown mix.  Shouldn't be able to get here!");
			break;
	}
	return destval;
}

void XGA_DrawLineVector(unsigned val) {
	int xat, yat;
	unsigned srcval;
	unsigned destval;
	unsigned dstdata;
	int i;

	int dx, sx, sy;

	dx = xga.MAPcount; 
	xat = xga.curx;
	yat = xga.cury;

	switch((val >> 5) & 0x7) {
		case 0x00: /* 0 degrees */
			sx = 1;
			sy = 0;
			break;
		case 0x01: /* 45 degrees */
			sx = 1;
			sy = -1;
			break;
		case 0x02: /* 90 degrees */
			sx = 0;
			sy = -1;
			break;
		case 0x03: /* 135 degrees */
			sx = -1;
			sy = -1;
			break;
		case 0x04: /* 180 degrees */
			sx = -1;
			sy = 0;
			break;
		case 0x05: /* 225 degrees */
			sx = -1;
			sy = 1;
			break;
		case 0x06: /* 270 degrees */
			sx = 0;
			sy = 1;
			break;
		case 0x07: /* 315 degrees */
			sx = 1;
			sy = 1;
			break;
		default:  // Should never get here
			sx = 0;
			sy = 0;
			break;
	}

	for (i=0;i<=dx;i++) {
		unsigned mixmode = (xga.pix_cntl >> 6) & 0x3;
		switch (mixmode) {
			case 0x00: /* FOREMIX always used */
				mixmode = xga.foremix;
				switch((mixmode >> 5) & 0x03) {
					case 0x00: /* Src is background color */
						srcval = xga.backcolor;
						break;
					case 0x01: /* Src is foreground color */
						srcval = xga.forecolor;
						break;
					case 0x02: /* Src is pixel data from PIX_TRANS register */
						//srcval = tmpval;
						//LOG_MSG("XGA: DrawRect: Wants data from PIX_TRANS register");
						break;
					case 0x03: /* Src is bitmap data */
						LOG_MSG("XGA: DrawRect: Wants data from srcdata");
						//srcval = srcdata;
						break;
					default:
						LOG_MSG("XGA: DrawRect: Shouldn't be able to get here!");
						break;
				}
				dstdata = XGA_GetPoint(xat,yat);

				destval = XGA_GetMixResult(mixmode, srcval, dstdata);

                XGA_DrawPoint(xat,yat, destval);
				break;
			default: 
				LOG_MSG("XGA: DrawLine: Needs mixmode %x", mixmode);
				break;
		}
		xat += sx;
		yat += sy;
	}

	xga.curx = xat-1;
	xga.cury = yat;
}

void XGA_DrawLineBresenham(unsigned val) {
	int xat, yat;
	unsigned srcval;
	unsigned destval;
	unsigned dstdata;
	int i;
	int tmpswap;
	bool steep;

#define SWAP(a,b) tmpswap = a; a = b; b = tmpswap;

	int dx, sx, dy, sy, e, dmajor, dminor,destxtmp;

	// Probably a lot easier way to do this, but this works.

	dminor = (int)((System::Int16)xga.desty);
	if(xga.desty&0x2000) dminor |= 0xffffe000;
	dminor >>= 1;

	destxtmp=(int)((System::Int16)xga.destx);
	if(xga.destx&0x2000) destxtmp |= 0xffffe000;


	dmajor = -(destxtmp - (dminor << 1)) >> 1;
	
	dx = dmajor;
	if((val >> 5) & 0x1) {
        sx = 1;
	} else {
		sx = -1;
	}
	dy = dminor;
	if((val >> 7) & 0x1) {
        sy = 1;
	} else {
		sy = -1;
	}
	e = (int)((System::Int16)xga.ErrTerm);
	if(xga.ErrTerm&0x2000) e |= 0xffffe000;
	xat = xga.curx;
	yat = xga.cury;

	if((val >> 6) & 0x1) {
		steep = false;
		SWAP(xat, yat);
		SWAP(sx, sy);
	} else {
		steep = true;
	}
    
	//LOG_MSG("XGA: Bresenham: ASC %d, LPDSC %d, sx %d, sy %d, err %d, steep %d, length %d, dmajor %d, dminor %d, xstart %d, ystart %d", dx, dy, sx, sy, e, steep, xga.MAPcount, dmajor, dminor,xat,yat);

	for (i=0;i<=xga.MAPcount;i++) { 
			unsigned mixmode = (xga.pix_cntl >> 6) & 0x3;
			switch (mixmode) {
				case 0x00: /* FOREMIX always used */
					mixmode = xga.foremix;
					switch((mixmode >> 5) & 0x03) {
						case 0x00: /* Src is background color */
							srcval = xga.backcolor;
							break;
						case 0x01: /* Src is foreground color */
							srcval = xga.forecolor;
							break;
						case 0x02: /* Src is pixel data from PIX_TRANS register */
							//srcval = tmpval;
							LOG_MSG("XGA: DrawRect: Wants data from PIX_TRANS register");
							break;
						case 0x03: /* Src is bitmap data */
							LOG_MSG("XGA: DrawRect: Wants data from srcdata");
							//srcval = srcdata;
							break;
						default:
							LOG_MSG("XGA: DrawRect: Shouldn't be able to get here!");
							break;
					}

					if(steep) {
						dstdata = XGA_GetPoint(xat,yat);
					} else {
						dstdata = XGA_GetPoint(yat,xat);
					}

					destval = XGA_GetMixResult(mixmode, srcval, dstdata);

					if(steep) {
						XGA_DrawPoint(xat,yat, destval);
					} else {
						XGA_DrawPoint(yat,xat, destval);
					}

					break;
				default: 
					LOG_MSG("XGA: DrawLine: Needs mixmode %x", mixmode);
					break;
			}
			while (e > 0) {
				yat += sy;
				e -= (dx << 1);
			}
			xat += sx;
			e += (dy << 1);
	}

	if(steep) {
		xga.curx = xat;
		xga.cury = yat;
	} else {
		xga.curx = yat;
		xga.cury = xat;
	}
	//	}
	//}
	
}

void XGA_DrawRectangle(unsigned val) {
	System::UInt32 xat, yat;
	unsigned srcval;
	unsigned destval;
	unsigned dstdata;

	int srcx, srcy, dx, dy;

	dx = -1;
	dy = -1;

	if(((val >> 5) & 0x01) != 0) dx = 1;
	if(((val >> 7) & 0x01) != 0) dy = 1;

	srcy = xga.cury;

	for(yat=0;yat<=xga.MIPcount;yat++) {
		srcx = xga.curx;
		for(xat=0;xat<=xga.MAPcount;xat++) {
			unsigned mixmode = (xga.pix_cntl >> 6) & 0x3;
			switch (mixmode) {
				case 0x00: /* FOREMIX always used */
					mixmode = xga.foremix;
					switch((mixmode >> 5) & 0x03) {
						case 0x00: /* Src is background color */
							srcval = xga.backcolor;
							break;
						case 0x01: /* Src is foreground color */
							srcval = xga.forecolor;
							break;
						case 0x02: /* Src is pixel data from PIX_TRANS register */
							//srcval = tmpval;
							LOG_MSG("XGA: DrawRect: Wants data from PIX_TRANS register");
							break;
						case 0x03: /* Src is bitmap data */
							LOG_MSG("XGA: DrawRect: Wants data from srcdata");
							//srcval = srcdata;
							break;
						default:
							LOG_MSG("XGA: DrawRect: Shouldn't be able to get here!");
							break;
					}
					dstdata = XGA_GetPoint(srcx,srcy);

					destval = XGA_GetMixResult(mixmode, srcval, dstdata);

                    XGA_DrawPoint(srcx,srcy, destval);
					break;
				default: 
					LOG_MSG("XGA: DrawRect: Needs mixmode %x", mixmode);
					break;
			}
			srcx += dx;
		}
		srcy += dy;
	}
	xga.curx = srcx;
	xga.cury = srcy;

	//LOG_MSG("XGA: Draw rect (%d, %d)-(%d, %d), %d", x1, y1, x2, y2, xga.forecolor);
}

bool XGA_CheckX(void) {
	bool newline = false;
	if(!xga.waitcmd.newline) {
	
	if((xga.waitcmd.curx<2048) && xga.waitcmd.curx > (xga.waitcmd.x2)) {
		xga.waitcmd.curx = xga.waitcmd.x1;
		xga.waitcmd.cury++;
		xga.waitcmd.cury&=0x0fff;
		newline = true;
		xga.waitcmd.newline = true;
		if((xga.waitcmd.cury<2048)&&(xga.waitcmd.cury > xga.waitcmd.y2))
			xga.waitcmd.wait = false;
	} else if(xga.waitcmd.curx>=2048) {
		System::UInt16 realx = 4096-xga.waitcmd.curx;
		if(xga.waitcmd.x2>2047) { // x end is negative too
			System::UInt16 realxend=4096-xga.waitcmd.x2;
			if(realx==realxend) {
				xga.waitcmd.curx = xga.waitcmd.x1;
				xga.waitcmd.cury++;
				xga.waitcmd.cury&=0x0fff;
				newline = true;
				xga.waitcmd.newline = true;
				if((xga.waitcmd.cury<2048)&&(xga.waitcmd.cury > xga.waitcmd.y2))
					xga.waitcmd.wait = false;
			}
		} else { // else overlapping
			if(realx==xga.waitcmd.x2) {
				xga.waitcmd.curx = xga.waitcmd.x1;
				xga.waitcmd.cury++;
				xga.waitcmd.cury&=0x0fff;
				newline = true;
				xga.waitcmd.newline = true;
				if((xga.waitcmd.cury<2048)&&(xga.waitcmd.cury > xga.waitcmd.y2))
					xga.waitcmd.wait = false;
				}
			}
		}
	} else {
        xga.waitcmd.newline = false;
	}
	return newline;
}

void XGA_DrawWaitSub(unsigned mixmode, unsigned srcval) {
	unsigned destval;
	unsigned dstdata;
	dstdata = XGA_GetPoint(xga.waitcmd.curx, xga.waitcmd.cury);
	destval = XGA_GetMixResult(mixmode, srcval, dstdata);
	//LOG_MSG("XGA: DrawPattern: Mixmode: %x srcval: %x", mixmode, srcval);

	XGA_DrawPoint(xga.waitcmd.curx, xga.waitcmd.cury, destval);
	xga.waitcmd.curx++;
	xga.waitcmd.curx&=0x0fff;
	XGA_CheckX();
}

void XGA_DrawWait(unsigned val, unsigned len) {
	if(!xga.waitcmd.wait) return;
	unsigned mixmode = (xga.pix_cntl >> 6) & 0x3;
	unsigned srcval;
	switch(xga.waitcmd.cmd) {
		case 2: /* Rectangle */
			switch(mixmode) {
				case 0x00: /* FOREMIX always used */
					mixmode = xga.foremix;

/*					switch((mixmode >> 5) & 0x03) {
						case 0x00: // Src is background color
							srcval = xga.backcolor;
							break;
						case 0x01: // Src is foreground color
							srcval = xga.forecolor;
							break;
						case 0x02: // Src is pixel data from PIX_TRANS register
*/
					if(((mixmode >> 5) & 0x03) != 0x2) {
						// those cases don't seem to occur
						LOG_MSG("XGA: unsupported drawwait operation");
						break;
					}
					switch(xga.waitcmd.buswidth) {
						case M_LIN8:		//  8 bit
							XGA_DrawWaitSub(mixmode, val);
							break;
						case 0x20 | M_LIN8: // 16 bit 
							for(unsigned i = 0; i < len; i++) {
								XGA_DrawWaitSub(mixmode, (val>>(8*i))&0xff);
								if(xga.waitcmd.newline) break;
							}
							break;
						case 0x40 | M_LIN8: // 32 bit
                            for(int i = 0; i < 4; i++)
								XGA_DrawWaitSub(mixmode, (val>>(8*i))&0xff);
							break;
						case (0x20 | M_LIN32):
							if(len!=4) { // Win 3.11 864 'hack?'
								if(xga.waitcmd.datasize == 0) {
									// set it up to wait for the next word
									xga.waitcmd.data = val; 
									xga.waitcmd.datasize = 2;
									return;
								} else {
									srcval = (val<<16)|xga.waitcmd.data;
									xga.waitcmd.data = 0;
									xga.waitcmd.datasize = 0;
									XGA_DrawWaitSub(mixmode, srcval);
								}
								break;
							} // fall-through
						case 0x40 | M_LIN32: // 32 bit
							XGA_DrawWaitSub(mixmode, val);
							break;
						case 0x20 | M_LIN15: // 16 bit 
						case 0x20 | M_LIN16: // 16 bit 
							XGA_DrawWaitSub(mixmode, val);
							break;
						case 0x40 | M_LIN15: // 32 bit 
						case 0x40 | M_LIN16: // 32 bit 
							XGA_DrawWaitSub(mixmode, val&0xffff);
							if(!xga.waitcmd.newline)
								XGA_DrawWaitSub(mixmode, val>>16);
							break;
						default:
							// Let's hope they never show up ;)
							LOG_MSG("XGA: unsupported bpp / datawidth combination %x",
								xga.waitcmd.buswidth);
							break;
					};
					break;
			
				case 0x02: // Data from PIX_TRANS selects the mix
					unsigned chunksize;
					unsigned chunks;
					switch(xga.waitcmd.buswidth&0x60) {
						case 0x0:
							chunksize=8;
							chunks=1;
							break;
						case 0x20: // 16 bit
							chunksize=16;
							if(len==4) chunks=2;
							else chunks = 1;
							break;
						case 0x40: // 32 bit
							chunksize=16;
							if(len==4) chunks=2;
							else chunks = 1;
                           	break;
						case 0x60: // undocumented guess (but works)
							chunksize=8;
							chunks=4;
							break;
					}
					
					for(unsigned k = 0; k < chunks; k++) { // chunks counter
						xga.waitcmd.newline = false;
						for(unsigned n = 0; n < chunksize; n++) { // pixels
							unsigned mixmode;
							
							// This formula can rule the world ;)
							unsigned mask = 1 << ((((n&0xF8)+(8-(n&0x7)))-1)+chunksize*k);
							if(val&mask) mixmode = xga.foremix;
							else mixmode = xga.backmix;
							
							switch((mixmode >> 5) & 0x03) {
								case 0x00: // Src is background color
									srcval = xga.backcolor;
									break;
								case 0x01: // Src is foreground color
									srcval = xga.forecolor;
									break;
								default:
									LOG_MSG("XGA: DrawBlitWait: Unsupported src %x",
										(mixmode >> 5) & 0x03);
									srcval=0;
									break;
							}
                            XGA_DrawWaitSub(mixmode, srcval);

							if((xga.waitcmd.cury<2048) &&
							  (xga.waitcmd.cury >= xga.waitcmd.y2)) {
								xga.waitcmd.wait = false;
								k=1000; // no more chunks
								break;
							}
							// next chunk goes to next line
							if(xga.waitcmd.newline) break; 
						} // pixels loop
					} // chunks loop
					break;

				default:
					LOG_MSG("XGA: DrawBlitWait: Unhandled mixmode: %d", mixmode);
					break;
			} // switch mixmode
			break;
		default:
			LOG_MSG("XGA: Unhandled draw command %x", xga.waitcmd.cmd);
			break;
	}
}

void XGA_BlitRect(unsigned val) {
	System::UInt32 xat, yat;
	unsigned srcdata;
	unsigned dstdata;

	unsigned srcval;
	unsigned destval;

	int srcx, srcy, tarx, tary, dx, dy;

	dx = -1;
	dy = -1;

	if(((val >> 5) & 0x01) != 0) dx = 1;
	if(((val >> 7) & 0x01) != 0) dy = 1;

	srcx = xga.curx;
	srcy = xga.cury;
	tarx = xga.destx;
	tary = xga.desty;

	unsigned mixselect = (xga.pix_cntl >> 6) & 0x3;
	unsigned mixmode = 0x67; /* Source is bitmap data, mix mode is src */
	switch(mixselect) {
		case 0x00: /* Foreground mix is always used */
			mixmode = xga.foremix;
			break;
		case 0x02: /* CPU Data determines mix used */
			LOG_MSG("XGA: DrawPattern: Mixselect data from PIX_TRANS register");
			break;
		case 0x03: /* Video memory determines mix */
			//LOG_MSG("XGA: Srcdata: %x, Forecolor %x, Backcolor %x, Foremix: %x Backmix: %x", srcdata, xga.forecolor, xga.backcolor, xga.foremix, xga.backmix);
			break;
		default:
			LOG_MSG("XGA: BlitRect: Unknown mix select register");
			break;
	}


	/* Copy source to video ram */
	for(yat=0;yat<=xga.MIPcount ;yat++) {
		srcx = xga.curx;
		tarx = xga.destx;

		for(xat=0;xat<=xga.MAPcount;xat++) {
			srcdata = XGA_GetPoint(srcx, srcy);
			dstdata = XGA_GetPoint(tarx, tary);

			if(mixselect == 0x3) {
				if(srcdata == xga.forecolor) {
					mixmode = xga.foremix;
				} else {
					if(srcdata == xga.backcolor) {
						mixmode = xga.backmix;
					} else {
						/* Best guess otherwise */
						mixmode = 0x67; /* Source is bitmap data, mix mode is src */
					}
				}
			}

			switch((mixmode >> 5) & 0x03) {
				case 0x00: /* Src is background color */
					srcval = xga.backcolor;
					break;
				case 0x01: /* Src is foreground color */
					srcval = xga.forecolor;
					break;
				case 0x02: /* Src is pixel data from PIX_TRANS register */
					LOG_MSG("XGA: DrawPattern: Wants data from PIX_TRANS register");
					break;
				case 0x03: /* Src is bitmap data */
					srcval = srcdata;
					break;
				default:
					LOG_MSG("XGA: DrawPattern: Shouldn't be able to get here!");
					srcval = 0;
					break;
			}

			destval = XGA_GetMixResult(mixmode, srcval, dstdata);
			//LOG_MSG("XGA: DrawPattern: Mixmode: %x Mixselect: %x", mixmode, mixselect);

			XGA_DrawPoint(tarx, tary, destval);

			srcx += dx;
			tarx += dx;
		}
		srcy += dy;
		tary += dy;
	}
}

void XGA_DrawPattern(unsigned val) {
	unsigned srcdata;
	unsigned dstdata;

	unsigned srcval;
	unsigned destval;

	int xat, yat, srcx, srcy, tarx, tary, dx, dy;

	dx = -1;
	dy = -1;

	if(((val >> 5) & 0x01) != 0) dx = 1;
	if(((val >> 7) & 0x01) != 0) dy = 1;

	srcx = xga.curx;
	srcy = xga.cury;

	tary = xga.desty;

	unsigned mixselect = (xga.pix_cntl >> 6) & 0x3;
	unsigned mixmode = 0x67; /* Source is bitmap data, mix mode is src */
	switch(mixselect) {
		case 0x00: /* Foreground mix is always used */
			mixmode = xga.foremix;
			break;
		case 0x02: /* CPU Data determines mix used */
			LOG_MSG("XGA: DrawPattern: Mixselect data from PIX_TRANS register");
			break;
		case 0x03: /* Video memory determines mix */
			//LOG_MSG("XGA: Pixctl: %x, Srcdata: %x, Forecolor %x, Backcolor %x, Foremix: %x Backmix: %x",xga.pix_cntl, srcdata, xga.forecolor, xga.backcolor, xga.foremix, xga.backmix);
			break;
		default:
			LOG_MSG("XGA: DrawPattern: Unknown mix select register");
			break;
	}

	for(yat=0;yat<=xga.MIPcount;yat++) {
		tarx = xga.destx;
		for(xat=0;xat<=xga.MAPcount;xat++) {

			srcdata = XGA_GetPoint(srcx + (tarx & 0x7), srcy + (tary & 0x7));
			//LOG_MSG("patternpoint (%3d/%3d)v%x",srcx + (tarx & 0x7), srcy + (tary & 0x7),srcdata);
			dstdata = XGA_GetPoint(tarx, tary);
			

			if(mixselect == 0x3) {
				// TODO lots of guessing here but best results this way
				/*if(srcdata == xga.forecolor)*/ mixmode = xga.foremix;
				// else 
				if(srcdata == xga.backcolor || srcdata == 0) 
					mixmode = xga.backmix;
			}

			switch((mixmode >> 5) & 0x03) {
				case 0x00: /* Src is background color */
					srcval = xga.backcolor;
					break;
				case 0x01: /* Src is foreground color */
					srcval = xga.forecolor;
					break;
				case 0x02: /* Src is pixel data from PIX_TRANS register */
					LOG_MSG("XGA: DrawPattern: Wants data from PIX_TRANS register");
					break;
				case 0x03: /* Src is bitmap data */
					srcval = srcdata;
					break;
				default:
					LOG_MSG("XGA: DrawPattern: Shouldn't be able to get here!");
					srcval = 0;
					break;
			}

			destval = XGA_GetMixResult(mixmode, srcval, dstdata);

			XGA_DrawPoint(tarx, tary, destval);
			
			tarx += dx;
		}
		tary += dy;
	}
}

void XGA_DrawCmd(unsigned val, unsigned len) {
	System::UInt16 cmd;
	cmd = val >> 13;
#if XGA_SHOW_COMMAND_TRACE == 1
	//LOG_MSG("XGA: Draw command %x", cmd);
#endif
	xga.curcommand = val;
	switch(cmd) {
		case 1: /* Draw line */
			if((val & 0x100) == 0) {
				if((val & 0x8) == 0) {
#if XGA_SHOW_COMMAND_TRACE == 1
					LOG_MSG("XGA: Drawing Bresenham line");
#endif
                    XGA_DrawLineBresenham(val);
				} else {
#if XGA_SHOW_COMMAND_TRACE == 1
					LOG_MSG("XGA: Drawing vector line");
#endif
					XGA_DrawLineVector(val);
				}
			} else {
				LOG_MSG("XGA: Wants line drawn from PIX_TRANS register!");
			}
			break;
		case 2: /* Rectangle fill */
			if((val & 0x100) == 0) {
				xga.waitcmd.wait = false;
#if XGA_SHOW_COMMAND_TRACE == 1
				LOG_MSG("XGA: Draw immediate rect: xy(%3d/%3d), len(%3d/%3d)",
					xga.curx,xga.cury,xga.MAPcount,xga.MIPcount);
#endif
				XGA_DrawRectangle(val);

			} else {
				
				xga.waitcmd.newline = true;
				xga.waitcmd.wait = true;
				xga.waitcmd.curx = xga.curx;
				xga.waitcmd.cury = xga.cury;
				xga.waitcmd.x1 = xga.curx;
				xga.waitcmd.y1 = xga.cury;
				xga.waitcmd.x2 = (System::UInt16)((xga.curx + xga.MAPcount)&0x0fff);
				xga.waitcmd.y2 = (System::UInt16)((xga.cury + xga.MIPcount + 1)&0x0fff);
				xga.waitcmd.sizex = xga.MAPcount;
				xga.waitcmd.sizey = xga.MIPcount + 1;
				xga.waitcmd.cmd = 2;
				xga.waitcmd.buswidth = vga.mode | ((val&0x600) >> 4);
				xga.waitcmd.data = 0;
				xga.waitcmd.datasize = 0;

#if XGA_SHOW_COMMAND_TRACE == 1
				LOG_MSG("XGA: Draw wait rect, w/h(%3d/%3d), x/y1(%3d/%3d), x/y2(%3d/%3d), %4x",
					xga.MAPcount+1, xga.MIPcount+1,xga.curx,xga.cury,
					(xga.curx + xga.MAPcount)&0x0fff,
					(xga.cury + xga.MIPcount + 1)&0x0fff,val&0xffff);
#endif
			
			}
			break;
		case 6: /* BitBLT */
#if XGA_SHOW_COMMAND_TRACE == 1
			LOG_MSG("XGA: Blit Rect");
#endif
			XGA_BlitRect(val);
			break;
		case 7: /* Pattern fill */
#if XGA_SHOW_COMMAND_TRACE == 1
			LOG_MSG("XGA: Pattern fill: src(%3d/%3d), dest(%3d/%3d), fill(%3d/%3d)",
				xga.curx,xga.cury,xga.destx,xga.desty,xga.MAPcount,xga.MIPcount);
#endif
			XGA_DrawPattern(val);
			break;
		default:
			LOG_MSG("XGA: Unhandled draw command %x", cmd);
			break;
	}
}

void XGA_SetDualReg(System::UInt32& reg, unsigned val) {
	switch(XGA_COLOR_MODE) {
	case M_LIN8:
		reg = (System::Byte)(val&0xff); break;
	case M_LIN15:
	case M_LIN16:
		reg = (System::UInt16)(val&0xffff); break;
	case M_LIN32:
		if (xga.control1 & 0x200)
			reg = val;
		else if (xga.control1 & 0x10)
			reg = (reg&0x0000ffff)|(val<<16);
		else
			reg = (reg&0xffff0000)|(val&0x0000ffff);
		xga.control1 ^= 0x10;
		break;
	}
}

unsigned XGA_GetDualReg(System::UInt32 reg) {
	switch(XGA_COLOR_MODE) {
	case M_LIN8:
		return (System::Byte)(reg&0xff);
	case M_LIN15: case M_LIN16:
		return (System::UInt16)(reg&0xffff);
	case M_LIN32:
		if (xga.control1 & 0x200) return reg;
		xga.control1 ^= 0x10;
		if (xga.control1 & 0x10) return reg&0x0000ffff;
		else return reg>>16;
	}
	return 0;
}

extern unsigned vga_read_p3da(unsigned port,unsigned iolen);

extern void vga_write_p3d4(unsigned port,unsigned val,unsigned iolen);
extern unsigned vga_read_p3d4(unsigned port,unsigned iolen);

extern void vga_write_p3d5(unsigned port,unsigned val,unsigned iolen);
extern unsigned vga_read_p3d5(unsigned port,unsigned iolen);

void XGA_Write(unsigned port, unsigned val, unsigned len) {
//	LOG_MSG("XGA: Write to port %x, val %8x, len %x", port,val, len);

	switch(port) {
		case 0x8100:// drawing control: row (low word), column (high word)
					// "CUR_X" and "CUR_Y" (see PORT 82E8h,PORT 86E8h)
			xga.cury = val & 0x0fff;
			if(len==4) xga.curx = (val>>16)&0x0fff;
			break;
		case 0x8102:
			xga.curx = val& 0x0fff;
			break;

		case 0x8108:// DWORD drawing control: destination Y and axial step
					// constant (low word), destination X and axial step
					// constant (high word) (see PORT 8AE8h,PORT 8EE8h)
			xga.desty = val&0x3FFF;
			if(len==4) xga.destx = (val>>16)&0x3fff;
			break;
		case 0x810a:
			xga.destx = val&0x3fff;
			break;
		case 0x8110: // WORD error term (see PORT 92E8h)
			xga.ErrTerm = val&0x3FFF;
			break;

		case 0x8120: // packed MMIO: DWORD background color (see PORT A2E8h)
			xga.backcolor = val;
			break;
		case 0x8124: // packed MMIO: DWORD foreground color (see PORT A6E8h)
			xga.forecolor = val;
			break;
		case 0x8128: // DWORD	write mask (see PORT AAE8h)
			xga.writemask = val;
			break;
		case 0x812C: // DWORD	read mask (see PORT AEE8h)
			xga.readmask = val;
			break;
		case 0x8134: // packed MMIO: DWORD	background mix (low word) and
					 // foreground mix (high word)	(see PORT B6E8h,PORT BAE8h)
			xga.backmix = val&0xFFFF;
			if(len==4) xga.foremix = (val>>16);
			break;
		case 0x8136:
			xga.foremix = val;
			break;
		case 0x8138:// DWORD top scissors (low word) and left scissors (high
					// word) (see PORT BEE8h,#P1047)
			xga.scissors.y1=val&0x0fff;
			if(len==4) xga.scissors.x1 = (val>>16)&0x0fff;
			break;
		case 0x813a:
			xga.scissors.x1 = val&0x0fff;
			break;
		case 0x813C:// DWORD bottom scissors (low word) and right scissors
					// (high word) (see PORT BEE8h,#P1047)
			xga.scissors.y2=val&0x0fff;
			if(len==4) xga.scissors.x2 = (val>>16)&0x0fff;
			break;
		case 0x813e:
			xga.scissors.x2 = val&0x0fff;
			break;

		case 0x8140:// DWORD data manipulation control (low word) and
					// miscellaneous 2 (high word) (see PORT BEE8h,#P1047)
			xga.pix_cntl=val&0xFFFF;
			if(len==4) xga.control2=(val>>16)&0x0fff;
			break;
		case 0x8144:// DWORD miscellaneous (low word) and read register select
					// (high word)(see PORT BEE8h,#P1047)
			xga.control1=val&0xffff;
			if(len==4)xga.read_sel=(val>>16)&0x7;
			break; 
		case 0x8148:// DWORD minor axis pixel count (low word) and major axis
					// pixel count (high word) (see PORT BEE8h,#P1047,PORT 96E8h)
			xga.MIPcount = val&0x0fff;
			if(len==4) xga.MAPcount = (val>>16)&0x0fff;
			break;
		case 0x814a:
			xga.MAPcount = val&0x0fff;
			break;
		case 0x92e8:
			xga.ErrTerm = val&0x3FFF;
			break;
		case 0x96e8:
			xga.MAPcount = val&0x0fff;
			break;
		case 0x9ae8:
		case 0x8118: // Trio64V+ packed MMIO
			XGA_DrawCmd(val, len);
			break;
		case 0xa2e8:
			XGA_SetDualReg(xga.backcolor, val);
			break;
		case 0xa6e8:
			XGA_SetDualReg(xga.forecolor, val);
			break;
		case 0xaae8:
			XGA_SetDualReg(xga.writemask, val);
			break;
		case 0xaee8:
			XGA_SetDualReg(xga.readmask, val);
			break;
		case 0x82e8:
			xga.cury = val&0x0fff;
			break;
		case 0x86e8:
			xga.curx = val&0x0fff;
			break;
		case 0x8ae8:
			xga.desty = val&0x3fff;
			break;
		case 0x8ee8:
			xga.destx = val&0x3fff;
			break;
		case 0xb2e8:
			LOG_MSG("COLOR_CMP not implemented");
			break;
		case 0xb6e8:
			xga.backmix = val;
			break;
		case 0xbae8:
			xga.foremix = val;
			break;
		case 0xbee8:
			XGA_Write_Multifunc(val, len);
			break;
		case 0xe2e8:
			xga.waitcmd.newline = false;
			XGA_DrawWait(val, len);
			break;
		case 0x83d4:
			if(len==1) vga_write_p3d4(0,val,1);
			else if(len==2) {
				vga_write_p3d4(0,val&0xff,1);
				vga_write_p3d5(0,val>>8,1);
			}
			else E_Exit("unimplemented XGA MMIO");
			break;
		case 0x83d5:
			if(len==1) vga_write_p3d5(0,val,1);
			else E_Exit("unimplemented XGA MMIO");
			break;
		default:
			if(port <= 0x4000) {
				//LOG_MSG("XGA: Wrote to port %4x with %08x, len %x", port, val, len);
				xga.waitcmd.newline = false;
				XGA_DrawWait(val, len);
				
			}
			else LOG_MSG("XGA: Wrote to port %x with %x, len %x", port, val, len);
			break;
	}
}

unsigned XGA_Read(unsigned port, unsigned len) {
	switch(port) {
		case 0x8118:
		case 0x9ae8:
			return 0x400; // nothing busy
			break;
		case 0x81ec: // S3 video data processor
			return 0x00007000; 
			break;
		case 0x83da:
			{
				int delaycyc = CPU_CycleMax/5000;
				if(GCC_UNLIKELY(CPU_Cycles < 3*delaycyc)) delaycyc = 0;
				CPU_Cycles -= delaycyc;
				CPU_IODelayRemoved += delaycyc;
				return vga_read_p3da(0,0);
				break;
			}
		case 0x83d4:
			if(len==1) return vga_read_p3d4(0,0);
			else E_Exit("unimplemented XGA MMIO");
			break;
		case 0x83d5:
			if(len==1) return vga_read_p3d5(0,0);
			else E_Exit("unimplemented XGA MMIO");
			break;
		case 0x9ae9:
			if(xga.waitcmd.wait) return 0x4;
			else return 0x0;
		case 0xbee8:
			return XGA_Read_Multifunc();
		case 0xa2e8:
			return XGA_GetDualReg(xga.backcolor);
			break;
		case 0xa6e8:
			return XGA_GetDualReg(xga.forecolor);
			break;
		case 0xaae8:
			return XGA_GetDualReg(xga.writemask);
			break;
		case 0xaee8:
			return XGA_GetDualReg(xga.readmask);
			break;
		default:
			//LOG_MSG("XGA: Read from port %x, len %x", port, len);
			break;
	}
	return 0xffffffff; 
}

void VGA_SetupXGA(void) {
	if (!IS_VGA_ARCH) return;

	memset(&xga, 0, sizeof(XGAStatus));

	xga.scissors.y1 = 0;
	xga.scissors.x1 = 0;
	xga.scissors.y2 = 0xFFF;
	xga.scissors.x2 = 0xFFF;
	
	IO_RegisterWriteHandler(0x42e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x42e8,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x46e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x4ae8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	
	IO_RegisterWriteHandler(0x82e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x82e8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x82e9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x82e9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x86e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x86e8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x86e9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x86e9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x8ae8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x8ae8,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x8ee8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x8ee8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x8ee9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x8ee9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x92e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x92e8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x92e9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x92e9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x96e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x96e8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x96e9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x96e9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x9ae8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x9ae8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x9ae9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x9ae9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0x9ee8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x9ee8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0x9ee9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0x9ee9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xa2e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xa2e8,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xa6e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xa6e8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xa6e9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xa6e9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xaae8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xaae8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xaae9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xaae9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xaee8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xaee8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xaee9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xaee9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xb2e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xb2e8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xb2e9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xb2e9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xb6e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xb6e8,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xbee8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xbee8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xbee9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xbee9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xbae8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xbae8,&XGA_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xbae9,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xbae9,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xe2e8,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xe2e8,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xe2e0,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xe2e0,&XGA_Read,IO_MB | IO_MW | IO_MD);

	IO_RegisterWriteHandler(0xe2ea,&XGA_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xe2ea,&XGA_Read,IO_MB | IO_MW | IO_MD);
}
