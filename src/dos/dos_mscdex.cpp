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

/* $Id: dos_mscdex.cpp,v 1.59 2009-04-16 12:28:30 qbix79 Exp $ */

#include <string.h>
#include <ctype.h>
#include "regs.h"
#include "callback.h"
#include "dos_system.h"
#include "dos_inc.h"
#include "setup.h"
#include "support.h"
#include "bios_disk.h"
#include "cpu.h"

#include "cdrom.h"

#define MSCDEX_LOG LOG(LOG_MISC,LOG_ERROR)
//#define MSCDEX_LOG

#define MSCDEX_VERSION_HIGH	2
#define MSCDEX_VERSION_LOW	23
#define MSCDEX_MAX_DRIVES	8

// Error Codes
#define MSCDEX_ERROR_INVALID_FUNCTION	1
#define MSCDEX_ERROR_BAD_FORMAT			11
#define MSCDEX_ERROR_UNKNOWN_DRIVE		15
#define MSCDEX_ERROR_DRIVE_NOT_READY	21

// Request Status
#define	REQUEST_STATUS_DONE		0x0100
#define	REQUEST_STATUS_ERROR	0x8000

// Use cdrom Interface
int useCdromInterface	= CDROM_USE_SDL;
int forceCD				= -1;

static unsigned MSCDEX_Strategy_Handler(void); 
static unsigned MSCDEX_Interrupt_Handler(void);

class DOS_DeviceHeader:public MemStruct {
public:
	DOS_DeviceHeader(PhysPt ptr)				{ pt = ptr; };
	
	void	SetNextDeviceHeader	(RealPt ptr)	{ sSave(sDeviceHeader,nextDeviceHeader,ptr);	};
	RealPt	GetNextDeviceHeader	(void)			{ return sGet(sDeviceHeader,nextDeviceHeader);	};
	void	SetAttribute		(System::UInt16 atr)	{ sSave(sDeviceHeader,devAttributes,atr);		};
	void	SetDriveLetter		(System::Byte letter)	{ sSave(sDeviceHeader,driveLetter,letter);		};
	void	SetNumSubUnits		(System::Byte num)		{ sSave(sDeviceHeader,numSubUnits,num);			};
	System::Byte	GetNumSubUnits		(void)			{ return sGet(sDeviceHeader,numSubUnits);		};
	void	SetName				(char const* _name)	{ MEM_BlockWrite(pt+offsetof(sDeviceHeader,name),_name,8); };
	void	SetInterrupt		(System::UInt16 ofs)	{ sSave(sDeviceHeader,interrupt,ofs);			};
	void	SetStrategy			(System::UInt16 ofs)	{ sSave(sDeviceHeader,strategy,ofs);			};

public:
	#ifdef _MSC_VER
	#pragma pack(1)
	#endif
	struct sDeviceHeader{
		RealPt	nextDeviceHeader;
		System::UInt16	devAttributes;
		System::UInt16	strategy;
		System::UInt16	interrupt;
		System::Byte	name[8];
		System::UInt16  wReserved;
		System::Byte	driveLetter;
		System::Byte	numSubUnits;
	} GCC_ATTRIBUTE(packed) TDeviceHeader;
	#ifdef _MSC_VER
	#pragma pack()
	#endif
};

class CMscdex {
public:
	CMscdex		(void);
	~CMscdex	(void);

	System::UInt16		GetVersion			(void)	{ return (MSCDEX_VERSION_HIGH<<8)+MSCDEX_VERSION_LOW; };
	System::UInt16		GetNumDrives		(void)	{ return numDrives;			};
	System::UInt16		GetFirstDrive		(void)	{ return dinfo[0].drive; };
	System::Byte		GetSubUnit			(System::UInt16 _drive);
	bool		GetUPC				(System::Byte subUnit, System::Byte& attr, char* upc);

	void		InitNewMedia		(System::Byte subUnit);
	bool		PlayAudioSector		(System::Byte subUnit, System::UInt32 start, System::UInt32 length);
	bool		PlayAudioMSF		(System::Byte subUnit, System::UInt32 start, System::UInt32 length);
	bool		StopAudio			(System::Byte subUnit);
	bool		GetAudioStatus		(System::Byte subUnit, bool& playing, bool& pause, TMSF& start, TMSF& end);

	bool		GetSubChannelData	(System::Byte subUnit, System::Byte& attr, System::Byte& track, System::Byte &index, TMSF& rel, TMSF& abs);

	int			RemoveDrive			(System::UInt16 _drive);
	int			AddDrive			(System::UInt16 _drive, char* physicalPath, System::Byte& subUnit);
	bool 		HasDrive			(System::UInt16 drive);
	void		ReplaceDrive		(CDROM_Interface* newCdrom, System::Byte subUnit);
	void		GetDrives			(PhysPt data);
	void		GetDriverInfo		(PhysPt data);
	bool		GetVolumeName		(System::Byte subUnit, char* name);
	bool		GetCopyrightName	(System::UInt16 drive, PhysPt data);
	bool		GetAbstractName		(System::UInt16 drive, PhysPt data);
	bool		GetDocumentationName(System::UInt16 drive, PhysPt data);
	bool		GetDirectoryEntry	(System::UInt16 drive, bool copyFlag, PhysPt pathname, PhysPt buffer, System::UInt16& error);
	bool		ReadVTOC			(System::UInt16 drive, System::UInt16 volume, PhysPt data, System::UInt16& error);
	bool		ReadSectors			(System::UInt16 drive, System::UInt32 sector, System::UInt16 num, PhysPt data);
	bool		ReadSectors			(System::Byte subUnit, bool raw, System::UInt32 sector, System::UInt16 num, PhysPt data);
	bool		ReadSectorsMSF		(System::Byte subUnit, bool raw, System::UInt32 sector, System::UInt16 num, PhysPt data);
	bool		SendDriverRequest	(System::UInt16 drive, PhysPt data);
	bool		IsValidDrive		(System::UInt16 drive);
	bool		GetCDInfo			(System::Byte subUnit, System::Byte& tr1, System::Byte& tr2, TMSF& leadOut);
	System::UInt32		GetVolumeSize		(System::Byte subUnit);
	bool		GetTrackInfo		(System::Byte subUnit, System::Byte track, System::Byte& attr, TMSF& start);
	System::UInt16		GetStatusWord		(System::Byte subUnit,System::UInt16 status);
	bool		GetCurrentPos		(System::Byte subUnit, TMSF& pos);
	System::UInt32		GetDeviceStatus		(System::Byte subUnit);
	bool		GetMediaStatus		(System::Byte subUnit, System::Byte& status);
	bool		LoadUnloadMedia		(System::Byte subUnit, bool unload);
	bool		ResumeAudio			(System::Byte subUnit);
	bool		GetMediaStatus		(System::Byte subUnit, bool& media, bool& changed, bool& trayOpen);

private:

	PhysPt		GetDefaultBuffer	(void);
	PhysPt		GetTempBuffer		(void);

	System::UInt16		numDrives;

	typedef struct SDriveInfo {
		System::Byte	drive;			// drive letter in dosbox
		System::Byte	physDrive;		// drive letter in system
		bool	audioPlay;		// audio playing active
		bool	audioPaused;	// audio playing paused
		System::UInt32	audioStart;		// StartLoc for resume
		System::UInt32	audioEnd;		// EndLoc for resume
		bool	locked;			// drive locked ?
		bool	lastResult;		// last operation success ?
		System::UInt32	volumeSize;		// for media change
	} TDriveInfo;

	System::UInt16				defaultBufSeg;
	TDriveInfo			dinfo[MSCDEX_MAX_DRIVES];
	CDROM_Interface*	cdrom[MSCDEX_MAX_DRIVES];
	
public:
	System::UInt16		rootDriverHeaderSeg;
};

CMscdex::CMscdex(void) {
	numDrives			= 0;
	rootDriverHeaderSeg	= 0;
	defaultBufSeg		= 0;

	memset(dinfo,0,sizeof(dinfo));
	for (System::UInt32 i=0; i<MSCDEX_MAX_DRIVES; i++) cdrom[i] = 0;
}

CMscdex::~CMscdex(void) {
	defaultBufSeg = 0;
	for (System::UInt16 i=0; i<GetNumDrives(); i++) {
		delete (cdrom)[i];
		cdrom[i] = 0;
	};
}

void CMscdex::GetDrives(PhysPt data)
{
	for (System::UInt16 i=0; i<GetNumDrives(); i++) mem_writeb(data+i,dinfo[i].drive);
}

bool CMscdex::IsValidDrive(System::UInt16 _drive)
{
	_drive &= 0xff; //Only lowerpart (Ultimate domain)
	for (System::UInt16 i=0; i<GetNumDrives(); i++) if (dinfo[i].drive==_drive) return true;
	return false;
}

System::Byte CMscdex::GetSubUnit(System::UInt16 _drive)
{
	_drive &= 0xff; //Only lowerpart (Ultimate domain)
	for (System::UInt16 i=0; i<GetNumDrives(); i++) if (dinfo[i].drive==_drive) return (System::Byte)i;
	return 0xff;
}

int CMscdex::RemoveDrive(System::UInt16 _drive)
{
	System::UInt16 idx = MSCDEX_MAX_DRIVES;
	for (System::UInt16 i=0; i<GetNumDrives(); i++) {
		if (dinfo[i].drive == _drive) {
			idx = i;
			break;
		}
	}

	if (idx == MSCDEX_MAX_DRIVES || (idx!=0 && idx!=GetNumDrives()-1)) return 0;
	delete (cdrom)[idx];
	if (idx==0) {
		for (System::UInt16 i=0; i<GetNumDrives(); i++) {
			if (i == MSCDEX_MAX_DRIVES-1) {
				cdrom[i] = 0;
				memset(&dinfo[i],0,sizeof(TDriveInfo));
			} else {
				dinfo[i] = dinfo[i+1];
				cdrom[i] = cdrom[i+1];
			}
		}
	} else {
		cdrom[idx] = 0;
		memset(&dinfo[idx],0,sizeof(TDriveInfo));
	}
	numDrives--;

	if (GetNumDrives() == 0) {
		DOS_DeviceHeader devHeader(PhysMake(rootDriverHeaderSeg,0));
		System::UInt16 off = sizeof(DOS_DeviceHeader::sDeviceHeader);
		devHeader.SetStrategy(off+4);		// point to the RETF (To deactivate MSCDEX)
		devHeader.SetInterrupt(off+4);		// point to the RETF (To deactivate MSCDEX)
		devHeader.SetDriveLetter(0);
	} else if (idx==0) {
		DOS_DeviceHeader devHeader(PhysMake(rootDriverHeaderSeg,0));
		devHeader.SetDriveLetter(GetFirstDrive()+1);
	}
	return 1;
}

int CMscdex::AddDrive(System::UInt16 _drive, char* physicalPath, System::Byte& subUnit)
{
	subUnit = 0;
	if ((unsigned)GetNumDrives()+1>=MSCDEX_MAX_DRIVES) return 4;
	if (GetNumDrives()) {
		// Error check, driveletter have to be in a row
		if (dinfo[0].drive-1!=_drive && dinfo[numDrives-1].drive+1!=_drive) 
			return 1;
	}
	// Set return type to ok
	int result = 0;
	// Get Mounttype and init needed cdrom interface
	switch (CDROM_GetMountType(physicalPath,forceCD)) {
	case 0x00: {	
		LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: Mounting physical cdrom: %s"	,physicalPath);
#if defined (WIN32)
		// Check OS
		OSVERSIONINFO osi;
		osi.dwOSVersionInfoSize = sizeof(osi);
		GetVersionEx(&osi);
		if ((osi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (osi.dwMajorVersion>4)) {
			// only WIN NT/200/XP
			if (useCdromInterface==CDROM_USE_IOCTL_DIO) {
				cdrom[numDrives] = new CDROM_Interface_Ioctl(CDROM_Interface_Ioctl::CDIOCTL_CDA_DIO);
				LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: IOCTL Interface.");
				break;
			}
			if (useCdromInterface==CDROM_USE_IOCTL_DX) {
				cdrom[numDrives] = new CDROM_Interface_Ioctl(CDROM_Interface_Ioctl::CDIOCTL_CDA_DX);
				LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: IOCTL Interface (digital audio extraction).");
				break;
			}
			if (useCdromInterface==CDROM_USE_IOCTL_MCI) {
				cdrom[numDrives] = new CDROM_Interface_Ioctl(CDROM_Interface_Ioctl::CDIOCTL_CDA_MCI);
				LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: IOCTL Interface (media control interface).");
				break;
			}
		}
		if (useCdromInterface==CDROM_USE_ASPI) {
			// all Wins - ASPI
			cdrom[numDrives] = new CDROM_Interface_Aspi();
			LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: ASPI Interface.");
			break;
		}
#endif
#if defined (LINUX) || defined(OS2)
		// Always use IOCTL in Linux or OS/2
		cdrom[numDrives] = new CDROM_Interface_Ioctl();
		LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: IOCTL Interface.");
#else
		// Default case windows and other oses
		cdrom[numDrives] = new CDROM_Interface_SDL();
		LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: SDL Interface.");
#endif
		} break;
	case 0x01:	// iso cdrom interface	
		LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: Mounting iso file as cdrom: %s", physicalPath);
		cdrom[numDrives] = new CDROM_Interface_Image((System::Byte)numDrives);
		break;
	case 0x02:	// fake cdrom interface (directories)
		cdrom[numDrives] = new CDROM_Interface_Fake;
		LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: Mounting directory as cdrom: %s",physicalPath);
		LOG(LOG_MISC,LOG_NORMAL)("MSCDEX: You wont have full MSCDEX support !");
		result = 5;
		break;
	default	:	// weird result
		return 6;
	};

	if (!cdrom[numDrives]->SetDevice(physicalPath,forceCD)) {
//		delete cdrom[numDrives] ; mount seems to delete it
		return 3;
	}


	if (rootDriverHeaderSeg==0) {
		
		System::UInt16 driverSize = sizeof(DOS_DeviceHeader::sDeviceHeader) + 10; // 10 = Bytes for 3 callbacks
		
		// Create Device Header
		System::UInt16 seg = DOS_GetMemory(driverSize/16+((driverSize%16)>0));
		DOS_DeviceHeader devHeader(PhysMake(seg,0));
		devHeader.SetNextDeviceHeader	(0xFFFFFFFF);
		devHeader.SetAttribute(0xc800);
		devHeader.SetDriveLetter		(_drive+1);
		devHeader.SetNumSubUnits		(1);
		devHeader.SetName				("MSCD001 ");

		//Link it in the device chain
		System::UInt32 start = dos_infoblock.GetDeviceChain();
		System::UInt16 segm  = (System::UInt16)(start>>16);
		System::UInt16 offm  = (System::UInt16)(start&0xFFFF);
		while(start != 0xFFFFFFFF) {
			segm  = (System::UInt16)(start>>16);
			offm  = (System::UInt16)(start&0xFFFF);
			start = real_readd(segm,offm);
		}
		real_writed(segm,offm,seg<<16);

		// Create Callback Strategy
		System::UInt16 off = sizeof(DOS_DeviceHeader::sDeviceHeader);
		System::UInt16 call_strategy=(System::UInt16)CALLBACK_Allocate();
		CallBack_Handlers[call_strategy]=MSCDEX_Strategy_Handler;
		real_writeb(seg,off+0,(System::Byte)0xFE);		//GRP 4
		real_writeb(seg,off+1,(System::Byte)0x38);		//Extra Callback instruction
		real_writew(seg,off+2,call_strategy);	//The immediate word
		real_writeb(seg,off+4,(System::Byte)0xCB);		//A RETF Instruction
		devHeader.SetStrategy(off);
		
		// Create Callback Interrupt
		off += 5;
		System::UInt16 call_interrupt=(System::UInt16)CALLBACK_Allocate();
		CallBack_Handlers[call_interrupt]=MSCDEX_Interrupt_Handler;
		real_writeb(seg,off+0,(System::Byte)0xFE);		//GRP 4
		real_writeb(seg,off+1,(System::Byte)0x38);		//Extra Callback instruction
		real_writew(seg,off+2,call_interrupt);	//The immediate word
		real_writeb(seg,off+4,(System::Byte)0xCB);		//A RETF Instruction
		devHeader.SetInterrupt(off);
		
		rootDriverHeaderSeg = seg;
	
	} else if (GetNumDrives() == 0) {
		DOS_DeviceHeader devHeader(PhysMake(rootDriverHeaderSeg,0));
		System::UInt16 off = sizeof(DOS_DeviceHeader::sDeviceHeader);
		devHeader.SetDriveLetter(_drive+1);
		devHeader.SetStrategy(off);
		devHeader.SetInterrupt(off+5);
	}

	subUnit = (System::Byte)numDrives;
	// Set drive
	DOS_DeviceHeader devHeader(PhysMake(rootDriverHeaderSeg,0));
	devHeader.SetNumSubUnits(devHeader.GetNumSubUnits()+1);

	if (dinfo[0].drive-1==_drive) {
		CDROM_Interface *_cdrom = cdrom[numDrives];
		for (System::UInt16 i=GetNumDrives(); i>0; i--) {
			dinfo[i] = dinfo[i-1];
			cdrom[i] = cdrom[i-1];
		}
		cdrom[0] = _cdrom;
		dinfo[0].drive		= (System::Byte)_drive;
		dinfo[0].physDrive	= (System::Byte)toupper(physicalPath[0]);
	} else {
		dinfo[numDrives].drive		= (System::Byte)_drive;
		dinfo[numDrives].physDrive	= (System::Byte)toupper(physicalPath[0]);
	}
	numDrives++;
	// stop audio
	StopAudio(subUnit);
	return result;
}

bool CMscdex::HasDrive(System::UInt16 drive) {
	return (GetSubUnit(drive) != 0xff);
}

void CMscdex::ReplaceDrive(CDROM_Interface* newCdrom, System::Byte subUnit) {
	delete cdrom[subUnit];
	cdrom[subUnit] = newCdrom;
	StopAudio(subUnit);
}

PhysPt CMscdex::GetDefaultBuffer(void) {
	if (defaultBufSeg==0) {
		System::UInt16 size = (2352*2+15)/16;
		defaultBufSeg = DOS_GetMemory(size);
	};
	return PhysMake(defaultBufSeg,2352);
}

PhysPt CMscdex::GetTempBuffer(void) {
	if (defaultBufSeg==0) {
		System::UInt16 size = (2352*2+15)/16;
		defaultBufSeg = DOS_GetMemory(size);
	};
	return PhysMake(defaultBufSeg,0);
}

void CMscdex::GetDriverInfo	(PhysPt data) {
	for (System::UInt16 i=0; i<GetNumDrives(); i++) {
		mem_writeb(data  ,(System::Byte)i);	// subunit
		mem_writed(data+1,RealMake(rootDriverHeaderSeg,0));
		data+=5;
	};
}

bool CMscdex::GetCDInfo(System::Byte subUnit, System::Byte& tr1, System::Byte& tr2, TMSF& leadOut)
{
	if (subUnit>=numDrives) return false;
	int tr1i,tr2i;
	// Assume Media change
	cdrom[subUnit]->InitNewMedia();
	dinfo[subUnit].lastResult = cdrom[subUnit]->GetAudioTracks(tr1i,tr2i,leadOut);
	if (!dinfo[subUnit].lastResult) {
		tr1 = tr2 = 0;
		memset(&leadOut,0,sizeof(leadOut));
	} else {
		tr1 = (System::Byte) tr1i;
		tr2 = (System::Byte) tr2i;
	}
	return dinfo[subUnit].lastResult;
}

bool CMscdex::GetTrackInfo(System::Byte subUnit, System::Byte track, System::Byte& attr, TMSF& start)
{
	if (subUnit>=numDrives) return false;
	dinfo[subUnit].lastResult = cdrom[subUnit]->GetAudioTrackInfo(track,start,attr);	
	if (!dinfo[subUnit].lastResult) {
		attr = 0;
		memset(&start,0,sizeof(start));
	};
	return dinfo[subUnit].lastResult;
}

bool CMscdex::PlayAudioSector(System::Byte subUnit, System::UInt32 sector, System::UInt32 length)
{
	if (subUnit>=numDrives) return false;
	// If value from last stop is used, this is meant as a resume
	// better start using resume command
	if (dinfo[subUnit].audioPaused && (sector==dinfo[subUnit].audioStart)) {
		dinfo[subUnit].lastResult = cdrom[subUnit]->PauseAudio(true);
	} else 
		dinfo[subUnit].lastResult = cdrom[subUnit]->PlayAudioSector(sector,length);

	if (dinfo[subUnit].lastResult) {
		dinfo[subUnit].audioPlay	= true;
		dinfo[subUnit].audioPaused	= false;
		dinfo[subUnit].audioStart	= sector;
		dinfo[subUnit].audioEnd		= length;
	};
	return dinfo[subUnit].lastResult;
}

bool CMscdex::PlayAudioMSF(System::Byte subUnit, System::UInt32 start, System::UInt32 length)
{
	if (subUnit>=numDrives) return false;
	System::Byte min		= (System::Byte)(start>>16) & 0xFF;
	System::Byte sec		= (System::Byte)(start>> 8) & 0xFF;
	System::Byte fr		= (System::Byte)(start>> 0) & 0xFF;
	System::UInt32 sector	= min*60*75+sec*75+fr - 150;
	return dinfo[subUnit].lastResult = PlayAudioSector(subUnit,sector,length);
}

bool CMscdex::GetSubChannelData(System::Byte subUnit, System::Byte& attr, System::Byte& track, System::Byte &index, TMSF& rel, TMSF& abs)
{
	if (subUnit>=numDrives) return false;
	dinfo[subUnit].lastResult = cdrom[subUnit]->GetAudioSub(attr,track,index,rel,abs);
	if (!dinfo[subUnit].lastResult) {
		attr = track = index = 0;
		memset(&rel,0,sizeof(rel));
		memset(&abs,0,sizeof(abs));
	};
	return dinfo[subUnit].lastResult;
}

bool CMscdex::GetAudioStatus(System::Byte subUnit, bool& playing, bool& pause, TMSF& start, TMSF& end)
{
	if (subUnit>=numDrives) return false;
	dinfo[subUnit].lastResult = cdrom[subUnit]->GetAudioStatus(playing,pause);
	if (dinfo[subUnit].lastResult) {
		// Start
		System::UInt32 addr	= dinfo[subUnit].audioStart + 150;
		start.fr	= (System::Byte)(addr%75);	addr/=75;
		start.sec	= (System::Byte)(addr%60); 
		start.min	= (System::Byte)(addr/60);
		// End
		addr		= dinfo[subUnit].audioEnd + 150;
		end.fr		= (System::Byte)(addr%75);	addr/=75;
		end.sec		= (System::Byte)(addr%60); 
		end.min		= (System::Byte)(addr/60);
	} else {
		playing		= false;
		pause		= false;
		memset(&start,0,sizeof(start));
		memset(&end,0,sizeof(end));
	};
	
	return dinfo[subUnit].lastResult;
}

bool CMscdex::StopAudio(System::Byte subUnit)
{
	if (subUnit>=numDrives) return false;
	if (dinfo[subUnit].audioPlay)	dinfo[subUnit].lastResult = cdrom[subUnit]->PauseAudio(false);
	else							dinfo[subUnit].lastResult = cdrom[subUnit]->StopAudio();
	
	if (dinfo[subUnit].lastResult) {
		if (dinfo[subUnit].audioPlay) {
			TMSF pos;
			GetCurrentPos(subUnit,pos);
			dinfo[subUnit].audioStart	= pos.min*60*75+pos.sec*75+pos.fr - 150;
			dinfo[subUnit].audioPaused  = true;
		} else {	
			dinfo[subUnit].audioPaused  = false;
			dinfo[subUnit].audioStart	= 0;
			dinfo[subUnit].audioEnd		= 0;
		};
		dinfo[subUnit].audioPlay = false;
	};
	return dinfo[subUnit].lastResult;
}

bool CMscdex::ResumeAudio(System::Byte subUnit) {
	if (subUnit>=numDrives) return false;
	return dinfo[subUnit].lastResult = PlayAudioSector(subUnit,dinfo[subUnit].audioStart,dinfo[subUnit].audioEnd);
}

System::UInt32 CMscdex::GetVolumeSize(System::Byte subUnit) {
	if (subUnit>=numDrives) return false;
	System::Byte tr1,tr2;
	TMSF leadOut;
	dinfo[subUnit].lastResult = GetCDInfo(subUnit,tr1,tr2,leadOut);
	if (dinfo[subUnit].lastResult) return (leadOut.min*60*75)+(leadOut.sec*75)+leadOut.fr;
	return 0;
}

bool CMscdex::ReadVTOC(System::UInt16 drive, System::UInt16 volume, PhysPt data, System::UInt16& error) { 
	System::Byte subunit = GetSubUnit(drive);
/*	if (subunit>=numDrives) {
		error=MSCDEX_ERROR_UNKNOWN_DRIVE;
		return false;
	} */
	if (!ReadSectors(subunit,false,16+volume,1,data)) {
		error=MSCDEX_ERROR_DRIVE_NOT_READY;
		return false;
	}
	char id[5];
	MEM_BlockRead(data + 1, id, 5);
	if (strncmp("CD001",id, 5)!=0) {
		error = MSCDEX_ERROR_BAD_FORMAT;
		return false;
	}
	System::Byte type = mem_readb(data);
	error = (type == 1) ? 1 : (type == 0xFF) ? 0xFF : 0;
	return true;
}

bool CMscdex::GetVolumeName(System::Byte subUnit, char* data) {	
	if (subUnit>=numDrives) return false;
	System::UInt16 drive = dinfo[subUnit].drive;

	System::UInt16 error;
	bool success = false;
	PhysPt ptoc = GetTempBuffer();
	success = ReadVTOC(drive,0x00,ptoc,error);
	if (success) {
		MEM_StrCopy(ptoc+40,data,31);
		data[31] = 0;
		rtrim(data);
	};

	return success; 
}

bool CMscdex::GetCopyrightName(System::UInt16 drive, PhysPt data) {	
	System::UInt16 error;
	bool success = false;
	PhysPt ptoc = GetTempBuffer();
	success = ReadVTOC(drive,0x00,ptoc,error);
	if (success) {
		MEM_BlockCopy(data,ptoc+702,37);
		mem_writeb(data+37,0);
	};
	return success; 
}

bool CMscdex::GetAbstractName(System::UInt16 drive, PhysPt data) { 
	System::UInt16 error;
	bool success = false;
	PhysPt ptoc = GetTempBuffer();
	success = ReadVTOC(drive,0x00,ptoc,error);
	if (success) {
		MEM_BlockCopy(data,ptoc+739,37);
		mem_writeb(data+37,0);
	};
	return success; 
}

bool CMscdex::GetDocumentationName(System::UInt16 drive, PhysPt data) { 
	System::UInt16 error;
	bool success = false;
	PhysPt ptoc = GetTempBuffer();
	success = ReadVTOC(drive,0x00,ptoc,error);
	if (success) {
		MEM_BlockCopy(data,ptoc+776,37);
		mem_writeb(data+37,0);
	};
	return success; 
}

bool CMscdex::GetUPC(System::Byte subUnit, System::Byte& attr, char* upc)
{
	if (subUnit>=numDrives) return false;
	return dinfo[subUnit].lastResult = cdrom[subUnit]->GetUPC(attr,&upc[0]);
}

bool CMscdex::ReadSectors(System::Byte subUnit, bool raw, System::UInt32 sector, System::UInt16 num, PhysPt data) {
	if (subUnit>=numDrives) return false;
	if ((4*num*2048+5) < CPU_Cycles) CPU_Cycles -= 4*num*2048;
	else CPU_Cycles = 5;
	dinfo[subUnit].lastResult = cdrom[subUnit]->ReadSectors(data,raw,sector,num);
	return dinfo[subUnit].lastResult;
}

bool CMscdex::ReadSectorsMSF(System::Byte subUnit, bool raw, System::UInt32 start, System::UInt16 num, PhysPt data) {
	if (subUnit>=numDrives) return false;
	System::Byte min		= (System::Byte)(start>>16) & 0xFF;
	System::Byte sec		= (System::Byte)(start>> 8) & 0xFF;
	System::Byte fr		= (System::Byte)(start>> 0) & 0xFF;
	System::UInt32 sector	= min*60*75+sec*75+fr - 150;
	return ReadSectors(subUnit,raw,sector,num,data);
}

// Called from INT 2F
bool CMscdex::ReadSectors(System::UInt16 drive, System::UInt32 sector, System::UInt16 num, PhysPt data) {
	return ReadSectors(GetSubUnit(drive),false,sector,num,data);
}

bool CMscdex::GetDirectoryEntry(System::UInt16 drive, bool copyFlag, PhysPt pathname, PhysPt buffer, System::UInt16& error)
{
	char	volumeID[6] = {0};
	char	searchName[256];
	char	entryName[256];
	bool	foundComplete = false;
	bool	foundName;
	char*	useName = 0;
	unsigned	entryLength,nameLength;
	// clear error
	error = 0;
	MEM_StrCopy(pathname+1,searchName,mem_readb(pathname));
	upcase(searchName);
	char* searchPos = searchName;

	//strip of tailing . (XCOM APOCALYPSE)
	size_t searchlen = strlen(searchName);
	if (searchlen > 1 && strcmp(searchName,".."))
		if (searchName[searchlen-1] =='.')  searchName[searchlen-1] = 0;

	//LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Get DirEntry : Find : %s",searchName);
	// read vtoc
	PhysPt defBuffer = GetDefaultBuffer();
	if (!ReadSectors(GetSubUnit(drive),false,16,1,defBuffer)) return false;
	// TODO: has to be iso 9960
	MEM_StrCopy(defBuffer+1,volumeID,5); volumeID[5] = 0;
	bool iso = (strcmp("CD001",volumeID)==0);
	if (!iso) E_Exit("MSCDEX: GetDirEntry: Not an ISO 9960 CD.");
	// get directory position
	unsigned dirEntrySector	= mem_readd(defBuffer+156+2);
	int dirSize		= mem_readd(defBuffer+156+10);
	unsigned index;
	while (dirSize>0) {
		index = 0;
		if (!ReadSectors(GetSubUnit(drive),false,dirEntrySector,1,defBuffer)) return false;
		// Get string part
		foundName	= false;
		if (searchPos) { 
			useName = searchPos; 
			searchPos = strchr(searchPos,'\\'); 
		}

	   	if (searchPos) { *searchPos = 0; searchPos++; }
		else foundComplete = true;

		do {
			entryLength = mem_readb(defBuffer+index);
			if (entryLength==0) break;
			nameLength  = mem_readb(defBuffer+index+32);
			MEM_StrCopy(defBuffer+index+33,entryName,nameLength);
			if (strcmp(entryName,useName)==0) {
				//LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Get DirEntry : Found : %s",useName);
				foundName = true;
				break;
			}
			/* Xcom Apocalipse searches for MUSIC. and expects to find MUSIC;1
			 * All Files on the CDROM are of the kind blah;1
			 */
			char* longername = strchr(entryName,';');
			if(longername) {
				*longername = 0;
				if (strcmp(entryName,useName)==0) {
					//LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Get DirEntry : Found : %s",useName);
					foundName = true;
					break;
				}
			}
			index += entryLength;
		} while (index+33<=2048);
		
		if (foundName) {
			if (foundComplete) {
				if (copyFlag) {
					LOG(LOG_MISC,LOG_WARN)("MSCDEX: GetDirEntry: Copyflag structure not entirely accurate maybe");
					System::Byte readBuf[256];
					System::Byte writeBuf[256];
					if (entryLength > 256)
						return false;
					MEM_BlockRead( defBuffer+index, readBuf, entryLength );
					writeBuf[0] = readBuf[1];						// 00h	BYTE	length of XAR in Logical Block Numbers
					memcpy( &writeBuf[1], &readBuf[0x2], 4);		// 01h	DWORD	Logical Block Number of file start
					writeBuf[5] = 0;writeBuf[6] = 8;				// 05h	WORD	size of disk in logical blocks
					memcpy( &writeBuf[7], &readBuf[0xa], 4);		// 07h	DWORD	file length in bytes
					memcpy( &writeBuf[0xb], &readBuf[0x12], 7);		// 0bh	DWORD	date and time
					writeBuf[0x12] = readBuf[0x19];					// 12h	BYTE	bit flags
					writeBuf[0x13] = readBuf[0x1a];					// 13h	BYTE	interleave size
					writeBuf[0x14] = readBuf[0x1b];					// 14h	BYTE	interleave skip factor
					memcpy( &writeBuf[0x15], &readBuf[0x1c], 2);	// 15h	WORD	volume set sequence number
					writeBuf[0x17] = readBuf[0x20];
					memcpy( &writeBuf[0x18], &readBuf[21], readBuf[0x20] <= 38 ? readBuf[0x20] : 38 );
					MEM_BlockWrite( buffer, writeBuf, 0x18 + 40 );
				} else {
					// Direct copy
					MEM_BlockCopy(buffer,defBuffer+index,entryLength);
				}
				error = iso ? 1:0;
				return true;
			}
			// change directory
			dirEntrySector = mem_readd(defBuffer+index+2);
			dirSize	= mem_readd(defBuffer+index+10);
		} else {
			// continue search in next sector
			dirSize -= 2048;
			dirEntrySector++;
		}
	};
	error = 2; // file not found
	return false; // not found
}

bool CMscdex::GetCurrentPos(System::Byte subUnit, TMSF& pos)
{
	if (subUnit>=numDrives) return false;
	TMSF rel;
	System::Byte attr,track,index;
	dinfo[subUnit].lastResult = GetSubChannelData(subUnit, attr, track, index, rel, pos);
	if (!dinfo[subUnit].lastResult) memset(&pos,0,sizeof(pos));
	return dinfo[subUnit].lastResult;
}

bool CMscdex::GetMediaStatus(System::Byte subUnit, bool& media, bool& changed, bool& trayOpen)
{
	if (subUnit>=numDrives) return false;
	dinfo[subUnit].lastResult = cdrom[subUnit]->GetMediaTrayStatus(media,changed,trayOpen);
	return dinfo[subUnit].lastResult;
}

System::UInt32 CMscdex::GetDeviceStatus(System::Byte subUnit)
{
	if (subUnit>=numDrives) return false;
	bool media,changed,trayOpen;

	dinfo[subUnit].lastResult = GetMediaStatus(subUnit,media,changed,trayOpen);
	System::UInt32 status = (trayOpen << 0)					|	// Drive is open ?
					(dinfo[subUnit].locked	<< 1)	|	// Drive is locked ?
					(1<<2)							|	// raw + cooked sectors
					(1<<4)							|	// Can read sudio
					(1<<9)							|	// Red book & HSG
					((!media) << 11);					// Drive is empty ?
	return status;
}

bool CMscdex::GetMediaStatus(System::Byte subUnit, System::Byte& status)
{
	if (subUnit>=numDrives) return false;
/*	bool media,changed,open,result;
	result = GetMediaStatus(subUnit,media,changed,open);
	status = changed ? 0xFF : 0x01;
	return result; */
	status = getSwapRequest() ? 0xFF : 0x01;
	return true;
}

bool CMscdex::LoadUnloadMedia(System::Byte subUnit, bool unload)
{
	if (subUnit>=numDrives) return false;
	dinfo[subUnit].lastResult = cdrom[subUnit]->LoadUnloadMedia(unload);
	return dinfo[subUnit].lastResult;
}

bool CMscdex::SendDriverRequest(System::UInt16 drive, PhysPt data)
{
	System::Byte subUnit = GetSubUnit(drive);
	if (subUnit>=numDrives) return false;
	// Get SubUnit
	mem_writeb(data+1,subUnit);
	// Call Strategy / Interrupt
	MSCDEX_Strategy_Handler();
	MSCDEX_Interrupt_Handler();
	return true;
}

System::UInt16 CMscdex::GetStatusWord(System::Byte subUnit,System::UInt16 status)
{
	if (subUnit>=numDrives) return REQUEST_STATUS_ERROR | 0x02; // error : Drive not ready

	if (dinfo[subUnit].lastResult)	status |= REQUEST_STATUS_DONE;				// ok
	else							status |= REQUEST_STATUS_ERROR; 

	if (dinfo[subUnit].audioPlay) {
		// Check if audio is still playing....
		TMSF start,end;
		bool playing,pause;
		if (GetAudioStatus(subUnit,playing,pause,start,end)) {
			dinfo[subUnit].audioPlay = playing;
		} else
			dinfo[subUnit].audioPlay = false;

		status |= (dinfo[subUnit].audioPlay<<9);
	} 
	dinfo[subUnit].lastResult	= true;
	return status;
}

void CMscdex::InitNewMedia(System::Byte subUnit) {
	if (subUnit<numDrives) {
		// Reopen new media
		cdrom[subUnit]->InitNewMedia();
	}
}

static CMscdex* mscdex = 0;
static PhysPt curReqheaderPtr = 0;

static System::UInt16 MSCDEX_IOCTL_Input(PhysPt buffer,System::Byte drive_unit) {
	unsigned ioctl_fct = mem_readb(buffer);
	MSCDEX_LOG("MSCDEX: IOCTL INPUT Subfunction %02X",ioctl_fct);
	switch (ioctl_fct) {
		case 0x00 : /* Get Device Header address */
					mem_writed(buffer+1,RealMake(mscdex->rootDriverHeaderSeg,0));
					break;
		case 0x01 :{/* Get current position */
					TMSF pos;
					mscdex->GetCurrentPos(drive_unit,pos);
					System::Byte addr_mode = mem_readb(buffer+1);
					if (addr_mode==0) {			// HSG
						System::UInt32 frames=MSF_TO_FRAMES(pos.min, pos.sec, pos.fr);
						if (frames<150) MSCDEX_LOG("MSCDEX: Get position: invalid position %d:%d:%d", pos.min, pos.sec, pos.fr);
						else frames-=150;
						mem_writed(buffer+2,frames);
					} else if (addr_mode==1) {	// Red book
						mem_writeb(buffer+2,pos.fr);
						mem_writeb(buffer+3,pos.sec);
						mem_writeb(buffer+4,pos.min);
						mem_writeb(buffer+5,0x00);
					} else {
						MSCDEX_LOG("MSCDEX: Get position: invalid address mode %x",addr_mode);
						return 0x03;		// invalid function
					}
				   }break;
		case 0x06 : /* Get Device status */
					mem_writed(buffer+1,mscdex->GetDeviceStatus(drive_unit)); 
					break;
		case 0x07 : /* Get sector size */
					if (mem_readb(buffer+1)==0) mem_writed(buffer+2,2048);
					else if (mem_readb(buffer+1)==1) mem_writed(buffer+2,2352);
					else return 0x03;		// invalid function
					break;
		case 0x08 : /* Get size of current volume */
					mem_writed(buffer+1,mscdex->GetVolumeSize(drive_unit));
					break;
		case 0x09 : /* Media change ? */
					System::Byte status;
					if (!mscdex->GetMediaStatus(drive_unit,status)) {
						status = 0;		// state unknown
					}
					mem_writeb(buffer+1,status);
					break;
		case 0x0A : /* Get Audio Disk info */	
					System::Byte tr1,tr2; TMSF leadOut;
					if (!mscdex->GetCDInfo(drive_unit,tr1,tr2,leadOut)) return 0x05;
					mem_writeb(buffer+1,tr1);
					mem_writeb(buffer+2,tr2);
					mem_writeb(buffer+3,leadOut.fr);
					mem_writeb(buffer+4,leadOut.sec);
					mem_writeb(buffer+5,leadOut.min);
					mem_writeb(buffer+6,0x00);
					break;
		case 0x0B :{/* Audio Track Info */
					System::Byte attr; TMSF start;
					System::Byte track = mem_readb(buffer+1);
					mscdex->GetTrackInfo(drive_unit,track,attr,start);		
					mem_writeb(buffer+2,start.fr);
					mem_writeb(buffer+3,start.sec);
					mem_writeb(buffer+4,start.min);
					mem_writeb(buffer+5,0x00);
					mem_writeb(buffer+6,attr);
					break; };
		case 0x0C :{/* Get Audio Sub Channel data */
					System::Byte attr,track,index; 
					TMSF abs,rel;
					mscdex->GetSubChannelData(drive_unit,attr,track,index,rel,abs);
					mem_writeb(buffer+1,attr);
					mem_writeb(buffer+2,track);
					mem_writeb(buffer+3,index);
					mem_writeb(buffer+4,rel.min);
					mem_writeb(buffer+5,rel.sec);
					mem_writeb(buffer+6,rel.fr);
					mem_writeb(buffer+7,0x00);
					mem_writeb(buffer+8,abs.min);
					mem_writeb(buffer+9,abs.sec);
					mem_writeb(buffer+10,abs.fr);
					break;
				   };
		case 0x0E :{ /* Get UPC */	
					System::Byte attr; char upc[8];
					mscdex->GetUPC(drive_unit,attr,&upc[0]);
					mem_writeb(buffer+1,attr);
					for (int i=0; i<7; i++) mem_writeb(buffer+2+i,upc[i]);
					mem_writeb(buffer+9,0x00);
					break;
				   };
		case 0x0F :{ /* Get Audio Status */	
					bool playing,pause;
					TMSF resStart,resEnd;
					mscdex->GetAudioStatus(drive_unit,playing,pause,resStart,resEnd);
					mem_writeb(buffer+1,pause);
					mem_writeb(buffer+3,resStart.min);
					mem_writeb(buffer+4,resStart.sec);
					mem_writeb(buffer+5,resStart.fr);
					mem_writeb(buffer+6,0x00);
					mem_writeb(buffer+7,resEnd.min);
					mem_writeb(buffer+8,resEnd.sec);
					mem_writeb(buffer+9,resEnd.fr);
					mem_writeb(buffer+10,0x00);
					break;
				   };
		default :	LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Unsupported IOCTL INPUT Subfunction %02X",ioctl_fct);
					return 0x03;	// invalid function
	}
	return 0x00;	// success
}

static System::UInt16 MSCDEX_IOCTL_Optput(PhysPt buffer,System::Byte drive_unit) {
	unsigned ioctl_fct = mem_readb(buffer);
//	MSCDEX_LOG("MSCDEX: IOCTL OUTPUT Subfunction %02X",ioctl_fct);
	switch (ioctl_fct) {
		case 0x00 :	// Unload /eject media
					if (!mscdex->LoadUnloadMedia(drive_unit,true)) return 0x02;
					break;
		case 0x03: //Audio Channel control
					MSCDEX_LOG("MSCDEX: Audio Channel Control used. Not handled. Faking succes!");
		case 0x01 : // (un)Lock door 
					// do nothing -> report as success
					break;
		case 0x02 : // Reset Drive
					LOG(LOG_MISC,LOG_WARN)("cdromDrive reset");
					if (!mscdex->StopAudio(drive_unit))  return 0x02;
					break;
		case 0x05 :	// load media
					if (!mscdex->LoadUnloadMedia(drive_unit,false)) return 0x02;
		default	:	LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Unsupported IOCTL OUTPUT Subfunction %02X",ioctl_fct);
					return 0x03;	// invalid function
	}
	return 0x00;	// success
}

static unsigned MSCDEX_Strategy_Handler(void) {
	curReqheaderPtr = PhysMake(SegValue(es),reg_bx);
//	MSCDEX_LOG("MSCDEX: Device Strategy Routine called, request header at %x",curReqheaderPtr);
	return CBRET_NONE;
}

static unsigned MSCDEX_Interrupt_Handler(void) {
	if (curReqheaderPtr==0) {
		MSCDEX_LOG("MSCDEX: invalid call to interrupt handler");						
		return CBRET_NONE;
	}
	System::Byte	subUnit		= mem_readb(curReqheaderPtr+1);
	System::Byte	funcNr		= mem_readb(curReqheaderPtr+2);
	System::UInt16	errcode		= 0;
	PhysPt	buffer		= 0;

	MSCDEX_LOG("MSCDEX: Driver Function %02X",funcNr);

	if ((funcNr==0x03) || (funcNr==0x0c) || (funcNr==0x80) || (funcNr==0x82)) {
		buffer = PhysMake(mem_readw(curReqheaderPtr+0x10),mem_readw(curReqheaderPtr+0x0E));
	}

 	switch (funcNr) {
		case 0x03	: {	/* IOCTL INPUT */
						System::UInt16 error=MSCDEX_IOCTL_Input(buffer,subUnit);
						if (error) errcode = error;
						break;
					  };
		case 0x0C	: {	/* IOCTL OUTPUT */
						System::UInt16 error=MSCDEX_IOCTL_Optput(buffer,subUnit);
						if (error) errcode = error;
						break;
					  };
		case 0x0D	:	// device open
		case 0x0E	:	// device close - dont care :)
						break;
		case 0x80	:	// Read long
		case 0x82	: { // Read long prefetch -> both the same here :)
						System::UInt32 start = mem_readd(curReqheaderPtr+0x14);
						System::UInt16 len	 = mem_readw(curReqheaderPtr+0x12);
						bool raw	 = (mem_readb(curReqheaderPtr+0x18)==1);
						if (mem_readb(curReqheaderPtr+0x0D)==0x00) // HSG
							mscdex->ReadSectors(subUnit,raw,start,len,buffer);
						else 
							mscdex->ReadSectorsMSF(subUnit,raw,start,len,buffer);
						break;
					  };
		case 0x83	:	// Seek - dont care :)
						break;
		case 0x84	: {	/* Play Audio Sectors */
						System::UInt32 start = mem_readd(curReqheaderPtr+0x0E);
						System::UInt32 len	 = mem_readd(curReqheaderPtr+0x12);
						if (mem_readb(curReqheaderPtr+0x0D)==0x00) // HSG
							mscdex->PlayAudioSector(subUnit,start,len);
						else // RED BOOK
							mscdex->PlayAudioMSF(subUnit,start,len);
						break;
					  };
		case 0x85	:	/* Stop Audio */
						mscdex->StopAudio(subUnit);
						break;
		case 0x88	:	/* Resume Audio */
						mscdex->ResumeAudio(subUnit);
						break;
		default		:	LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Unsupported Driver Request %02X",funcNr);
						break;
	
	};
	
	// Set Statusword
	mem_writew(curReqheaderPtr+3,mscdex->GetStatusWord(subUnit,errcode));
	MSCDEX_LOG("MSCDEX: Status : %04X",mem_readw(curReqheaderPtr+3));						
	return CBRET_NONE;
}

static bool MSCDEX_Handler(void) {
	if(reg_ah == 0x11) {
		if(reg_al == 0x00) { 
			PhysPt check = PhysMake(SegValue(ss),reg_sp);
			if(mem_readw(check+6) == 0xDADA) {
				//MSCDEX sets word on stack to ADAD if it DADA on entry.
				mem_writew(check+6,0xADAD);
			}
			reg_al = 0xff;
			return true;
		} else {
			LOG(LOG_MISC,LOG_ERROR)("NETWORK REDIRECTOR USED!!!");
			reg_ax = 0x49;//NETWERK SOFTWARE NOT INSTALLED
			CALLBACK_SCF(true);
			return true;
		}
	}

	if (reg_ah!=0x15) return false;		// not handled here, continue chain

	PhysPt data = PhysMake(SegValue(es),reg_bx);
	MSCDEX_LOG("MSCDEX: INT 2F %04X BX= %04X CX=%04X",reg_ax,reg_bx,reg_cx);
	switch (reg_ax) {
	
		case 0x1500:	/* Install check */
						reg_bx = mscdex->GetNumDrives();
						if (reg_bx>0) reg_cx = mscdex->GetFirstDrive();
						reg_al = 0xff;
						return true;
		case 0x1501:	/* Get cdrom driver info */
						mscdex->GetDriverInfo(data);
						return true;
		case 0x1502:	/* Get Copyright filename */
						if (mscdex->GetCopyrightName(reg_cx,data)) {
							CALLBACK_SCF(false);
						} else {
							reg_ax = MSCDEX_ERROR_UNKNOWN_DRIVE;
							CALLBACK_SCF(true);							
						};
						return true;		
		case 0x1503:	/* Get Abstract filename */
						if (mscdex->GetAbstractName(reg_cx,data)) {
							CALLBACK_SCF(false);
						} else {
							reg_ax = MSCDEX_ERROR_UNKNOWN_DRIVE;
							CALLBACK_SCF(true);							
						};
						return true;		
		case 0x1504:	/* Get Documentation filename */
						if (mscdex->GetDocumentationName(reg_cx,data)) {
							CALLBACK_SCF(false);
						} else {
							reg_ax = MSCDEX_ERROR_UNKNOWN_DRIVE;
							CALLBACK_SCF(true);							
						};
						return true;		
		case 0x1505: {	// read vtoc 
						System::UInt16 error = 0;
						if (mscdex->ReadVTOC(reg_cx,reg_dx,data,error)) {
//							reg_ax = error;	// return code
							CALLBACK_SCF(false);
						} else {
							reg_ax = error;
							CALLBACK_SCF(true);							
						};
					 };
						return true;
		case 0x1508: {	// read sectors 
						System::UInt32 sector = (reg_si<<16)+reg_di;
						if (mscdex->ReadSectors(reg_cx,sector,reg_dx,data)) {
							reg_ax = 0;
							CALLBACK_SCF(false);
						} else {
							// possibly: MSCDEX_ERROR_DRIVE_NOT_READY if sector is beyond total length
							reg_ax = MSCDEX_ERROR_UNKNOWN_DRIVE;
							CALLBACK_SCF(true);
						};
						return true;
					 };
		case 0x1509:	// write sectors - not supported 
						reg_ax = MSCDEX_ERROR_INVALID_FUNCTION;
						CALLBACK_SCF(true);
						return true;
		case 0x150B:	/* Valid CDROM drive ? */
						reg_ax = (mscdex->IsValidDrive(reg_cx) ? 0x5ad8 : 0x0000);
						reg_bx = 0xADAD;
						return true;
		case 0x150C:	/* Get MSCDEX Version */
						reg_bx = mscdex->GetVersion();
						return true;
		case 0x150D:	/* Get drives */
						mscdex->GetDrives(data);
						return true;
		case 0x150E:	/* Get/Set Volume Descriptor Preference */
						if (mscdex->IsValidDrive(reg_cx)) {
							if (reg_bx == 0) {
								// get preference
								reg_dx = 0x100;	// preference?
								CALLBACK_SCF(false);
							} else if (reg_bx == 1) {
								// set preference
								if (reg_dh == 1) {
									// valid
									CALLBACK_SCF(false);
								} else {
									reg_ax = MSCDEX_ERROR_INVALID_FUNCTION;
									CALLBACK_SCF(true);
								}
							} else {
								reg_ax = MSCDEX_ERROR_INVALID_FUNCTION;
								CALLBACK_SCF(true);
							}
						} else {
							reg_ax = MSCDEX_ERROR_UNKNOWN_DRIVE;
							CALLBACK_SCF(true);
						}
						return true;
		case 0x150F: {	// Get directory entry
						System::UInt16 error;
						bool success = mscdex->GetDirectoryEntry(reg_cl,reg_ch&1,data,PhysMake(reg_si,reg_di),error);
						reg_ax = error;
						CALLBACK_SCF(!success);
					 };	return true;
		case 0x1510:	/* Device driver request */
						if (mscdex->SendDriverRequest(reg_cx,data)) {
							CALLBACK_SCF(false);
						} else {
							reg_ax = MSCDEX_ERROR_UNKNOWN_DRIVE;
							CALLBACK_SCF(true);
						}
						return true;
	};
	LOG(LOG_MISC,LOG_ERROR)("MSCDEX: Unknwon call : %04X",reg_ax);
	return true;
}

class device_MSCDEX : public DOS_Device {
public:
	device_MSCDEX() { SetName("MSCD001"); }
	bool Read (System::Byte * /*data*/,System::UInt16 * /*size*/) { return false;}
	bool Write(System::Byte * /*data*/,System::UInt16 * /*size*/) { 
		LOG(LOG_ALL,LOG_NORMAL)("Write to mscdex device");	
		return false;
	}
	bool Seek(System::UInt32 * /*pos*/,System::UInt32 /*type*/){return false;}
	bool Close(){return false;}
	System::UInt16 GetInformation(void){return 0xc880;}
	bool ReadFromControlChannel(PhysPt bufptr,System::UInt16 size,System::UInt16 * retcode);
	bool WriteToControlChannel(PhysPt bufptr,System::UInt16 size,System::UInt16 * retcode);
private:
//	System::Byte cache;
};

bool device_MSCDEX::ReadFromControlChannel(PhysPt bufptr,System::UInt16 size,System::UInt16 * retcode) { 
	if (MSCDEX_IOCTL_Input(bufptr,0)==0) {
		*retcode=size;
		return true;
	}
	return false;
}

bool device_MSCDEX::WriteToControlChannel(PhysPt bufptr,System::UInt16 size,System::UInt16 * retcode) { 
	if (MSCDEX_IOCTL_Optput(bufptr,0)==0) {
		*retcode=size;
		return true;
	}
	return false;
}

int MSCDEX_AddDrive(char driveLetter, const char* physicalPath, System::Byte& subUnit)
{
	int result = mscdex->AddDrive(driveLetter-'A',(char*)physicalPath,subUnit);
	return result;
}

int MSCDEX_RemoveDrive(char driveLetter)
{
	if(!mscdex) return 0;
	return mscdex->RemoveDrive(driveLetter-'A');
}

bool MSCDEX_HasDrive(char driveLetter)
{
	return mscdex->HasDrive(driveLetter-'A');
}

void MSCDEX_ReplaceDrive(CDROM_Interface* cdrom, System::Byte subUnit)
{
	mscdex->ReplaceDrive(cdrom, subUnit);
}

bool MSCDEX_GetVolumeName(System::Byte subUnit, char* name)
{
	return mscdex->GetVolumeName(subUnit,name);
}

bool MSCDEX_HasMediaChanged(System::Byte subUnit)
{
	static TMSF leadOut[MSCDEX_MAX_DRIVES];

	TMSF leadnew;
	System::Byte tr1,tr2;
	if (mscdex->GetCDInfo(subUnit,tr1,tr2,leadnew)) {
		bool changed = (leadOut[subUnit].min!=leadnew.min) || (leadOut[subUnit].sec!=leadnew.sec) || (leadOut[subUnit].fr!=leadnew.fr);
		if (changed) {
			leadOut[subUnit].min = leadnew.min;
			leadOut[subUnit].sec = leadnew.sec;
			leadOut[subUnit].fr	 = leadnew.fr;
			mscdex->InitNewMedia(subUnit);
		}
		return changed;
	};
	if (subUnit<MSCDEX_MAX_DRIVES) {
		leadOut[subUnit].min = 0;
		leadOut[subUnit].sec = 0;
		leadOut[subUnit].fr	 = 0;
	}
	return true;
}

void MSCDEX_SetCDInterface(int intNr, int numCD) {
	useCdromInterface = intNr;
	forceCD	= numCD;
}

void MSCDEX_ShutDown(Section* /*sec*/) {
	delete mscdex;
	mscdex = 0;
	curReqheaderPtr = 0;
}

void MSCDEX_Init(Section* sec) {
	// AddDestroy func
	sec->AddDestroyFunction(&MSCDEX_ShutDown);
	/* Register the mscdex device */
	DOS_Device * newdev = new device_MSCDEX();
	DOS_AddDevice(newdev);
	curReqheaderPtr = 0;
	/* Add Multiplexer */
	DOS_AddMultiplexHandler(MSCDEX_Handler);
	/* Create MSCDEX */
	mscdex = new CMscdex;
}
