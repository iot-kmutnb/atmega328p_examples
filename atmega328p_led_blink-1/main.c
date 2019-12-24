// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>

#define DELAY_MS (500)

//----------------------------------------------------
int main(void) { 

  DDRB |= (1 << DDB5); // use PB5 as output
  
  while (1) { 
     PORTB |= (1 << PORTB5);
     _delay_ms( DELAY_MS );
	 
     PORTB &= ~(1 << PORTB5);
     _delay_ms( DELAY_MS );
  }
}

////////////////////////////////////////////////////////////////////////////
