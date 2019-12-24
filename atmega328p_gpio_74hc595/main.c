// Date: 2019-12-12
// MCU Board: Arduino Nano v3 (ATmega328P) + 1x 74HC595 module
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>

//------------------------------------------------------

void gpio_init(void) {
  // use PB2 (/SS), PB3 (MOSI) and PB5 (SCK) for output
  DDRB |= (1 << DDB2)|(1 << DDB3)| (1 << DDB5);
  PORTB &= ~(1 << PORTB5); // set SH_CP low
  PORTB &= ~(1 << PORTB2); // set ST_CP low
}

void shiftout( uint8_t wdata ) {
  uint8_t i;
  for ( i=0; i < 8; i++ ) {
     if ( wdata & 0x80 ) { // check the MSB 
        PORTB |= (1 << PORTB3);  // set DS high
     } else {
        PORTB &= ~(1 << PORTB3); // set DS low
     }
     wdata <<= 1; // shift bits to the left
     PORTB |=  (1 << PORTB5); // set SH_CP high
     PORTB &= ~(1 << PORTB5); // set SH_CP low
  }
  PORTB |=  (1 << PORTB2);  // set ST_CP high
  PORTB &= ~(1 << PORTB2);  // set ST_CP low
}

//----------------------------------------------------
int main(void) { 

  uint8_t wdata = 0x01; // only one bit is 1, the rest is 0.
  gpio_init();          // initialize GPIO pins
  
  while (1) { 
     shiftout( ~wdata );
     wdata = (wdata << 1) | (wdata >> 7); // rotate-shift-left
     _delay_ms(100);
  }
}

////////////////////////////////////////////////////////////////////////////
