#include "BufferFiller.h"
#include <stdarg.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
// Avoid spurious pgmspace warnings - http://forum.jeelabs.net/node/327
// See also http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#undef PROGMEM
#define PROGMEM __attribute__(( section(".progmem.data") ))
#undef PSTR
#define PSTR(s) (__extension__({static prog_char c[] PROGMEM = (s); &c[0];}))


void BufferFiller::emit_p(PGM_P fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (;;) {
       char c = pgm_read_byte(fmt);
		fmt++;
        if (c == 0)
            break;
        if (c != '$') {
		*ptr++ = c;
 		continue;
        }
		c = pgm_read_byte(fmt);
		fmt++;
		switch (c) {
            case 'D':
                itoa(va_arg(ap, int), (char*) ptr, 10);
                break;
            case 'S':
                strcpy((char*) ptr, va_arg(ap, const char*));
                break;
            case 'F': {
                PGM_P s = va_arg(ap, PGM_P);
                char c;
                while ((c = pgm_read_byte(s++)) != 0)
                    *ptr++ = c;
                continue;
            }
            case 'E': {
                byte* s = va_arg(ap, byte*);
                char c;
                while ((c = eeprom_read_byte(s++)) != 0)
                    *ptr++ = c;
                continue;
            }
            default:
                *ptr++ = c;
                continue;
        }
        ptr += strlen((char*) ptr);
    }
	
    va_end(ap);
}

void BufferFiller::emit_raw(const char* s, byte len) {
    memcpy(ptr, s, len);
    ptr += len;
}
