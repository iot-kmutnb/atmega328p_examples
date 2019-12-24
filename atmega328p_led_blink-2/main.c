// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz CPU frequency
#include <avr/io.h>        // for accessing SFRs
#include <util/delay.h>

typedef struct _port {
  uint8_t PIN; 
  uint8_t DIR;
  uint8_t OUT; 
} port_t;

#define porta  ((volatile port_t *)0x20)
#define portb  ((volatile port_t *)0x23)
#define portc  ((volatile port_t *)0x26)
#define portd  ((volatile port_t *)0x29)

#define DELAY_MS   (500)

int main(void) { 
  portb->DIR |= (1 << 5);  // use PB5 as output
  
  while (1) { 
    portb->PIN |= (1 << 5);
    _delay_ms( DELAY_MS );
   }
}
////////////////////////////////////////////////////////////////////////////
