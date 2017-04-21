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

/* $Id: drives.h,v 1.41 2009-05-27 09:15:41 qbix79 Exp $ */

#ifndef _DRIVES_H__
#define _DRIVES_H__

#include <vector>
#include <sys/types.h>
#include "dos_system.h"
#include "shell.h" /* for DOS_Shell */
#include "bios_disk.h"  /* for fatDrive */

bool WildFileCmp(const char * file, const char * wild);
void Set_Label(char const * const input, char * const output, bool cdrom);

class DriveManager {
public:
	static void AppendDisk(int drive, DOS_Drive* disk);
	static void InitializeDrive(int drive);
	static int UnmountDrive(int drive);
//	static void CycleDrive(bool pressed);
//	static void CycleDisk(bool pressed);
	static void CycleAllDisks(void);
	static void Init(Section* sec);
	
private:
	static struct DriveInfo {
		std::vector<DOS_Drive*> disks;
		System::UInt32 currentDisk;
	} driveInfos[DOS_DRIVES];
	
	static int currentDrive;
};

class localDrive : public DOS_Drive {
public:
	localDrive(const char * startdir,System::UInt16 _bytes_sector,System::Byte _sectors_cluster,System::UInt16 _total_clusters,System::UInt16 _free_clusters,System::Byte _mediaid);
	virtual bool FileOpen(DOS_File * * file,char * name,System::UInt32 flags);
	virtual FILE *GetSystemFilePtr(char const * const name, char const * const type);
	virtual bool GetSystemFilename(char* sysName, char const * const dosName);
	virtual bool FileCreate(DOS_File * * file,char * name,System::UInt16 attributes);
	virtual bool FileUnlink(char * name);
	virtual bool RemoveDir(char * dir);
	virtual bool MakeDir(char * dir);
	virtual bool TestDir(char * dir);
	virtual bool FindFirst(char * _dir,DOS_DTA & dta,bool fcb_findfirst=false);
	virtual bool FindNext(DOS_DTA & dta);
	virtual bool GetFileAttr(char * name,System::UInt16 * attr);
	virtual bool Rename(char * oldname,char * newname);
	virtual bool AllocationInfo(System::UInt16 * _bytes_sector,System::Byte * _sectors_cluster,System::UInt16 * _total_clusters,System::UInt16 * _free_clusters);
	virtual bool FileExists(const char* name);
	virtual bool FileStat(const char* name, FileStat_Block * const stat_block);
	virtual System::Byte GetMediaByte(void);
	virtual bool isRemote(void);
	virtual bool isRemovable(void);
	virtual int UnMount(void);
private:
	char basedir[CROSS_LEN];
	friend void DOS_Shell::CMD_SUBST(char* args); 	
	struct {
		char srch_dir[CROSS_LEN];
	} srchInfo[MAX_OPENDIRS];

	struct {
		System::UInt16 bytes_sector;
		System::Byte sectors_cluster;
		System::UInt16 total_clusters;
		System::UInt16 free_clusters;
		System::Byte mediaid;
	} allocation;
};

#ifdef _MSC_VER
#pragma pack (1)
#endif
struct bootstrap {
	System::Byte  nearjmp[3];
	System::Byte  oemname[8];
	System::UInt16 bytespersector;
	System::Byte  sectorspercluster;
	System::UInt16 reservedsectors;
	System::Byte  fatcopies;
	System::UInt16 rootdirentries;
	System::UInt16 totalsectorcount;
	System::Byte  mediadescriptor;
	System::UInt16 sectorsperfat;
	System::UInt16 sectorspertrack;
	System::UInt16 headcount;
	/* 32-bit FAT extensions */
	System::UInt32 hiddensectorcount;
	System::UInt32 totalsecdword;
	System::Byte  bootcode[474];
	System::Byte  magic1; /* 0x55 */
	System::Byte  magic2; /* 0xaa */
} GCC_ATTRIBUTE(packed);

struct direntry {
	System::Byte entryname[11];
	System::Byte attrib;
	System::Byte NTRes;
	System::Byte milliSecondStamp;
	System::UInt16 crtTime;
	System::UInt16 crtDate;
	System::UInt16 accessDate;
	System::UInt16 hiFirstClust;
	System::UInt16 modTime;
	System::UInt16 modDate;
	System::UInt16 loFirstClust;
	System::UInt32 entrysize;
} GCC_ATTRIBUTE(packed);

struct partTable {
	System::Byte booter[446];
	struct {
		System::Byte bootflag;
		System::Byte beginchs[3];
		System::Byte parttype;
		System::Byte endchs[3];
		System::UInt32 absSectStart;
		System::UInt32 partSize;
	} pentry[4];
	System::Byte  magic1; /* 0x55 */
	System::Byte  magic2; /* 0xaa */
} GCC_ATTRIBUTE(packed);

#ifdef _MSC_VER
#pragma pack ()
#endif

class fatDrive : public DOS_Drive {
public:
	fatDrive(const char * sysFilename, System::UInt32 bytesector, System::UInt32 cylsector, System::UInt32 headscyl, System::UInt32 cylinders, System::UInt32 startSector);
	virtual bool FileOpen(DOS_File * * file,char * name,System::UInt32 flags);
	virtual bool FileCreate(DOS_File * * file,char * name,System::UInt16 attributes);
	virtual bool FileUnlink(char * name);
	virtual bool RemoveDir(char * dir);
	virtual bool MakeDir(char * dir);
	virtual bool TestDir(char * dir);
	virtual bool FindFirst(char * _dir,DOS_DTA & dta,bool fcb_findfirst=false);
	virtual bool FindNext(DOS_DTA & dta);
	virtual bool GetFileAttr(char * name,System::UInt16 * attr);
	virtual bool Rename(char * oldname,char * newname);
	virtual bool AllocationInfo(System::UInt16 * _bytes_sector,System::Byte * _sectors_cluster,System::UInt16 * _total_clusters,System::UInt16 * _free_clusters);
	virtual bool FileExists(const char* name);
	virtual bool FileStat(const char* name, FileStat_Block * const stat_block);
	virtual System::Byte GetMediaByte(void);
	virtual bool isRemote(void);
	virtual bool isRemovable(void);
	virtual int UnMount(void);
public:
	System::UInt32 getAbsoluteSectFromBytePos(System::UInt32 startClustNum, System::UInt32 bytePos);
	System::UInt32 getSectorSize(void);
	System::UInt32 getAbsoluteSectFromChain(System::UInt32 startClustNum, System::UInt32 logicalSector);
	bool allocateCluster(System::UInt32 useCluster, System::UInt32 prevCluster);
	System::UInt32 appendCluster(System::UInt32 startCluster);
	void deleteClustChain(System::UInt32 startCluster);
	System::UInt32 getFirstFreeClust(void);
	bool directoryBrowse(System::UInt32 dirClustNumber, direntry *useEntry, System::Int32 entNum);
	bool directoryChange(System::UInt32 dirClustNumber, direntry *useEntry, System::Int32 entNum);
	imageDisk *loadedDisk;
	bool created_successfully;
private:
	System::UInt32 getClusterValue(System::UInt32 clustNum);
	void setClusterValue(System::UInt32 clustNum, System::UInt32 clustValue);
	System::UInt32 getClustFirstSect(System::UInt32 clustNum);
	bool FindNextInternal(System::UInt32 dirClustNumber, DOS_DTA & dta, direntry *foundEntry);
	bool getDirClustNum(char * dir, System::UInt32 * clustNum, bool parDir);
	bool getFileDirEntry(char const * const filename, direntry * useEntry, System::UInt32 * dirClust, System::UInt32 * subEntry);
	bool addDirectoryEntry(System::UInt32 dirClustNumber, direntry useEntry);
	void zeroOutCluster(System::UInt32 clustNumber);
	bool getEntryName(char *fullname, char *entname);
	friend void DOS_Shell::CMD_SUBST(char* args); 	
	struct {
		char srch_dir[CROSS_LEN];
	} srchInfo[MAX_OPENDIRS];

	struct {
		System::UInt16 bytes_sector;
		System::Byte sectors_cluster;
		System::UInt16 total_clusters;
		System::UInt16 free_clusters;
		System::Byte mediaid;
	} allocation;
	
	bootstrap bootbuffer;
	System::Byte fattype;
	System::UInt32 CountOfClusters;
	System::UInt32 partSectOff;
	System::UInt32 firstDataSector;
	System::UInt32 firstRootDirSect;

	System::UInt32 cwdDirCluster;
	System::UInt32 dirPosition; /* Position in directory search */
};


class cdromDrive : public localDrive
{
public:
	cdromDrive(const char driveLetter, const char * startdir,System::UInt16 _bytes_sector,System::Byte _sectors_cluster,System::UInt16 _total_clusters,System::UInt16 _free_clusters,System::Byte _mediaid, int& error);
	virtual bool FileOpen(DOS_File * * file,char * name,System::UInt32 flags);
	virtual bool FileCreate(DOS_File * * file,char * name,System::UInt16 attributes);
	virtual bool FileUnlink(char * name);
	virtual bool RemoveDir(char * dir);
	virtual bool MakeDir(char * dir);
	virtual bool Rename(char * oldname,char * newname);
	virtual bool GetFileAttr(char * name,System::UInt16 * attr);
	virtual bool FindFirst(char * _dir,DOS_DTA & dta,bool fcb_findfirst=false);
	virtual void SetDir(const char* path);
	virtual bool isRemote(void);
	virtual bool isRemovable(void);
	virtual int UnMount(void);
private:
	System::Byte subUnit;
	char driveLetter;
};

#ifdef _MSC_VER
#pragma pack (1)
#endif
struct isoPVD {
	System::Byte type;
	System::Byte standardIdent[5];
	System::Byte version;
	System::Byte unused1;
	System::Byte systemIdent[32];
	System::Byte volumeIdent[32];
	System::Byte unused2[8];
	System::UInt32 volumeSpaceSizeL;
	System::UInt32 volumeSpaceSizeM;
	System::Byte unused3[32];
	System::UInt16 volumeSetSizeL;
	System::UInt16 volumeSetSizeM;
	System::UInt16 volumeSeqNumberL;
	System::UInt16 volumeSeqNumberM;
	System::UInt16 logicBlockSizeL;
	System::UInt16 logicBlockSizeM;
	System::UInt32 pathTableSizeL;
	System::UInt32 pathTableSizeM;
	System::UInt32 locationPathTableL;
	System::UInt32 locationOptPathTableL;
	System::UInt32 locationPathTableM;
	System::UInt32 locationOptPathTableM;
	System::Byte rootEntry[34];
	System::UInt32 unused4[1858];
} GCC_ATTRIBUTE(packed);

struct isoDirEntry {
	System::Byte length;
	System::Byte extAttrLength;
	System::UInt32 extentLocationL;
	System::UInt32 extentLocationM;
	System::UInt32 dataLengthL;
	System::UInt32 dataLengthM;
	System::Byte dateYear;
	System::Byte dateMonth;
	System::Byte dateDay;
	System::Byte timeHour;
	System::Byte timeMin;
	System::Byte timeSec;
	System::Byte timeZone;
	System::Byte fileFlags;
	System::Byte fileUnitSize;
	System::Byte interleaveGapSize;
	System::UInt16 VolumeSeqNumberL;
	System::UInt16 VolumeSeqNumberM;
	System::Byte fileIdentLength;
	System::Byte ident[222];
} GCC_ATTRIBUTE(packed);

#ifdef _MSC_VER
#pragma pack ()
#endif

#if defined (WORDS_BIGENDIAN)
#define EXTENT_LOCATION(de)	((de).extentLocationM)
#define DATA_LENGTH(de)		((de).dataLengthM)
#else
#define EXTENT_LOCATION(de)	((de).extentLocationL)
#define DATA_LENGTH(de)		((de).dataLengthL)
#endif

#define ISO_FRAMESIZE		2048
#define ISO_DIRECTORY		2
#define ISO_HIDDEN		1
#define ISO_MAX_FILENAME_LENGTH 37
#define ISO_MAXPATHNAME		256
#define ISO_FIRST_VD		16
#define IS_DIR(fileFlags)	(fileFlags & ISO_DIRECTORY)
#define IS_HIDDEN(fileFlags)	(fileFlags & ISO_HIDDEN)
#define ISO_MAX_HASH_TABLE_SIZE 	100

class isoDrive : public DOS_Drive {
public:
	isoDrive(char driveLetter, const char* device_name, System::Byte mediaid, int &error);
	~isoDrive();
	virtual bool FileOpen(DOS_File **file, char *name, System::UInt32 flags);
	virtual bool FileCreate(DOS_File **file, char *name, System::UInt16 attributes);
	virtual bool FileUnlink(char *name);
	virtual bool RemoveDir(char *dir);
	virtual bool MakeDir(char *dir);
	virtual bool TestDir(char *dir);
	virtual bool FindFirst(char *_dir, DOS_DTA &dta, bool fcb_findfirst);
	virtual bool FindNext(DOS_DTA &dta);
	virtual bool GetFileAttr(char *name, System::UInt16 *attr);
	virtual bool Rename(char * oldname,char * newname);
	virtual bool AllocationInfo(System::UInt16 *bytes_sector, System::Byte *sectors_cluster, System::UInt16 *total_clusters, System::UInt16 *free_clusters);
	virtual bool FileExists(const char *name);
   	virtual bool FileStat(const char *name, FileStat_Block *const stat_block);
	virtual System::Byte GetMediaByte(void);
	virtual void EmptyCache(void){}
	virtual bool isRemote(void);
	virtual bool isRemovable(void);
	virtual int UnMount(void);
	bool readSector(System::Byte *buffer, System::UInt32 sector);
	virtual char const* GetLabel(void) {return discLabel;};
	virtual void Activate(void);
private:
	int  readDirEntry(isoDirEntry *de, System::Byte *data);
	bool loadImage();
	bool lookupSingle(isoDirEntry *de, const char *name, System::UInt32 sectorStart, System::UInt32 length);
	bool lookup(isoDirEntry *de, const char *path);
	int  UpdateMscdex(char driveLetter, const char* physicalPath, System::Byte& subUnit);
	int  GetDirIterator(const isoDirEntry* de);
	bool GetNextDirEntry(const int dirIterator, isoDirEntry* de);
	void FreeDirIterator(const int dirIterator);
	bool ReadCachedSector(System::Byte** buffer, const System::UInt32 sector);
	
	struct DirIterator {
		bool valid;
		bool root;
		System::UInt32 currentSector;
		System::UInt32 endSector;
		System::UInt32 pos;
	} dirIterators[MAX_OPENDIRS];
	
	int nextFreeDirIterator;
	
	struct SectorHashEntry {
		bool valid;
		System::UInt32 sector;
		System::Byte data[ISO_FRAMESIZE];
	} sectorHashEntries[ISO_MAX_HASH_TABLE_SIZE];

	bool dataCD;
	isoDirEntry rootEntry;
	System::Byte mediaid;
	char fileName[CROSS_LEN];
	System::Byte subUnit;
	char driveLetter;
	char discLabel[32];
};

struct VFILE_Block;

class Virtual_Drive: public DOS_Drive {
public:
	Virtual_Drive();
	bool FileOpen(DOS_File * * file,char * name,System::UInt32 flags);
	bool FileCreate(DOS_File * * file,char * name,System::UInt16 attributes);
	bool FileUnlink(char * name);
	bool RemoveDir(char * dir);
	bool MakeDir(char * dir);
	bool TestDir(char * dir);
	bool FindFirst(char * _dir,DOS_DTA & dta,bool fcb_findfirst);
	bool FindNext(DOS_DTA & dta);
	bool GetFileAttr(char * name,System::UInt16 * attr);
	bool Rename(char * oldname,char * newname);
	bool AllocationInfo(System::UInt16 * _bytes_sector,System::Byte * _sectors_cluster,System::UInt16 * _total_clusters,System::UInt16 * _free_clusters);
	bool FileExists(const char* name);
	bool FileStat(const char* name, FileStat_Block* const stat_block);
	System::Byte GetMediaByte(void);
	void EmptyCache(void){}
	bool isRemote(void);
	virtual bool isRemovable(void);
	virtual int UnMount(void);
private:
	VFILE_Block * search_file;
};



#endif
