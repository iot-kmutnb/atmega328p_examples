// Date: 2019-12-19
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz CPU frequency
#include <avr/io.h>        // for accessing SFRs

typedef struct PORT_struct {
   uint8_t IN;
   uint8_t DIR;
   uint8_t OUT;
} PORT_t;

// Base address for I/O port registers
#define IO_PORTB  (0x23) // PORT B 
#define IO_PORTC  (0x26) // PORT C 
#define IO_PORTD  (0x29) // PORT D 

template <int port_addr, int pin> class DigitalInOut {
  public:
    DigitalInOut() {}
    void out()    { port()->DIR |=  pin_bm(); }
    void high()   { port()->OUT |= pin_bm();  }
    void low()    { port()->OUT &= ~pin_bm(); }
    void toggle() { port()->IN  |= pin_bm(); }
    bool read()   { 
      return (port()->IN & pin_bm()) ? true:false; 
    }
    void in_pullup( bool enable=true ) { 
      port()->DIR &= ~pin_bm();
      if (enable) { 
        port()->OUT |=  pin_bm();
      } else { 
        port()->OUT &= ~pin_bm();
      }
    } 
    static volatile PORT_t* port() { 
      return (PORT_t*)port_addr; 
    }
    static uint8_t pin_bm() { return (1 << pin); }
};

DigitalInOut< IO_PORTB, 5 > led; 
DigitalInOut< IO_PORTD, 2 > btn; 

int main(void) {
  led.out();
  btn.in_pullup(true);
  
  while (1) {
    if ( !btn.read() ) {
      led.high();
    } else {
      led.low();
    }
  }
}
