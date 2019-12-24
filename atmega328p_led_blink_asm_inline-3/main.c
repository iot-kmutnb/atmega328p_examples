// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz CPU Frequency
#include <avr/io.h>        // for accessing SFRs

int main(void) { 
   uint8_t pin5_bm = (1<<5);

  // set PB5 as output 
  // toggle PB5 output (read-modify-write cycle)
  
  asm volatile (
     "out %0, %1   \n"
     "_LOOP:       \n"
     "  sbi %2, %3 \n"  /* set bit at PORTB5   */
     "  cbi %2, %3 \n"  /* clear bit at PORTB5 */
     "  rjmp _LOOP \n"
     : : "I" (_SFR_IO_ADDR(DDRB)),  "r" (pin5_bm),
         "I" (_SFR_IO_ADDR(PORTB)), "I" (PORTB5)
  );
}
