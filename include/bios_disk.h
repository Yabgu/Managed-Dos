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

#ifndef DOSBOX_BIOS_DISK_H
#define DOSBOX_BIOS_DISK_H

#include <stdio.h>
#ifndef DOSBOX_MEM_H
#include "mem.h"
#endif
#ifndef DOSBOX_DOS_INC_H
#include "dos_inc.h"
#endif
#ifndef DOSBOX_BIOS_H
#include "bios.h"
#endif

/* The Section handling Bios Disk Access */
#define BIOS_MAX_DISK 10

#define MAX_SWAPPABLE_DISKS 20
struct diskGeo {
	System::UInt32 ksize;  /* Size in kilobytes */
	System::UInt16 secttrack; /* Sectors per track */
	System::UInt16 headscyl;  /* Heads per cylinder */
	System::UInt16 cylcount;  /* Cylinders per side */
	System::UInt16 biosval;   /* Type to return from BIOS */
};
extern diskGeo DiskGeometryList[];

class imageDisk  {
public:
	System::Byte Read_Sector(System::UInt32 head,System::UInt32 cylinder,System::UInt32 sector,void * data);
	System::Byte Write_Sector(System::UInt32 head,System::UInt32 cylinder,System::UInt32 sector,void * data);
	System::Byte Read_AbsoluteSector(System::UInt32 sectnum, void * data);
	System::Byte Write_AbsoluteSector(System::UInt32 sectnum, void * data);

	void Set_Geometry(System::UInt32 setHeads, System::UInt32 setCyl, System::UInt32 setSect, System::UInt32 setSectSize);
	void Get_Geometry(System::UInt32 * getHeads, System::UInt32 *getCyl, System::UInt32 *getSect, System::UInt32 *getSectSize);
	System::Byte GetBiosType(void);
	System::UInt32 getSectSize(void);
	imageDisk(FILE *imgFile, System::Byte *imgName, System::UInt32 imgSizeK, bool isHardDisk);
	~imageDisk() { if(diskimg != NULL) { fclose(diskimg); }	};

	bool hardDrive;
	bool active;
	FILE *diskimg;
	System::Byte diskname[512];
	System::Byte floppytype;

	System::UInt32 sector_size;
	System::UInt32 heads,cylinders,sectors;
};

void updateDPT(void);

#define MAX_HDD_IMAGES 2

extern imageDisk *imageDiskList[2 + MAX_HDD_IMAGES];
extern imageDisk *diskSwap[20];
extern int swapPosition;
extern System::UInt16 imgDTASeg; /* Real memory location of temporary DTA pointer for fat image disk access */
extern RealPt imgDTAPtr; /* Real memory location of temporary DTA pointer for fat image disk access */
extern DOS_DTA *imgDTA;

void swapInDisks(void);
void swapInNextDisk(void);
bool getSwapRequest(void);

#endif
