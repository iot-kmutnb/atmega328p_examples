// Date: 2019-12-12
// MCU Board: Arduino Nano v3 (ATmega328P) + 2x 74HC595 (via SPI) + 4-digit 7segment display
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <avr/interrupt.h>
#include <util/delay.h>

#include <avr/interrupt.h>

volatile uint16_t count = 0;  // global variable

void init_ext0(void) {
  DDRD &= ~(1 << DDD2);     // clear the PD2 pin
  PORTD |= (1 << PORTD2);   // turn on the Pull-up
  EICRA |= (1 << ISC01);    // set INT0 to trigger on falling edge
  EIMSK |= (1 << INT0);     // enable INT0
  sei();                    // enable global interrupts
}

ISR(INT0_vect) {
  count = (count+1) % 10000;    // count: 0..9999
}

//------------------------------------------------------

void spi_master_init(void) {
  // set /SS, MOSI and SCK output
  DDRB |= (1 << DDB2)|(1 << DDB3)| (1 << DDB5);
  // enable pull-up on MOSI pin
  PORTB |= (1 << PORTB4);
  // set /SS high
  PORTB |= (1 << PORTB2);
  // enable SPI in master mode, set clock rate: f_osc/4
  SPCR = (1 << SPE)|(1 << MSTR);
}

uint16_t spi_shiftout_bytes( uint8_t *wdata, uint8_t nbytes ) {
  uint8_t i;

  PORTB &= ~(1 << PORTB2); // set ST_CP low
  for (i=0; i < nbytes; i++ ) {
    // start SPI transmission
    SPDR = wdata[i]; 
    // wait for transmission complete (polling, no timeout check)
    while (!(SPSR & (1 << SPIF))) {}
  }
  PORTB |= (1 << PORTB2);  // set ST_CP high
  PORTB &= ~(1 << PORTB2); // set ST_CP low
}

#include <avr/pgmspace.h>
// see: https://www.nongnu.org/avr-libc/user-manual/pgmspace.html
const uint8_t DIGITS_TABLE[] PROGMEM = { // bcd-to-7seg decoder
  0b00111111, // Digit 0
  0b00000110, // Digit 1
  0b01011011, // Digit 2
  0b01001111, // Digit 3
  0b01100110, // Digit 4
  0b01101101, // Digit 5
  0b01111101, // Digit 6
  0b00000111, // Digit 7
  0b01111111, // Digit 8
  0b01101111  // Digit 9
};

// convert the value of count to 4-digit BCD for 7-segment display
void update_digits( uint8_t digits[4] ) {
  uint8_t i;
  uint16_t x = count; 
  for ( i=0; i < 4; i++ ) {
    digits[i] = pgm_read_byte( &DIGITS_TABLE[x%10] );
    x /= 10;
  }
}

int main(void) { 
  uint8_t i, wdata[2], digits[4]; 

  spi_master_init(); // initialize SPI as master
  init_ext0();       // initialize PD2 as INT0

  while (1) { 
     update_digits( digits );
     for ( i=0; i < 4; i++ ) {
        wdata[0] = digits[i] ^ 0xff; // inverse bits (active-low)
        wdata[1] = (1 << i);
        spi_shiftout_bytes( wdata, 2 );
        _delay_ms(5);
     }
  }
}

////////////////////////////////////////////////////////////////////////////
