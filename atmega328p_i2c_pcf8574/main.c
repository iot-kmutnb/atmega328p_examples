// Date: 2019-11-26
// MCU Board: Arduino Nano v3 (ATmega328P) + PCF8574(A)
// IoT Engineering Education @ KMUTNB 
// I2C pins:  PC4 = SDA/A4, PC5 = SCL/A5

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>
#include <stdio.h>         // for sprintf()

#define I2C_ADDR  0x20     // define the 7-bit I2C address for PCF8574(A)

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

#define I2C_FREQ     (400000UL)
#define TWI_BRG      (((F_CPU/I2C_FREQ)-16)/2)

// global variable
static uint8_t twi_status = 0x00;

void init_twi() {
  TWCR &= ~(1<<TWEN); // Disable TWI 
  TWSR = 0;
  TWBR = (uint8_t)TWI_BRG; // Set bitrate generator
  TWCR = (1<<TWEN);   // Enable TWI 
}

uint8_t i2c_start() {
  // send START condition
  TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA); 
  while ( !(TWCR & (1<<TWINT)) ) {} // wait for START 
  twi_status = (TWSR & 0xf8);
  // TW_START or TW_RESTART
  return (twi_status==0x08 || twi_status==0x10); 
}

void i2c_stop() {
  TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); 
  while( !(TWCR & (1<<TWSTO)) ) {} // wait for STOP
}

uint8_t i2c_write_addr( uint8_t addr, uint8_t read_not_write ) {
  TWDR = (addr << 1) | (read_not_write & 1); // send address + R/W
  TWCR = (1<<TWEN)|(1<<TWINT);
  while ( !(TWCR & (1<<TWINT)) ) {} // wait for address transmission
  twi_status = (TWSR & 0xf8);
  // TW_MT_SLA_ACK or TW_MR_SLA_ACK
  return ( twi_status==0x18 || twi_status==0x40 );
}

uint8_t i2c_write_byte( uint8_t wdata ) {
  TWDR = wdata; // send a byte
  TWCR = (1<<TWEN)|(1<<TWINT);
  // wait until the byte is sent
  while ( !(TWCR & (1<<TWINT)) ) {} // wait for data transmission
  twi_status = (TWSR & 0xf8);
  return ( twi_status==0x28 ); // TW_MT_DATA_ACK
}

uint8_t i2c_read_byte( uint8_t *rdata, uint8_t ack ) {
  if (ack) { // read a byte with ACK
     TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
  } else {   // read a byte without ACK (No ACK)
     TWCR = (1<<TWINT)|(1<<TWEN);
  }
  while ( !(TWCR & (1<<TWINT)) ) {} // wait for data reception
  *rdata = TWDR;
  twi_status = (TWSR & 0xf8);
  // TW_MR_DATA_ACK or TW_MR_DATA_NACK
  return ( twi_status==0x50 || twi_status==0x58 );
}

//----------------------------------------------------

int scan( uint8_t addr ) {
  if ( i2c_start() && i2c_write_addr( addr, 0 ) ) { 
     i2c_stop();
     return 0; // ACK received from slave (found)
  } 
  i2c_stop();
  return -1;
}

void scan_slaves( uint8_t start_addr, uint8_t end_addr ) {
   uint8_t addr, cnt=0;
   char sbuf[32];
   for ( addr=start_addr; addr <= end_addr; addr++ ) {
     if ( scan(addr)==0 ) {
       cnt++;
       sprintf( sbuf, "Found: 0x%02x\r\n", addr );
       send_string( sbuf );
     } 
     _delay_ms(5);
   }
   if (cnt==0) {
     send_string( "No I2C device found.." );
   }
}

//------------------------------------------------------

int pcf8574_test( uint8_t addr, uint8_t wdata, uint8_t *rdata ) {
  uint8_t err = 0;
  if ( !err && !i2c_start() )               { err++; }
  if ( !err && !i2c_write_addr( addr, 0 ) ) { err++; }
  if ( !err && !i2c_write_byte( wdata ) )   { err++; }
  if ( !err && !i2c_start() )               { err++; }
  if ( !err && !i2c_write_addr( addr, 1 ) ) { err++; }
  if ( !err && !i2c_read_byte( rdata, 0) )  { err++; }
  i2c_stop();
  return (err==0);
}

int main(void) { 
  char sbuf[32];
  uint8_t wdata = 0x01, rdata;
  
  init_uart();
  init_twi();
  
  while (1) {
    if ( pcf8574_test( I2C_ADDR, ~wdata, &rdata ) ) {
      sprintf( sbuf, "ok: 0x%02x\r\n", rdata );
      wdata = ((wdata << 1) | (wdata >> 2)) & 0b111;
    } else {
      sprintf( sbuf, "PCF8574 I/O error\r\n" );
    }
    send_string( sbuf );   
    _delay_ms(200);
  }
}
////////////////////////////////////////////////////////////////////////////
