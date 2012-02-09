
/* $Id: xapBSCTEMP.ino mm-wave.com $ 
  xAP 1wire monitoring with Hackspace Nanode

  Requires Arduino IDE 1.0 or up
  
  THIS VERSION FOR ETHERCARD LIBRARY
  
 Arduino 
  GND   ------1+--------\
   A3    ------2+ ds1820 | 
  +5v   ------3+--------/
   
  If the pull up resistor isn't on the board you need a 4k7 between PIN A3 and +5V/VCC
  NOTE: THIS SOFTWARE ASSUMES POWERED DEVICES FOR SPEED
  
  This code is for the Nanode and supports xAP 1.3
  Requires updated xAP libraries, SPIRAM and Ethershield
  
  Lehane Kellett
  lehane@mm-wave.com
  
  Loosely based upon HAH dbzoo.com xAP code
  
  Released under GPL v3.0 
  
  No warranty expressed or implied
  Please feel free to use this code as you see fit but I am unable to answer questions regarding it.
  However, if you make improvements or fix bugs then please publicise them/let me know.
  
  
*/

// CHANGE THE DEBUG FOR YOUR REQUIREMENTS

//#define DEBUG_THIS

#ifdef DEBUG_THIS
	#define BSCTEMP_VERSION "0.951D"
#else
	#define BSCTEMP_VERSION "0.951"
#endif
// TURN THIS OFF FOR FIXED IP AND MAKE SURE YOU UPDATE ALL THE IP DETAILS (AND MAC IF NOT NANODE)
#define USE_DHCP
// TURN THIS OFF IF YOU DO NOT HAVE AN EXTERNAL SRAM OR FRAM (23K256)
#define USE_FRAM

#ifdef DEBUG_THIS
	#define HRS24 60
#else
	#define HRS24 1440
#endif

// TURN THIS OFF FOR ARDUINO (NOT TESTED!)
#define NANODE

#include <EtherCard.h>
#ifdef NANODE
#include <NanodeMAC.h>
#endif
#include <xAP.h>
#include <xAPEther2.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#ifdef USE_FRAM
// YMMV - change if using another form of EEPROM. Good for SRAM and FRAM. Check HOLD =1 is set in SpiRAM.h
#include <SpiRAM.h>
#endif
//#include <avr/pgmspace.h>

int freeram;

/**********************************************************
			xAP LOGO
**********************************************************/
PROGMEM prog_char xaplogo[] ={
0x42, 0x4D, 0x96, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x28, 0x00,
0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80,
0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80,
0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xF8, 0x88, 0xFF, 0xFF, 0xF8, 0x88,
0xF8, 0x88, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xF6, 0x67, 0xFF, 0xFF, 0x8E, 0x68,
0x86, 0x68, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xF8, 0xEE, 0xFF, 0xFF, 0xEE, 0xE8,
0x8E, 0xE8, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xF8, 0xEE, 0x88, 0x88, 0xEE, 0x7F,
0x8E, 0xE8, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xF8, 0xEE, 0x66, 0x66, 0xEE, 0x8F,
0x8E, 0xE8, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0x8F,
0x8E, 0xEE, 0x88, 0x8F, 0xFF, 0xFE, 0x8F, 0x87, 0x78, 0xF8, 0x77, 0x7E, 0xEE, 0xEE, 0xEE, 0x8F,
0x8E, 0xEE, 0xE6, 0x68, 0xFF, 0xFE, 0x8F, 0xF7, 0x67, 0xF7, 0x68, 0x8E, 0xE8, 0xFE, 0xE7, 0xFF,
0x8E, 0xEE, 0xEE, 0xE6, 0x8F, 0xFE, 0x8F, 0xF8, 0x66, 0xE6, 0x6F, 0x8E, 0xE8, 0xF6, 0xE8, 0xFF,
0x8E, 0xEE, 0x8E, 0xEE, 0x6F, 0xFE, 0x8F, 0xFF, 0x8E, 0xE6, 0x8F, 0xFE, 0xE7, 0x8E, 0xE8, 0xFF,
0x8E, 0xE7, 0xFF, 0xEE, 0xE8, 0xFE, 0x8F, 0xFF, 0x86, 0xEE, 0xFF, 0xF8, 0xE6, 0x8E, 0xE8, 0xFF,
0x8E, 0xE7, 0xFF, 0x8E, 0xE8, 0xFE, 0x8F, 0xFF, 0xF6, 0xEE, 0xFF, 0xF8, 0xEE, 0xEE, 0x7F, 0xFF,
0x8E, 0xE8, 0xFF, 0x8E, 0xE8, 0xFE, 0x8F, 0xFF, 0x86, 0xE6, 0x8F, 0xF8, 0xEE, 0xEE, 0x8F, 0xFF,
0x8E, 0xE8, 0xF8, 0xEE, 0xE8, 0xFE, 0x8F, 0xF8, 0x66, 0xE6, 0x68, 0xFF, 0xEE, 0xEE, 0x8F, 0xFF,
0x8E, 0xE6, 0x76, 0xEE, 0xEF, 0xFE, 0x8F, 0xFE, 0x67, 0xF6, 0x68, 0xFF, 0xEE, 0xEE, 0x8F, 0xFF,
0x8E, 0xEE, 0xEE, 0xEE, 0x8F, 0xFE, 0x8F, 0xF7, 0x88, 0xF8, 0x77, 0xFF, 0x8E, 0xE6, 0xFF, 0xFF,
0x86, 0xEE, 0xE6, 0xE8, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF,
0xF8, 0x88, 0x88, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};


// ethernet interface mac address
#ifdef NANODE
static uint8_t mymac[6] = { 0,0,0,0,0,0 };
#else
static uint8_t mymac[6] = { 0x54,0x55,0x58,0x12,0x34,0x56 }; // CHANGE THIS FOR YOUR NETWORK MAC ADDRESS - MUST BE UNIQUE PER DEVICE
#endif
static uint8_t myip[4] = { 192,168,0,55 };
static uint8_t mynetmask[4] = { 255,255,255,0 };

// IP address of the host being queried to contact (IP of the first portion of the URL):
//static uint8_t websrvip[4] = { 0, 0, 0, 0 };

// Default gateway. The ip address of your DSL router. It can be set to the same as
// websrvip the case where there is no default GW to access the 
// web server (=web server is on the same lan as this host) 
static uint8_t gwip[4] = { 192,168,0,1};

static uint8_t dnsip[4] = { 0,0,0,0 };
static uint8_t dhcpsvrip[4] = { 0,0,0,0 };
// Semaphore
byte semaphore = 0;
enum semaphores { RENEWDHCP =0x01, SENDBSCINFO = 0x02, UPDATETEMPERATURES = 0x04};

/* The following are the address offsets and lengths for each of the variables stored in FRAM (or SRAM)
   in order to free up SRAM onchip
*/

// Counter
int renewDHCPcount = HRS24 ; //24 hrs in minutes
int uptimeDays = 0;


#ifdef DEBUG_THIS
char DEBUG_THIS_MESSAGE[33]="";
#endif

/***********************************************************
 XAP Identification
 xAP Version is in the xap header file
***********************************************************/
#define UID "FFA55700"
#define UID_NETWORK "FF"
#ifdef XAP_VERSION_1.3
	#define UID_VENDOR "A5575A00"
char base_UID[17];
#else
	#define UID_VENDOR "A557"
char base_UID[9];
#endif
char xAPSource[33]; // yes, yes, I know the spec says 128 - go ahead, change it, make my day :-)
#define SOURCE_VENDOR "mmwave.bsctemp"
// can reconfigure instance if using external memory to a max of 16 chars
#define INSTANCE_DEFAULT  "instance" 



/****************************************************
PIN DEFINITIONS
*****************************************************/


// PIN A3 Analog 3
#define ONE_WIRE_BUS A3

// SPI chip select pin 9 (B1)
#define SS_PIN 9
// Ethernet Pin
#define ENET_PIN 8
/******************************
Resolution 				9 bit 	10 bit 	11 bit 	12 bit
Conversion Time (ms) 	93.75 	187.5 	375 	750
LSB (°C) 				0.5 	0.25 	0.125 	0.0625
******************************/
#define TEMPERATURE_PRECISION 11
#define CONVERSION_TIME 375
#define TEMP_LSB 0.125
// Change the above as required


#ifdef USE_FRAM
#define MAXSENSORS 16
#else
#define MAXSENSORS 4
#endif



// listen port for tcp/www:
#define MYWWWPORT 80

/**********************************************************
TIMER 2
**********************************************************/
/* Timer2 reload value, globally available */  
unsigned int tcnt2;  
  
/* Toggle HIGH or LOW digital write */  
int toggle = 0;  
int tmx;

/************************************************************
Main packet buffer - as big as possible
************************************************************/
#define BUFFER_SIZE 810
byte Ethernet::buffer[BUFFER_SIZE+1];

//static uint8_t buf[BUFFER_SIZE+1];

//byte *xapbuf;
//word xapbuflen;

#ifdef NANODE
NanodeMAC mac( mymac ); // Go get the MAC from the chip
#endif
//EtherShield es=EtherShield();
BufferFiller bfill;
XapEther2 xap;
//XapEther2 xap(SOURCE, UID);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

byte numberOfDevices; // Number of temperature devices found
char tempS[10];
char tempFS[10];

// Minimum difference for the change in xAP BSC state
float xAPDelta = 0.25;

struct _sensor {     
  DeviceAddress deviceAddress; // this is a uint8_t[8] array
  float temp;
#define SIZEOF_SENSOR 12
#ifdef USE_FRAM
  float setpoint; // Value to change from off to on
  char xAP_name[17]; // otherwise will send device number
				// 33 bytes for the sensor array
#define SIZEOF_SENSOR 33
#endif
}
#ifdef USE_FRAM
	sensor; // one ram copy - rest in FRAM
#else
	sensor[MAXSENSORS];
#endif
/**************************************************************
FRAM or EEPROM offsets (mini FS)
**************************************************************/

// This is the offset in FRAM for the arrays to start at 
#define SCRATCH_FRAM_LEN 4
#define UID_FRAM_OFFSET SCRATCH_FRAM_LEN
#ifdef XAP_VERSION_1.3
	#define UID_FRAM_LEN 17
#else
	#define UID_FRAM_LEN 9
#endif
#define SOURCE_FRAM_OFFSET UID_FRAM_OFFSET + UID_FRAM_LEN
#define SOURCE_FRAM_LEN 40
#define SENSOR_FRAM_OFFSET  SOURCE_FRAM_OFFSET + SOURCE_FRAM_LEN
#define SENSOR_FRAM_LEN SIZEOF_SENSOR


const static  _sensor sensor_empty = { }; // keep a copy of inited (there's probably a better way...must take a look)

/******************************************************************************
					S E T U P
******************************************************************************/


void setup () {
  char test_bytes[4] ="!x!"; // flag to check FRAM/EEPROM/RAM integrity
  byte read_eeprom;
 
#ifdef USE_FRAM
	// YOU MUST do this first or you'll corrupt byte 0 of the FRAM/EEPROM
  pinMode(SS_PIN, OUTPUT); // Set up SS_PIN
  digitalWrite(SS_PIN, HIGH); // Deselect FRAM CS
#endif

	Serial.begin(19200);
	Serial.print(F("xAP BSC TEMP ONE WIRE V"));
	Serial.println(F(BSCTEMP_VERSION));
#ifdef USE_FRAM
	Serial.println(F("Ext mem"));
#else
	Serial.println(F("No ext mem"));
#endif
	for( int i=0; i<6; i++ ) {
		Serial.print( mymac[i], HEX );
		Serial.print( i < 5 ? ":" : "" );
	}
  Serial.println();

if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
		Serial.println(F("ERR:ENC28J60"));			
while(1);
}

#ifdef USE_DHCP

	if (!ether.dhcpSetup()) {
		Serial.println(F("ERR:DHCP"));
	while(1);
	}
#else
	ether.staticSetup(myip, gwip);
#endif
    Serial.print( F("My IP: " ));
    printIP( EtherCard::myip );
    Serial.println(); 
	delay(1500); // Let ARP's etc settle
// Set promiscuous mode or Broadcast on the Ethernet chip YMMV
//    ether.enablePromiscuous ();
    ether.enableBroadcast ();
/*********************************************************
			RAM (FRAM/SRAM) TEST
**********************************************************/
#ifdef USE_FRAM
	for (byte i = 0; i< 4; i++){
	read_eeprom = SpiRam.read_byte(i);
	test_bytes[i]=read_eeprom;
	}
	if (strcmp(test_bytes, "!x!") != 0) {
		strcpy(test_bytes , "!x!");
		for (byte i = 0; i< 4; i++){
		SpiRam.write_byte(i,test_bytes[i]);
		}
		test_bytes[0] = 0x00;
		for (byte i = 0; i< 4; i++){
		read_eeprom = SpiRam.read_byte(i);
		test_bytes[i]=read_eeprom;
		}
		if (strcmp(test_bytes, "!x!") != 0) {
			Serial.println(F("ERR:FRAM"));
			while(1);
		}
		//
		// Make and save default setup information
		// 
		makeXapDefaults();
		writeSourceToFRAM();
		writeUIDToFRAM();
		Serial.println(F("NEW:FRAM"));
	} else {
	// FRAM exists and has initialised signature
	// read UID and SOURCE information
		Serial.println("OK:FRAM");
		for (int i = 0; i < SOURCE_FRAM_LEN; i++){
			xAPSource[i] = (char)SpiRam.read_byte(i+SOURCE_FRAM_OFFSET);
		}
		for (int i = 0; i < UID_FRAM_LEN; i++){
			base_UID[i] = (char)SpiRam.read_byte(i+UID_FRAM_OFFSET);
		}
	}
#else
// No FRAM/EEPROM
		// Make default setup information
		// Source ID / Instance first
		makeXapDefaults();
	
#endif  // FRAM



/******************************************************************************
One Wire Init Code
******************************************************************************/
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  if(numberOfDevices > MAXSENSORS) numberOfDevices = MAXSENSORS;
  Serial.print(F("Found "));
  Serial.print(numberOfDevices, DEC);
  Serial.println(F(" Sensors"));

  for(byte i=0;i<numberOfDevices; i++) {
  
#ifdef USE_FRAM
  byte xx;
 		if(sensors.getAddress(sensor.deviceAddress, i)) {
		  sensors.setResolution(sensor.deviceAddress, TEMPERATURE_PRECISION);
		  if (!findSensorbyaddressFRAM(&sensor, &xx)){
			saveSensortoFRAM(&sensor); // doesn't exist. Add it
			}
		}
	
#else
    if(sensors.getAddress(sensor[i].deviceAddress, i)) {
      sensors.setResolution(sensor[i].deviceAddress, TEMPERATURE_PRECISION);
    }
#endif	
  }
#ifdef USE_FRAM
  if (numberOfDevices == 0) {
		for (int FRAM_address = SENSOR_FRAM_OFFSET; FRAM_address < SENSOR_FRAM_OFFSET + (MAXSENSORS*SIZEOF_SENSOR); FRAM_address++) {
			SpiRam.write_byte (FRAM_address, 0);
		}
		SpiRam.write_byte(0,0x00); //clear FRAM signature
		Serial.println(F("CLR:FRAM"));
		makeXapDefaults();
	
  }
#endif
  
/*******************************************************
Configure xap object
********************************************************/  
	xap.setSOURCE(xAPSource);

	xap.setUID(base_UID);
	xap.setUPTIME(&uptimeDays);
// Wait  
    delay(800);
    sensors.requestTemperatures(); // Send the command to get initial temperatures

#ifdef USE_FRAM
    for(byte i=0;i<MAXSENSORS; i++) {
	  updateTemperature(&sensor, i); // we'll get the right one from the FRAM/EEPROM memory
	  }
#else
    for(byte i=0;i<numberOfDevices; i++) {
      updateTemperature(&sensor[i], i);
	  }
#endif
	  

  sendInfo();
/*******************************************************
  TIMER 2 setup
*******************************************************/
   /* First disable the timer overflow interrupt while we're configuring */  
  TIMSK2 &= ~(1<<TOIE2);  
  
  /* Configure timer2 in normal mode (pure counting, no PWM etc.) */  
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));  
  TCCR2B &= ~(1<<WGM22);  
  
  /* Select clock source: internal I/O clock */  
  ASSR &= ~(1<<AS2);  
  
  /* Disable Compare Match A interrupt enable (only want overflow) */  
  TIMSK2 &= ~(1<<OCIE2A);  
  
  /* Now configure the prescaler to CPU clock divided by 1024 */  
  TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20); // Set bits  
//  TCCR2B &= ~(1<<CS21);             // Clear bit  
  
  /* We need to calculate a proper value to load the timer counter. 
   * The following loads the value 131 into the Timer 2 counter register 
   * The math behind this is: 
   * (CPU frequency) / (prescaler value) = 15.625 KHz = 64us. 
   * (desired period) / 64us = 156. 
   * MAX(uint8) + 1 - 156 = 100; 
   */  
  /* Save value globally for later reload in ISR */  
  tcnt2 = 100;   
  
  /* Finally load end enable the timer */  
  TCNT2 = tcnt2;  
  TIMSK2 |= (1<<TOIE2);  
//  Serial.println(freeRam());
#ifdef DEBUG_THIS
			sprintf(DEBUG_THIS_MESSAGE, "%s %d", "START", freeRam());
//			Serial.println(F("S"));
#endif

}


// END OF SETUP


/*******************************************************
TIMER 2 INT SERVICE ROUTINE
*******************************************************/
ISR(TIMER2_OVF_vect) {  
  /* Reload the timer */  
  TCNT2 = tcnt2;
  tmx++;
  if ((tmx % 250) == 0) { // every 2.5 second
  		sensors.requestTemperatures(); // Send the command to get temperatures (here not loop)
		semaphore |= UPDATETEMPERATURES; // and in the loop
  }
  if (tmx >= (XAP_HEARTBEAT-1000)/10) // every minute 'ish (60000 shows some creep over days)
  { 
#ifdef DEBUG_THIS
		sprintf(DEBUG_THIS_MESSAGE, "%s %d", "HB", freeRam());
		xap.sendHeartbeat(freeRam());
#else
		xap.sendHeartbeat();
#endif

//	Serial.println(freeRam());
	tmx = 0;
	if ((renewDHCPcount % 10) == 0)
		sendInfo(); //ten minutes
	if (renewDHCPcount-- == 0)
		semaphore |= RENEWDHCP;	// Every 24 Hrs renew the DHCP address in case of router changes/etc.
		}
 // add any other tasks here  
}  

/*******************************************************
 Output an ip address from buffer
********************************************************/
void printIP( uint8_t *buf ) {
  for( byte i = 0; i < 4; i++ ) {
    Serial.print( buf[i], DEC );
    if( i<3 )
      Serial.print( "." );
  }
}

static void sendInfo() {
#ifdef DEBUG_THIS
			sprintf(DEBUG_THIS_MESSAGE, "%s %d", "INF", freeRam());
//			Serial.println(F("I"));
#endif
#ifdef USE_FRAM
    for(byte i=0;i<MAXSENSORS; i++) {
	   readSensorfromFRAM(&sensor, i);
       if ((sensor.deviceAddress[0] == 0) && (sensor.deviceAddress[7] == 0)) { break; } // no sensor (family and CRC are both zero)
	   
       sendXapState("xAPBSC.info", &sensor, i);
	   }
#else	
    for(byte i=0;i<numberOfDevices; i++) {
      sendXapState("xAPBSC.info", &sensor[i], i);
	  }
#endif

	      
}
static void sendInfo(int i) {
#ifdef DEBUG_THIS
			sprintf(DEBUG_THIS_MESSAGE, "%s %d", "INF", freeRam());
//			Serial.println(F("I"));
#endif
	
#ifdef USE_FRAM
	   readSensorfromFRAM(&sensor, i);
       if ((sensor.deviceAddress[0] == 0) && (sensor.deviceAddress[7] == 0)) { return; } // no sensor (family and CRC are both zero)
	   
       sendXapState("xAPBSC.info", &sensor, i);
#else	
      sendXapState("xAPBSC.info", &sensor[i], i);
#endif

	      
}


static void OK200Page() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html><title>xAP BSCTEMP</title>"
	"<body style=\"background:#A9F5F2\">"
    "<img src=\"xaplogo.bmp\" >"
    "<H1>Done</H1>"
	"<A HREF=\"/\">Home</A><P>"
	"</html>"
	));
 ether.httpServerReply(bfill.position()); // send web page data

}
static void failedPage() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html><title>xAP BSCTEMP</title>"
	"<body style=\"background:#A9F5F2\">"
    "<img src=\"xaplogo.bmp\" >"
    "<H1>Failed</H1>"
	"<A HREF=\"/\">Home</A><P>"
	"</body></html>"
	));
 ether.httpServerReply(bfill.position()); // send web page data

}

static void UA401Page() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 401 Not Auth\r\n"
	));
 ether.httpServerReply(bfill.position()); // send web page data
}
static void NF404Page() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 404 Not Found\r\n"
	));
 ether.httpServerReply(bfill.position()); // send web page data

}

static void configPage() {
//Serial.println("CONFIG");
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "<html><head><title>xAP Configuration</title></head>"
  "<body style=\"background:#A9F5F2\">"
  "<img src=\"xaplogo.bmp\" >"
  "<h3>xAP Config</h3><hr/>"
  "<form METHOD=\"POST\" action=\"/c\">"
  "Source Instance "
  "<input type=\"text\" name=\"source\" size=\"16\" maxlength=\"16\"/><BR />"
#ifdef XAP_VERSION_1.3
  "Vendor UID FF."
  "<input type=\"text\" name=\"uid\" size=\"8\" maxlength=\"8\"/>:0000<BR />"
#else
  "Vendor UID FF"
  "<input type=\"text\" name=\"uid\" size=\"4\" maxlength=\"4\"/>00<BR />"
#endif
  "xAP BSC Delta Temp value:"
  "<input type=\"text\" name=\"delta\" size=\"5\" maxlength=\"5\"/><BR />"
  "Amount temperature has to change to generate a BSCEvent<BR />"
  "<input type=\"submit\" value=\"Go\"/>"
  "</form><hr/></body></html>"
	));
 ether.httpServerReply(bfill.position()); // send web page data

}

static void configSubs() {
//Serial.println("SUBS");
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "<html><head><title>xAP SubAddress</title></head>"
  "<body style=\"background:#A9F5F2\">"
  "<img src=\"xaplogo.bmp\" >"
  "<h3>xAP SubAddress</h3><hr/>"
  "<form METHOD=\"POST\" action=\"/s\">"
  "Sub Address:"
  "<input type=\"text\" name=\"node\" size=\"4\" maxlength=\"4\"/><BR />"
  "Friendly Name:"
  "<input type=\"text\" name=\"name\" size=\"16\" maxlength=\"16\"/><BR />"
  "xAP BSC Setpoint value:"
  "<input type=\"text\" name=\"set\" size=\"5\" maxlength=\"5\"/><BR />"
  "Value used to determine state on/off<BR />"
  "<input type=\"submit\" value=\"Go\"/>"
  "</form><hr/></body></html>"
	));
 ether.httpServerReply(bfill.position()); // send web page data

}

// Mini logo as bmp file
static void weblogo() {
  // send the mini logo
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
	"Content-Type: image/bmp\r\n"
	"Content-Length: $D\r\n\r\n"),sizeof(xaplogo));
	memcpy_P(Ethernet::buffer+0x36+bfill.position(), xaplogo, sizeof(xaplogo)); //length of the header data and 662 the bmp
 ether.httpServerReply((bfill.position()+sizeof(xaplogo))); // send web page data
}
static void homePage() {
#ifdef USE_FRAM
_sensor sensor_temp;
readSensorfromFRAM(&sensor_temp, 0);
#endif
  // Web server only reports the 1st sensor
  bfill = ether.tcpOffset();
#ifdef XAP_VERSION_1.3
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<html><title>xAP BSCTEMP</title>"
	"<body style=\"background:#A9F5F2\">"
    "<img src=\"xaplogo.bmp\" >"
	"Version $S<P>"
	"<H2>UID $S</H2>"
	"<H2>Source $S</H2>"
	"$D devices<P>"
	"1st Sensor Temp $S&deg;C<p>"
	"xAP Delta $S&deg;C<p>"
	
#ifdef DEBUG_THIS
	"Uptime $D H<p>"
#else
	"Uptime $D D<p>"
#endif
#ifdef USE_FRAM
	"<A HREF=\"/c\">Cfg xAP</A><P>"
	"<A HREF=\"/s\">Cfg SubAddress</A>"
#endif
	"</body></html>"
	), BSCTEMP_VERSION,
	xap.getUID(),xap.getSOURCE(), numberOfDevices,
#ifdef USE_FRAM
    dtostrf(sensor_temp.temp,3,1,tempS),dtostrf(xAPDelta,3,2,tempFS), uptimeDays); 
//,dtostrf(xAPDelta,3,2,tempS)
#else
   dtostrf(sensor[0].temp,3,1,tempS),dtostrf(xAPDelta,3,2,tempFS), uptimeDays); 
#endif	
#else
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<html><title>xAP BSCTEMP</title>"
    "<H1>UID $S</H1>"
	"$D devices<P>"
	"1st Sensor Temp$S&deg;C<p>"
	"Uptime $D D<P>"
#ifdef USE_FRAM
	"<A HREF=\"/c\">Cfg xAP</A><P>"
	"<A HREF=\"/s\">Cfg SubAddress</A><P>"

#endif
	"</body></html>"
	), 
	xap.getUID(), numberOfDevices, uptimeDays,
#ifdef USE_FRAM
    dtostrf(sensor_temp.temp,3,1,tempS)); 
#else
   dtostrf(sensor[0].temp,3,1,tempS)); 
#endif	
#endif
 ether.httpServerReply(bfill.position()); // send web page data
}

#ifdef DEBUG_THIS
// Since we've got no serial port permanently connected to debug
// Have a page which displays a debug message
static void debugPage() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html><title>xAP DMX512b</title>"
	"<body style=\"background:#A9F5F2\">"
    "<P>$S</P>"
	"<A HREF=\"/\">Home</A><P>"
	"</body></html>"
	), DEBUG_THIS_MESSAGE);
 ether.httpServerReply(bfill.position()); // send web page data
}
#endif

void sendXapState(char *clazz, struct _sensor *sensor_temp, byte node) {
char addrstr[17] = "";
char vendor[9]="";
char *p, *d;
// Amended to get friendly name/subaddress
#ifdef XAP_VERSION_1.3
char node_str[5] = {'0','0','0','0',0x00};
node_str[2]= ((node+1) >> 4) + 0x30;
if (node_str[2] > 0x39) node_str[2] +=7;
node_str[3]= ((node+1) & 0x0f) + 0x30;
if (node_str[3] > 0x39) node_str[3] +=7;
// format the UID
// Network is always the same
// Need to extract the vendor
// betweeen . and :
p=&base_UID[3]; // First char past the .
d=&vendor[0];
for (; *p!=0 && *p!=':'; p++, d++){
	*d=*p;
	}
	d++;
	*d='\0';
	
#else
char node_str[3]  = {'0','0',0x00};
node_str[0]= ((node+1) >> 4) + 0x30;
if (node_str[0] > 0x39) node_str[0] +=7;
node_str[1]= ((node+1) & 0x0f) + 0x30;
if (node_str[1] > 0x39) node_str[1] +=7;
strncpy(vendor, base_UID[2], 4); //easy
#endif

// Format the one wire address
byte c=0;
  for (byte i =0; i <8; i++){
	  addrstr[c]= (sensor_temp->deviceAddress[i] >>4) + 0x30;
	  if (addrstr[c] > 0x39) addrstr[c] +=7;
	  c++;
	  addrstr[c]= (sensor_temp->deviceAddress[i] & 0x0f) + 0x30;
	  if (addrstr[c] > 0x39) addrstr[c] +=7;
	  c++;
  }

  
  bfill = ether.udpOffset();
  bfill.emit_p(PSTR("xap-header\n"
    "{\n"
#ifdef XAP_VERSION_1.3
    "v=13\n"
    "hop=1\n"
    "uid=$S.$S:$S\n"
#else
    "v=12\n"
    "hop=1\n"
    "uid=$S$S$S\n"
#endif
    "class=$S\n"
    "source=$S"
	":$S\n"
    "}\n"
    "input.state\n"
    "{\n"
    "state=$S\n"
    "text=$SC\n"
	"owaddress=$S\n"
#ifdef USE_FRAM
    "}"), UID_NETWORK, vendor, node_str, clazz, xAPSource, sensor_temp->xAP_name, (sensor_temp->temp > sensor_temp->setpoint ? "on" : "off"), dtostrf(sensor_temp->temp,3,1,tempS), addrstr);
#else
    "}"), UID_NETWORK, vendor, node_str, clazz, xAPSource, node_str, "on", dtostrf(sensor_temp->temp,3,1,tempS));
#endif
    ether.sendUdpBroadcast((char *)ether.udpOffset(), bfill.position(), XAP_PORT, XAP_PORT);    
//	Serial.println(sensor_temp->xAP_name);
}
/***************************************************************
processXapMsg is a callback function from the xAP processing
to handle incoming xAP messages
Currently only xAPBSC.query but extend as required
(could refactor to check address match first and then
xAP class second)
****************************************************************/
void processXapMsg() {
const char *targetptr = xap.getValue("xap-header","target");
char *tp; // pointer to ':' in target string
struct _sensor sensor_temp; //local sensor structure
char temp_node[5]="";
  //Handy for debugging - parsed XAP message is dumped to serial port.
  //xap.dumpParsedMsg();
	if(xap.getType() == XAP_MSG_ORDINARY && xap.isValue("xap-header","class","xAPBSC.query")){
		if (xap.partialMatchSource(xAPSource, (char *)targetptr )) { 
			// Match on lefthandside of source and target, so it is for this device
//			Serial.println(F("info1"));
			// need to iterate through all node subaddress names and see if one matches the target subbaddress (if there is one)
			tp=strpbrk(targetptr,":"); // point to the :
			if (tp){
				tp++ ; //point to string past the ':'
//				Serial.print(tp);
				for(int i=0; i < MAXSENSORS && *tp != '\0'; i++){ // note this is MAXSENSORS not numberofDevices
#ifdef USE_FRAM
					readSensorfromFRAM(&sensor_temp, i); // get struct
					if (isvalidSensor(&sensor_temp) ){ // valid struct 
						if(xap.partialMatchSource((char *)sensor_temp.xAP_name, tp)){
#else
					if (isvalidSensor(&sensor[i]) ){ // valid struct 

#ifdef XAP_VERSION_1.3
						sprintf(temp_node, "%04d", i);
#else
						sprintf(temp_node, "%02d", i & 0xff);
#endif
						if(xap.partialMatchSource(temp_node, tp)){
#endif
//							Serial.println(F("info2"));
							sendInfo(i);
						}
					}
				}
			} else {
				// we matched but no subaddress since ':' not found or was >
				// Send info for all subaddresses
//							Serial.println(F("info3"));
				sendInfo();
			}
		}
    }
}

/******************************************************************************
			M A I N   L O O P
		This processes incoming packets. Temperature updates take place in 
		the Timer2 interrupt routine every few seconds.
******************************************************************************/
void loop () {
  word len, pos;
  while(1){
#ifdef USE_DHCP
			if ((semaphore & RENEWDHCP) | (ether.dhcpExpired()))
			{
			
/**************
				if( es.allocateIPAddress(buf, BUFFER_SIZE, mymac, 80, myip, mynetmask, gwip, dhcpsvrip, dnsip ) > 0 ) {
					// Display the results:
					Serial.print( F("Updated IP: " ));
					printIP( myip );
					Serial.println(); 
				} else {
					Serial.println(F("ERR:DHCP"));
				}	
****************/
				if (!ether.dhcpSetup()) {
					Serial.println(F("ERR:DHCP"));
				} else {
					Serial.print( F("My IP: " ));
					printIP( myip );
					Serial.println(); 
					renewDHCPcount = HRS24;
					uptimeDays++;
					semaphore &= ~RENEWDHCP; // reset flag
				}
			}
#else
			if (semaphore & RENEWDHCP)
			{
				renewDHCPcount = HRS24;
				semaphore &= ~RENEWDHCP; // reset flag
				uptimeDays++;
			}
#endif 
//		memset(buf, 0x00, sizeof(buf)); // clear it
		
		
		if (semaphore & SENDBSCINFO) {
		
			sendInfo();
			semaphore &= ~SENDBSCINFO;
		}
		len = ether.packetReceive();
		// ENC28J60 loop runner: handle ping and wait for a tcp packet
		pos = ether.packetLoop(len);
		if(pos) {  // Check if valid www data is received.
			switch(pageType(pos))
			{
				case 0:
					homePage();
					break;
				case 1:
					configPage();
					break;
				case 2:
					OK200Page();
					break;
				case 3:
					NF404Page();
					break;
				case 4:
					failedPage();
					break;
				case 5:
					configSubs();
					break;
				case 6:
					weblogo();
					break;
				case 7:
#ifdef DEBUG_THIS
					debugPage();
#else
					UA401Page();     
#endif
					break;
				default:
					UA401Page();     
			}
		} else { // Handle xAP UDP

		xap.process(len, processXapMsg);
		}
			if (semaphore & UPDATETEMPERATURES){
#ifdef USE_FRAM
				for(int i=0;i<MAXSENSORS; i++) {
					updateTemperature(&sensor, i); // we'll get the right one from the memory in the update routine
				}
#else
				for(int i=0;i<numberOfDevices; i++) {
					updateTemperature(&sensor[i], i);
				}
#endif
			semaphore &= ~UPDATETEMPERATURES; // reset flag
		}
  }
}
void makeXapDefaults(void){
		xAPSource[0]=0x00;
		strcat(xAPSource, SOURCE_VENDOR);
		strcat(xAPSource, ".");
		strcat(xAPSource, INSTANCE_DEFAULT);
#ifdef XAP_VERSION_1.3
	   base_UID[0] = 0x00;
	   strcat(base_UID, UID_NETWORK);
	   strcat(base_UID, ".");
	   strcat(base_UID, UID_VENDOR);
	   strcat(base_UID, ":");
   	   strcat(base_UID, "0000");
#else
	   base_UID[0] = 0x00;
	   strcat(base_UID, UID_NETWORK);
	   strcat(base_UID, UID_VENDOR);
   	   strcat(base_UID, "00");

#endif	// XAP 1.3
}

byte pageType(word dat_p){
char *s;
int nodeval;
//Serial.println((char *)&Ethernet::buffer[dat_p]);
    if (strncmp("GET ",(char *)&(Ethernet::buffer[dat_p]),4)!=0){
      // head, post and other methods:
	  // check for return values and update FRAM
				// locate the body section we're looking for \r\n\r\n
				char * locater = strstr((char *)&Ethernet::buffer[dat_p], "\r\n\r\n"); // search for the body section
				if (locater != 0 && (strncmp("/c ",(char *)&(Ethernet::buffer[dat_p+5]),3)==0)){ // config
//					Serial.println(locater+4); //show the body
					// we have two inputs - Instance and Vendor UID
					if(strncmp("source=",locater+4,7)==0){
						locater+=11; //move to start of source value
						s=locater;
						for ( ; *locater != '&' && *locater !='\0'; locater++); //find EOS
						// locater is now at either a null or '&'
						// Value of source is between the two. Should validate and if we get enough spare ROM..........
						xAPUpdateSourceInstance(s, locater-s); // call with pointer and length
						}
  					if (*locater != '\0') s=++locater; //next along
					if(strncmp("uid=",locater,4)==0){
						locater +=4;
						s=locater;
						for ( ; *locater != '&' && *locater !='\0'; locater++); //find EOS
						xAPUpdateUIDInstance(s, locater-s); // call with pointer and length
						}
  					if (*locater != '\0') s=++locater; //next along
					if(strncmp("delta=",locater,6)==0){
						locater +=6;
						s=locater;
						for ( ; *locater != '&' && *locater !='\0'; locater++); //find EOS
						char strtemp[8] ="";
						strncpy(strtemp, s, locater-s);
						xAPDelta = atof(strtemp);
						if (xAPDelta < TEMP_LSB) xAPDelta = TEMP_LSB; // Set as a minimum to stop hunting
						}
						//add more here in sequence corresponding to the FORM POST
					}
				if (locater != 0 && (strncmp("/s ",(char *)&(Ethernet::buffer[dat_p+5]),3)==0)){ // config subs
//						Serial.println(F("Subs"));
//					Serial.println(locater+4); //show the body
					if(strncmp("node=",locater+4,5)==0){
						locater+=9; //move to start of source value
						s=locater;
						for ( ; *locater != '&' && *locater !='\0'; locater++); //find EOS
						// locater is now at either a null or '&'
						// Value of node/subaddr is between the two. Should validate and if we get enough spare ROM..........
						char nodestr[5]="";
						strncpy(nodestr, s, locater-s);
						if (!isnumeric(nodestr)) return 4; // Not numeric
						nodeval=(atoi(nodestr)); // save node value (zero up, not one up)
						if (nodeval-- == 0) return 4; // Can't be zero input but want from 0 for index
						}
  					if (*locater != '\0') s=++locater; //next along
					if(strncmp("name=",locater,5)==0){
						locater +=5;
						s=locater;
						for ( ; *locater != '&' && *locater !='\0'; locater++); //find EOS
						char subaddr[18]="";
						strncpy(subaddr, s, locater-s);
						if (!updateSubaddress(nodeval, subaddr)) // update
							return 4;// Error
						}
  					if (*locater != '\0') s=++locater; //next along
					if(strncmp("set=",locater,4)==0){
						locater +=4;
						s=locater;
						for ( ; *locater != '&' && *locater !='\0'; locater++); //find EOS
						char strtemp[8] ="";
						strncpy(strtemp, s, locater-s);
						float set;
						set= atof(strtemp);
						if (!updateSetpoint(nodeval, &set)) // update
							return 4;// Error
						}
						//add more here in sequence corresponding to the FORM POST
					}
				
  
	  // and return for an 200 OK
	  return 2;
	}
	else { // GET
				if (strncmp("/ ",(char *)&(Ethernet::buffer[dat_p+4]),2)==0){
//				Serial.println("0");
				return 0;
			}
				if (strncmp("/s ",(char *)&(Ethernet::buffer[dat_p+4]),2)==0){
//				Serial.println("5");
				return 5;
			}
				if (strncmp("/xap ",(char *)&(Ethernet::buffer[dat_p+4]),4)==0){ //logo
//				Serial.println("6");
				return 6;
			}
				if (strncmp("/debug",(char *)&(Ethernet::buffer[dat_p+4]),6)==0){ // debug
//				Serial.println("7");
				return 7;
			}

				else { 	return 1;} // config page
		}
//				Serial.println("3");
	return 3;
}


bool updateSubaddress(int node, char *name){
#ifdef USE_FRAM
struct _sensor sensor_temp;
	if (node > MAXSENSORS) return false; // Sense check
	readSensorfromFRAM(&sensor_temp, node);
	// clunky check if this is a valid address
	if (sensor_temp.deviceAddress[0] == 0x00 && sensor_temp.xAP_name[0] == 0x00){
		return false; // empty slot
	}
	strcpy(sensor_temp.xAP_name, name);
	saveSensortoFRAM(&sensor_temp);
#endif
return true;
}
bool isvalidSensor(struct _sensor *sensor_ptr){
	// check this is a valid sensor struct - first byte is family type and xAP name should have been configured as something
	// add other checks as desired
	
#ifdef USE_FRAM	
	if(sensor_ptr->deviceAddress[0] != 0x00 && sensor_ptr->xAP_name[0] != 0x00)
#else
	if(sensor_ptr->deviceAddress[0] != 0x00)
#endif
		return true;
	else
		return false;

}
bool updateSetpoint(int node, float *setpoint){
#ifdef USE_FRAM
struct _sensor sensor_temp;
	if (node > MAXSENSORS) return false; // Sense check
	readSensorfromFRAM(&sensor_temp, node);
	// clunky check if this is a valid address
	if (sensor_temp.deviceAddress[0] == 0x00 && sensor_temp.xAP_name[0] == 0x00){
		return false; // empty slot
	}
	memcpy(&sensor_temp.setpoint, setpoint, sizeof(float));
	saveSensortoFRAM(&sensor_temp);
#endif
return true;
}


/*********************************************************************
 Update Temperature for a specific device, specified by the xAP node
 Will send a BSC event if temperature differs by xAPDelta value
*********************************************************************/
void updateTemperature(struct _sensor *sensor, byte node) {
//Serial.print("Update "); Serial.println(node, HEX);Serial.print(" ");
#ifdef USE_FRAM
  readSensorfromFRAM(sensor, node);
  if ((memcmp(sensor_empty.deviceAddress, sensor->deviceAddress, sizeof(DeviceAddress)) != 0) && (numberOfDevices > 0)){
//  Serial.print(sensor->temp); Serial.print(" ");
	  float tempC = sensors.getTempC(sensor->deviceAddress);
//	  Serial.print(tempC); Serial.print(" ");
	  // sensors offline report -127.0C
	  if (tempC > -126.0){
		  float intermediateC = sensor->temp-tempC; // don't put in the abs() call
//	  Serial.println(intermediateC);
		  if (fabs(intermediateC) > xAPDelta) {
			sensor->temp = tempC;
//			Serial.println(tempC);
			saveSensortoFRAM(sensor);
			sendXapState("xAPBSC.event", sensor, node);
			}
		}
	}
#else
  float tempC = sensors.getTempC(sensor->deviceAddress);
  if(sensor->temp != tempC) {
    sensor->temp = tempC;
//	Serial.println(tempC);
    sendXapState("xAPBSC.event", sensor, node);
	}
#endif
  
}


void xAPUpdateSourceInstance(char *source, int len){
// find the last position of the xAPsource '.'
// and then copy the source to it and append a null
// affects nothing
if (len==0) return; //nothing to see here... move along....
byte i=0;
char * s = source; // don't destroy pointer
char * p = (char *) (xAPSource+strlen(xAPSource)); // point to the end of string (0x00)
char * q = (char *) (xAPSource+sizeof(xAPSource)-1); 
for( ; *p != '.'; p--);
//OK we're at the .
p++; // now at first char of instance
while(p != q){
	*p++=*s++;
	if (i++ == len-1 || *p==' ') break; // at end or someone put a space in the Instance name
	}
*p='\0';
xap.setSOURCE(xAPSource);
#ifdef USE_FRAM
writeSourceToFRAM();
#endif
}
void xAPUpdateUIDInstance(char *source, int len){
// Update UID
// fixed lengths so easy
// affects nothing
//Serial.println(len);
#ifdef XAP_VERSION_1.3
if (len ==0 || len != 8) return; // not correct length for xAP 1.3/1.2
char * p = (char *)base_UID + 3; // skip the FF.
#else
if (len ==0 || len !=4) return; // not correct length for xAP 1.3/1.2
char * p = (char *)base_UID + 2; // skip the FF
#endif
byte i=0;
char * s = source; // don't destroy pointer
while(true){
	*p++=toupper(*s++);
	if (i++ == len-1 || *p==' ') break; // at end or someone put a space in the UID! (should also validate hex)
}
xap.setUID(base_UID);
#ifdef USE_FRAM
writeUIDToFRAM();
#endif
}

// Write to FRAM and FRAM utility routines

#ifdef USE_FRAM
void writeUIDToFRAM(void){

		for (int i = 0; i< UID_FRAM_LEN; i++){
		SpiRam.write_byte(i+UID_FRAM_OFFSET,base_UID[i]);
		}

}

void writeSourceToFRAM(void){

		for (int i = 0; i< SOURCE_FRAM_LEN; i++){
		SpiRam.write_byte(i+SOURCE_FRAM_OFFSET,xAPSource[i]);
		}
}

void readSensorfromFRAM(struct _sensor *sensor_in, byte index) {
// Note index is not OneWire index but FRAM/EEPROM index (xAP Subaddress-1)
// starting from zero

int FRAM_address;
byte *ptr = (byte *) sensor_in;
	FRAM_address = (index * SIZEOF_SENSOR) + SENSOR_FRAM_OFFSET;
	SpiRam.read_stream(FRAM_address, (char *)sensor_in, SIZEOF_SENSOR);

}
	
	
bool findSensorbyaddressFRAM(struct _sensor *sensor_in, byte *index){
int FRAM_address;
struct _sensor sensor_temp;
byte *ptr = (byte *) sensor_in;
//bool found = false;
byte counter = 0;

	FRAM_address = SENSOR_FRAM_OFFSET;		//starting point
	while(counter < MAXSENSORS) {
		SpiRam.read_stream(FRAM_address, (char *)&sensor_temp, SIZEOF_SENSOR);
		if (memcmp(sensor_in->deviceAddress, sensor_temp.deviceAddress, sizeof(DeviceAddress)) == 0){
			*index = counter;
			return true;
			} else {
				counter++;
				FRAM_address +=SIZEOF_SENSOR;
		}
	}
return false;
}

/******************************************************
Save a sensor structure to FRAM/EEPROM
This will locate a slot in the memory and write the
structure  - will use old slot if exists
******************************************************/
void saveSensortoFRAM(struct _sensor *sensor_in) {
int FRAM_address; // address in FRAM/EEPROM NOT a pointer
struct _sensor sensor_temp;
byte *ptr = (byte *) sensor_in;
bool found = false;
byte counter = 0;
// Will sort through list to try and find the existing entry (by OW address)
// If does not exist then place on the end of the list - identified by null address
// This is done so xAP node number/subaddress and name don't change past reboot
// though currently do not delete from FRAM, so if you remove sensor it will
// remain in FRAM. For clean start disconnect all sensors and restart
//
// This divorces the OneWire index from the xAP sensor node number (subaddress)
//
// Note: No point doing this for CPU RAM based system as starts fresh every time :-(
//
//			for (int xx = 0; xx <9; xx++){Serial.print(sensor_in->deviceAddress[xx], HEX);	Serial.print( xx < 8 ? ":" : "" );}Serial.println("<");
	FRAM_address = SENSOR_FRAM_OFFSET;		//starting point address in FRAM	
	while(counter < MAXSENSORS) {
//Serial.println(counter);
		SpiRam.read_stream(FRAM_address, (char *)&sensor_temp, SIZEOF_SENSOR);
//				for (int xx = 0; xx <9; xx++){Serial.print(sensor_temp.deviceAddress[xx], HEX);	Serial.print( xx < 8 ? ":" : "" );}Serial.println(">");
		if ((memcmp(sensor_temp.deviceAddress, sensor_in->deviceAddress, sizeof(DeviceAddress)) == 0) || (memcmp(sensor_empty.deviceAddress, sensor_in->deviceAddress, sizeof(DeviceAddress)) == 0)) { //match
//			Serial.println("MATCH");
			for (int byte_addr=FRAM_address; byte_addr < (FRAM_address+SIZEOF_SENSOR); byte_addr++){ // Save it (assumes nth plus one is already zeroed)
				SpiRam.write_byte (byte_addr, *ptr++);
			}
			break; // done here
		} else {
			if ((memcmp(sensor_empty.deviceAddress, sensor_temp.deviceAddress, sizeof(DeviceAddress)) == 0)){ //empty item in memory
//			Serial.println("EMPTY");
			//OK so reached blank entry - insert current
		    // Set friendly name to node number and use web interface to change
				sensor_in->xAP_name[0] = '\0'; // set clear
				#ifdef XAP_VERSION_1.3
				sensor_in->xAP_name[0] = '0'; //might look messy but is small CPU cycles
				sensor_in->xAP_name[1] = '0';
				sensor_in->xAP_name[4] = '\0';
				sensor_in->xAP_name[2]= ((counter+1) >> 4) + 0x30;
				if (sensor_in->xAP_name[2] > 0x39) sensor_in->xAP_name[2] +=7;
				sensor_in->xAP_name[3]= ((counter+1) & 0x0f) + 0x30;
				if (sensor_in->xAP_name[3] > 0x39) sensor_in->xAP_name[3] +=7;
				#else
				sensor_in->xAP_name[2]  = '\0';
				sensor_in->xAP_name[0]= ((counter+1) >> 4) + 0x30;
				if (sensor_in->xAP_name[0] > 0x39) sensor_in->xAP_name[0] +=7;
				sensor_in->xAP_name[1]= ((counter+1) & 0x0f) + 0x30;
				if (sensor_in->xAP_name[1] > 0x39) sensor_in->xAP_name[1] +=7;
				#endif
				for ( int byte_addr=FRAM_address; byte_addr < (FRAM_address+SIZEOF_SENSOR); byte_addr++){ // Save it
					SpiRam.write_byte (byte_addr, *ptr++);
				}
				// mark end nth plus one (unless last entry in array)
				FRAM_address += SIZEOF_SENSOR; //next slot
				ptr = (byte *)&sensor_empty;
				for (int byte_addr=FRAM_address; byte_addr < (FRAM_address+SIZEOF_SENSOR); byte_addr++){ // Save it
					SpiRam.write_byte (byte_addr, *ptr++);
					
				}
				break;

			} else {
				counter++;
				FRAM_address +=SIZEOF_SENSOR;
		}
	}
}
}


#endif


bool isnumeric(char *in_string){
char *str = in_string;
		  while(*str)
		  {
			if(!isDigit(*str))
			  return false;
			str++;
		  }
return true;
}

#ifdef DEBUG_THIS
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif
