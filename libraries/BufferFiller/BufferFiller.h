
#include <stdarg.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
 #if defined(ARDUINO) && ARDUINO >= 100
 #include "Arduino.h"
 #else
 #include "WProgram.h"
 #endif


class BufferFiller : public Print {
    uint8_t *start, *ptr;
public:
    BufferFiller () {}
    BufferFiller (uint8_t* buf) : start (buf), ptr (buf) {}
        
    void emit_p(PGM_P fmt, ...);
    void emit_raw(const char* s, uint8_t len);
    uint8_t* buffer() const { return start; }
    uint16_t position() const { return ptr - start; }
    
    virtual size_t write(uint8_t v) { *ptr++ = v; }
};

