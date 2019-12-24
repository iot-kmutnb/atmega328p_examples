// Date: 2019-12-20
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

typedef void (*callback_t)(void);

static callback_t __timer1_callback = 0;

static uint32_t __interval_msec = 0;
static uint32_t __ticks = 0;

ISR(TIMER1_COMPA_vect) {
  __ticks++;
  if ( __ticks >= __interval_msec ) {
     __ticks = 0;
     if ( __timer1_callback ) {
        __timer1_callback();
    }
  }
}

class Timer {
  public:
    void start() {
      // CTC mode, fCPU/64 = 4usec
      TCCR1B |= (1<<WGM12)|(1<<CS11)|(1<<CS10); 
      TIMSK1 |= (1<<OCIE1A); // enable Timer1 compare interupt A
      TIFR1  |= (1<<OCF1A);  // clear the OCF1A flag
      sei();
    } 
	
    void stop() {
      TIMSK1 &= ~(1<<OCIE1A); // disable Timer1 compare interupt A
      TIFR1  |=  (1<<OCF1A);  // clear the OCF1A flag
    }
	
    void init() {
      TCCR1A = TCCR1B = TCCR1C = 0;
      TCNT1 = 0;
      OCR1A = 249;
    }
	
    void setCallback( callback_t cb, uint32_t interval_msec ) { 
      __timer1_callback = cb;
      __interval_msec = interval_msec;
      __ticks = 0;
    }
	
    static Timer *getInstance();

  private:
    static Timer* timer;
    Timer() { init(); }
};

Timer *Timer::timer = 0;

Timer *Timer::getInstance() {
  if ( timer == 0 ) {
    timer = new Timer();
  }
  return timer;
}

void toggle(void) {
  PORTB ^= (1 << 5);  
}

int main(void) {
  DDRB |= (1 << 5); // use PB5 as output
  
  Timer *timer = Timer::getInstance();
  timer->setCallback( toggle, 500 );
  timer->start();
  
  while (1) {
    _delay_ms(1);
  }
}
