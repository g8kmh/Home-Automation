/*
  ||
  || @file 	xAPEther2.h
  || @version	1.0
  || @author	Brett England
  || @contact	brett@dbzoo.com
  ||
  || @description
  || | Provide an xAP capabilities for NuElectronic ethernet module
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
#ifndef XAPETHER_H
#define XAPETHER_H


#include "xAP.h"

#define XAP_PORT 3639

class XapEther2 : public XapClass {
 public:
  XapEther2();
  XapEther2(char *source, char *uid);

  void setBuffer(byte *buf, word len);
  void setUID(char *uid);
  void setSOURCE(char *source);
  void setUPTIME(int *uptime);
  char *getUID(void);
  char *getSOURCE(void);
  void process(word len, void (*callback)());
  void sendHeartbeat(void);
  void sendHeartbeat(int freeram);
 private:
 byte *xapbuf;
 word xapbuflen;
  uint8_t *udpOffset(uint8_t *buf);

  uint8_t* tcpOffset(uint8_t* buf);

};
#ifdef EtherCard_h
extern EtherCard ether;
#else
extern EtherShield es;
#endif
extern BufferFiller bfill;
#endif
