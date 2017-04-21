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

#include "adlib.h"
#include "dosbox.h"

//Use 8 handlers based on a small logatirmic wavetabe and an exponential table for volume
#define WAVE_HANDLER	10
//Use a logarithmic wavetable with an exponential table for volume
#define WAVE_TABLELOG	11
//Use a linear wavetable with a multiply table for volume
#define WAVE_TABLEMUL	12

//Select the type of wave generator routine
#define DBOPL_WAVE WAVE_TABLEMUL

namespace DBOPL {

struct Chip;
struct Operator;
struct Channel;

#if (DBOPL_WAVE == WAVE_HANDLER)
typedef int ( DB_FASTCALL *WaveHandler) ( unsigned i, unsigned volume );
#endif

typedef int ( DBOPL::Operator::*VolumeHandler) ( );
typedef Channel* ( DBOPL::Channel::*SynthHandler) ( Chip* chip, System::UInt32 samples, System::Int32* output );

//Different synth modes that can generate blocks of data
typedef enum {
	sm2AM,
	sm2FM,
	sm3AM,
	sm3FM,
	sm4Start,
	sm3FMFM,
	sm3AMFM,
	sm3FMAM,
	sm3AMAM,
	sm6Start,
	sm2Percussion,
	sm3Percussion,
} SynthMode;

//Shifts for the values contained in chandata variable
enum {
	SHIFT_KSLBASE = 16,
	SHIFT_KEYCODE = 24,
};

struct Operator {
public:
	//Masks for operator 20 values
	enum {
		MASK_KSR = 0x10,
		MASK_SUSTAIN = 0x20,
		MASK_VIBRATO = 0x40,
		MASK_TREMOLO = 0x80,
	};

	typedef enum {
		OFF,
		RELEASE,
		SUSTAIN,
		DECAY,
		ATTACK,
	} State;

	VolumeHandler volHandler;

#if (DBOPL_WAVE == WAVE_HANDLER)
	WaveHandler waveHandler;	//Routine that generate a wave 
#else
	System::Int16* waveBase;
	System::UInt32 waveMask;
	System::UInt32 waveStart;
#endif
	System::UInt32 waveIndex;			//WAVE_int shifted counter of the frequency index
	System::UInt32 waveAdd;				//The base frequency without vibrato
	System::UInt32 waveCurrent;			//waveAdd + vibratao

	System::UInt32 chanData;			//Frequency/octave and derived data coming from whatever channel controls this
	System::UInt32 freqMul;				//Scale channel frequency with this, TODO maybe remove?
	System::UInt32 vibrato;				//Scaled up vibrato strength
	System::Int32 sustainLevel;		//When stopping at sustain level stop here
	System::Int32 totalLevel;			//totalLevel is added to every generated volume
	System::UInt32 currentLevel;		//totalLevel + tremolo
	System::Int32 volume;				//The currently active volume
	
	System::UInt32 attackAdd;			//Timers for the different states of the envelope
	System::UInt32 decayAdd;
	System::UInt32 releaseAdd;
	System::UInt32 rateIndex;			//Current position of the evenlope

	System::Byte rateZero;				//int for the different states of the envelope having no changes
	System::Byte keyOn;				//Bitmask of different values that can generate keyon
	//Registers, also used to check for changes
	System::Byte reg20, reg40, reg60, reg80, regE0;
	//Active part of the envelope we're in
	System::Byte state;
	//0xff when tremolo is enabled
	System::Byte tremoloMask;
	//Strength of the vibrato
	System::Byte vibStrength;
	//Keep track of the calculated KSR so we can check for changes
	System::Byte ksr;
private:
	void SetState( System::Byte s );
	void UpdateAttack( const Chip* chip );
	void UpdateRelease( const Chip* chip );
	void UpdateDecay( const Chip* chip );
public:
	void UpdateAttenuation();
	void UpdateRates( const Chip* chip );
	void UpdateFrequency( );

	void Write20( const Chip* chip, System::Byte val );
	void Write40( const Chip* chip, System::Byte val );
	void Write60( const Chip* chip, System::Byte val );
	void Write80( const Chip* chip, System::Byte val );
	void WriteE0( const Chip* chip, System::Byte val );

	bool Silent() const;
	void Prepare( const Chip* chip );

	void KeyOn( System::Byte mask);
	void KeyOff( System::Byte mask);

	template< State state>
	int TemplateVolume( );

	System::Int32 RateForward( System::UInt32 add );
	unsigned ForwardWave();
	unsigned ForwardVolume();

	int GetSample( int modulation );
	int GetWave( unsigned index, unsigned vol );
public:
	Operator();
};

struct Channel {
	Operator op[2];
	inline Operator* Op( unsigned index ) {
		return &( ( this + (index >> 1) )->op[ index & 1 ]);
	}
	SynthHandler synthHandler;
	System::UInt32 chanData;		//Frequency/octave and derived values
	System::Int32 old[2];			//Old data for feedback

	System::Byte feedback;			//Feedback shift
	System::Byte regB0;			//Register values to check for changes
	System::Byte regC0;
	//This should correspond with reg104, bit 6 indicates a Percussion channel, bit 7 indicates a silent channel
	System::Byte fourMask;
	System::SByte maskLeft;		//Sign extended values for both channel's panning
	System::SByte maskRight;

	//Forward the channel data to the operators of the channel
	void SetChanData( const Chip* chip, System::UInt32 data );
	//Change in the chandata, check for new values and if we have to forward to operators
	void UpdateFrequency( const Chip* chip, System::Byte fourOp );
	void WriteA0( const Chip* chip, System::Byte val );
	void WriteB0( const Chip* chip, System::Byte val );
	void WriteC0( const Chip* chip, System::Byte val );
	void ResetC0( const Chip* chip );

	//call this for the first channel
	template< bool opl3Mode >
	void GeneratePercussion( Chip* chip, System::Int32* output );

	//Generate blocks of data in specific modes
	template<SynthMode mode>
	Channel* BlockTemplate( Chip* chip, System::UInt32 samples, System::Int32* output );
	Channel();
};

struct Chip {
	//This is used as the base counter for vibrato and tremolo
	System::UInt32 lfoCounter;
	System::UInt32 lfoAdd;
	

	System::UInt32 noiseCounter;
	System::UInt32 noiseAdd;
	System::UInt32 noiseValue;

	//Frequency scales for the different multiplications
	System::UInt32 freqMul[16];
	//Rates for decay and release for rate of this chip
	System::UInt32 linearRates[76];
	//Best match attack rates for the rate of this chip
	System::UInt32 attackRates[76];

	//18 channels with 2 operators each
	Channel chan[18];

	System::Byte reg104;
	System::Byte reg08;
	System::Byte reg04;
	System::Byte regBD;
	System::Byte vibratoIndex;
	System::Byte tremoloIndex;
	System::SByte vibratoSign;
	System::Byte vibratoShift;
	System::Byte tremoloValue;
	System::Byte vibratoStrength;
	System::Byte tremoloStrength;
	//Mask for allowed wave forms
	System::Byte waveFormMask;
	//0 or -1 when enabled
	System::SByte opl3Active;

	//Return the maximum amount of samples before and LFO change
	System::UInt32 ForwardLFO( System::UInt32 samples );
	System::UInt32 ForwardNoise();

	void WriteBD( System::Byte val );
	void WriteReg(System::UInt32 reg, System::Byte val );

	System::UInt32 WriteAddr( System::UInt32 port, System::Byte val );

	void GenerateBlock2( unsigned samples, System::Int32* output );
	void GenerateBlock3( unsigned samples, System::Int32* output );

	void Generate( System::UInt32 samples );
	void Setup( System::UInt32 r );

	Chip();
};

struct Handler : public Adlib::Handler {
	DBOPL::Chip chip;
	virtual System::UInt32 WriteAddr( System::UInt32 port, System::Byte val );
	virtual void WriteReg( System::UInt32 addr, System::Byte val );
	virtual void Generate( MixerChannel* chan, unsigned samples );
	virtual void Init( unsigned rate );
};


};		//Namespace
