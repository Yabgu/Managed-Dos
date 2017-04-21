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

#ifndef DOSBOX_IPXSERVER_H_
#define DOSBOX_IPXSERVER_H_

#if C_IPX

#include "SDL_net.h"

struct packetBuffer {
	System::Byte buffer[1024];
	System::Int16 packetSize;  // Packet size remaining in read
	System::Int16 packetRead;  // Bytes read of total packet
	bool inPacket;      // In packet reception flag
	bool connected;		// Connected flag
	bool waitsize;
};

#define SOCKETTABLESIZE 16
#define CONVIP(hostvar) hostvar & 0xff, (hostvar >> 8) & 0xff, (hostvar >> 16) & 0xff, (hostvar >> 24) & 0xff
#define CONVIPX(hostvar) hostvar[0], hostvar[1], hostvar[2], hostvar[3], hostvar[4], hostvar[5]


void IPX_StopServer();
bool IPX_StartServer(System::UInt16 portnum);
bool IPX_isConnectedToServer(int tableNum, IPaddress^% ptrAddr);

System::Byte packetCRC(cli::array<System::Byte>^ buffer);
System::Byte packetCRC(System::Byte const * const buffer, const int bufferLength);
System::Byte packetCRC(cli::array<System::Byte>^ buffer, const int bufferLength);

#endif

#endif
