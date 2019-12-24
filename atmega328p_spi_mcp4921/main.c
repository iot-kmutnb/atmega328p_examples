// Date: 2019-12-09
// MCU Board: Arduino Nano v3 (ATmega328P) + MCP4921 (1-channel, 12-bit DAC, SPI)
// IoT Engineering Education @ KMUTNB 
  
#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>

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

void mcp4921_output( uint16_t wdata ) {
  // Input Buffered, Gain 1x, Output Enable
  wdata |= (0b0111 << 12); 
  PORTB &= ~(1 << PORTB2); // set /SS low
  // Start SPI transmission
  SPDR = (wdata >> 8) & 0xff;
  // Wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  // Start SPI transmission
  SPDR = wdata & 0xff;
  // Wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  PORTB |= (1 << PORTB2);   // set /SS high
  DDRD |= (1 << DDD2); // use D2 pin for /LDAC 
  PORTD &= ~(1 << PORTD2);
  asm volatile ("nop; nop;");
  PORTD |= (1 << PORTD2);
}

//----------------------------------------------------

uint16_t levels[] = { 0, 1023, 2047, 3071, 4095 };

int main(void) { 
  uint8_t index = 0;
  
  spi_master_init();
    
  while (1) {
    mcp4921_output( levels[index] );
    index = (index+1) % 5;
    _delay_ms(5);
  }
}

////////////////////////////////////////////////////////////////////////////
