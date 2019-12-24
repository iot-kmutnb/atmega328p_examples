// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL // 16MHz CPU frequency
#include <avr/io.h>      // for accessing SFRs
#include <util/delay.h>

typedef union { // anonymous union
  struct { // anonymous struct
    uint8_t bit0: 1;
    uint8_t bit1: 1;
    uint8_t bit2: 1;
    uint8_t bit3: 1;
    uint8_t bit4: 1;
    uint8_t bit5: 1;
    uint8_t bit6: 1;
    uint8_t bit7: 1;
  };
  uint8_t byte;
} port_reg_t;

#define PINB_REG  ((volatile port_reg_t *)0x23)
#define DDRB_REG  ((volatile port_reg_t *)0x24)
#define PORTB_REG ((volatile port_reg_t *)0x25)

#define DELAY_MS (500)

int main(void) { 
  DDRB_REG->bit5  = 1;  // PB5 output
  PORTB_REG->bit5 = 0;  // output low at PB5

  while (1) { 
     // toggle output at PB5
     PINB_REG->byte |= (1 << 5); 
     _delay_ms( DELAY_MS );
  }
}


////////////////////////////////////////////////////////////////////////////
