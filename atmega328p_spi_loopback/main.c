// Date: 2019-12-09
// MCU Board: Arduino Nano v3 (ATmega328P) - SPI loopback testing
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>
#include <stdio.h>         // for sprintf()

// MISO = PB4, /SS = PB2, MOSI = PB3, SCK = PB5
//------------------------------------------------------

#define BAUD         115200UL
#define UBRR_VALUE   ((F_CPU/8/BAUD) - 1)

void init_uart() {
  UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
  UBRR0L = (uint8_t) (UBRR_VALUE);
  UCSR0A = (1 << U2X0); // Asynchronous double speed
  // Async., no parity, 1-stop bit, 8-bit data
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); 
  UCSR0B = (1<<TXEN0) |(1<<RXEN0); // Enable Tx & Rx  
}

void send_char( uint8_t ch ) {
  while ( !(UCSR0A & (1<<UDRE0)) ){ }
  UDR0 = ch;
}

void send_string( const char *str ) {
  const char *p = str;
  while ( *p ) {
    send_char( (uint8_t) *p++ );
  }
}

//------------------------------------------------------

void spi_master_init(void) {
  // set /SS, MOSI and SCK output
  DDRB |= (1 << DDB2)|(1 << DDB3)| (1 << DDB5);
  // enable pull-up on MOSI pin
  PORTB |= (1 << PORTB4);
  // set /SS high
  PORTB |= (1 << PORTB2);
  // enable SPI in master mode, set clock rate: f_osc/2
  SPCR = (1 << SPE)|(1 << MSTR);
  SPSR |= (1 << SPI2X);
}

void spi_master_transmit( uint8_t wdata, uint8_t *rdata ) {
  PORTB &= ~(1 << PORTB2); // set /SS low
  // Start SPI transmission
  SPDR = wdata;
  // Wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  *rdata = SPDR;
  PORTB |= (1 << PORTB2);   // set /SS high
}

//----------------------------------------------------

int main(void) { 
  uint8_t wdata = 0x00, rdata;
  char sbuf[32];
  
  init_uart();       // initialize UART
  spi_master_init(); // initialize SPI (master)

  while (1) { // SPI loopback test (connect MOSI to MISO)
    spi_master_transmit( wdata, &rdata ); 
    sprintf( sbuf, "W: 0x%02X, R: 0x%02X\r\n", wdata, rdata );
    send_string( sbuf );
    wdata++;
    _delay_ms( 100 );
  }
}
////////////////////////////////////////////////////////////////////////////
