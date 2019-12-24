// Date: 2019-12-12
// MCU Board: Arduino Nano v3 (ATmega328P) + MAX7219 (via SPI) + 8x8 LED dot matrix 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>

//------------------------------------------------------
#define REG_DIGIT(x)      (0x1+(x))
#define REG_DECODE_MODE   (0x9)
#define REG_INTENSITY     (0xA)
#define REG_SCAN_LIMIT    (0xB)
#define REG_SHUTDOWN      (0xC)
#define REG_DISP_TEST     (0xF)

void max7219_write_reg( uint8_t addr, uint8_t wdata ) {
  PORTB &= ~(1 << PORTB2); // set LOAD low
  // start SPI transmission for the first byte
  SPDR = addr;
  // wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  // start SPI transmission for the second byte
  SPDR = wdata;
  // wait for transmission complete (no timeout check)
  while (!(SPSR & (1 << SPIF))) {}
  PORTB |= (1 << PORTB2);  // set LOAD high
  PORTB &= ~(1 << PORTB2); // set LOAD low
}

void max7219_init(void) {
  uint8_t i;
  max7219_write_reg( REG_DECODE_MODE, 0x00 ); // no decode for digits 0-7
  max7219_write_reg( REG_INTENSITY,   0x07 ); // set intensity: 0x07=15/32
  max7219_write_reg( REG_SCAN_LIMIT,  0x07 ); // scan limit: display digits 0-7
  max7219_write_reg( REG_SHUTDOWN,    0x01 ); // shutdown: normal operation
  max7219_write_reg( REG_DISP_TEST,   0x00 ); // no display test
  for (i=0; i < 8; i++) { 
     max7219_write_reg( REG_DIGIT(i), 0x00 );  
  }
}

void max7219_flashing( uint8_t n ) {
  uint8_t i;
  for ( i=0; i < n; i++ ) {
    max7219_write_reg( REG_DISP_TEST, 0x01 ); // enter display test mode
    max7219_write_reg( REG_SHUTDOWN,  0x00 ); // shutdown operation
    _delay_ms(100);
    max7219_write_reg( REG_DISP_TEST, 0x00 ); // exit display test mode
    max7219_write_reg( REG_SHUTDOWN,  0x01 ); // normal operation
    _delay_ms(100);
  }
}

#include <avr/pgmspace.h>
// see: https://www.nongnu.org/avr-libc/user-manual/pgmspace.html

/*
 00111100
 01000010
 10100101
 10000001
 10100101
 10011001
 01000010
 00111100
 */
const uint8_t SMILE_FACE[8] PROGMEM = {
  0x3c,0x42,0xa9,0x85,0x85,0xa9,0x42,0x3c
};
void display_icon_8x8( uint8_t *icon ) {
  uint8_t i, wdata;
  for (i=0; i < 8; i++) {
     wdata = pgm_read_byte( &icon[i] );
     max7219_write_reg( REG_DIGIT(i), wdata );
  }
}

//------------------------------------------------------

void spi_master_init(void) {
  // set /SS, MOSI and SCK output
  DDRB |= (1 << DDB2)|(1 << DDB3)| (1 << DDB5);
  PORTB |= (1 << PORTB2); // set /SS high
  // enable pull-up on MOSI pin
  PORTB |= (1 << PORTB4);
  // enable SPI in master mode, set clock rate: f_osc/16
  SPCR = (1 << SPE)|(1 << MSTR)|(1<<SPR0);
}

int main(void) { 
  spi_master_init();   // initialize the SPI as master
  max7219_init();      // initialize the MAX7219-based LED Matrix Driver
  max7219_flashing(5); // flash the 8x8 LED matrix 5 times
  display_icon_8x8( SMILE_FACE ); // show a smile face
  while (1) {}
}

////////////////////////////////////////////////////////////////////////////
