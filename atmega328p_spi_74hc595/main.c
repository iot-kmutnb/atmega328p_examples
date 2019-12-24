// Date: 2019-12-12
// MCU Board: Arduino Nano v3 (ATmega328P) + 74HC595 via SPI bus
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>

//------------------------------------------------------

void spi_master_init(void) {
  // set /SS (PB2), MOSI (PB3) and SCK (PB5) output
  DDRB |= (1 << DDB2)|(1 << DDB3)| (1 << DDB5);
  // enable pull-up on MOSI pin (PB4)
  PORTB |= (1 << PORTB4);
  // set /SS high (PB2)
  PORTB |= (1 << PORTB2);
  // enable SPI in master mode, set clock rate: f_osc/4
  SPCR = (1 << SPE)|(1 << MSTR);
}

uint16_t spi_shiftout( uint8_t wdata ) {
  PORTB &= ~(1 << PORTB2); // set ST_CP low
  // start SPI transmission 
  SPDR = wdata;
  // wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  PORTB |=  (1 << PORTB2);  // set ST_CP high
  PORTB &= ~(1 << PORTB2);  // set ST_CP low
}

//----------------------------------------------------
int main(void) { 
  uint8_t wdata = 0x01; // only one bit is 1, the rest is 0.
  
  spi_master_init(); // initialize SPI as master
  
  while (1) { 
     // inverse all bits and shift out to 74HC595
     spi_shiftout( ~wdata );
     wdata = (wdata << 1) | (wdata >> 7); // rotate-shift-left
     _delay_ms(100);
  }
}
////////////////////////////////////////////////////////////////////////////
