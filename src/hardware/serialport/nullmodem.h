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

/* $Id: nullmodem.h,v 1.4 2009-09-25 23:40:47 h-a-l-9000 Exp $ */

// include guard
#ifndef DOSBOX_NULLMODEM_WIN32_H
#define DOSBOX_NULLMODEM_WIN32_H

#include "dosbox.h"

#if C_MODEM

#include "misc_util.h"
#include "serialport.h"

#define SERIAL_SERVER_POLLING_EVENT	SERIAL_BASE_EVENT_COUNT+1
#define SERIAL_TX_REDUCTION		SERIAL_BASE_EVENT_COUNT+2
#define SERIAL_NULLMODEM_DTR_EVENT	SERIAL_BASE_EVENT_COUNT+3
#define SERIAL_NULLMODEM_EVENT_COUNT	SERIAL_BASE_EVENT_COUNT+3

class CNullModem : public CSerial {
public:
	TCPServerSocket* serversocket;
	TCPClientSocket* clientsocket;

	CNullModem(unsigned id, CommandLine* cmd);
	~CNullModem();
	bool receiveblock;		// It's not a block of data it rather blocks
	System::UInt16 serverport;		// we are a server if this is nonzero
	System::UInt16 clientport;

	System::Byte hostnamebuffer[128]; // the name passed to us by the user

	void updatePortConfig(System::UInt16 divider, System::Byte lcr);
	void updateMSR();
	void transmitByte(System::Byte val, bool first);
	void setBreak(bool value);
	
	void setRTSDTR(bool rts, bool dtr);
	void setRTS(bool val);
	void setDTR(bool val);
	void handleUpperEvent(System::UInt16 type);

	unsigned rx_state;
#define N_RX_IDLE		0
#define N_RX_WAIT		1
#define N_RX_BLOCKED	2
#define N_RX_FASTWAIT	3
#define N_RX_DISC		4

	bool doReceive();
	void ClientConnect();
    void Disconnect();
	int readChar();
	void WriteChar(System::Byte data);

	bool tx_block;		// true while the SERIAL_TX_REDUCTION event
						// is pending

	unsigned rx_retry;		// counter of retries

	unsigned rx_retry_max;	// how many POLL_EVENTS to wait before causing
						// a overrun error.

	unsigned tx_gather;		// how long to gather tx data before
						// sending all of them [milliseconds]

	
	bool dtrrespect;	// dtr behavior - only send data to the serial
						// port when DTR is on

	bool transparent;	// if true, don't send 0xff 0xXX to toggle
						// DSR/CTS.

	bool telnet;		// Do Telnet parsing.

	// Telnet's brain
#define TEL_CLIENT 0
#define TEL_SERVER 1

	int TelnetEmulation(System::Byte data);

	// Telnet's memory
	struct {
		bool binary[2];
		bool echo[2];
		bool supressGA[2];
		bool timingMark[2];
					
		bool inIAC;
		bool recCommand;
		System::Byte command;
	} telClient;
};

#endif	// C_MODEM
#endif	// include guard
