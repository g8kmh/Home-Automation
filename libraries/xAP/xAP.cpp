/*
  ||
  || @file 	xAP.cpp
  || @version	1.1
  || @author	Brett England
  || @contact	brett@dbzoo.com
  ||
  || @description
  || | Provide an xAP capabilities
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

#include <string.h>
#include "xAP.h"

XapClass::XapClass(char *source, char *uid) {
     SOURCE = source;
     UID = uid;
     resetHeartbeat();
}

XapClass::XapClass(void) {
#ifdef XAP_VERSION_1.3
     XapClass("mm-wave.arduino.demo","FF:A55AEE00.0000");
#else
     XapClass("mm-wave.arduino.demo","FFA55AEE");
#endif
}

void XapClass::heartbeat(void) {
     if (after(heartbeatTimeout)) {      
       sendHeartbeat();
       resetHeartbeat();
     }
}

void XapClass::resetHeartbeat() {
     heartbeatTimeout = smillis() + XAP_HEARTBEAT;
}

int XapClass::after(long timeout)
{
     return smillis()-timeout > 0;
}

int XapClass::getType(void) {
     if (xapMsgPairs==0) return XAP_MSG_NONE;
     if (strcasecmp(xapMsg[0].section,"xap-hbeat")==0) return XAP_MSG_HBEAT;
     if (strcasecmp(xapMsg[0].section,"xap-header")==0) return XAP_MSG_ORDINARY;
     return XAP_MSG_UNKNOWN;
}

char *XapClass::getValue(char *section, char *key) {
     for(int i=0; i<xapMsgPairs; i++) {
	  if(strcasecmp(section, xapMsg[i].section) == 0 && strcasecmp(key, xapMsg[i].key) == 0)
	       return xapMsg[i].value;
     }
     return (char *)NULL;
}

int XapClass::decode_state(char *msg) {
     static const char *value[] = {"on","off","true","false","yes","no","1","0"};
     static const int state[] = {1,0,1,0,1,0,1,0};
     if (msg == NULL) return -1;
     for(int i=0; i < sizeof(value); i++) {
	  if(strcasecmp(msg, value[i]) == 0)
	       return state[i];
     }
     return -1;
}

int XapClass::getState(char *section, char *key) {
     return decode_state(getValue(section, key));
}

int XapClass::isValue(char *section, char *key, char *value) {
     char *kvalue = getValue(section, key);
     return kvalue && strcasecmp(kvalue, value) == 0;
}

void XapClass::rtrim( byte *msg,  byte *p) {
     while(*p < 32 && p > msg)
	  *p-- = '\0';
}

// buf is modified.
int XapClass::parseMsg(byte *msg, int size) {
     enum {
	  START_SECTION_NAME, IN_SECTION_NAME, START_KEYNAME, IN_KEYNAME, START_KEYVALUE, IN_KEYVALUE  
     } state = START_SECTION_NAME;
     char *current_section = NULL;
     xapMsgPairs = 0;
  
     for(byte *buf = msg; buf < msg+size; buf++) {
	  switch (state) {
	  case START_SECTION_NAME:
	       if ( (*buf>32) && (*buf<128) ) {
		    state = IN_SECTION_NAME;
		    current_section = (char *)buf;
	       }
	       break;
	  case IN_SECTION_NAME:
	       if (*buf == '{') {
		    *buf = '\0';
		    rtrim(msg, buf);
		    state = START_KEYNAME;
	       }
	       break;
	  case START_KEYNAME:
	       if (*buf == '}') {
		    state = START_SECTION_NAME;
	       } 
	       else if ((*buf>32) && (*buf<128)) {
		    xapMsg[xapMsgPairs].section = current_section;
		    xapMsg[xapMsgPairs].key = (char *)buf;
		    state = IN_KEYNAME;
	       }               
	       break;
	  case IN_KEYNAME:
	       if ((*buf < 32) || (*buf == '=')) {
		    *buf = '\0';
		    rtrim(msg, buf);
		    state = START_KEYVALUE;
	       }
	       break;
	  case START_KEYVALUE:
	       if ((*buf>32) && (*buf<128)) {
		    state = IN_KEYVALUE;
		    xapMsg[xapMsgPairs].value = (char *)buf;
	       }
	       break;
	  case IN_KEYVALUE:
	       if (*buf < 32) {
		    *buf = '\0';
		    rtrim(msg, buf);
		    state = START_KEYNAME;
		    xapMsgPairs++;
		    if (xapMsgPairs >= MAX_XAP_PAIRS) {
			 xapMsgPairs = 0;
		    }
	       }        
	       break;
	  }
     }
     return xapMsgPairs;
}
/*
 Reconstruct an XAP packet from the parsed components
   and dump it to the serial port.  Useful for debugging.

   
   DISABLED AS DMX USES SERIAL PORT
****/

#ifndef IS_DMX

void XapClass::dumpParsedMsg() {
  char *currentSection = NULL;
  for(int i=0; i < xapMsgPairs; i++) {
    if (currentSection == NULL || currentSection != xapMsg[i].section) {
      if(currentSection != NULL) {
	Serial.println("}");
      }
      Serial.println(xapMsg[i].section);
      Serial.println("{");
      currentSection = xapMsg[i].section;
    }
    Serial.print(xapMsg[i].key);
    Serial.print("=");
    Serial.println(xapMsg[i].value);
  }
  Serial.println("}");
}
#endif

bool XapClass::partialMatchSource(char *source, char *target)
{
// This looks to see if the target up to the : matches with our source address
// 
// quick check for > wildcard
// 
//
//	
//
bool unmatched = false;
	if (*target == '>') return true; // First char is wildcard - OK
    for(; *source != '\0' && *target!='\0' && *target!= ':'; source++, target++)  {  
	   if (tolower((int)*source) != tolower((int)*target)){ // case insensitive match
		   unmatched = true;
		   break;
		   }
	}
	if (!unmatched && (*source == '\0') && ((*target == '\0') || (*target==':'))) return true; //match all the way to the end of the source string delimiter
	
	if (unmatched && (*source != '\0') && ((*target != '\0')|| (*target!=':'))) {
	// there's a difference and we're not at the end of either string (cos if we are it is a fail anyway)
	// move back one to check that we failed after delimeter
	if (*(--target) == '.'){
	// it is so check if the next is a wildcard or >
		target++;
		if (*target == '*'){
			target++; //next in the target which will be . or end of string (so .*. or .*)
			// need to spin down the source to next delimeter (wildcard matches against part)
			while (*source != '.' && *source != '\0') {*source++;}
			// source is now . or end of string
			return (partialMatchSource(source, target));	//recursive
		}
		else if (*target == '>'){return true;} // done 
	}
}
return false;
	
}

