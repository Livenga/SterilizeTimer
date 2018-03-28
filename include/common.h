#ifndef _COMMON_H
#define _COMMON_H

#define MHz 000000
//#define _XTAL_FREQ 4MHz
#define _XTAL_FREQ 8MHz
//#define _XTAL_FREQ 20MHz

enum ANALOG {
  AN0 = 0,
  AN1,
  AN2,
  AN3,
  AN4,
  AN5,
  AN6,
  AN7
};

#ifndef _XC_H
#include <xc.h>
#endif

#define _mmss(mm, ss) mm * 60 + ss

#define display_7seg(value)\
  switch(value) {\
    case 0: PORTD = 0b01111110; break;\
    case 1: PORTD = 0b00110000; break;\
    case 2: PORTD = 0b01101101; break;\
    case 3: PORTD = 0b01111001; break;\
    case 4: PORTD = 0b00110011; break;\
    case 5: PORTD = 0b01011011; break;\
    case 6: PORTD = 0b01011111; break;\
    case 7: PORTD = 0b01110010; break;\
    case 8: PORTD = 0b01111111; break;\
    case 9: PORTD = 0b01111011; break;\
  }

#endif
