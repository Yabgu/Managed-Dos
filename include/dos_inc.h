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

/* $Id: dos_inc.h,v 1.83 2009-10-28 21:45:12 qbix79 Exp $ */

#ifndef DOSBOX_DOS_INC_H
#define DOSBOX_DOS_INC_H

#ifndef DOSBOX_DOS_SYSTEM_H
#include "dos_system.h"
#endif
#ifndef DOSBOX_MEM_H
#include "mem.h"
#endif

#ifdef _MSC_VER
#pragma pack (1)
#endif
struct CommandTail{
  System::Byte count;				/* number of bytes returned */
  char buffer[127];			 /* the buffer itself */
} GCC_ATTRIBUTE(packed);
#ifdef _MSC_VER
#pragma pack ()
#endif

struct DOS_Date {
	System::UInt16 year;
	System::Byte month;
	System::Byte day;
};

struct DOS_Version {
	System::Byte major,minor,revision;
};


#ifdef _MSC_VER
#pragma pack (1)
#endif
union bootSector {
	struct entries {
		System::Byte jump[3];
		System::Byte oem_name[8];
		System::UInt16 bytesect;
		System::Byte sectclust;
		System::UInt16 reserve_sect;
		System::Byte misc[496];
	} bootdata;
	System::Byte rawdata[512];
} GCC_ATTRIBUTE(packed);
#ifdef _MSC_VER
#pragma pack ()
#endif


enum { MCB_FREE=0x0000,MCB_DOS=0x0008 };
enum { RETURN_EXIT=0,RETURN_CTRLC=1,RETURN_ABORT=2,RETURN_TSR=3};

#define DOS_FILES 127
#define DOS_DRIVES 26
#define DOS_DEVICES 10


// dos swappable area is 0x320 bytes beyond the sysvars table
// device driver chain is inside sysvars
#define DOS_INFOBLOCK_SEG 0x80	// sysvars (list of lists)
#define DOS_CONDRV_SEG 0xa0
#define DOS_CONSTRING_SEG 0xa8
#define DOS_SDA_SEG 0xb2		// dos swappable area
#define DOS_SDA_OFS 0
#define DOS_CDS_SEG 0x108
#define DOS_FIRST_SHELL 0x118
#define DOS_MEM_START 0x16f		//First Segment that DOS can use

#define DOS_PRIVATE_SEGMENT 0xc800
#define DOS_PRIVATE_SEGMENT_END 0xd000

/* internal Dos Tables */

extern DOS_File * Files[DOS_FILES];
extern DOS_Drive * Drives[DOS_DRIVES];
extern DOS_Device * Devices[DOS_DEVICES];

extern System::Byte dos_copybuf[0x10000];


void DOS_SetError(System::UInt16 code);

/* File Handling Routines */

enum { STDIN=0,STDOUT=1,STDERR=2,STDAUX=3,STDPRN=4};
enum { HAND_NONE=0,HAND_FILE,HAND_DEVICE};

/* Routines for File Class */
void DOS_SetupFiles (void);
bool DOS_ReadFile(System::UInt16 handle,System::Byte * data,System::UInt16 * amount);
bool DOS_WriteFile(System::UInt16 handle,System::Byte * data,System::UInt16 * amount);
bool DOS_SeekFile(System::UInt16 handle,System::UInt32 * pos,System::UInt32 type);
bool DOS_CloseFile(System::UInt16 handle);
bool DOS_FlushFile(System::UInt16 handle);
bool DOS_DuplicateEntry(System::UInt16 entry,System::UInt16 * newentry);
bool DOS_ForceDuplicateEntry(System::UInt16 entry,System::UInt16 newentry);
bool DOS_GetFileDate(System::UInt16 entry, System::UInt16* otime, System::UInt16* odate);

/* Routines for Drive Class */
bool DOS_OpenFile(char const * name,System::Byte flags,System::UInt16 * entry);
bool DOS_OpenFileExtended(char const * name, System::UInt16 flags, System::UInt16 createAttr, System::UInt16 action, System::UInt16 *entry, System::UInt16* status);
bool DOS_CreateFile(char const * name,System::UInt16 attribute,System::UInt16 * entry);
bool DOS_UnlinkFile(char const * const name);
bool DOS_FindFirst(char *search,System::UInt16 attr,bool fcb_findfirst=false);
bool DOS_FindNext(void);
bool DOS_Canonicalize(char const * const name,char * const big);
bool DOS_CreateTempFile(char * const name,System::UInt16 * entry);
bool DOS_FileExists(char const * const name);

/* Helper Functions */
bool DOS_MakeName(char const * const name,char * const fullname,System::Byte * drive);
/* Drive Handing Routines */
System::Byte DOS_GetDefaultDrive(void);
void DOS_SetDefaultDrive(System::Byte drive);
bool DOS_SetDrive(System::Byte drive);
bool DOS_GetCurrentDir(System::Byte drive,char * const buffer);
bool DOS_ChangeDir(char const * const dir);
bool DOS_MakeDir(char const * const dir);
bool DOS_RemoveDir(char const * const dir);
bool DOS_Rename(char const * const oldname,char const * const newname);
bool DOS_GetFreeDiskSpace(System::Byte drive,System::UInt16 * bytes,System::Byte * sectors,System::UInt16 * clusters,System::UInt16 * free);
bool DOS_GetFileAttr(char const * const name,System::UInt16 * attr);
bool DOS_SetFileAttr(char const * const name,System::UInt16 attr);

/* IOCTL Stuff */
bool DOS_IOCTL(void);
bool DOS_GetSTDINStatus();
System::Byte DOS_FindDevice(char const * name);
void DOS_SetupDevices(void);

/* Execute and new process creation */
bool DOS_NewPSP(System::UInt16 pspseg,System::UInt16 size);
bool DOS_ChildPSP(System::UInt16 pspseg,System::UInt16 size);
bool DOS_Execute(char * name,PhysPt block,System::Byte flags);
void DOS_Terminate(System::UInt16 pspseg,bool tsr,System::Byte exitcode);

/* Memory Handling Routines */
void DOS_SetupMemory(void);
bool DOS_AllocateMemory(System::UInt16 * segment,System::UInt16 * blocks);
bool DOS_ResizeMemory(System::UInt16 segment,System::UInt16 * blocks);
bool DOS_FreeMemory(System::UInt16 segment);
void DOS_FreeProcessMemory(System::UInt16 pspseg);
System::UInt16 DOS_GetMemory(System::UInt16 pages);
bool DOS_SetMemAllocStrategy(System::UInt16 strat);
System::UInt16 DOS_GetMemAllocStrategy(void);
void DOS_BuildUMBChain(bool umb_active,bool ems_active);
bool DOS_LinkUMBsToMemChain(System::UInt16 linkstate);

/* FCB stuff */
bool DOS_FCBOpen(System::UInt16 seg,System::UInt16 offset);
bool DOS_FCBCreate(System::UInt16 seg,System::UInt16 offset);
bool DOS_FCBClose(System::UInt16 seg,System::UInt16 offset);
bool DOS_FCBFindFirst(System::UInt16 seg,System::UInt16 offset);
bool DOS_FCBFindNext(System::UInt16 seg,System::UInt16 offset);
System::Byte DOS_FCBRead(System::UInt16 seg,System::UInt16 offset, System::UInt16 numBlocks);
System::Byte DOS_FCBWrite(System::UInt16 seg,System::UInt16 offset,System::UInt16 numBlocks);
System::Byte DOS_FCBRandomRead(System::UInt16 seg,System::UInt16 offset,System::UInt16 numRec,bool restore);
System::Byte DOS_FCBRandomWrite(System::UInt16 seg,System::UInt16 offset,System::UInt16 numRec,bool restore);
bool DOS_FCBGetFileSize(System::UInt16 seg,System::UInt16 offset);
bool DOS_FCBDeleteFile(System::UInt16 seg,System::UInt16 offset);
bool DOS_FCBRenameFile(System::UInt16 seg, System::UInt16 offset);
void DOS_FCBSetRandomRecord(System::UInt16 seg, System::UInt16 offset);
System::Byte FCB_Parsename(System::UInt16 seg,System::UInt16 offset,System::Byte parser ,char *string, System::Byte *change);
bool DOS_GetAllocationInfo(System::Byte drive,System::UInt16 * _bytes_sector,System::Byte * _sectors_cluster,System::UInt16 * _total_clusters);

/* Extra DOS Interrupts */
void DOS_SetupMisc(void);

/* The DOS Tables */
void DOS_SetupTables(void);

/* Internal DOS Setup Programs */
void DOS_SetupPrograms(void);

/* Initialize Keyboard Layout */
void DOS_KeyboardLayout_Init(Section* sec);

bool DOS_LayoutKey(unsigned key, System::Byte flags1, System::Byte flags2, System::Byte flags3);

enum {
	KEYB_NOERROR=0,
	KEYB_FILENOTFOUND,
	KEYB_INVALIDFILE,
	KEYB_LAYOUTNOTFOUND,
	KEYB_INVALIDCPFILE
};


static INLINE System::UInt16 long2para(System::UInt32 size) {
	if (size>0xFFFF0) return 0xffff;
	if (size&0xf) return (System::UInt16)((size>>4)+1);
	else return (System::UInt16)(size>>4);
}


static INLINE System::UInt16 DOS_PackTime(System::UInt16 hour,System::UInt16 min,System::UInt16 sec) {
	return (hour&0x1f)<<11 | (min&0x3f) << 5 | ((sec/2)&0x1f);
}

static INLINE System::UInt16 DOS_PackDate(System::UInt16 year,System::UInt16 mon,System::UInt16 day) {
	return ((year-1980)&0x7f)<<9 | (mon&0x3f) << 5 | (day&0x1f);
}

/* Dos Error Codes */
#define DOSERR_NONE 0
#define DOSERR_FUNCTION_NUMBER_INVALID 1
#define DOSERR_FILE_NOT_FOUND 2
#define DOSERR_PATH_NOT_FOUND 3
#define DOSERR_TOO_MANY_OPEN_FILES 4
#define DOSERR_ACCESS_DENIED 5
#define DOSERR_INVALID_HANDLE 6
#define DOSERR_MCB_DESTROYED 7
#define DOSERR_INSUFFICIENT_MEMORY 8
#define DOSERR_MB_ADDRESS_INVALID 9
#define DOSERR_ENVIRONMENT_INVALID 10
#define DOSERR_FORMAT_INVALID 11
#define DOSERR_ACCESS_CODE_INVALID 12
#define DOSERR_DATA_INVALID 13
#define DOSERR_RESERVED 14
#define DOSERR_FIXUP_OVERFLOW 14
#define DOSERR_INVALID_DRIVE 15
#define DOSERR_REMOVE_CURRENT_DIRECTORY 16
#define DOSERR_NOT_SAME_DEVICE 17
#define DOSERR_NO_MORE_FILES 18
#define DOSERR_FILE_ALREADY_EXISTS 80


/* Remains some classes used to access certain things */
#define sOffset(s,m) ((char*)&(((s*)NULL)->m)-(char*)NULL)
#define sGet(s,m) GetIt(sizeof(((s *)&pt)->m),(PhysPt)sOffset(s,m))
#define sSave(s,m,val) SaveIt(sizeof(((s *)&pt)->m),(PhysPt)sOffset(s,m),val)

class MemStruct {
public:
	unsigned GetIt(unsigned size,PhysPt addr) {
		switch (size) {
		case 1:return mem_readb(pt+addr);
		case 2:return mem_readw(pt+addr);
		case 4:return mem_readd(pt+addr);
		}
		return 0;
	}
	void SaveIt(unsigned size,PhysPt addr,unsigned val) {
		switch (size) {
		case 1:mem_writeb(pt+addr,(System::Byte)val);break;
		case 2:mem_writew(pt+addr,(System::UInt16)val);break;
		case 4:mem_writed(pt+addr,(System::UInt32)val);break;
		}
	}
	void SetPt(System::UInt16 seg) { pt=PhysMake(seg,0);}
	void SetPt(System::UInt16 seg,System::UInt16 off) { pt=PhysMake(seg,off);}
	void SetPt(RealPt addr) { pt=Real2Phys(addr);}
protected:
	PhysPt pt;
};

class DOS_PSP :public MemStruct {
public:
	DOS_PSP						(System::UInt16 segment)		{ SetPt(segment);seg=segment;};
	void	MakeNew				(System::UInt16 memSize);
	void	CopyFileTable		(DOS_PSP* srcpsp,bool createchildpsp);
	System::UInt16	FindFreeFileEntry	(void);
	void	CloseFiles			(void);

	void	SaveVectors			(void);
	void	RestoreVectors		(void);
	void	SetSize				(System::UInt16 size)			{ sSave(sPSP,next_seg,size);		};
	System::UInt16	GetSize				(void)					{ return (System::UInt16)sGet(sPSP,next_seg);		};
	void	SetEnvironment		(System::UInt16 envseg)			{ sSave(sPSP,environment,envseg);	};
	System::UInt16	GetEnvironment		(void)					{ return (System::UInt16)sGet(sPSP,environment);	};
	System::UInt16	GetSegment			(void)					{ return seg;						};
	void	SetFileHandle		(System::UInt16 index, System::Byte handle);
	System::Byte	GetFileHandle		(System::UInt16 index);
	void	SetParent			(System::UInt16 parent)			{ sSave(sPSP,psp_parent,parent);	};
	System::UInt16	GetParent			(void)					{ return (System::UInt16)sGet(sPSP,psp_parent);		};
	void	SetStack			(RealPt stackpt)		{ sSave(sPSP,stack,stackpt);		};
	RealPt	GetStack			(void)					{ return sGet(sPSP,stack);			};
	void	SetInt22			(RealPt int22pt)		{ sSave(sPSP,int_22,int22pt);		};
	RealPt	GetInt22			(void)					{ return sGet(sPSP,int_22);			};
	void	SetFCB1				(RealPt src);
	void	SetFCB2				(RealPt src);
	void	SetCommandTail		(RealPt src);	
	bool	SetNumFiles			(System::UInt16 fileNum);
	System::UInt16	FindEntryByHandle	(System::Byte handle);
			
private:
	#ifdef _MSC_VER
	#pragma pack(1)
	#endif
	struct sPSP {
		System::Byte	exit[2];			/* CP/M-like exit poimt */
		System::UInt16	next_seg;			/* Segment of first byte beyond memory allocated or program */
		System::Byte	fill_1;				/* single char fill */
		System::Byte	far_call;			/* far call opcode */
		RealPt	cpm_entry;			/* CPM Service Request address*/
		RealPt	int_22;				/* Terminate Address */
		RealPt	int_23;				/* Break Address */
		RealPt	int_24;				/* Critical Error Address */
		System::UInt16	psp_parent;			/* Parent PSP Segment */
		System::Byte	files[20];			/* File Table - 0xff is unused */
		System::UInt16	environment;		/* Segment of evironment table */
		RealPt	stack;				/* SS:SP Save point for int 0x21 calls */
		System::UInt16	max_files;			/* Maximum open files */
		RealPt	file_table;			/* Pointer to File Table PSP:0x18 */
		RealPt	prev_psp;			/* Pointer to previous PSP */
		System::Byte interim_flag;
		System::Byte truename_flag;
		System::UInt16 nn_flags;
		System::UInt16 dos_version;
		System::Byte	fill_2[14];			/* Lot's of unused stuff i can't care aboue */
		System::Byte	service[3];			/* INT 0x21 Service call int 0x21;retf; */
		System::Byte	fill_3[9];			/* This has some blocks with FCB info */
		System::Byte	fcb1[16];			/* first FCB */
		System::Byte	fcb2[16];			/* second FCB */
		System::Byte	fill_4[4];			/* unused */
		CommandTail cmdtail;		
	} GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack()
	#endif
	System::UInt16	seg;
public:
	static	System::UInt16 rootpsp;
};

class DOS_ParamBlock:public MemStruct {
public:
	DOS_ParamBlock(PhysPt addr) {pt=addr;}
	void Clear(void);
	void LoadData(void);
	void SaveData(void);		/* Save it as an exec block */
	#ifdef _MSC_VER
	#pragma pack (1)
	#endif
	struct sOverlay {
		System::UInt16 loadseg;
		System::UInt16 relocation;
	} GCC_ATTRIBUTE(packed);
	struct sExec {
		System::UInt16 envseg;
		RealPt cmdtail;
		RealPt fcb1;
		RealPt fcb2;
		RealPt initsssp;
		RealPt initcsip;
	}GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack()
	#endif
	sExec exec;
	sOverlay overlay;
};

class DOS_InfoBlock:public MemStruct {
public:
	DOS_InfoBlock			() {};
	void SetLocation(System::UInt16  seg);
	void SetFirstMCB(System::UInt16 _first_mcb);
	void SetBuffers(System::UInt16 x,System::UInt16 y);
	void SetCurDirStruct(System::UInt32 _curdirstruct);
	void SetFCBTable(System::UInt32 _fcbtable);
	void SetDeviceChainStart(System::UInt32 _devchain);
	void SetDiskBufferHeadPt(System::UInt32 _dbheadpt);
	void SetStartOfUMBChain(System::UInt16 _umbstartseg);
	void SetUMBChainState(System::Byte _umbchaining);
	System::UInt16	GetStartOfUMBChain(void);
	System::Byte	GetUMBChainState(void);
	RealPt	GetPointer(void);
	System::UInt32 GetDeviceChain(void);

	#ifdef _MSC_VER
	#pragma pack(1)
	#endif
	struct sDIB {		
		System::Byte	unknown1[4];
		System::UInt16	magicWord;			// -0x22 needs to be 1
		System::Byte	unknown2[8];
		System::UInt16	regCXfrom5e;		// -0x18 CX from last int21/ah=5e
		System::UInt16	countLRUcache;		// -0x16 LRU counter for FCB caching
		System::UInt16	countLRUopens;		// -0x14 LRU counter for FCB openings
		System::Byte	stuff[6];		// -0x12 some stuff, hopefully never used....
		System::UInt16	sharingCount;		// -0x0c sharing retry count
		System::UInt16	sharingDelay;		// -0x0a sharing retry delay
		RealPt	diskBufPtr;		// -0x08 pointer to disk buffer
		System::UInt16	ptrCONinput;		// -0x04 pointer to con input
		System::UInt16	firstMCB;		// -0x02 first memory control block
		RealPt	firstDPB;		//  0x00 first drive parameter block
		RealPt	firstFileTable;		//  0x04 first system file table
		RealPt	activeClock;		//  0x08 active clock device header
		RealPt	activeCon;		//  0x0c active console device header
		System::UInt16	maxSectorLength;	//  0x10 maximum bytes per sector of any block device;
		RealPt	diskInfoBuffer;		//  0x12 pointer to disk info buffer
		RealPt  curDirStructure;	//  0x16 pointer to current array of directory structure
		RealPt	fcbTable;		//  0x1a pointer to system FCB table
		System::UInt16	protFCBs;		//  0x1e protected fcbs
		System::Byte	blockDevices;		//  0x20 installed block devices
		System::Byte	lastdrive;		//  0x21 lastdrive
		System::UInt32	nulNextDriver;	//  0x22 NUL driver next pointer
		System::UInt16	nulAttributes;	//  0x26 NUL driver aattributes
		System::UInt32	nulStrategy;	//  0x28 NUL driver strategy routine
		System::Byte	nulString[8];	//  0x2c NUL driver name string
		System::Byte	joindedDrives;		//  0x34 joined drives
		System::UInt16	specialCodeSeg;		//  0x35 special code segment
		RealPt  setverPtr;		//  0x37 pointer to setver
		System::UInt16  a20FixOfs;		//  0x3b a20 fix routine offset
		System::UInt16  pspLastIfHMA;		//  0x3d psp of last program (if dos in hma)
		System::UInt16	buffers_x;		//  0x3f x in BUFFERS x,y
		System::UInt16	buffers_y;		//  0x41 y in BUFFERS x,y
		System::Byte	bootDrive;		//  0x43 boot drive
		System::Byte	useDwordMov;		//  0x44 use dword moves
		System::UInt16	extendedSize;		//  0x45 size of extended memory
		System::UInt32	diskBufferHeadPt;	//  0x47 pointer to least-recently used buffer header
		System::UInt16	dirtyDiskBuffers;	//  0x4b number of dirty disk buffers
		System::UInt32	lookaheadBufPt;		//  0x4d pointer to lookahead buffer
		System::UInt16	lookaheadBufNumber;		//  0x51 number of lookahead buffers
		System::Byte	bufferLocation;			//  0x53 workspace buffer location
		System::UInt32	workspaceBuffer;		//  0x54 pointer to workspace buffer
		System::Byte	unknown3[11];			//  0x58
		System::Byte	chainingUMB;			//  0x63 bit0: UMB chain linked to MCB chain
		System::UInt16	minMemForExec;			//  0x64 minimum paragraphs needed for current program
		System::UInt16	startOfUMBChain;		//  0x66 segment of first UMB-MCB
		System::UInt16	memAllocScanStart;		//  0x68 start paragraph for memory allocation
	} GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack ()
	#endif
	System::UInt16	seg;
};

class DOS_DTA:public MemStruct{
public:
	DOS_DTA(RealPt addr) { SetPt(addr); }

	void SetupSearch(System::Byte _sdrive,System::Byte _sattr,char * _pattern);
	void SetResult(const char * _name,System::UInt32 _size,System::UInt16 _date,System::UInt16 _time,System::Byte _attr);
	
	System::Byte GetSearchDrive(void);
	void GetSearchParams(System::Byte & _sattr,char * _spattern);
	void GetResult(char * _name,System::UInt32 & _size,System::UInt16 & _date,System::UInt16 & _time,System::Byte & _attr);

	void	SetDirID(System::UInt16 entry)			{ sSave(sDTA,dirID,entry); };
	void	SetDirIDCluster(System::UInt16 entry)	{ sSave(sDTA,dirCluster,entry); };
	System::UInt16	GetDirID(void)				{ return (System::UInt16)sGet(sDTA,dirID); };
	System::UInt16	GetDirIDCluster(void)		{ return (System::UInt16)sGet(sDTA,dirCluster); };
private:
	#ifdef _MSC_VER
	#pragma pack(1)
	#endif
	struct sDTA {
		System::Byte sdrive;						/* The Drive the search is taking place */
		System::Byte sname[8];						/* The Search pattern for the filename */		
		System::Byte sext[3];						/* The Search pattern for the extenstion */
		System::Byte sattr;						/* The Attributes that need to be found */
		System::UInt16 dirID;						/* custom: dir-search ID for multiple searches at the same time */
		System::UInt16 dirCluster;					/* custom (drive_fat only): cluster number for multiple searches at the same time */
		System::Byte fill[4];
		System::Byte attr;
		System::UInt16 time;
		System::UInt16 date;
		System::UInt32 size;
		char name[DOS_NAMELENGTH_ASCII];
	} GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack()
	#endif
};

class DOS_FCB: public MemStruct {
public:
	DOS_FCB(System::UInt16 seg,System::UInt16 off,bool allow_extended=true);
	void Create(bool _extended);
	void SetName(System::Byte _drive,char * _fname,char * _ext);
	void SetSizeDateTime(System::UInt32 _size,System::UInt16 _date,System::UInt16 _time);
	void GetSizeDateTime(System::UInt32 & _size,System::UInt16 & _date,System::UInt16 & _time);
	void GetName(char * fillname);
	void FileOpen(System::Byte _fhandle);
	void FileClose(System::Byte & _fhandle);
	void GetRecord(System::UInt16 & _cur_block,System::Byte & _cur_rec);
	void SetRecord(System::UInt16 _cur_block,System::Byte _cur_rec);
	void GetSeqData(System::Byte & _fhandle,System::UInt16 & _rec_size);
	void GetRandom(System::UInt32 & _random);
	void SetRandom(System::UInt32  _random);
	System::Byte GetDrive(void);
	bool Extended(void);
	void GetAttr(System::Byte & attr);
	void SetAttr(System::Byte attr);
	bool Valid(void);
private:
	bool extended;
	PhysPt real_pt;
	#ifdef _MSC_VER
	#pragma pack (1)
	#endif
	struct sFCB {
		System::Byte drive;			/* Drive number 0=default, 1=A, etc */
		System::Byte filename[8];		/* Space padded name */
		System::Byte ext[3];			/* Space padded extension */
		System::UInt16 cur_block;		/* Current Block */
		System::UInt16 rec_size;		/* Logical record size */
		System::UInt32 filesize;		/* File Size */
		System::UInt16 date;
		System::UInt16 time;
		/* Reserved Block should be 8 bytes */
		System::Byte sft_entries;
		System::Byte share_attributes;
		System::Byte extra_info;
		System::Byte file_handle;
		System::Byte reserved[4];
		/* end */
		System::Byte  cur_rec;			/* Current record in current block */
		System::UInt32 rndm;			/* Current relative record number */
	} GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack ()
	#endif
};

class DOS_MCB : public MemStruct{
public:
	DOS_MCB(System::UInt16 seg) { SetPt(seg); }
	void SetFileName(char const * const _name) { MEM_BlockWrite(pt+offsetof(sMCB,filename),_name,8); }
	void GetFileName(char * const _name) { MEM_BlockRead(pt+offsetof(sMCB,filename),_name,8);_name[8]=0;}
	void SetType(System::Byte _type) { sSave(sMCB,type,_type);}
	void SetSize(System::UInt16 _size) { sSave(sMCB,size,_size);}
	void SetPSPSeg(System::UInt16 _pspseg) { sSave(sMCB,psp_segment,_pspseg);}
	System::Byte GetType(void) { return (System::Byte)sGet(sMCB,type);}
	System::UInt16 GetSize(void) { return (System::UInt16)sGet(sMCB,size);}
	System::UInt16 GetPSPSeg(void) { return (System::UInt16)sGet(sMCB,psp_segment);}
private:
	#ifdef _MSC_VER
	#pragma pack (1)
	#endif
	struct sMCB {
		System::Byte type;
		System::UInt16 psp_segment;
		System::UInt16 size;	
		System::Byte unused[3];
		System::Byte filename[8];
	} GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack ()
	#endif
};

class DOS_SDA : public MemStruct {
public:
	DOS_SDA(System::UInt16 _seg,System::UInt16 _offs) { SetPt(_seg,_offs); }
	void Init();   
	void SetDrive(System::Byte _drive) { sSave(sSDA,current_drive, _drive); }
	void SetDTA(System::UInt32 _dta) { sSave(sSDA,current_dta, _dta); }
	void SetPSP(System::UInt16 _psp) { sSave(sSDA,current_psp, _psp); }
	System::Byte GetDrive(void) { return (System::Byte)sGet(sSDA,current_drive); }
	System::UInt16 GetPSP(void) { return (System::UInt16)sGet(sSDA,current_psp); }
	System::UInt32 GetDTA(void) { return (System::UInt32)sGet(sSDA,current_dta); }
	
	
private:
	#ifdef _MSC_VER
	#pragma pack (1)
	#endif
	struct sSDA {
		System::Byte crit_error_flag;		/* 0x00 Critical Error Flag */
		System::Byte inDOS_flag;		/* 0x01 InDOS flag (count of active INT 21 calls) */
		System::Byte drive_crit_error;		/* 0x02 Drive on which current critical error occurred or FFh */
		System::Byte locus_of_last_error;	/* 0x03 locus of last error */
		System::UInt16 extended_error_code;	/* 0x04 extended error code of last error */
		System::Byte suggested_action;		/* 0x06 suggested action for last error */
		System::Byte error_class;		/* 0x07 class of last error*/
		System::UInt32 last_error_pointer; 	/* 0x08 ES:DI pointer for last error */
		System::UInt32 current_dta;		/* 0x0C current DTA (Disk Transfer Address) */
		System::UInt16 current_psp; 		/* 0x10 current PSP */
		System::UInt16 sp_int_23;		/* 0x12 stores SP across an INT 23 */
		System::UInt16 return_code;		/* 0x14 return code from last process termination (zerod after reading with AH=4Dh) */
		System::Byte current_drive;		/* 0x16 current drive */
		System::Byte extended_break_flag; 	/* 0x17 extended break flag */
		System::Byte fill[2];			/* 0x18 flag: code page switching || flag: copy of previous byte in case of INT 24 Abort*/
	} GCC_ATTRIBUTE(packed);
	#ifdef _MSC_VER
	#pragma pack()
	#endif
};
extern DOS_InfoBlock dos_infoblock;

struct DOS_Block {
	DOS_Date date;
	DOS_Version version;
	System::UInt16 firstMCB;
	System::UInt16 errorcode;
	System::UInt16 psp(){return DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).GetPSP();};
	void psp(System::UInt16 _seg){ DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).SetPSP(_seg);};
	System::UInt16 env;
	RealPt cpmentry;
	RealPt dta(){return DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).GetDTA();};
	void dta(RealPt _dta){DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).SetDTA(_dta);};
	System::Byte return_code,return_mode;
	
	System::Byte current_drive;
	bool verify;
	bool breakcheck;
	bool echo;          // if set to true dev_con::read will echo input 
	struct  {
		RealPt mediaid;
		RealPt tempdta;
		RealPt tempdta_fcbdelete;
		RealPt dbcs;
		RealPt filenamechar;
		RealPt collatingseq;
		RealPt upcase;
		System::Byte* country;//Will be copied to dos memory. resides in real mem
		System::UInt16 dpb; //Fake Disk parameter system using only the first entry so the drive letter matches
	} tables;
	System::UInt16 loaded_codepage;
};

extern DOS_Block dos;

static System::Byte RealHandle(System::UInt16 handle) {
	DOS_PSP psp(dos.psp());	
	return psp.GetFileHandle(handle);
}

#endif
