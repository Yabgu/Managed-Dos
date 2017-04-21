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

/* $Id: dos_classes.cpp,v 1.58 2009-07-09 20:06:57 c2woody Exp $ */

#include <string.h>
#include <stdlib.h>
#include "dosbox.h"
#include "mem.h"
#include "dos_inc.h"
#include "support.h"


void DOS_ParamBlock::Clear(void) {
	memset(&exec,0,sizeof(exec));
	memset(&overlay,0,sizeof(overlay));
}

void DOS_ParamBlock::LoadData(void) {
	exec.envseg=(System::UInt16)sGet(sExec,envseg);
	exec.cmdtail=sGet(sExec,cmdtail);
	exec.fcb1=sGet(sExec,fcb1);
	exec.fcb2=sGet(sExec,fcb2);
	exec.initsssp=sGet(sExec,initsssp);
	exec.initcsip=sGet(sExec,initcsip);
	overlay.loadseg=(System::UInt16)sGet(sOverlay,loadseg);
	overlay.relocation=(System::UInt16)sGet(sOverlay,relocation);
}

void DOS_ParamBlock::SaveData(void) {
	sSave(sExec,envseg,exec.envseg);
	sSave(sExec,cmdtail,exec.cmdtail);
	sSave(sExec,fcb1,exec.fcb1);
	sSave(sExec,fcb2,exec.fcb2);
	sSave(sExec,initsssp,exec.initsssp);
	sSave(sExec,initcsip,exec.initcsip);
}


void DOS_InfoBlock::SetLocation(System::UInt16 segment) {
	seg = segment;
	pt=PhysMake(seg,0);
	/* Clear the initial Block */
	for(unsigned i=0;i<sizeof(sDIB);i++) mem_writeb(pt+i,0xff);
	for(unsigned i=0;i<14;i++) mem_writeb(pt+i,0);

	sSave(sDIB,regCXfrom5e,(System::UInt16)0);
	sSave(sDIB,countLRUcache,(System::UInt16)0);
	sSave(sDIB,countLRUopens,(System::UInt16)0);

	sSave(sDIB,protFCBs,(System::UInt16)0);
	sSave(sDIB,specialCodeSeg,(System::UInt16)0);
	sSave(sDIB,joindedDrives,(System::Byte)0);
	sSave(sDIB,lastdrive,(System::Byte)0x01);//increase this if you add drives to cds-chain

	sSave(sDIB,diskInfoBuffer,RealMake(segment,offsetof(sDIB,diskBufferHeadPt)));
	sSave(sDIB,setverPtr,(System::UInt32)0);

	sSave(sDIB,a20FixOfs,(System::UInt16)0);
	sSave(sDIB,pspLastIfHMA,(System::UInt16)0);
	sSave(sDIB,blockDevices,(System::Byte)0);
	
	sSave(sDIB,bootDrive,(System::Byte)0);
	sSave(sDIB,useDwordMov,(System::Byte)1);
	sSave(sDIB,extendedSize,(System::UInt16)(MEM_TotalPages()*4-1024));
	sSave(sDIB,magicWord,(System::UInt16)0x0001);		// dos5+

	sSave(sDIB,sharingCount,(System::UInt16)0);
	sSave(sDIB,sharingDelay,(System::UInt16)0);
	sSave(sDIB,ptrCONinput,(System::UInt16)0);			// no unread input available
	sSave(sDIB,maxSectorLength,(System::UInt16)0x200);

	sSave(sDIB,dirtyDiskBuffers,(System::UInt16)0);
	sSave(sDIB,lookaheadBufPt,(System::UInt32)0);
	sSave(sDIB,lookaheadBufNumber,(System::UInt16)0);
	sSave(sDIB,bufferLocation,(System::Byte)0);		// buffer in base memory, no workspace
	sSave(sDIB,workspaceBuffer,(System::UInt32)0);

	sSave(sDIB,minMemForExec,(System::UInt16)0);
	sSave(sDIB,memAllocScanStart,(System::UInt16)DOS_MEM_START);
	sSave(sDIB,startOfUMBChain,(System::UInt16)0xffff);
	sSave(sDIB,chainingUMB,(System::Byte)0);

	sSave(sDIB,nulNextDriver,(System::UInt32)0xffffffff);
	sSave(sDIB,nulAttributes,(System::UInt16)0x8004);
	sSave(sDIB,nulStrategy,(System::UInt32)0x00000000);
	sSave(sDIB,nulString[0],(System::Byte)0x4e);
	sSave(sDIB,nulString[1],(System::Byte)0x55);
	sSave(sDIB,nulString[2],(System::Byte)0x4c);
	sSave(sDIB,nulString[3],(System::Byte)0x20);
	sSave(sDIB,nulString[4],(System::Byte)0x20);
	sSave(sDIB,nulString[5],(System::Byte)0x20);
	sSave(sDIB,nulString[6],(System::Byte)0x20);
	sSave(sDIB,nulString[7],(System::Byte)0x20);

	/* Create a fake SFT, so programs think there are 100 file handles */
	System::UInt16 sftOffset=offsetof(sDIB,firstFileTable)+0xa2;
	sSave(sDIB,firstFileTable,RealMake(segment,sftOffset));
	real_writed(segment,sftOffset+0x00,RealMake(segment+0x26,0));	//Next File Table
	real_writew(segment,sftOffset+0x04,100);		//File Table supports 100 files
	real_writed(segment+0x26,0x00,0xffffffff);		//Last File Table
	real_writew(segment+0x26,0x04,100);				//File Table supports 100 files
}

void DOS_InfoBlock::SetFirstMCB(System::UInt16 _firstmcb) {
	sSave(sDIB,firstMCB,_firstmcb); //c2woody
}

void DOS_InfoBlock::SetBuffers(System::UInt16 x,System::UInt16 y) {
	sSave(sDIB,buffers_x,x);
	sSave(sDIB,buffers_y,y);
}

void DOS_InfoBlock::SetCurDirStruct(System::UInt32 _curdirstruct) {
	sSave(sDIB,curDirStructure,_curdirstruct);
}

void DOS_InfoBlock::SetFCBTable(System::UInt32 _fcbtable) {
	sSave(sDIB,fcbTable,_fcbtable);
}

void DOS_InfoBlock::SetDeviceChainStart(System::UInt32 _devchain) {
	sSave(sDIB,nulNextDriver,_devchain);
}

void DOS_InfoBlock::SetDiskBufferHeadPt(System::UInt32 _dbheadpt) {
	sSave(sDIB,diskBufferHeadPt,_dbheadpt);
}

System::UInt16 DOS_InfoBlock::GetStartOfUMBChain(void) {
	return (System::UInt16)sGet(sDIB,startOfUMBChain);
}

void DOS_InfoBlock::SetStartOfUMBChain(System::UInt16 _umbstartseg) {
	sSave(sDIB,startOfUMBChain,_umbstartseg);
}

System::Byte DOS_InfoBlock::GetUMBChainState(void) {
	return (System::Byte)sGet(sDIB,chainingUMB);
}

void DOS_InfoBlock::SetUMBChainState(System::Byte _umbchaining) {
	sSave(sDIB,chainingUMB,_umbchaining);
}

RealPt DOS_InfoBlock::GetPointer(void) {
	return RealMake(seg,offsetof(sDIB,firstDPB));
}

System::UInt32 DOS_InfoBlock::GetDeviceChain(void) {
	return sGet(sDIB,nulNextDriver);
}


/* program Segment prefix */

System::UInt16 DOS_PSP::rootpsp = 0;

void DOS_PSP::MakeNew(System::UInt16 mem_size) {
	/* get previous */
//	DOS_PSP prevpsp(dos.psp());
	/* Clear it first */
	unsigned i;
	for (i=0;i<sizeof(sPSP);i++) mem_writeb(pt+i,0);
	// Set size
	sSave(sPSP,next_seg,seg+mem_size);
	/* far call opcode */
	sSave(sPSP,far_call,0xea);
	// far call to interrupt 0x21 - faked for bill & ted 
	// lets hope nobody really uses this address
	sSave(sPSP,cpm_entry,RealMake(0xDEAD,0xFFFF));
	/* Standard blocks,int 20  and int21 retf */
	sSave(sPSP,exit[0],0xcd);
	sSave(sPSP,exit[1],0x20);
	sSave(sPSP,service[0],0xcd);
	sSave(sPSP,service[1],0x21);
	sSave(sPSP,service[2],0xcb);
	/* psp and psp-parent */
	sSave(sPSP,psp_parent,dos.psp());
	sSave(sPSP,prev_psp,0xffffffff);
	sSave(sPSP,dos_version,0x0005);
	/* terminate 22,break 23,crititcal error 24 address stored */
	SaveVectors();

	/* FCBs are filled with 0 */
	// ....
	/* Init file pointer and max_files */
	sSave(sPSP,file_table,RealMake(seg,offsetof(sPSP,files)));
	sSave(sPSP,max_files,20);
	for (System::UInt16 ct=0;ct<20;ct++) SetFileHandle(ct,0xff);

	/* User Stack pointer */
//	if (prevpsp.GetSegment()!=0) sSave(sPSP,stack,prevpsp.GetStack());

	if (rootpsp==0) rootpsp = seg;
}

System::Byte DOS_PSP::GetFileHandle(System::UInt16 index) {
	if (index>=sGet(sPSP,max_files)) return 0xff;
	PhysPt files=Real2Phys(sGet(sPSP,file_table));
	return mem_readb(files+index);
}

void DOS_PSP::SetFileHandle(System::UInt16 index, System::Byte handle) {
	if (index<sGet(sPSP,max_files)) {
		PhysPt files=Real2Phys(sGet(sPSP,file_table));
		mem_writeb(files+index,handle);
	}
}

System::UInt16 DOS_PSP::FindFreeFileEntry(void) {
	PhysPt files=Real2Phys(sGet(sPSP,file_table));
	for (System::UInt16 i=0;i<sGet(sPSP,max_files);i++) {
		if (mem_readb(files+i)==0xff) return i;
	}	
	return 0xff;
}

System::UInt16 DOS_PSP::FindEntryByHandle(System::Byte handle) {
	PhysPt files=Real2Phys(sGet(sPSP,file_table));
	for (System::UInt16 i=0;i<sGet(sPSP,max_files);i++) {
		if (mem_readb(files+i)==handle) return i;
	}	
	return 0xFF;
}

void DOS_PSP::CopyFileTable(DOS_PSP* srcpsp,bool createchildpsp) {
	/* Copy file table from calling process */
	for (System::UInt16 i=0;i<20;i++) {
		System::Byte handle = srcpsp->GetFileHandle(i);
		if(createchildpsp)
		{	//copy obeying not inherit flag.(but dont duplicate them)
			bool allowCopy = true;//(handle==0) || ((handle>0) && (FindEntryByHandle(handle)==0xff));
			if((handle<DOS_FILES) && Files[handle] && !(Files[handle]->flags & DOS_NOT_INHERIT) && allowCopy)
			{   
				Files[handle]->AddRef();
				SetFileHandle(i,handle);
			}
			else
			{
				SetFileHandle(i,0xff);
			}
		}
		else
		{	//normal copy so don't mind the inheritance
			SetFileHandle(i,handle);
		}
	}
}

void DOS_PSP::CloseFiles(void) {
	for (System::UInt16 i=0;i<sGet(sPSP,max_files);i++) {
		DOS_CloseFile(i);
	}
}

void DOS_PSP::SaveVectors(void) {
	/* Save interrupt 22,23,24 */
	sSave(sPSP,int_22,RealGetVec(0x22));
	sSave(sPSP,int_23,RealGetVec(0x23));
	sSave(sPSP,int_24,RealGetVec(0x24));
}

void DOS_PSP::RestoreVectors(void) {
	/* Restore interrupt 22,23,24 */
	RealSetVec(0x22,sGet(sPSP,int_22));
	RealSetVec(0x23,sGet(sPSP,int_23));
	RealSetVec(0x24,sGet(sPSP,int_24));
}

void DOS_PSP::SetCommandTail(RealPt src) {
	if (src) {	// valid source
		MEM_BlockCopy(pt+offsetof(sPSP,cmdtail),Real2Phys(src),128);
	} else {	// empty
		sSave(sPSP,cmdtail.count,0x00);
		mem_writeb(pt+offsetof(sPSP,cmdtail.buffer),0x0d);
	};
}

void DOS_PSP::SetFCB1(RealPt src) {
	if (src) MEM_BlockCopy(PhysMake(seg,offsetof(sPSP,fcb1)),Real2Phys(src),16);
}

void DOS_PSP::SetFCB2(RealPt src) {
	if (src) MEM_BlockCopy(PhysMake(seg,offsetof(sPSP,fcb2)),Real2Phys(src),16);
}

bool DOS_PSP::SetNumFiles(System::UInt16 fileNum) {
	if (fileNum>20) {
		// Allocate needed paragraphs
		fileNum+=2;	// Add a few more files for safety
		System::UInt16 para = (fileNum/16)+((fileNum%16)>0);
		RealPt data	= RealMake(DOS_GetMemory(para),0);
		sSave(sPSP,file_table,data);
		sSave(sPSP,max_files,fileNum);
		System::UInt16 i;
		for (i=0; i<20; i++)		SetFileHandle(i,(System::Byte)sGet(sPSP,files[i]));
		for (i=20; i<fileNum; i++)	SetFileHandle(i,0xFF);
	} else {
		sSave(sPSP,max_files,fileNum);
	};
	return true;
}


void DOS_DTA::SetupSearch(System::Byte _sdrive,System::Byte _sattr,char * pattern) {
	sSave(sDTA,sdrive,_sdrive);
	sSave(sDTA,sattr,_sattr);
	/* Fill with spaces */
	unsigned i;
	for (i=0;i<11;i++) mem_writeb(pt+offsetof(sDTA,sname)+i,' ');
	char * find_ext;
	find_ext=strchr(pattern,'.');
	if (find_ext) {
		unsigned size=(unsigned)(find_ext-pattern);
		if (size>8) size=8;
		MEM_BlockWrite(pt+offsetof(sDTA,sname),pattern,size);
		find_ext++;
		MEM_BlockWrite(pt+offsetof(sDTA,sext),find_ext,(strlen(find_ext)>3) ? 3 : (unsigned)strlen(find_ext));
	} else {
		MEM_BlockWrite(pt+offsetof(sDTA,sname),pattern,(strlen(pattern) > 8) ? 8 : (unsigned)strlen(pattern));
	}
}

void DOS_DTA::SetResult(const char * _name,System::UInt32 _size,System::UInt16 _date,System::UInt16 _time,System::Byte _attr) {
	MEM_BlockWrite(pt+offsetof(sDTA,name),(void *)_name,DOS_NAMELENGTH_ASCII);
	sSave(sDTA,size,_size);
	sSave(sDTA,date,_date);
	sSave(sDTA,time,_time);
	sSave(sDTA,attr,_attr);
}


void DOS_DTA::GetResult(char * _name,System::UInt32 & _size,System::UInt16 & _date,System::UInt16 & _time,System::Byte & _attr) {
	MEM_BlockRead(pt+offsetof(sDTA,name),_name,DOS_NAMELENGTH_ASCII);
	_size=sGet(sDTA,size);
	_date=(System::UInt16)sGet(sDTA,date);
	_time=(System::UInt16)sGet(sDTA,time);
	_attr=(System::Byte)sGet(sDTA,attr);
}

System::Byte DOS_DTA::GetSearchDrive(void) {
	return (System::Byte)sGet(sDTA,sdrive);
}

void DOS_DTA::GetSearchParams(System::Byte & attr,char * pattern) {
	attr=(System::Byte)sGet(sDTA,sattr);
	char temp[11];
	MEM_BlockRead(pt+offsetof(sDTA,sname),temp,11);
	memcpy(pattern,temp,8);
	pattern[8]='.';
	memcpy(&pattern[9],&temp[8],3);
	pattern[12]=0;

}

DOS_FCB::DOS_FCB(System::UInt16 seg,System::UInt16 off,bool allow_extended) { 
	SetPt(seg,off); 
	real_pt=pt;
	extended=false;
	if (allow_extended) {
		if (sGet(sFCB,drive)==0xff) {
			pt+=7;
			extended=true;
		}
	}
}

bool DOS_FCB::Extended(void) {
	return extended;
}

void DOS_FCB::Create(bool _extended) {
	unsigned fill;
	if (_extended) fill=36+7;
	else fill=36;
	unsigned i;
	for (i=0;i<fill;i++) mem_writeb(real_pt+i,0);
	pt=real_pt;
	if (_extended) {
		mem_writeb(real_pt,0xff);
		pt+=7;
		extended=true;
	} else extended=false;
}

void DOS_FCB::SetName(System::Byte _drive,char * _fname,char * _ext) {
	sSave(sFCB,drive,_drive);
	MEM_BlockWrite(pt+offsetof(sFCB,filename),_fname,8);
	MEM_BlockWrite(pt+offsetof(sFCB,ext),_ext,3);
}

void DOS_FCB::SetSizeDateTime(System::UInt32 _size,System::UInt16 _date,System::UInt16 _time) {
	sSave(sFCB,filesize,_size);
	sSave(sFCB,date,_date);
	sSave(sFCB,time,_time);
}

void DOS_FCB::GetSizeDateTime(System::UInt32 & _size,System::UInt16 & _date,System::UInt16 & _time) {
	_size=sGet(sFCB,filesize);
	_date=(System::UInt16)sGet(sFCB,date);
	_time=(System::UInt16)sGet(sFCB,time);
}

void DOS_FCB::GetRecord(System::UInt16 & _cur_block,System::Byte & _cur_rec) {
	_cur_block=(System::UInt16)sGet(sFCB,cur_block);
	_cur_rec=(System::Byte)sGet(sFCB,cur_rec);

}

void DOS_FCB::SetRecord(System::UInt16 _cur_block,System::Byte _cur_rec) {
	sSave(sFCB,cur_block,_cur_block);
	sSave(sFCB,cur_rec,_cur_rec);
}

void DOS_FCB::GetSeqData(System::Byte & _fhandle,System::UInt16 & _rec_size) {
	_fhandle=(System::Byte)sGet(sFCB,file_handle);
	_rec_size=(System::UInt16)sGet(sFCB,rec_size);
}


void DOS_FCB::GetRandom(System::UInt32 & _random) {
	_random=sGet(sFCB,rndm);
}

void DOS_FCB::SetRandom(System::UInt32 _random) {
	sSave(sFCB,rndm,_random);
}

void DOS_FCB::FileOpen(System::Byte _fhandle) {
	sSave(sFCB,drive,GetDrive()+1);
	sSave(sFCB,file_handle,_fhandle);
	sSave(sFCB,cur_block,0);
	sSave(sFCB,rec_size,128);
//	sSave(sFCB,rndm,0); // breaks Jewels of darkness. 
	System::Byte temp = RealHandle(_fhandle);
	System::UInt32 size = 0;
	Files[temp]->Seek(&size,DOS_SEEK_END);
	sSave(sFCB,filesize,size);
	size = 0;
	Files[temp]->Seek(&size,DOS_SEEK_SET);
	sSave(sFCB,time,Files[temp]->time);
	sSave(sFCB,date,Files[temp]->date);
}

bool DOS_FCB::Valid() {
	//Very simple check for Oubliette
	if(sGet(sFCB,filename[0]) == 0 && sGet(sFCB,file_handle) == 0) return false;
	return true;
}

void DOS_FCB::FileClose(System::Byte & _fhandle) {
	_fhandle=(System::Byte)sGet(sFCB,file_handle);
	sSave(sFCB,file_handle,0xff);
}

System::Byte DOS_FCB::GetDrive(void) {
	System::Byte drive=(System::Byte)sGet(sFCB,drive);
	if (!drive) return  DOS_GetDefaultDrive();
	else return drive-1;
}

void DOS_FCB::GetName(char * fillname) {
	fillname[0]=GetDrive()+'A';
	fillname[1]=':';
	MEM_BlockRead(pt+offsetof(sFCB,filename),&fillname[2],8);
	fillname[10]='.';
	MEM_BlockRead(pt+offsetof(sFCB,ext),&fillname[11],3);
	fillname[14]=0;
}

void DOS_FCB::GetAttr(System::Byte& attr) {
	if(extended) attr=mem_readb(pt - 1);
}

void DOS_FCB::SetAttr(System::Byte attr) {
	if(extended) mem_writeb(pt - 1,attr);
}

void DOS_SDA::Init() {
	/* Clear */
	for(unsigned i=0;i<sizeof(sSDA);i++) mem_writeb(pt+i,0x00);
	sSave(sSDA,drive_crit_error,0xff);   
}
