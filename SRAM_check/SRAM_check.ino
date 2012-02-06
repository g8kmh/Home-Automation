/**********************************************************

23K256 SRAM TEST CODE FOR THE NANODE AVR ATMEGA328P

This code will work and has been tested on a V5 Nanode with the
Ethernet chip on the board and works correctly. You must
have performed the 3V3 modifications on the V5

Works with Arduino IDE V1.0

This is not a speed test - just to visibly test you have the
SRAM working and there are no issues with CS on the SRAM
or the Ethernet chip and SPI is working correctly. Like:

Nanode SRAM Test lehane@mm-wave.com V1.0
Status response1
10
15
Duration 5
Following is RAM dump
0-FF
1-0
2-1
3-2

It only tests the first 255 bytes, enough to show any wiring
errors. You should see the address and then one less than the
address output on the serial port at 192000.

Lehane Kellett
lehane@mm-wave.com
www.honeycottage.eu

**********************************************************/

#define VERSION "1.0"


#define DATAOUT 11//MOSI
#define DATAIN  12//MISO 
#define SPICLOCK  13//sck
#define RAMSELECT 9//ss
#define LED 6 // LED
#define ENET 8// ethernet chip (make sure set CS high)

//opcodes
#define WREN  B00000110
#define WRDI  B00000010
#define RDSR  B00000101
#define WRSR  B00000001
#define READ  B00000011
#define WRITE B00000010

byte sram_output_data;
byte sram_input_data=0;
byte clr;
int address=0;
long start_time;
long stop_time;
long duration;


char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}

void setup()
{
  Serial.begin(19200);
  
  DDRB = DDRB | B11101111;  // Sets pins 12 as input, 8-15 output

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(ENET, OUTPUT);
  digitalWrite(ENET,HIGH); //CS ENET CHIP Disables
  
  digitalWrite(LED,HIGH); //Nanode LED Off
  pinMode(RAMSELECT,OUTPUT);
  
  digitalWrite(SPICLOCK, LOW);
  digitalWrite(DATAOUT, LOW);
  digitalWrite(RAMSELECT,HIGH); //disable device seq
  digitalWrite(RAMSELECT,LOW); // Flip CS
  digitalWrite(RAMSELECT,HIGH); //disable device seq

  SPCR = (1<<SPE)|(1<<MSTR);

  clr=SPSR;
  clr=SPDR;
  delay(10);

  Serial.print(F("Nanode SRAM Test lehane@mm-wave.com V"));
  Serial.println(F(VERSION));
  
  digitalWrite(RAMSELECT,LOW);
  spi_transfer(WRSR); //write to status register
  spi_transfer(0x01);  // byte mode hold disabled
  digitalWrite(RAMSELECT,HIGH);
  digitalWrite(RAMSELECT,LOW);
  spi_transfer(RDSR); //read status register
  sram_output_data = spi_transfer(0xff); 
  digitalWrite(RAMSELECT,HIGH);

  Serial.print(F("Status response "));
  Serial.println(sram_output_data, HEX); // Show the status register to confirm
  
  
  digitalWrite(LED,LOW); // Turn the LED on
  start_time = millis(); 
  for (int I=0;I<255;I++)   // Write 
  {
	digitalWrite(RAMSELECT,LOW);
	spi_transfer(WRITE); //write instruction
	address=I;
	spi_transfer((char)(address>>8));   //send MSByte address first
	spi_transfer((char)(address));      //send LSByte address
    spi_transfer(I-1); //just so not same as address!
	digitalWrite(RAMSELECT,HIGH); //release chip 
  }
   stop_time = millis();
   duration = stop_time-start_time;
   delay(2000);
   Serial.println(start_time);
   Serial.println(stop_time);
   Serial.print(F("Duration "));
   Serial.println(duration);
   digitalWrite(LED,HIGH);
   delay(1000);
   Serial.println(F("Following is RAM dump"));
   address=0;
}

byte read_sram(int SRAM_address)
{
  //READ SRAM
  int data;
  digitalWrite(RAMSELECT,LOW);
  spi_transfer(READ); //transmit read opcode
  spi_transfer((char)(SRAM_address>>8));   //send MSByte address first
  spi_transfer((char)(SRAM_address));      //send LSByte address
  data = spi_transfer(0xFF); //get data byte
  digitalWrite(RAMSELECT,HIGH); //release chip, signal end transfer
  return data;
}

void loop()
{
  sram_output_data = read_sram(address);
  Serial.print(address,HEX);
  Serial.print("-");
  Serial.print(sram_output_data,HEX);
  Serial.print("\n");
  address++;
  if (address == 255)
    address = 0;
  delay(50);
}

