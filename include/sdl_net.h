
#pragma once
#include <vcclr.h>

public ref class IPaddress
{
	System::UInt32 m_host;			/* 32-bit IPv4 host address */
	System::UInt16 m_port;			/* 16-bit protocol port */
public:


	property System::UInt32 host
	{
		System::UInt32 get()
		{
			return m_host;
		}

		void set(System::UInt32 value)
		{
			m_host = value;
		}
	}

	property System::UInt16 port
	{
		System::UInt16 get()
		{
			return m_port;
		}

		void set(System::UInt16 value)
		{
			m_port = value;
		}
	}
};

class UDPpacket {
public:
	UDPpacket()
		: address(gcnew IPaddress)
	{

	}

	int channel;		/* The src/dst channel of the packet */
	gcroot<cli::array<System::Byte>^> data;		/* The packet data */
	int len;		/* The length of the packet data */
	int maxlen;		/* The size of the data buffer */
	int status;		/* packet status after sending */
	gcroot<IPaddress^> address;		/* The source/dest address of an incoming/outgoing packet */
};

inline void SDLNet_Write16(System::UInt16 value, void *area)
{
	System::UInt16 *hval = static_cast<System::UInt16 *>(area);
	*hval = System::Net::IPAddress::HostToNetworkOrder(value);
}

inline void SDLNet_Write32(System::Int32 value, void *area)
{
	System::Int32 *hval = static_cast<System::Int32*>(area);
	*hval = System::Net::IPAddress::HostToNetworkOrder(value);
}

inline System::UInt16 SDLNet_Read16(void *area)
{
	System::UInt16 *nval = static_cast<System::UInt16 *>(area);
	return System::Net::IPAddress::NetworkToHostOrder(*nval);
}

inline System::Int32 SDLNet_Read32(void *area)
{
	System::Int32 *nval = static_cast<System::Int32*>(area);
	return System::Net::IPAddress::NetworkToHostOrder(*nval);
}
