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

 /* $Id: ipxserver.cpp,v 1.10 2009-05-27 09:15:41 qbix79 Exp $ */

#include "dosbox.h"

#if C_IPX

#include "dosbox.h"
#include "ipxserver.h"
#include "timer.h"
#include <stdlib.h>
#include <string.h>
#include "ipx.h"
#include <vcclr.h>

gcroot<IPaddress^> ipxServerIp = gcnew IPaddress();  // IPAddress for server's listening port

gcroot<System::Net::Sockets::Socket^> ipxServerSocket = gcnew System::Net::Sockets::Socket(
	System::Net::Sockets::AddressFamily::InterNetwork,
	System::Net::Sockets::SocketType::Dgram, System::Net::Sockets::ProtocolType::Udp);  // Listening server socket

packetBuffer connBuffer[SOCKETTABLESIZE];

gcroot<cli::array<System::Byte>^> inBuffer = gcnew cli::array<System::Byte>(IPXBUFFERSIZE);
gcroot<cli::array<IPaddress^>^> ipconn = gcnew cli::array<IPaddress^>(SOCKETTABLESIZE);  // Active TCP/IP connection 

gcroot<System::Collections::Generic::List<System::Net::Sockets::Socket^>^> tcpconn
= gcnew System::Collections::Generic::List<System::Net::Sockets::Socket^>();


TIMER_TickHandler* serverTimer;

System::Byte packetCRC(cli::array<System::Byte>^ buffer) {
	System::Byte tmpCRC = 0;

	for each(System::Byte data in buffer)
	{
		tmpCRC ^= data;
	}

	return tmpCRC;
}


System::Byte packetCRC(System::Byte const * const buffer, const int bufferLength) {
	System::Byte tmpCRC = 0;

	for(int i=0; i<bufferLength; ++i)
	{
		tmpCRC ^= buffer[i];
	}

	return tmpCRC;
}

System::Byte packetCRC(cli::array<System::Byte>^ buffer, const int bufferLength)
{
	System::Byte tmpCRC = 0;

	for (int i = 0; i<bufferLength; ++i)
	{
		tmpCRC ^= buffer[i];
	}

	return tmpCRC;
}

static void sendIPXPacket(cli::array<System::Byte>^ buffer) {
	System::UInt16 srcport, destport;
	System::UInt32 srchost, desthost;
	System::UInt16 i;
	int result;

	{
		// Will modify packet
		pin_ptr<System::Byte> p = &buffer[0];
		System::Byte* nativePtr = p;


		IPXHeader *tmpHeader;
		tmpHeader = (IPXHeader *)nativePtr;

		srchost = tmpHeader->src.addr.byIP.host;
		desthost = tmpHeader->dest.addr.byIP.host;

		srcport = tmpHeader->src.addr.byIP.port;
		destport = tmpHeader->dest.addr.byIP.port;
	}

	if (desthost == 0xffffffff) {
		// Broadcast
		for (i = 0; i < ipconn->Length; i++) {
			if (connBuffer[i].connected && ((ipconn[i]->host != srchost) || (ipconn[i]->port != srcport))) {
				ipxServerSocket->SendTo(buffer, gcnew System::Net::IPEndPoint(ipconn[i]->host, ipconn[i]->port));
				//LOG_MSG("IPXSERVER: Packet of %d bytes sent from %d.%d.%d.%d to %d.%d.%d.%d (BROADCAST) (%x CRC)", bufSize, CONVIP(srchost), CONVIP(ipconn[i].host), packetCRC(&buffer[30], bufSize-30));
			}
		}
	}
	else {
		// Specific address
		for(int i = 0; i < ipconn->Length; ++i) {
			if (connBuffer[i].connected && ((ipconn[i]->host == srchost) || (ipconn[i]->port == srcport))) {
				ipxServerSocket->SendTo(buffer, gcnew System::Net::IPEndPoint(ipconn[i]->host, ipconn[i]->port));
				//LOG_MSG("IPXSERVER: Packet sent from %d.%d.%d.%d to %d.%d.%d.%d", CONVIP(srchost), CONVIP(desthost));
			}
		}
	}
}

bool IPX_isConnectedToServer(int tableNum, IPaddress^% ptrAddr) {
	if (tableNum >= SOCKETTABLESIZE) return false;
	ptrAddr = ipconn[tableNum];
	return connBuffer[tableNum].connected;
}

static void ackClient(IPaddress^ clientAddr) {
	IPXHeader regHeader;
	int result;

	SDLNet_Write16(0xffff, regHeader.checkSum);
	SDLNet_Write16(sizeof(regHeader), regHeader.length);

	SDLNet_Write32(0, regHeader.dest.network);
	PackIP(clientAddr, &regHeader.dest.addr.byIP);
	SDLNet_Write16(0x2, regHeader.dest.socket);

	SDLNet_Write32(1, regHeader.src.network);
	PackIP(ipxServerIp, &regHeader.src.addr.byIP);
	SDLNet_Write16(0x2, regHeader.src.socket);
	regHeader.transControl = 0;

	
	cli::array<System::Byte>^ packet = gcnew cli::array<System::Byte>(sizeof(regHeader));
	{
		pin_ptr<System::Byte> pinPtrArray = &packet[0];

		System::Byte* np = pinPtrArray;   // pointer to the first element in arr  
		memcpy(np, &regHeader, sizeof(regHeader));
	}

	// Send registration string to client.  If client doesn't get this, client will not be registered
	ipxServerSocket->SendTo(packet, gcnew System::Net::IPEndPoint(clientAddr->host, clientAddr->port));
}

static void IPX_ServerLoop() {
	IPaddress^ tmpAddr;

	//char regString[] = "IPX Register\0";
	System::Net::EndPoint^ endpoint;
	int numberOfBytesRead = ipxServerSocket->ReceiveFrom(inBuffer, endpoint);

	if (endpoint->AddressFamily != System::Net::Sockets::AddressFamily::InterNetwork)
	{
		throw gcnew System::Exception("Unknown address family");
	}
	System::Net::IPEndPoint^ ipendpoint = (System::Net::IPEndPoint^) endpoint;

	IPaddress ^inaddress = gcnew IPaddress();
	inaddress->host = ipendpoint->Address->Address;
	inaddress->port = ipendpoint->Port;

	cli::array<System::Byte>^ readBytes = gcnew cli::array<System::Byte>(numberOfBytesRead);
	inBuffer->CopyTo(readBytes, numberOfBytesRead);

	// Check to see if incoming packet is a registration packet
	// For this, I just spoofed the echo protocol packet designation 0x02
	pin_ptr<System::Byte> p = &inBuffer[0];
	void* np = p;
	IPXHeader *tmpHeader = static_cast<IPXHeader *>(np);

	// Check to see if echo packet
	if (SDLNet_Read16(tmpHeader->dest.socket) == 0x2) {
		// Null destination node means its a server registration packet
		if (tmpHeader->dest.addr.byIP.host == 0x0) {
			UnpackIP(tmpHeader->src.addr.byIP, tmpAddr);
			for (int i = 0; i < SOCKETTABLESIZE; i++) {
				if (!connBuffer[i].connected) {
					// Use prefered host IP rather than the reported source IP
					// It may be better to use the reported source
					ipconn[i] = inaddress;

					connBuffer[i].connected = true;
					System::UInt32 host = ipconn[i]->host;
					LOG_MSG("IPXSERVER: Connect from %d.%d.%d.%d", CONVIP(host));
					ackClient(ipconn[i]);
					return;
				}
				else {
					if ((ipconn[i]->host == tmpAddr->host) && (ipconn[i]->port == tmpAddr->port)) {

						LOG_MSG("IPXSERVER: Reconnect from %d.%d.%d.%d", CONVIP(tmpAddr->host));
						// Update anonymous port number if changed
						ipconn[i]->port = inaddress->port;
						ackClient(inaddress);
						return;
					}
				}

			}
		}
	}

	// IPX packet is complete.  Now interpret IPX header and send to respective IP address
	sendIPXPacket(readBytes);
}

void IPX_StopServer() {
	TIMER_DelTickHandler(&IPX_ServerLoop);
	ipxServerSocket->Close();
}

bool IPX_StartServer(System::UInt16 portnum) {
	System::UInt16 i;

	ipxServerSocket->Bind(gcnew System::Net::IPEndPoint(ipxServerIp->host, portnum));

	for (i = 0; i < SOCKETTABLESIZE; i++) 
	{
		connBuffer[i].connected = false;
	}

	TIMER_AddTickHandler(&IPX_ServerLoop);
	return true;
}


#endif
