/*
 * SpiRAM.cpp - Library for driving a 23k256 SPI attached SRAM chip
 *
 * Phil Stewart, 18/10/2009
 * 
 * Copyright (c) 2009, Phil Stewart
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *             
 */
     
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #else
  #include "WProgram.h"
  #endif

#include "SpiRAM.h"
#include <avr/interrupt.h>

// Constructor

SpiRAM::SpiRAM(byte clock)
{
  // Ensure the RAM chip is disabled in the first instance
  //disable();

  // Set the spi mode using the requested clock speed
  //SPI.setClockDivider(clock);
  
  // Set the RAM operarion mode flag to meaningless to ensure first Op gets sent to chip
  _current_mode = 0xFF;
}

// Enable and disable helper functions

void SpiRAM::enable()
{
  cli();
   digitalWrite(SS_PIN, LOW);
  
}

void SpiRAM::disable()
{
  digitalWrite(SS_PIN, HIGH);
  sei();
}

// Byte transfer functions

char SpiRAM::read_byte(int address)
{
  char read_byte;

  // Set byte mode
  //_set_mode(BYTE_MODE);
  
  // Write address, read data
  enable();
  SPI.transfer(READ);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  read_byte = SPI.transfer(0xFF);
  disable();
  
  return read_byte;
}

char SpiRAM::write_byte(int address, char data_byte)
{
  // Set byte mode
  _set_mode(BYTE_MODE);
  
  // Write address, read data
  enable();
  SPI.transfer(WREN);
  digitalWrite(SS_PIN, HIGH);
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  SPI.transfer(data_byte);
  disable();
  // while (read_status() & WRITE_CYCLE_IN_PROGRESS); //EEPROM only
  return data_byte;
}

// Page transfer functions
void SpiRAM::read_page(int address, char *buffer)
{
  int i;

  // Set byte mode
  _set_mode(PAGE_MODE);
  
  // Write address, read data
  enable();
  SPI.transfer(READ);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  for (i = 0; i < 32; i++) {
    buffer[i] = SPI.transfer(0xFF);
  }    
  disable();
}

void SpiRAM::write_page(int address, char *buffer)
{
  int i;

  // Set byte mode
  _set_mode(PAGE_MODE);
  
  // Write address, read data
  enable();
  SPI.transfer(WREN);
  digitalWrite(SS_PIN, HIGH);
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  for (i = 0; i < 32; i++) {
    SPI.transfer(buffer[i]);
  }    
  disable();
}

// Stream transfer functions
void SpiRAM::read_stream(int address, char *buffer, int length)
{
  int i;

  // Set byte mode
  //_set_mode(STREAM_MODE);
  
  // Write address, read data
  enable();
  SPI.transfer(READ);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  for (i = 0; i < length; i++) {
    buffer[i] = SPI.transfer(0xFF);
  }    
  disable();
}

void SpiRAM::write_stream(int address, char *buffer, int length)
{
  int i;

  // Set byte mode
  //_set_mode(STREAM_MODE);
  
  // Write address, read data
  enable();
    SPI.transfer(WREN);
  digitalWrite(SS_PIN, HIGH);
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  for (i = 0; i < length; i++) {
    SPI.transfer(buffer[i]);
  }    
  disable();
}


char SpiRAM::read_status()
{
  char read_byte = 0;
 // Write address, read data
  enable();
  SPI.transfer(RDSR);
  read_byte = SPI.transfer(0xFF);
  return read_byte;
  disable();
}


// Mode handling
void SpiRAM::_set_mode(char mode)
{
  if (mode != _current_mode)
  {
    enable();
    SPI.transfer(WREN);
  digitalWrite(SS_PIN, HIGH);
  digitalWrite(SS_PIN, LOW);
    SPI.transfer(WRSR);
    SPI.transfer(mode);
    disable();
    _current_mode = mode;
  }
}

// Preinstantiate SpiRAM object;
SpiRAM SpiRam = SpiRAM(SPI_CLOCK_DIV32);
