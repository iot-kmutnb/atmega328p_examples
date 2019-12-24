// Date: 2019-12-13
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>    // for _delay_ms() funtion
#include <avr/interrupt.h> // for ISR()
#include <stdio.h>         // for sprintf()

//------------------------------------------------------

#define BAUD         115200UL
#define UBRR_VALUE   ((F_CPU/8/BAUD) - 1)

volatile int tx_done = 0;
volatile uint8_t *p; 

ISR(USART_TX_vect) {
  if ( *p == 0 ) {
    tx_done = 1;
  } else {
    UDR0 = *p++; 
  }
}

void init_uart() {
  // set a 16-bit value for the baudrate generator 
  UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
  UBRR0L = (uint8_t) (UBRR_VALUE);
  UCSR0A = (1 << U2X0); // Asynchronous double speed
  // Async., no parity, 1-stop bit, 8-bit data
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); 
  UCSR0B = (1<<TXEN0) |(1<<RXEN0); // Enable Tx & Rx
  UCSR0B |= (1 << TXCIE0); // enable interrupt on Tx complete
}

void send_string( char *s ) {
  p = (uint8_t *)s;
  if ( *p != 0 ) {
    tx_done = 0;
    while( !(UCSR0A & (11 <<UDRE0)) ) {}
    UDR0 = *p++;  // send the first char
  } else {
    tx_done = 1;
  }
}

//----------------------------------------------------

int main(void) {
  char sbuf[32], ch;
  uint16_t cnt = 0;
    
  init_uart();  // initialize UART
  sei();        // enable global interrupt
  DDRB |= (1 << DDB5); // use PB5 as output

  // wait for an incoming char from serial ...
  while ( !(UCSR0A & (1<<RXC0)) ) {} // wait until RX data is complete
  ch = UDR0; // read data from the UART data register

  while (1) {
     sprintf( sbuf, "Hello World!!!! [0x%X]\r\n", cnt++ );
     PORTB |= (1 << PORTB5);   // write 1 to PB5 output
	 
     send_string( sbuf );      // start sending string to serial 
     while ( !tx_done ) {}     // wait until string sending process done
	 
     PORTB &= ~(1 << PORTB5);  // write 0 to PB5 output
     _delay_ms(100);
  } 
}
////////////////////////////////////////////////////////////////////////////
