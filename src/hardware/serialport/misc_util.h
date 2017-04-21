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

 /* $Id: misc_util.h,v 1.5 2009-09-25 23:40:47 h-a-l-9000 Exp $ */

#ifndef SDLNETWRAPPER_H
#define SDLNETWRAPPER_H

#ifndef DOSBOX_DOSBOX_H
#include "dosbox.h"
#endif

#if C_MODEM

# ifndef DOSBOX_SUPPORT_H
#include "support.h"
#endif

// Netwrapper Capabilities
#define NETWRAPPER_TCP 1
#define NETWRAPPER_TCP_NATIVESOCKET 2

#if defined WIN32
#include <winsock2.h>
#include <ws2tcpip.h> //for socklen_t
//typedef int  socklen_t;

//Tests for BSD/OS2/LINUX
#elif defined HAVE_STDLIB_H && defined HAVE_SYS_TYPES_H && defined HAVE_SYS_SOCKET_H && defined HAVE_NETINET_IN_H
#define NATIVESOCKETS
#define SOCKET int
#include <stdio.h> //darwin
#include <stdlib.h> //darwin
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//socklen_t should be handled by configure
#endif

#ifdef NATIVESOCKETS
#define CAPWORD (NETWRAPPER_TCP|NETWRAPPER_TCP_NATIVESOCKET)
#else
#define CAPWORD NETWRAPPER_TCP
#endif

#include "SDL_net.h"
#include <vcclr.h>

System::UInt32 Netwrapper_GetCapabilities();

class TCPClientSocket {
public:

	TCPClientSocket(System::Net::Sockets::TcpClient^ connected)
	{
		client = connected;
	}

	TCPClientSocket(const char* destination, System::UInt16 port)
	{
		client = gcnew System::Net::Sockets::TcpClient(gcnew System::String(destination), port);
	}

	// return:
	// -1: no data
	// -2: socket closed
	// >0: data char
	int GetcharNonBlock()
	{
		if (static_cast<System::Net::Sockets::TcpClient^>(client) == nullptr || !client->Connected)
		{
			return -2;
		}
		else if (client->Available == 0)
		{
			return -1;
		}
		return 0;
	}

	bool Putchar(System::Byte data)
	{
		client->GetStream()->WriteByte(data);
		return true;
	}

	bool SendArray(System::Byte* data, unsigned bufsize)
	{
		for (unsigned i = 0; i < bufsize; ++i)
		{
			client->GetStream()->WriteByte(data[i]);
		}
		return true;
	}

	bool ReceiveArray(System::Byte* data, unsigned* size)
	{
		unsigned avail = client->Available;
		if (avail > 0)
		{
			if (avail > *size) {
				*size = avail;
			}
			for (unsigned i = 0; i < avail; ++i)
			{
				data[i] = client->GetStream()->ReadByte();
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	System::String^ GetRemoteAddressString()
	{
		System::Net::IPEndPoint^ endpoint = (System::Net::IPEndPoint^)client->Client->RemoteEndPoint;
		return endpoint->ToString();
	}

	void FlushBuffer()
	{
		client->GetStream()->Flush();
	}

	void SetSendBufferSize(unsigned bufsize)
	{
		client->SendBufferSize = bufsize;
		client->ReceiveBufferSize = bufsize;
	}

	// buffered send functions
	bool SendByteBuffered(System::Byte data)
	{
		return Putchar(data);
	}

	bool SendArrayBuffered(System::Byte* data, unsigned bufsize)
	{
		return SendArray(data, bufsize);
	}

	bool IsOpen()
	{
		return client->Connected;
	}

private:
	gcroot<System::Net::Sockets::TcpClient^> client;
};

class TCPServerSocket {
public:
	TCPServerSocket(System::UInt16 port)
	{
		server = gcnew System::Net::Sockets::TcpListener(port);
	}
	
	TCPClientSocket* Accept()
	{
		return new TCPClientSocket(server->AcceptTcpClient());
	}

	bool IsOpen()
	{
		return server->Server->Connected;
	}

private:
	gcroot<System::Net::Sockets::TcpListener^> server;
};


#endif //C_MODEM

#endif //# SDLNETWRAPPER_H
