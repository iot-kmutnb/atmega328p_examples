// Date: 2019-12-13
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>
#include <avr/interrupt.h>

//------------------------------------------------------

#define BAUD         115200UL
#define UBRR_VALUE   ((F_CPU/8/BAUD) - 1)

volatile uint8_t recv_data;

ISR(USART_RX_vect) {
  recv_data = UDR0;  // read data from the RX buffer
  UDR0 = recv_data;  // send data to the TX buffer
}

void init_uart() {
  // set a 16-bit value for the baudrate generator 
  UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
  UBRR0L = (uint8_t) (UBRR_VALUE);
  UCSR0A = (1 << U2X0); // Asynchronous double speed
  // Async., no parity, 1-stop bit, 8-bit data
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); 
  UCSR0B = (1<<TXEN0) |(1<<RXEN0); // Enable Tx & Rx
  UCSR0B |= (1 << RXCIE0); // enable interrupt on complete reception of a byte
}

void send_char( uint8_t ch ) {
  while ( !(UCSR0A & (1<<UDRE0)) ) {} // wait until TX buffer is empty
  UDR0 = ch;
}

//----------------------------------------------------
int main(void) { 
  init_uart();  // initialize UART
  sei();        // enable global interrupt
  while (1) {} 
}

////////////////////////////////////////////////////////////////////////////
