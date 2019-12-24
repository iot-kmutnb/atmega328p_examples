// Date: 2019-12-18
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 


#define F_CPU 16000000UL // 16MHz CPU frequency
#include <avr/io.h>      // for accessing SFRs
#include <util/delay.h>

typedef struct { // anonymous struct
  uint8_t mpcm:1;
  uint8_t u2x:1;
  uint8_t pe:1;
  uint8_t dor:1;
  uint8_t fe:1;
  uint8_t udre:1;
  uint8_t txc:1;
  uint8_t rxc:1;
} UCSRA_t;

typedef struct { // anonymous struct
  uint8_t txb8:1;
  uint8_t rxb8:1;
  uint8_t ucsz2:1;
  uint8_t txen:1;
  uint8_t rxen:1;
  uint8_t udrie:1;
  uint8_t tcie:1;
  uint8_t rxcie:1;
} UCSRB_t;

typedef struct { // anonymous struct
  uint8_t ucpol:1;
  uint8_t ucsz0:1;
  uint8_t ucsz1:1;
  uint8_t usbs:1;
  uint8_t upm0:1;
  uint8_t upm1:1;
  uint8_t umsel0:1;
  uint8_t umsel1:1;
} UCSRC_t;

typedef struct { // anonymous struct
  union { uint8_t UCSRA;  UCSRA_t UCSRA_bits; };
  union { uint8_t UCSRB;  UCSRB_t UCSRB_bits; };
  union { uint8_t UCSRC;  UCSRC_t UCSRC_bits; };
  uint8_t RESERVED;
  uint16_t UBRR;
  uint8_t  UDR;
} USART_t;

#define USART0  ((volatile USART_t *)0xC0)

#define BAUD 115200UL
#define UBRR_VALUE ((F_CPU/8/BAUD) - 1)

void init_USART0() {
  USART0->UBRR = UBRR_VALUE;
  USART0->UCSRA_bits.u2x   = 1; 
  USART0->UCSRC_bits.ucsz1 = 1;
  USART0->UCSRC_bits.ucsz0 = 1;
  USART0->UCSRB_bits.txen  = 1;
  USART0->UCSRB_bits.rxen  = 1;
}

int main(void) { 
  uint8_t ch;
  init_USART0();
  while (1) { 
    // wait for incoming byte
    while ( !(USART0->UCSRA_bits.rxc) ) {}
    ch = USART0->UDR; // read the received byte
	
    while ( !(USART0->UCSRA_bits.udre) ) {}
    USART0->UDR = ch; // send back
  }
}

////////////////////////////////////////////////////////////////////////////
