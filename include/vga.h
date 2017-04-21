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

/* $Id: vga.h,v 1.48 2009-11-03 21:06:59 h-a-l-9000 Exp $ */

#ifndef DOSBOX_VGA_H
#define DOSBOX_VGA_H

#ifndef DOSBOX_DOSBOX_H
#include "dosbox.h"
#endif

//Don't enable keeping changes and mapping lfb probably...
#define VGA_LFB_MAPPED
//#define VGA_KEEP_CHANGES
#define VGA_CHANGE_SHIFT	9

class PageHandler;


enum VGAModes {
	M_CGA2, M_CGA4,
	M_EGA, M_VGA,
	M_LIN4, M_LIN8, M_LIN15, M_LIN16, M_LIN32,
	M_TEXT,
	M_HERC_GFX, M_HERC_TEXT,
	M_CGA16, M_TANDY2, M_TANDY4, M_TANDY16, M_TANDY_TEXT,
	M_ERROR
};


#define CLK_25 25175
#define CLK_28 28322

#define MIN_VCO	180000
#define MAX_VCO 360000

#define S3_CLOCK_REF	14318	/* KHz */
#define S3_CLOCK(_M,_N,_R)	((S3_CLOCK_REF * ((_M) + 2)) / (((_N) + 2) * (1 << (_R))))
#define S3_MAX_CLOCK	150000	/* KHz */

#define S3_XGA_1024		0x00
#define S3_XGA_1152		0x01
#define S3_XGA_640		0x40
#define S3_XGA_800		0x80
#define S3_XGA_1280		0xc0
#define S3_XGA_WMASK	(S3_XGA_640|S3_XGA_800|S3_XGA_1024|S3_XGA_1152|S3_XGA_1280)

#define S3_XGA_8BPP  0x00
#define S3_XGA_16BPP 0x10
#define S3_XGA_32BPP 0x30
#define S3_XGA_CMASK (S3_XGA_8BPP|S3_XGA_16BPP|S3_XGA_32BPP)

typedef struct {
	bool attrindex;
} VGA_Internal;

typedef struct {
/* Memory handlers */
	unsigned mh_mask;

/* Video drawing */
	unsigned display_start;
	unsigned real_start;
	bool retrace;					/* A retrace is active */
	unsigned scan_len;
	unsigned cursor_start;

/* Some other screen related variables */
	unsigned line_compare;
	bool chained;					/* Enable or Disabled Chain 4 Mode */
	bool compatible_chain4;

	/* Pixel Scrolling */
	System::Byte pel_panning;				/* Amount of pixels to skip when starting horizontal line */
	System::Byte hlines_skip;
	System::Byte bytes_skip;
	System::Byte addr_shift;

/* Specific stuff memory write/read handling */
	
	System::Byte read_mode;
	System::Byte write_mode;
	System::Byte read_map_select;
	System::Byte color_dont_care;
	System::Byte color_compare;
	System::Byte data_rotate;
	System::Byte raster_op;

	System::UInt32 full_bit_mask;
	System::UInt32 full_map_mask;
	System::UInt32 full_not_map_mask;
	System::UInt32 full_set_reset;
	System::UInt32 full_not_enable_set_reset;
	System::UInt32 full_enable_set_reset;
	System::UInt32 full_enable_and_set_reset;
} VGA_Config;

typedef enum {
	PART,
	LINE,
	//EGALINE
} Drawmode;

typedef struct {
	bool resizing;
	unsigned width;
	unsigned height;
	unsigned blocks;
	unsigned address;
	unsigned panning;
	unsigned bytes_skip;
	System::Byte *linear_base;
	unsigned linear_mask;
	unsigned address_add;
	unsigned line_length;
	unsigned address_line_total;
	unsigned address_line;
	unsigned lines_total;
	unsigned vblank_skip;
	unsigned lines_done;
	unsigned lines_scaled;
	unsigned split_line;
	unsigned parts_total;
	unsigned parts_lines;
	unsigned parts_left;
	unsigned byte_panning_shift;
	struct {
		double framestart;
		double vrstart, vrend;		// V-retrace
		double hrstart, hrend;		// H-retrace
		double hblkstart, hblkend;	// H-blanking
		double vblkstart, vblkend;	// V-Blanking
		double vdend, vtotal;
		double hdend, htotal;
		double parts;
	} delay;
	unsigned bpp;
	double aspect_ratio;
	bool double_scan;
	bool doublewidth,doubleheight;
	System::Byte font[64*1024];
	System::Byte * font_tables[2];
	unsigned blinking;
	struct {
		unsigned address;
		System::Byte sline,eline;
		System::Byte count,delay;
		System::Byte enabled;
	} cursor;
	Drawmode mode;
	bool vret_triggered;
} VGA_Draw;

typedef struct {
	System::Byte curmode;
	System::UInt16 originx, originy;
	System::Byte fstackpos, bstackpos;
	System::Byte forestack[3];
	System::Byte backstack[3];
	System::UInt16 startaddr;
	System::Byte posx, posy;
	System::Byte mc[64][64];
} VGA_HWCURSOR;

typedef struct {
	System::Byte reg_lock1;
	System::Byte reg_lock2;
	System::Byte reg_31;
	System::Byte reg_35;
	System::Byte reg_36; // RAM size
	System::Byte reg_3a; // 4/8/doublepixel bit in there
	System::Byte reg_40; // 8415/A functionality register
	System::Byte reg_41; // BIOS flags 
	System::Byte reg_43;
	System::Byte reg_45; // Hardware graphics cursor
	System::Byte reg_50;
	System::Byte reg_51;
	System::Byte reg_52;
	System::Byte reg_55;
	System::Byte reg_58;
	System::Byte reg_6b; // LFB BIOS scratchpad
	System::Byte ex_hor_overflow;
	System::Byte ex_ver_overflow;
	System::UInt16 la_window;
	System::Byte misc_control_2;
	System::Byte ext_mem_ctrl;
	unsigned xga_screen_width;
	VGAModes xga_color_mode;
	struct {
		System::Byte r;
		System::Byte n;
		System::Byte m;
	} clk[4],mclk;
	struct {
		System::Byte lock;
		System::Byte cmd;
	} pll;
	VGA_HWCURSOR hgc;
} VGA_S3;

typedef struct {
	System::Byte mode_control;
	System::Byte enable_int;
} VGA_HERC;

typedef struct {
	System::Byte index;
	System::Byte htotal;
	System::Byte hdend;
	System::Byte hsyncp;
	System::Byte hsyncw;
	System::Byte vtotal;
	System::Byte vdend;
	System::Byte vadjust;
	System::Byte vsyncp;
	System::Byte vsyncw;
	System::Byte max_scanline;
	System::UInt16 lightpen;
	bool lightpen_triggered;
	System::Byte cursor_start;
	System::Byte cursor_end;
} VGA_OTHER;

typedef struct {
	System::Byte pcjr_flipflop;
	System::Byte mode_control;
	System::Byte color_select;
	System::Byte disp_bank;
	System::Byte reg_index;
	System::Byte gfx_control;
	System::Byte palette_mask;
	System::Byte extended_ram;
	System::Byte border_color;
	System::Byte line_mask, line_shift;
	System::Byte draw_bank, mem_bank;
	System::Byte *draw_base, *mem_base;
	unsigned addr_mask;
} VGA_TANDY;

typedef struct {
	System::Byte index;
	System::Byte reset;
	System::Byte clocking_mode;
	System::Byte map_mask;
	System::Byte character_map_select;
	System::Byte memory_mode;
} VGA_Seq;

typedef struct {
	System::Byte palette[16];
	System::Byte mode_control;
	System::Byte horizontal_pel_panning;
	System::Byte overscan_color;
	System::Byte color_plane_enable;
	System::Byte color_select;
	System::Byte index;
	System::Byte disabled; // Used for disabling the screen.
					// Bit0: screen disabled by attribute controller index
					// Bit1: screen disabled by sequencer index 1 bit 5
					// These are put together in one variable for performance reasons:
					// the line drawing function is called maybe 60*480=28800 times/s,
					// and we only need to check one variable for zero this way.
} VGA_Attr;

typedef struct {
	System::Byte horizontal_total;
	System::Byte horizontal_display_end;
	System::Byte start_horizontal_blanking;
	System::Byte end_horizontal_blanking;
	System::Byte start_horizontal_retrace;
	System::Byte end_horizontal_retrace;
	System::Byte vertical_total;
	System::Byte overflow;
	System::Byte preset_row_scan;
	System::Byte maximum_scan_line;
	System::Byte cursor_start;
	System::Byte cursor_end;
	System::Byte start_address_high;
	System::Byte start_address_low;
	System::Byte cursor_location_high;
	System::Byte cursor_location_low;
	System::Byte vertical_retrace_start;
	System::Byte vertical_retrace_end;
	System::Byte vertical_display_end;
	System::Byte offset;
	System::Byte underline_location;
	System::Byte start_vertical_blanking;
	System::Byte end_vertical_blanking;
	System::Byte mode_control;
	System::Byte line_compare;

	System::Byte index;
	bool read_only;
} VGA_Crtc;

typedef struct {
	System::Byte index;
	System::Byte set_reset;
	System::Byte enable_set_reset;
	System::Byte color_compare;
	System::Byte data_rotate;
	System::Byte read_map_select;
	System::Byte mode;
	System::Byte miscellaneous;
	System::Byte color_dont_care;
	System::Byte bit_mask;
} VGA_Gfx;

typedef struct  {
	System::Byte red;
	System::Byte green;
	System::Byte blue;
} RGBEntry;

typedef struct {
	System::Byte bits;						/* DAC int, usually 6 or 8 */
	System::Byte pel_mask;
	System::Byte pel_index;	
	System::Byte state;
	System::Byte write_index;
	System::Byte read_index;
	unsigned first_changed;
	System::Byte combine[16];
	RGBEntry rgb[0x100];
	System::UInt16 xlat16[256];
} VGA_Dac;

typedef struct {
	unsigned	readStart, writeStart;
	unsigned	bankMask;
	unsigned	bank_read_full;
	unsigned	bank_write_full;
	System::Byte	bank_read;
	System::Byte	bank_write;
	unsigned	bank_size;
} VGA_SVGA;

typedef union {
	System::UInt32 d;
	System::Byte b[4];
} VGA_Latch;

typedef struct {
	System::Byte* linear;
	System::Byte* linear_orgptr;
} VGA_Memory;

typedef struct {
	//Add a few more just to be safe
	System::Byte*	map; /* allocated dynamically: [(VGA_MEMORY >> VGA_CHANGE_SHIFT) + 32] */
	System::Byte	checkMask, frame, writeMask;
	bool	active;
	System::UInt32  clearMask;
	System::UInt32	start, last;
	System::UInt32	lastAddress;
} VGA_Changes;

typedef struct {
	System::UInt32 page;
	System::UInt32 addr;
	System::UInt32 mask;
	PageHandler *handler;
} VGA_LFB;

typedef struct {
	VGAModes mode;								/* The mode the vga system is in */
	System::Byte misc_output;
	VGA_Draw draw;
	VGA_Config config;
	VGA_Internal internal;
/* Internal module groups */
	VGA_Seq seq;
	VGA_Attr attr;
	VGA_Crtc crtc;
	VGA_Gfx gfx;
	VGA_Dac dac;
	VGA_Latch latch;
	VGA_S3 s3;
	VGA_SVGA svga;
	VGA_HERC herc;
	VGA_TANDY tandy;
	VGA_OTHER other;
	VGA_Memory mem;
	System::UInt32 vmemwrap; /* this is assumed to be power of 2 */
	System::Byte* fastmem;  /* memory for fast (usually 16-color) rendering, always twice as big as vmemsize */
	System::Byte* fastmem_orgptr;
	System::UInt32 vmemsize;
#ifdef VGA_KEEP_CHANGES
	VGA_Changes changes;
#endif
	VGA_LFB lfb;
} VGA_Type;


/* Hercules Palette function */
void Herc_Palette(void);

/* Functions for different resolutions */
void VGA_SetMode(VGAModes mode);
void VGA_DetermineMode(void);
void VGA_SetupHandlers(void);
void VGA_StartResize(unsigned delay=50);
void VGA_SetupDrawing(unsigned val);
void VGA_CheckScanLength(void);
void VGA_ChangedBank(void);

/* Some DAC/Attribute functions */
void VGA_DAC_CombineColor(System::Byte attr,System::Byte pal);
void VGA_DAC_SetEntry(unsigned entry,System::Byte red,System::Byte green,System::Byte blue);
void VGA_ATTR_SetPalette(System::Byte index,System::Byte val);

/* The VGA Subfunction startups */
void VGA_SetupAttr(void);
void VGA_SetupMemory(Section* sec);
void VGA_SetupDAC(void);
void VGA_SetupCRTC(void);
void VGA_SetupMisc(void);
void VGA_SetupGFX(void);
void VGA_SetupSEQ(void);
void VGA_SetupOther(void);
void VGA_SetupXGA(void);

/* Some Support Functions */
void VGA_SetClock(unsigned which,unsigned target);
void VGA_DACSetEntirePalette(void);
void VGA_StartRetrace(void);
void VGA_StartUpdateLFB(void);
void VGA_SetBlinking(unsigned enabled);
void VGA_SetCGA2Table(System::Byte val0,System::Byte val1);
void VGA_SetCGA4Table(System::Byte val0,System::Byte val1,System::Byte val2,System::Byte val3);
void VGA_ActivateHardwareCursor(void);
void VGA_KillDrawing(void);

extern VGA_Type vga;

/* Support for modular SVGA implementation */
/* Video mode extra data to be passed to FinishSetMode_SVGA().
   This structure will be in flux until all drivers (including S3)
   are properly separated. Right now it contains only three overflow
   fields in S3 format and relies on drivers re-interpreting those.
   For reference:
   ver_overflow:X|line_comp10|X|vretrace10|X|vbstart10|vdispend10|vtotal10
   hor_overflow:X|X|X|hretrace8|X|hblank8|hdispend8|htotal8
   offset is not currently used by drivers (useful only for S3 itself)
   It also contains basic int10 mode data - number, vtotal, htotal
   */
typedef struct {
	System::Byte ver_overflow;
	System::Byte hor_overflow;
	unsigned offset;
	unsigned modeNo;
	unsigned htotal;
	unsigned vtotal;
} VGA_ModeExtraData;

// Vector function prototypes
typedef void (*tWritePort)(unsigned reg,unsigned val,unsigned iolen);
typedef unsigned (*tReadPort)(unsigned reg,unsigned iolen);
typedef void (*tFinishSetMode)(unsigned crtc_base, VGA_ModeExtraData* modeData);
typedef void (*tDetermineMode)();
typedef void (*tSetClock)(unsigned which,unsigned target);
typedef unsigned (*tGetClock)();
typedef bool (*tHWCursorActive)();
typedef bool (*tAcceptsMode)(unsigned modeNo);

struct SVGA_Driver {
	tWritePort write_p3d5;
	tReadPort read_p3d5;
	tWritePort write_p3c5;
	tReadPort read_p3c5;
	tWritePort write_p3c0;
	tReadPort read_p3c1;
	tWritePort write_p3cf;
	tReadPort read_p3cf;

	tFinishSetMode set_video_mode;
	tDetermineMode determine_mode;
	tSetClock set_clock;
	tGetClock get_clock;
	tHWCursorActive hardware_cursor_active;
	tAcceptsMode accepts_mode;
};

extern SVGA_Driver svga;

void SVGA_Setup_S3Trio(void);
void SVGA_Setup_TsengET4K(void);
void SVGA_Setup_TsengET3K(void);
void SVGA_Setup_ParadisePVGA1A(void);
void SVGA_Setup_Driver(void);

// Amount of video memory required for a mode, implemented in int10_modes.cpp
unsigned VideoModeMemSize(unsigned mode);

extern System::UInt32 ExpandTable[256];
extern System::UInt32 FillTable[16];
extern System::UInt32 CGA_2_Table[16];
extern System::UInt32 CGA_4_Table[256];
extern System::UInt32 CGA_4_HiRes_Table[256];
extern System::UInt32 CGA_16_Table[256];
extern System::UInt32 TXT_Font_Table[16];
extern System::UInt32 TXT_FG_Table[16];
extern System::UInt32 TXT_BG_Table[16];
extern System::UInt32 Expand16Table[4][16];
extern System::UInt32 Expand16BigTable[0x10000];


#endif
