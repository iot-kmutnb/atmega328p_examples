// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 


#define F_CPU 16000000UL // 16MHz CPU frequency
#include <avr/io.h>      // for accessing SFRs
#include <util/delay.h>

typedef struct { // anonymous struct
  uint8_t  UCSRA;
  uint8_t  UCSRB;
  uint8_t  UCSRC;
  uint8_t  RESERVED;
  uint8_t  UBRRL;
  uint8_t  UBRRH;
  uint8_t  UDR;
} USART_t;

#define USART0  ((volatile USART_t *)0xC0)

#define BAUD 115200UL
#define UBRR_VALUE ((F_CPU/8/BAUD) - 1)

void init_USART0() {
  USART0->UBRRH = (uint8_t) (UBRR_VALUE >> 8);
  USART0->UBRRL = (uint8_t) (UBRR_VALUE);
  USART0->UCSRA = (1<<U2X0); 
  USART0->UCSRC = (1<<UCSZ01) | (1<<UCSZ00);
  USART0->UCSRB = (1<<TXEN0)  | (1<<RXEN0);
}

int main(void) { 
  uint8_t ch;
  
  init_USART0();
  
  while (1) { 
    // wait for incoming byte
    while ( !(USART0->UCSRA & (1<<RXC0) ) ) {}
    ch = USART0->UDR; // read the received byte
	
    while ( !(USART0->UCSRA & (1<<UDRE0)) ) {}
    USART0->UDR = ch; // send back
  }
}
////////////////////////////////////////////////////////////////////////////
