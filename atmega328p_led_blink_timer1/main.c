// Date: 2019-11-26
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <avr/interrupt.h> 

void init_timer1() { // 4 usec per tick
  TCCR1A = TCCR1B = TCCR1C = 0;
  TCNT1  = 0;
  TCCR1B |= (1 << CS11)|(1 << CS10);   // Start timer 1 with Fcpu/64
}

int main(void) {
  uint16_t t_old, t_new;  
  
  DDRB |= (1 << DDB5); // use PB5 pin as output
  
  init_timer1();
  
  t_old = TCNT1;
  while (1) {
     // Toggle the LED every 100 msec (25000 * 4 usec)
     t_new = TCNT1;	 // read 16-bit Timer1 counter value
     if ( (t_new - t_old) >= 25000 ) {
        t_old = t_new;
        PORTB ^= (1 << PORTB5); // Toggle the LED at PB5 pin
     }
  }
}
