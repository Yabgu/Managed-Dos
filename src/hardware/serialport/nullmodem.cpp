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

/* $Id: nullmodem.cpp,v 1.8 2009-09-25 23:40:47 h-a-l-9000 Exp $ */

#include "dosbox.h"

#if C_MODEM

#include "control.h"
#include "serialport.h"
#include "nullmodem.h"

CNullModem::CNullModem(unsigned id, CommandLine* cmd):CSerial (id, cmd) {
	unsigned temptcpport=23;
	memset(&telClient, 0, sizeof(telClient));
	InstallationSuccessful = false;
	serversocket = 0;
	clientsocket = 0;
	serverport = 0;
	clientport = 0;

	rx_retry = 0;
	rx_retry_max = 20;
	rx_state=N_RX_DISC;

	tx_gather = 12;
	
	dtrrespect=false;
	tx_block=false;
	receiveblock=false;
	transparent=false;
	telnet=false;
	
	unsigned bool_temp=0;

	// usedtr: The nullmodem will
	// 1) when it is client connect to the server not immediately but
	//    as soon as a modem-aware application is started (DTR is switched on).
	// 2) only receive data when DTR is on.
	if(getunsignedSubstring("usedtr:", &bool_temp, cmd)) {
		if(bool_temp==1) {
			dtrrespect=true;
			transparent=true;
		}
	}
	// transparent: don't add additional handshake control.
	if(getunsignedSubstring("transparent:", &bool_temp, cmd)) {
		if(bool_temp==1) transparent=true;
		else transparent=false;
	}
	// telnet: interpret telnet commands.
	if(getunsignedSubstring("telnet:", &bool_temp, cmd)) {
		if(bool_temp==1) {
			transparent=true;
			telnet=true;
		}
	}
	// rxdelay: How many milliseconds to wait before causing an
	// overflow when the application is unresponsive.
	if(getunsignedSubstring("rxdelay:", &rx_retry_max, cmd)) {
		if(!(rx_retry_max<=10000)) {
			rx_retry_max=50;
		}
	}
	// txdelay: How many milliseconds to wait before sending data.
	// This reduces network overhead quite a lot.
	if(getunsignedSubstring("txdelay:", &tx_gather, cmd)) {
		if(!(tx_gather<=500)) {
			tx_gather=12;
		}
	}
	// port is for both server and client
	if(getunsignedSubstring("port:", &temptcpport, cmd)) {
		if(!(temptcpport>0&&temptcpport<65536)) {
			temptcpport=23;
		}
	}
	// socket inheritance
	if(getunsignedSubstring("inhsocket:", &bool_temp, cmd)) {
		LOG_MSG("Serial%d: socket inheritance not supported on this platform.", COMNUMBER);
		return;
	}
	std::string tmpstring;
	if (cmd->FindStringBegin("server:",tmpstring,false)) {
		// we are a client
		const char* hostnamechar=tmpstring.c_str();
		size_t hostlen=strlen(hostnamechar)+1;
		if(hostlen>sizeof(hostnamebuffer)) {
			hostlen=sizeof(hostnamebuffer);
			hostnamebuffer[sizeof(hostnamebuffer)-1]=0;
		}
		memcpy(hostnamebuffer,hostnamechar,hostlen);
		clientport=(System::UInt16)temptcpport;
		if(dtrrespect) {
			// we connect as soon as DTR is switched on
			setEvent(SERIAL_NULLMODEM_DTR_EVENT, 50);
			LOG_MSG("Serial%d: Waiting for DTR...",COMNUMBER);
		} else ClientConnect();
	} else {
		// we are a server
		serverport = (System::UInt16)temptcpport;
		serversocket = new TCPServerSocket(serverport);
		if(!serversocket->IsOpen()) return;
		LOG_MSG("Serial%d: Nullmodem server waiting for connection on port %d...",
			COMNUMBER,serverport);
		setEvent(SERIAL_SERVER_POLLING_EVENT, 50);
	}

	// ....

	CSerial::Init_Registers ();
	InstallationSuccessful = true;

	setCTS(dtrrespect||transparent);
	setDSR(dtrrespect||transparent);
	setRI (false);
	setCD (dtrrespect);
}

CNullModem::~CNullModem () {
	if(serversocket) delete serversocket;
	if(clientsocket) delete clientsocket;
	// remove events
	for(System::UInt16 i = SERIAL_BASE_EVENT_COUNT+1;
			i <= SERIAL_NULLMODEM_EVENT_COUNT; i++) {
		removeEvent(i);
	}
}

void CNullModem::WriteChar(System::Byte data) {
	
	if(clientsocket)clientsocket->SendByteBuffered(data);
	if(!tx_block) {
		//LOG_MSG("setevreduct");
		setEvent(SERIAL_TX_REDUCTION, (float)tx_gather);
		tx_block=true;
	}
}

int CNullModem::readChar() {
	
	int rxchar = clientsocket->GetcharNonBlock();
	if(telnet && rxchar>=0) return TelnetEmulation((System::Byte)rxchar);
	else if(rxchar==0xff && !transparent) {// escape char
		// get the next char
		int rxchar = clientsocket->GetcharNonBlock();
		if(rxchar==0xff) return rxchar; // 0xff 0xff -> 0xff was meant
		rxchar&0x1? setCTS(true) : setCTS(false);
		rxchar&0x2? setDSR(true) : setDSR(false);
		if(rxchar&0x4) receiveByteEx(0x0,0x10);
		return -1;	// no "payload" received
	} else return rxchar;
}

void CNullModem::ClientConnect(){
	System::Byte peernamebuf[16];
	clientsocket = new TCPClientSocket((char*)hostnamebuffer,
										(System::UInt16)clientport); 
	if(!clientsocket->IsOpen()) {
		LOG_MSG("Serial%d: Connection failed.",idnumber+1);
		delete clientsocket;
		clientsocket=0;
		return;
	}
	clientsocket->SetSendBufferSize(256);
	System::String^ peerName = clientsocket->GetRemoteAddressString();
	// transmit the line status
	if(!transparent) setRTSDTR(getRTS(), getDTR());
	rx_state=N_RX_IDLE;
	//LOG_MSG("Serial%d: Connected to %s", idnumber+1, peernamebuf);
	setEvent(SERIAL_POLLING_EVENT, 1);
}

void CNullModem::Disconnect() {
	removeEvent(SERIAL_POLLING_EVENT);
	removeEvent(SERIAL_RX_EVENT);
	// it was disconnected; free the socket and restart the server socket
	LOG_MSG("Serial%d: Disconnected.",COMNUMBER);
	delete clientsocket;
	clientsocket=0;
	setDSR(false);
	setCTS(false);
	if(serverport) {
		serversocket = new TCPServerSocket(serverport);
		if(serversocket->IsOpen()) 
			setEvent(SERIAL_SERVER_POLLING_EVENT, 50);
		else delete serversocket;
	}
}

void CNullModem::handleUpperEvent(System::UInt16 type) {
	
	switch(type) {
		case SERIAL_POLLING_EVENT: {
			// periodically check if new data arrived, disconnect
			// if required. Add it back.
			setEvent(SERIAL_POLLING_EVENT, 1.0f);
			// update Modem input line states
			updateMSR();
			switch(rx_state) {
				case N_RX_IDLE:
					if(CanReceiveByte()) {
						if(doReceive()) {
							// a byte was received
							rx_state=N_RX_WAIT;
							setEvent(SERIAL_RX_EVENT, bytetime*0.9f);
						} // else still idle
					} else {
#if SERIAL_DEBUG
						log_ser(dbg_aux,"Nullmodem: block on polling.");
#endif
						rx_state=N_RX_BLOCKED;
						// have both delays (1ms + bytetime)
						setEvent(SERIAL_RX_EVENT, bytetime*0.9f);
					}
					break;
				case N_RX_BLOCKED:
                    // one timeout tick
					if(!CanReceiveByte()) {
						rx_retry++;
						if(rx_retry>=rx_retry_max) {
							// it has timed out:
							rx_retry=0;
							removeEvent(SERIAL_RX_EVENT);
							if(doReceive()) {
								// read away everything
								while(doReceive());
								rx_state=N_RX_WAIT;
								setEvent(SERIAL_RX_EVENT, bytetime*0.9f);
							} else {
								// much trouble about nothing
                                rx_state=N_RX_IDLE;
#if SERIAL_DEBUG
								log_ser(dbg_aux,"Nullmodem: unblock due to no more data",rx_retry);
#endif
							}
						} // else wait further
					} else {
						// good: we can receive again
						removeEvent(SERIAL_RX_EVENT);
						rx_retry=0;
						if(doReceive()) {
							rx_state=N_RX_FASTWAIT;
							setEvent(SERIAL_RX_EVENT, bytetime*0.65f);
						} else {
							// much trouble about nothing
							rx_state=N_RX_IDLE;
						}
					}
					break;

				case N_RX_WAIT:
				case N_RX_FASTWAIT:
					break;
			}
			break;
		}
		case SERIAL_RX_EVENT: {
			switch(rx_state) {
				case N_RX_IDLE:
					LOG_MSG("internal error in nullmodem");
					break;

				case N_RX_BLOCKED: // try to receive
				case N_RX_WAIT:
				case N_RX_FASTWAIT:
					if(CanReceiveByte()) {
						// just works or unblocked
						if(doReceive()) {
							rx_retry=0; // not waiting anymore
							if(rx_state==N_RX_WAIT) setEvent(SERIAL_RX_EVENT, bytetime*0.9f);
							else {
								// maybe unblocked
								rx_state=N_RX_FASTWAIT;
								setEvent(SERIAL_RX_EVENT, bytetime*0.65f);
							}
						} else {
							// didn't receive anything
							rx_retry=0;
							rx_state=N_RX_IDLE;
						}
					} else {
						// blocking now or still blocked
#if SERIAL_DEBUG
						if(rx_state==N_RX_BLOCKED)
							log_ser(dbg_aux,"Nullmodem: rx still blocked (retry=%d)",rx_retry);
						else log_ser(dbg_aux,"Nullmodem: block on continued rx (retry=%d).",rx_retry);
#endif
						setEvent(SERIAL_RX_EVENT, bytetime*0.65f);
						rx_state=N_RX_BLOCKED;
					}

					break;
			}
			break;
		}
		case SERIAL_TX_EVENT: {
			// Maybe echo cirquit works a bit better this way
			if(rx_state==N_RX_IDLE && CanReceiveByte() && clientsocket) {
				if(doReceive()) {
					// a byte was received
					rx_state=N_RX_WAIT;
					setEvent(SERIAL_RX_EVENT, bytetime*0.9f);
				}
			}
			ByteTransmitted();
			break;
		}
		case SERIAL_THR_EVENT: {
			ByteTransmitting();
			// actually send it
			setEvent(SERIAL_TX_EVENT,bytetime+0.01f);
			break;				   
		}
		case SERIAL_SERVER_POLLING_EVENT: {
			// As long as nothing is connected to our server poll the
			// connection.
			clientsocket=serversocket->Accept();
			if(clientsocket) {
				//System::Byte peeripbuf[16];
				auto peeripbuf = clientsocket->GetRemoteAddressString();
				//LOG_MSG("Serial%d: A client (%s) has connected.",COMNUMBER,peeripbuf);
#if SERIAL_DEBUG
				log_ser(dbg_aux,"Nullmodem: A client (%s) has connected.", peeripbuf);
#endif// new socket found...
				clientsocket->SetSendBufferSize(256);
				rx_state=N_RX_IDLE;
				setEvent(SERIAL_POLLING_EVENT, 1);
				
				// we don't accept further connections
				delete serversocket;
				serversocket=0;

				// transmit the line status
				setRTSDTR(getRTS(), getDTR());
			} else {
				// continue looking
				setEvent(SERIAL_SERVER_POLLING_EVENT, 50);
			}
			break;
		}
		case SERIAL_TX_REDUCTION: {
			// Flush the data in the transmitting buffer.
			if(clientsocket) clientsocket->FlushBuffer();
			tx_block=false;
			break;						  
		}
		case SERIAL_NULLMODEM_DTR_EVENT: {
			if(getDTR()) ClientConnect();
			else setEvent(SERIAL_NULLMODEM_DTR_EVENT,50);
			break;
		}
	}
}

/*****************************************************************************/
/* updatePortConfig is called when emulated app changes the serial port     **/
/* parameters baudrate, stopint, number of dataint, parity.               **/
/*****************************************************************************/
void CNullModem::updatePortConfig (System::UInt16 /*divider*/, System::Byte /*lcr*/) {
	
}

void CNullModem::updateMSR () {
	
}

bool CNullModem::doReceive () {
		int rxchar = readChar();
		if(rxchar>=0) {
			receiveByteEx((System::Byte)rxchar,0);
			return true;
		}
		else if(rxchar==-2) {
			Disconnect();
		}
		return false;
}
 
void CNullModem::transmitByte (System::Byte val, bool first) {
 	// transmit it later in THR_Event
	if(first) setEvent(SERIAL_THR_EVENT, bytetime/8);
	else setEvent(SERIAL_TX_EVENT, bytetime);

	// disable 0xff escaping when transparent mode is enabled
	if (!transparent && (val==0xff)) WriteChar(0xff);
	
	WriteChar(val);
}

int CNullModem::TelnetEmulation(System::Byte data) {
	System::Byte response[3];
	if(telClient.inIAC) {
		if(telClient.recCommand) {
			if((data != 0) && (data != 1) && (data != 3)) {
				LOG_MSG("Serial%d: Unrecognized telnet option %d",COMNUMBER, data);
				if(telClient.command>250) {
					/* Reject anything we don't recognize */
					response[0]=0xff;
					response[1]=252;
					response[2]=data; /* We won't do crap! */
					if(clientsocket) clientsocket->SendArray(response, 3);
				}
			}
			switch(telClient.command) {
				case 251: /* Will */
					if(data == 0) telClient.binary[TEL_SERVER] = true;
					if(data == 1) telClient.echo[TEL_SERVER] = true;
					if(data == 3) telClient.supressGA[TEL_SERVER] = true;
					break;
				case 252: /* Won't */
					if(data == 0) telClient.binary[TEL_SERVER] = false;
					if(data == 1) telClient.echo[TEL_SERVER] = false;
					if(data == 3) telClient.supressGA[TEL_SERVER] = false;
					break;
				case 253: /* Do */
					if(data == 0) {
						telClient.binary[TEL_CLIENT] = true;
							response[0]=0xff;
							response[1]=251;
							response[2]=0; /* Will do binary transfer */
							if(clientsocket) clientsocket->SendArray(response, 3);
					}
					if(data == 1) {
						telClient.echo[TEL_CLIENT] = false;
							response[0]=0xff;
							response[1]=252;
							response[2]=1; /* Won't echo (too lazy) */
							if(clientsocket) clientsocket->SendArray(response, 3);
					}
					if(data == 3) {
						telClient.supressGA[TEL_CLIENT] = true;
							response[0]=0xff;
							response[1]=251;
							response[2]=3; /* Will Suppress GA */
							if(clientsocket) clientsocket->SendArray(response, 3);
					}
					break;
				case 254: /* Don't */
					if(data == 0) {
						telClient.binary[TEL_CLIENT] = false;
						response[0]=0xff;
						response[1]=252;
						response[2]=0; /* Won't do binary transfer */
						if(clientsocket) clientsocket->SendArray(response, 3);
					}
					if(data == 1) {
						telClient.echo[TEL_CLIENT] = false;
						response[0]=0xff;
						response[1]=252;
						response[2]=1; /* Won't echo (fine by me) */
						if(clientsocket) clientsocket->SendArray(response, 3);
					}
					if(data == 3) {
						telClient.supressGA[TEL_CLIENT] = true;
						response[0]=0xff;
						response[1]=251;
						response[2]=3; /* Will Suppress GA (too lazy) */
						if(clientsocket) clientsocket->SendArray(response, 3);
					}
					break;
				default:
					LOG_MSG("MODEM: Telnet client sent IAC %d", telClient.command);
					break;
			}
			telClient.inIAC = false;
			telClient.recCommand = false;
			return -1; //continue;
		} else {
			if(data==249) {
				/* Go Ahead received */
				telClient.inIAC = false;
				return -1; //continue;
			}
			telClient.command = data;
			telClient.recCommand = true;
			
			if((telClient.binary[TEL_SERVER]) && (data == 0xff)) {
				/* Binary data with value of 255 */
				telClient.inIAC = false;
				telClient.recCommand = false;
					return 0xff;
			}
		}
	} else {
		if(data == 0xff) {
			telClient.inIAC = true;
			return -1;
		}
		return data;
	}
	return -1; // ???
}
	

/*****************************************************************************/
/* setBreak(val) switches break on or off                                   **/
/*****************************************************************************/

void CNullModem::setBreak (bool /*value*/) {
	CNullModem::setRTSDTR(getRTS(), getDTR());
}

/*****************************************************************************/
/* updateModemControlLines(mcr) sets DTR and RTS.                           **/
/*****************************************************************************/
void CNullModem::setRTSDTR(bool xrts, bool xdtr) {
	if(!transparent) {
		System::Byte control[2];
		control[0]=0xff;
		control[1]=0x0;
		if(xrts) control[1]|=1;
		if(xdtr) control[1]|=2;
		if(LCR&LCR_BREAK_MASK) control[1]|=4;
		if(clientsocket) clientsocket->SendArray(control, 2);
	}
}
void CNullModem::setRTS(bool val) {
	setRTSDTR(val, getDTR());
}
void CNullModem::setDTR(bool val) {
	setRTSDTR(getRTS(), val);
}
#endif
