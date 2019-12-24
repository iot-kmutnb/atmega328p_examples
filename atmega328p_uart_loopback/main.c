// Date: 2019-12-13
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>    // for _delay_ms() funtion

//------------------------------------------------------

#define BAUD         115200UL
#define UBRR_VALUE   ((F_CPU/8/BAUD) - 1)

void init_uart() {
  // set a 16-bit value for the baudrate generator 
  UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
  UBRR0L = (uint8_t) (UBRR_VALUE);
  UCSR0A = (1 << U2X0); // Asynchronous double speed
  // Async., no parity, 1-stop bit, 8-bit data
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); 
  UCSR0B = (1<<TXEN0) |(1<<RXEN0); // Enable Tx & Rx
}

void send_char( uint8_t ch ) {
  while ( !(UCSR0A & (1<<UDRE0)) ) {} // wait until TX buffer is empty
  UDR0 = ch; // send data to the TX buffer
}

uint8_t recv_char( void ) {
  while ( !(UCSR0A & (1<<RXC0)) ) {} // wait until RX data is complete
  return UDR0;  // read data from the UART RX data register
}

//----------------------------------------------------
int main(void) {
  DDRB |= (1<< DDB5); // use PB5 as output 

  init_uart();  // initialize UART
 
  while (1) { // UART loopback test (blocking calls)
    uint8_t recv_data = recv_char();
    send_char( recv_data );
    PINB |= (1 << PINB5); // toggle PB5
  }
}

////////////////////////////////////////////////////////////////////////////
