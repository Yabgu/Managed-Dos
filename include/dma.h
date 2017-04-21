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

/* $Id: dma.h,v 1.20 2009-07-24 09:56:14 c2woody Exp $ */

#ifndef DOSBOX_DMA_H
#define DOSBOX_DMA_H

enum DMAEvent {
	DMA_REACHED_TC,
	DMA_MASKED,
	DMA_UNMASKED,
	DMA_TRANSFEREND
};

class DmaChannel;
typedef void (* DMA_CallBack)(DmaChannel * chan,DMAEvent event);

class DmaChannel {
public:
	System::UInt32 pagebase;
	System::UInt16 baseaddr;
	System::UInt32 curraddr;
	System::UInt16 basecnt;
	System::UInt16 currcnt;
	System::Byte channum;
	System::Byte pagenum;
	System::Byte DMA16;
	bool increment;
	bool autoinit;
	System::Byte trantype;
	bool masked;
	bool tcount;
	bool request;
	DMA_CallBack callback;

	DmaChannel(System::Byte num, bool dma16);
	void DoCallBack(DMAEvent event) {
		if (callback)	(*callback)(this,event);
	}
	void SetMask(bool _mask) {
		masked=_mask;
		DoCallBack(masked ? DMA_MASKED : DMA_UNMASKED);
	}
	void Register_Callback(DMA_CallBack _cb) { 
		callback = _cb; 
		SetMask(masked);
		if (callback) Raise_Request();
		else Clear_Request();
	}
	void ReachedTC(void) {
		tcount=true;
		DoCallBack(DMA_REACHED_TC);
	}
	void SetPage(System::Byte val) {
		pagenum=val;
		pagebase=(pagenum >> DMA16) << (16+DMA16);
	}
	void Raise_Request(void) {
		request=true;
	}
	void Clear_Request(void) {
		request=false;
	}
	unsigned Read(unsigned size, System::Byte * buffer);
	unsigned Write(unsigned size, System::Byte * buffer);
};

class DmaController {
private:
	System::Byte ctrlnum;
	bool flipflop;
	DmaChannel *DmaChannels[4];
public:
	IO_ReadHandleObject DMA_ReadHandler[0x11];
	IO_WriteHandleObject DMA_WriteHandler[0x11];
	DmaController(System::Byte num) {
		flipflop = false;
		ctrlnum = num;		/* first or second DMA controller */
		for(System::Byte i=0;i<4;i++) {
			DmaChannels[i] = new DmaChannel(i+ctrlnum*4,ctrlnum==1);
		}
	}
	~DmaController(void) {
		for(System::Byte i=0;i<4;i++) {
			delete DmaChannels[i];
		}
	}
	DmaChannel * GetChannel(System::Byte chan) {
		if (chan<4) return DmaChannels[chan];
		else return NULL;
	}
	void WriteControllerReg(unsigned reg,unsigned val,unsigned len);
	unsigned ReadControllerReg(unsigned reg,unsigned len);
};

DmaChannel * GetDMAChannel(System::Byte chan);

void CloseSecondDMAController(void);
bool SecondDMAControllerAvailable(void);

static System::UInt32 dma_wrapping = 0xffff;

#endif
