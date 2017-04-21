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

/* $Id: mixer.h,v 1.19 2009-04-28 21:48:24 harekiet Exp $ */

#ifndef DOSBOX_MIXER_H
#define DOSBOX_MIXER_H

#ifndef DOSBOX_DOSBOX_H
#include "dosbox.h"
#endif

typedef void (*MIXER_MixHandler)(System::Byte * sampdate,System::UInt32 len);
typedef void (*MIXER_Handler)(unsigned len);

enum BlahModes {
	MIXER_8MONO,MIXER_8STEREO,
	MIXER_16MONO,MIXER_16STEREO
};

enum MixerModes {
	M_8M,M_8S,
	M_16M,M_16S
};

#define MIXER_BUFSIZE (16*1024)
#define MIXER_BUFMASK (MIXER_BUFSIZE-1)
extern System::Byte MixTemp[MIXER_BUFSIZE];

#define MAX_AUDIO ((1<<(16-1))-1)
#define MIN_AUDIO -(1<<(16-1))

class MixerChannel {
public:
	void SetVolume(float _left,float _right);
	void SetScale( float f );
	void UpdateVolume(void);
	void SetFreq(unsigned _freq);
	void Mix(unsigned _needed);
	void AddSilence(void);			//Fill up until needed

	template<class Type,bool stereo,bool signeddata,bool nativeorder>
	void AddSamples(unsigned len, const Type* data);

	void AddSamples_m8(unsigned len, const System::Byte * data);
	void AddSamples_s8(unsigned len, const System::Byte * data);
	void AddSamples_m8s(unsigned len, const System::SByte * data);
	void AddSamples_s8s(unsigned len, const System::SByte * data);
	void AddSamples_m16(unsigned len, const System::Int16 * data);
	void AddSamples_s16(unsigned len, const System::Int16 * data);
	void AddSamples_m16u(unsigned len, const System::UInt16 * data);
	void AddSamples_s16u(unsigned len, const System::UInt16 * data);
	void AddSamples_m32(unsigned len, const System::Int32 * data);
	void AddSamples_s32(unsigned len, const System::Int32 * data);
	void AddSamples_m16_nonnative(unsigned len, const System::Int16 * data);
	void AddSamples_s16_nonnative(unsigned len, const System::Int16 * data);
	void AddSamples_m16u_nonnative(unsigned len, const System::UInt16 * data);
	void AddSamples_s16u_nonnative(unsigned len, const System::UInt16 * data);
	void AddSamples_m32_nonnative(unsigned len, const System::Int32 * data);
	void AddSamples_s32_nonnative(unsigned len, const System::Int32 * data);

	void AddStretched(unsigned len,System::Int16 * data);		//Strech block up into needed data
	void FillUp(void);
	void Enable(bool _yesno);
	MIXER_Handler handler;
	float volmain[2];
	float scale;
	System::Int32 volmul[2];
	unsigned freq_add,freq_index;
	unsigned done,needed;
	int last[2];
	const char * name;
	bool enabled;
	MixerChannel * next;
};

MixerChannel * MIXER_AddChannel(MIXER_Handler handler,unsigned freq,const char * name);
MixerChannel * MIXER_FindChannel(const char * name);
/* Find the device you want to delete with findchannel "delchan gets deleted" */
void MIXER_DelChannel(MixerChannel* delchan); 

/* Object to maintain a mixerchannel; As all objects it registers itself with create
 * and removes itself when destroyed. */
class MixerObject{
private:
	bool installed;
	char m_name[32];
public:
	MixerObject():installed(false){};
	MixerChannel* Install(MIXER_Handler handler,unsigned freq,const char * name);
	~MixerObject();
};


/* PC Speakers functions, tightly related to the timer functions */
void PCSPEAKER_SetCounter(unsigned cntr,unsigned mode);
void PCSPEAKER_SetType(unsigned mode);

#endif
