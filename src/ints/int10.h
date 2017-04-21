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

/* $Id: int10.h,v 1.42 2009-09-06 19:25:34 c2woody Exp $ */

#include "vga.h"

#define S3_LFB_BASE		0xC0000000

#define BIOSMEM_SEG		0x40

#define BIOSMEM_INITIAL_MODE  0x10
#define BIOSMEM_CURRENT_MODE  0x49
#define BIOSMEM_NB_COLS       0x4A
#define BIOSMEM_PAGE_SIZE     0x4C
#define BIOSMEM_CURRENT_START 0x4E
#define BIOSMEM_CURSOR_POS    0x50
#define BIOSMEM_CURSOR_TYPE   0x60
#define BIOSMEM_CURRENT_PAGE  0x62
#define BIOSMEM_CRTC_ADDRESS  0x63
#define BIOSMEM_CURRENT_MSR   0x65
#define BIOSMEM_CURRENT_PAL   0x66
#define BIOSMEM_NB_ROWS       0x84
#define BIOSMEM_CHAR_HEIGHT   0x85
#define BIOSMEM_VIDEO_CTL     0x87
#define BIOSMEM_SWITCHES      0x88
#define BIOSMEM_MODESET_CTL   0x89
#define BIOSMEM_DCC_INDEX     0x8A
#define BIOSMEM_CRTCPU_PAGE   0x8A
#define BIOSMEM_VS_POINTER    0xA8


/*
 *
 * VGA registers
 *
 */
#define VGAREG_ACTL_ADDRESS            0x3c0
#define VGAREG_ACTL_WRITE_DATA         0x3c0
#define VGAREG_ACTL_READ_DATA          0x3c1

#define VGAREG_INPUT_STATUS            0x3c2
#define VGAREG_WRITE_MISC_OUTPUT       0x3c2
#define VGAREG_VIDEO_ENABLE            0x3c3
#define VGAREG_SEQU_ADDRESS            0x3c4
#define VGAREG_SEQU_DATA               0x3c5

#define VGAREG_PEL_MASK                0x3c6
#define VGAREG_DAC_STATE               0x3c7
#define VGAREG_DAC_READ_ADDRESS        0x3c7
#define VGAREG_DAC_WRITE_ADDRESS       0x3c8
#define VGAREG_DAC_DATA                0x3c9

#define VGAREG_READ_FEATURE_CTL        0x3ca
#define VGAREG_READ_MISC_OUTPUT        0x3cc

#define VGAREG_GRDC_ADDRESS            0x3ce
#define VGAREG_GRDC_DATA               0x3cf

#define VGAREG_MDA_CRTC_ADDRESS        0x3b4
#define VGAREG_MDA_CRTC_DATA           0x3b5
#define VGAREG_VGA_CRTC_ADDRESS        0x3d4
#define VGAREG_VGA_CRTC_DATA           0x3d5

#define VGAREG_MDA_WRITE_FEATURE_CTL   0x3ba
#define VGAREG_VGA_WRITE_FEATURE_CTL   0x3da
#define VGAREG_ACTL_RESET              0x3da
#define VGAREG_TDY_RESET               0x3da
#define VGAREG_TDY_ADDRESS             0x3da
#define VGAREG_TDY_DATA                0x3de
#define VGAREG_PCJR_DATA               0x3da

#define VGAREG_MDA_MODECTL             0x3b8
#define VGAREG_CGA_MODECTL             0x3d8
#define VGAREG_CGA_PALETTE             0x3d9

/* Video memory */
#define VGAMEM_GRAPH 0xA000
#define VGAMEM_CTEXT 0xB800
#define VGAMEM_MTEXT 0xB000

#define BIOS_NCOLS System::UInt16 ncols=real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
#define BIOS_NROWS System::UInt16 nrows=(System::UInt16)real_readb(BIOSMEM_SEG,BIOSMEM_NB_ROWS)+1;

extern System::Byte int10_font_08[256 * 8];
extern System::Byte int10_font_14[256 * 14];
extern System::Byte int10_font_16[256 * 16];

struct VideoModeBlock {
	System::UInt16	mode;
	VGAModes	type;
	unsigned	swidth, sheight;
	unsigned	twidth, theight;
	unsigned	cwidth, cheight;
	unsigned	ptotal,pstart,plength;

	unsigned	htotal,vtotal;
	unsigned	hdispend,vdispend;
	unsigned	special;
	
};
extern VideoModeBlock ModeList_VGA[];
extern VideoModeBlock * CurMode;

typedef struct {
	struct {
		RealPt font_8_first;
		RealPt font_8_second;
		RealPt font_14;
		RealPt font_16;
		RealPt font_14_alternate;
		RealPt font_16_alternate;
		RealPt static_state;
		RealPt video_save_pointers;
		RealPt video_parameter_table;
		RealPt video_save_pointer_table;
		RealPt video_dcc_table;
		RealPt oemstring;
		RealPt vesa_modes;
		RealPt pmode_interface;
		System::UInt16 pmode_interface_size;
		System::UInt16 pmode_interface_start;
		System::UInt16 pmode_interface_window;
		System::UInt16 pmode_interface_palette;
		System::UInt16 used;
	} rom;
	System::UInt16 vesa_setmode;
	bool vesa_nolfb;
	bool vesa_oldvbe;
} Int10Data;

extern Int10Data int10;

static System::Byte CURSOR_POS_COL(System::Byte page) {
	return real_readb(BIOSMEM_SEG,BIOSMEM_CURSOR_POS+page*2);
}

static System::Byte CURSOR_POS_ROW(System::Byte page) {
	return real_readb(BIOSMEM_SEG,BIOSMEM_CURSOR_POS+page*2+1);
}

bool INT10_SetVideoMode(System::UInt16 mode);

void INT10_ScrollWindow(System::Byte rul,System::Byte cul,System::Byte rlr,System::Byte clr,System::SByte nlines,System::Byte attr,System::Byte page);

void INT10_SetActivePage(System::Byte page);
void INT10_GetFuncStateInformation(PhysPt save);

void INT10_SetCursorShape(System::Byte first,System::Byte last);
void INT10_SetCursorPos(System::Byte row,System::Byte col,System::Byte page);
void INT10_TeletypeOutput(System::Byte chr,System::Byte attr);
void INT10_TeletypeOutputAttr(System::Byte chr,System::Byte attr,bool useattr);
void INT10_ReadCharAttr(System::UInt16 * result,System::Byte page);
void INT10_WriteChar(System::Byte chr,System::Byte attr,System::Byte page,System::UInt16 count,bool showattr);
void INT10_WriteString(System::Byte row,System::Byte col,System::Byte flag,System::Byte attr,PhysPt string,System::UInt16 count,System::Byte page);

/* Graphics Stuff */
void INT10_PutPixel(System::UInt16 x,System::UInt16 y,System::Byte page,System::Byte color);
void INT10_GetPixel(System::UInt16 x,System::UInt16 y,System::Byte page,System::Byte * color);

/* Font Stuff */
void INT10_LoadFont(PhysPt font,bool reload,unsigned count,unsigned offset,unsigned map,unsigned height);
void INT10_ReloadFont(void);

/* Palette Group */
void INT10_SetBackgroundBorder(System::Byte val);
void INT10_SetColorSelect(System::Byte val);
void INT10_SetSinglePaletteRegister(System::Byte reg,System::Byte val);
void INT10_SetOverscanBorderColor(System::Byte val);
void INT10_SetAllPaletteRegisters(PhysPt data);
void INT10_ToggleBlinkingBit(System::Byte state);
void INT10_GetSinglePaletteRegister(System::Byte reg,System::Byte * val);
void INT10_GetOverscanBorderColor(System::Byte * val);
void INT10_GetAllPaletteRegisters(PhysPt data);
void INT10_SetSingleDacRegister(System::Byte index,System::Byte red,System::Byte green,System::Byte blue);
void INT10_GetSingleDacRegister(System::Byte index,System::Byte * red,System::Byte * green,System::Byte * blue);
void INT10_SetDACBlock(System::UInt16 index,System::UInt16 count,PhysPt data);
void INT10_GetDACBlock(System::UInt16 index,System::UInt16 count,PhysPt data);
void INT10_SelectDACPage(System::Byte function,System::Byte mode);
void INT10_GetDACPage(System::Byte* mode,System::Byte* page);
void INT10_SetPelMask(System::Byte mask);
void INT10_GetPelMask(System::Byte & mask);
void INT10_PerformGrayScaleSumming(System::UInt16 start_reg,System::UInt16 count);


/* Vesa Group */
System::Byte VESA_GetSVGAInformation(System::UInt16 seg,System::UInt16 off);
System::Byte VESA_GetSVGAModeInformation(System::UInt16 mode,System::UInt16 seg,System::UInt16 off);
System::Byte VESA_SetSVGAMode(System::UInt16 mode);
System::Byte VESA_GetSVGAMode(System::UInt16 & mode);
System::Byte VESA_SetCPUWindow(System::Byte window,System::Byte address);
System::Byte VESA_GetCPUWindow(System::Byte window,System::UInt16 & address);
System::Byte VESA_ScanLineLength(System::Byte subcall, System::UInt16 val, System::UInt16 & bytes,System::UInt16 & pixels,System::UInt16 & lines);
System::Byte VESA_SetDisplayStart(System::UInt16 x,System::UInt16 y);
System::Byte VESA_GetDisplayStart(System::UInt16 & x,System::UInt16 & y);
System::Byte VESA_SetPalette(PhysPt data,unsigned index,unsigned count);
System::Byte VESA_GetPalette(PhysPt data,unsigned index,unsigned count);

/* Sub Groups */
void INT10_SetupRomMemory(void);
void INT10_SetupRomMemoryChecksum(void);
void INT10_SetupVESA(void);

/* EGA RIL */
RealPt INT10_EGA_RIL_GetVersionPt(void);
void INT10_EGA_RIL_ReadRegister(System::Byte & bl, System::UInt16 dx);
void INT10_EGA_RIL_WriteRegister(System::Byte & bl, System::Byte bh, System::UInt16 dx);
void INT10_EGA_RIL_ReadRegisterRange(System::Byte ch, System::Byte cl, System::UInt16 dx, PhysPt dst);
void INT10_EGA_RIL_WriteRegisterRange(System::Byte ch, System::Byte cl, System::UInt16 dx, PhysPt dst);
void INT10_EGA_RIL_ReadRegisterSet(System::UInt16 cx, PhysPt tbl);
void INT10_EGA_RIL_WriteRegisterSet(System::UInt16 cx, PhysPt tbl);

/* Video State */
unsigned INT10_VideoState_GetSize(unsigned state);
bool INT10_VideoState_Save(unsigned state,RealPt buffer);
bool INT10_VideoState_Restore(unsigned state,RealPt buffer);

/* Video Parameter Tables */
System::UInt16 INT10_SetupVideoParameterTable(PhysPt basepos);
void INT10_SetupBasicVideoParameterTable(void);
