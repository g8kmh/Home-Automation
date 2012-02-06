/*
  ||
  || @file 	xAPEther2.cpp
  || @version	1.3
  || @author	Brett England/Lehane Kellett
  || @contact	lehane@mm-wave.com
  ||
  || @description
  || | Provide an xAP capabilities for Nuelectronics ethernet module
  || #
  ||
  || @license
  || | This library is free software; you can redistribute it and/or
  || | modify it under the terms of the GNU Lesser General Public
  || | License as published by the Free Software Foundation; version
  || | 2.1 of the License.
  || |
  || | This library is distributed in the hope that it will be useful,
  || | but WITHOUT ANY WARRANTY; without even the implied warranty of
  || | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  || | Lesser General Public License for more details.
  || |
  || | You should have received a copy of the GNU Lesser General Public
  || | License along with this library; if not, write to the Free Software
  || | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  || #
  ||
*/
#include <EtherShield.h>
#include <BufferFiller.h>
#include "xAPEther2.h"


// Defaults if using an empty constructor.
static byte mymac[6] = { 0x54,0x55,0x58,0x10,0x00,0x26 };
static byte myip[4] = { 192,168,1,15 };

XapEther2::XapEther2(void) : XapClass() {
}



XapEther2::XapEther2(char *source, char *uid) : XapClass(source,uid) {
}

void XapEther2::setBuffer(byte *buf, word len) {
  xapbuf = buf;
  xapbuflen = len;
}

void XapEther2::setUID(char *uid) {
     UID = uid;
}
void XapEther2::setSOURCE(char *source) {
     SOURCE = source;

}
void XapEther2::setUPTIME(int *uptime) {
     UPTIME = uptime;

}


char *XapEther2::getUID(void) {
     return (char *)UID;
}
char *XapEther2::getSOURCE(void) {
     return (char *)SOURCE;
}
/*********************************************************
Process incoming packets
This routine needs to be fast at rejecting non xAP packets
as the promiscous mode of the ENC28J60 is going to be throwing
packets at this routine....
**********************************************************/
void XapEther2::process(word len, void (*xapCallback)()) {
  // UDP port 3639 - 0x0e37
	if (len ==0 || xapbuf[IP_PROTO_P] != IP_PROTO_UDP_V ) return; // ASAP before processing further
	if(xapCallback && xapbuf[UDP_DST_PORT_H_P] == 0x0e && xapbuf[UDP_DST_PORT_L_P] == 0x37) {
		word udp_data_len = (((word)xapbuf[UDP_LEN_H_P]) << 8 | xapbuf[UDP_LEN_L_P]) - UDP_HEADER_LEN;
		if(parseMsg(udpOffset(xapbuf), udp_data_len)) {
			(*xapCallback)();
		}
	}
  //heartbeat(); Now called from Timer 2
}

void XapEther2::sendHeartbeat(void) {
  // What's going on here?
  // We push the UDP data directly into the buffer
  // then create the PACKET datagram around it and transmit.
  bfill = udpOffset(xapbuf);
#ifdef XAP_VERSION_1.3
  bfill.emit_p(PSTR("xap-hbeat\n{\nv=13\nhop=1\nuid=$S\nsource=$S\ninterval=$D\nport=3639\nclass=xap-hbeat.alive\n}\nheartbeat\n{\nuptime=$D\n}\n"), UID, SOURCE, (int)(XAP_HEARTBEAT/1000), *UPTIME);
#else
  bfill.emit_p(PSTR("xap-hbeat\n{\nv=12\nhop=1\nuid=$S\nsource=$S\ninterval=$D\nport=3639\nclass=xap-hbeat.alive\n}"
  ), UID, SOURCE, XAP_HEARTBEAT/1000);
#endif
  es.ES_send_udp_broadcast(xapbuf, bfill.position(), 3639, 3639);
}

uint8_t* XapEther2::udpOffset(uint8_t *buf) 
		{ return buf + 0x2a; } // UDP_DATA_P

uint8_t* XapEther2::tcpOffset(uint8_t* buf)
		{ return buf + 0x36; }

