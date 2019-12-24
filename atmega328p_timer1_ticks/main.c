// Date: 2019-12-22
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU   16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t _timer1_ticks = 0;

uint32_t get_ticks() {
  uint32_t t;
  uint8_t sreg = SREG; // save SREG
  cli();
  t = _timer1_ticks;
  SREG = sreg;         // restoe SREG
  return t;
}

void init_timer1() {
  TCCR1A = TCCR1B = TCCR1C = 0;
  TCNT1 = 0;
  OCR1A = (250-1);
  TIMSK1 |= (1<<OCIE1A);
  TIFR1  |= (1<<OCF1A);
  TCCR1B |= (1<<WGM12)|(1<<CS11)|(1<<CS10);
}

ISR(TIMER1_COMPA_vect) {
  _timer1_ticks++;
}

int main(void) {
   uint32_t ts, tc;
   
   DDRB |= (1 << PIN5);  // use PB5 as output
   init_timer1();        // initialize Timer1 
   sei();                // enable global interrupts
   
   ts = get_ticks();
   while(1) {
     tc = get_ticks(); // get timer1 tick count
     if ( (tc - ts) >= 500 ) { // toggle PB5 every 500 msec
        ts = tc;
        PORTB ^= (1 << PIN5); 
     }
   }
}
