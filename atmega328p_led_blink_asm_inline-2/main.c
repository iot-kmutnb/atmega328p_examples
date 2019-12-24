// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency
#include <avr/io.h>        // for accessing the SFRs

int main(void) { 
  // set PB5 as output
  asm volatile (
     "out %0, %1 \n"
     : : "I" (_SFR_IO_ADDR(DDRB)), "r" (1 << DDB5)
  );
  
  while (1) { // toggle PB5 pin as fast as possible
     // write 1 and then 0 to PORTB5
     asm volatile (
       "sbi %0, %1 \n"
       "cbi %0, %1 \n"
       : : "I" (_SFR_IO_ADDR(PORTB)), "I" (PORTB5)
     );
  }
}


////////////////////////////////////////////////////////////////////////////
