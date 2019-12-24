// Date: 2019-12-11
// MCU Board: Arduino Nano v3 (ATmega328P) + MCP3208 (12-bit SPI ADC, SPI)
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>
#include <stdio.h>         // for sprintf()

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
  // enable SPI in master mode, set clock rate: f_osc/16
  SPCR = (1 << SPE)|(1 << MSTR)|(1<<SPR0);
}

uint16_t mcp3208_read( uint8_t channel ) {
  uint16_t wdata, rdata = 0;
  wdata = (0b11 << 9); // start bit=1, single-ended=1
  wdata |= ((channel & 0b111) << 6); // set channel bits

  PORTB &= ~(1 << PORTB2); // set /SS low
  // Start SPI transmission for the first byte
  SPDR = (wdata >> 8) & 0xff;
  // Wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  // Start SPI transmission for the second byte
  SPDR = wdata & 0xff;
  // Wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  rdata = (uint16_t)SPDR;
  // Start SPI transmission for the third byte
  SPDR = 0x00;
  // Wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  rdata = (rdata << 8) | SPDR;
  PORTB |= (1 << PORTB2); // set /SS high
  return rdata & 0x0fff;
}

//----------------------------------------------------

int main(void) { 
  uint8_t channel = 0;
  char sbuf[32], fstr[8];
  uint32_t mvolt;
  
  init_uart();       // initialize UART
  spi_master_init(); // initialize SPI as master

  while (1) { // read the first 3 of 8 analog channels
    uint16_t x = mcp3208_read( channel );
    mvolt = (5000UL*x) / 4096; // assume Vref = 5V 
    sprintf( sbuf, "CH%u: %4u, %4u [mV]\r\n", channel, x, mvolt );
    send_string( sbuf );
    if ( ++channel == 3 ) { // read only the first three channels
       channel = 0;
       _delay_ms(1000);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
