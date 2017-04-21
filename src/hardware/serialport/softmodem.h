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

/* $Id: softmodem.h,v 1.12 2009-10-04 20:57:40 h-a-l-9000 Exp $ */

#ifndef DOSBOX_SERIALMODEM_H
#define DOSBOX_SERIALMODEM_H

#include "dosbox.h"
#if C_MODEM
#include "serialport.h"

#include "misc_util.h"

#define MODEMSPD 57600
#define SREGS 100

//If it's too high you overflow terminal clients buffers i think
#define MODEM_BUFFER_QUEUE_SIZE 1024

#define MODEM_DEFAULT_PORT 23

#define MODEM_TX_EVENT SERIAL_BASE_EVENT_COUNT + 1
#define MODEM_RX_POLLING SERIAL_BASE_EVENT_COUNT + 2
#define MODEM_RING_EVENT SERIAL_BASE_EVENT_COUNT + 3
#define SERIAL_MODEM_EVENT_COUNT SERIAL_BASE_EVENT_COUNT+3


enum ResTypes {
	ResNONE,
	ResOK,ResERROR,
	ResCONNECT,ResRING,
	ResBUSY,ResNODIALTONE,ResNOCARRIER
};

#define TEL_CLIENT 0
#define TEL_SERVER 1

class CFifo {
public:
	CFifo(unsigned _size) {
		size=_size;
		pos=used=0;
		data=new System::Byte[size];
	}
	~CFifo() {
		delete[] data;
	}
	INLINE unsigned left(void) {
		return size-used;
	}
	INLINE unsigned inuse(void) {
		return used;
	}
	void clear(void) {
		used=pos=0;
	}

	void addb(System::Byte _val) {
		if(used>=size) {
			static int lcount=0;
			if (lcount<1000) {
				lcount++;
				LOG_MSG("MODEM: FIFO Overflow! (addb)");
			}
			return;
		}
		//assert(used<size);
		unsigned where=pos+used;
		if (where>=size) where-=size;
		data[where]=_val;
		//LOG_MSG("+%x",_val);
		used++;
	}
	void adds(System::Byte * _str,unsigned _len) {
		if((used+_len)>size) {
			static int lcount=0;
			if (lcount<1000) {
				lcount++;
				LOG_MSG("MODEM: FIFO Overflow! (adds len %u)",_len);
			}
			return;
		}
		
		//assert((used+_len)<=size);
		unsigned where=pos+used;
		used+=_len;
		while (_len--) {
			if (where>=size) where-=size;
			//LOG_MSG("+'%x'",*_str);
			data[where++]=*_str++;
		}
	}
	System::Byte getb(void) {
		if (!used) {
			static int lcount=0;
			if (lcount<1000) {
				lcount++;
				LOG_MSG("MODEM: FIFO UNDERFLOW! (getb)");
			}
			return data[pos];
		}
			unsigned where=pos;
		if (++pos>=size) pos-=size;
		used--;
		//LOG_MSG("-%x",data[where]);
		return data[where];
	}
	void gets(System::Byte * _str,unsigned _len) {
		if (!used) {
			static int lcount=0;
			if (lcount<1000) {
				lcount++;
				LOG_MSG("MODEM: FIFO UNDERFLOW! (gets len %d)",_len);
			}
			return;
		}
			//assert(used>=_len);
		used-=_len;
		while (_len--) {
			//LOG_MSG("-%x",data[pos]);
			*_str++=data[pos];
			if (++pos>=size) pos-=size;
		}
	}
private:
	System::Byte * data;
	unsigned size,pos,used;
};
#define MREG_AUTOANSWER_COUNT 0
#define MREG_RING_COUNT 1
#define MREG_ESCAPE_CHAR 2
#define MREG_CR_CHAR 3
#define MREG_LF_CHAR 4
#define MREG_BACKSPACE_CHAR 5


class CSerialModem : public CSerial {
public:

	CFifo *rqueue;
	CFifo *tqueue;

	CSerialModem(unsigned id, CommandLine* cmd);
	~CSerialModem();

	void Reset();

	void SendLine(const char *line);
	void SendRes(ResTypes response);
	void SendNumber(unsigned val);

	void EnterIdleState();
	void EnterConnectedState();

	void openConnection(void);
	bool Dial(char * host);
	void AcceptIncomingCall(void);
	unsigned ScanNumber(char * & scan);
	char GetChar(char * & scan);

	void DoCommand();
	
	void MC_Changed(unsigned new_mc);

	void TelnetEmulation(System::Byte * data, unsigned size);

	//TODO
	void Timer2(void);
	void handleUpperEvent(System::UInt16 type);

	void RXBufferEmpty();

	void transmitByte(System::Byte val, bool first);
	void updatePortConfig(System::UInt16 divider, System::Byte lcr);
	void updateMSR();

	void setBreak(bool);

	void setRTSDTR(bool rts, bool dtr);
	void setRTS(bool val);
	void setDTR(bool val);

protected:
	char cmdbuf[255];
	bool commandmode;		// true: interpret input as commands
	bool echo;				// local echo on or off

	bool oldDTRstate;
	bool ringing;
	//bool response;
	bool numericresponse;	// true: send control response as number.
							// false: send text (i.e. NO DIALTONE)
	bool telnetmode;		// true: process IAC commands.
	
	bool connected;
	unsigned doresponse;

	System::Byte waiting_tx_character;

	unsigned cmdpause;
	int ringtimer;
	int ringcount;
	unsigned plusinc;
	unsigned cmdpos;
	unsigned flowcontrol;

	System::Byte tmpbuf[MODEM_BUFFER_QUEUE_SIZE];

	unsigned listenport;
	System::Byte reg[SREGS];
	
	
	TCPServerSocket* serversocket;
	TCPClientSocket* clientsocket;
	TCPClientSocket* waitingclientsocket;

	struct {
		bool binary[2];
		bool echo[2];
		bool supressGA[2];
		bool timingMark[2];
					
		bool inIAC;
		bool recCommand;
		System::Byte command;
	} telClient;
	struct {
		bool active;
		double f1, f2;
		unsigned len,pos;
		char str[256];
	} dial;
};
#endif
#endif 
