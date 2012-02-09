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
/*
#include <EtherShield.h>
#include <BufferFiller.h>
*/
#include <EtherCard.h>

#include "xAPEther2.h"

//BufferFiller bfill;
XapEther2::XapEther2(void) : XapClass() {
}



XapEther2::XapEther2(char *source, char *uid) : XapClass(source,uid) {
}

// deprecated as we use Ethernet::buffer
/**************
void XapEther2::setBuffer(byte *buf, word len) {
  Ethernet::buffer = buf;
  Ethernet::bufferlen = len;
}
*************/
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

	if (len ==0 || Ethernet::buffer[IP_PROTO_P] != IP_PROTO_UDP_V ) return; // ASAP before processing further
	if(xapCallback && Ethernet::buffer[UDP_DST_PORT_H_P] == 0x0e && Ethernet::buffer[UDP_DST_PORT_L_P] == 0x37) {
		word udp_data_len = (((word)Ethernet::buffer[UDP_LEN_H_P]) << 8 | Ethernet::buffer[UDP_LEN_L_P]) - UDP_HEADER_LEN;
		if(parseMsg(ether.udpOffset(), udp_data_len)) {
			(*xapCallback)();
		}
	}
  //heartbeat(); Now called from Timer 2
}

void XapEther2::sendHeartbeat(void) {
  // What's going on here?
  // We push the UDP data directly into the buffer
  // then create the PACKET datagram around it and transmit.
  bfill = ether.udpOffset();
#ifdef XAP_VERSION_1.3
  bfill.emit_p(PSTR("xap-hbeat\n{\nv=13\nhop=1\nuid=$S\nsource=$S\ninterval=$D\nport=3639\nclass=xap-hbeat.alive\n}\nheartbeat\n{\nuptime=$D\n}\n"), UID, SOURCE, (int)(XAP_HEARTBEAT/1000), *UPTIME);
#else
  bfill.emit_p(PSTR("xap-hbeat\n{\nv=12\nhop=1\nuid=$S\nsource=$S\ninterval=$D\nport=3639\nclass=xap-hbeat.alive\n}"
  ), UID, SOURCE, XAP_HEARTBEAT/1000);
#endif
  ether.sendUdpBroadcast((char *)ether.udpOffset(), bfill.position(), 3639, 3639);
}

void XapEther2::sendHeartbeat(int freeram) {
  // What's going on here?
  // We push the UDP data directly into the buffer
  // then create the PACKET datagram around it and transmit.
  bfill = ether.udpOffset();
#ifdef XAP_VERSION_1.3
  bfill.emit_p(PSTR("xap-hbeat\n{\nv=13\nhop=1\nuid=$S\nsource=$S\ninterval=$D\nport=3639\nclass=xap-hbeat.alive\n}\nheartbeat\n{\nuptime=$D\nfreeram=$D\n}\n"), UID, SOURCE, (int)(XAP_HEARTBEAT/1000), *UPTIME, (int)freeram);
#else
  bfill.emit_p(PSTR("xap-hbeat\n{\nv=12\nhop=1\nuid=$S\nsource=$S\ninterval=$D\nport=3639\nclass=xap-hbeat.alive\n}"
  ), UID, SOURCE, XAP_HEARTBEAT/1000);
#endif
  ether.sendUdpBroadcast((char *)ether.udpOffset(), bfill.position(), 3639, 3639);
}


