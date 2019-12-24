// Date: 2019-12-21
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

typedef void (*callback_t)(void);
callback_t timer1_cb = 0;

ISR(TIMER1_COMPA_vect) {
  TIFR1 |= (1<<OCF1A);
  if ( timer1_cb ) { 
    timer1_cb(); 
  }
}

void init_timer1() {
  TCCR1A = TCCR1B = TCCR1C = 0;
  TCNT1 = 0;
  OCR1A = (250-1);
  TIMSK1 |= (1<<OCIE1A);
  TIFR1  |= (1<<OCF1A);
  TCCR1B |= (1<<WGM12)|(1<<CS11)|(1<<CS10);
}

int main(void) {
   DDRB |= (1 << PIN5);
   
   init_timer1();
   sei();
   
   timer1_cb = [](void) { // set anonymous callback function
      static uint16_t cnt = 0;
      if ( ++cnt == 500 ) {
        PORTB ^= (1 << PIN5); 
        cnt = 0;
      } 
   };
   
   while(1) {}
}
