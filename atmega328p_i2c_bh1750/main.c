// Date: 2019-12-02
// MCU Board: Arduino Nano v3 (ATmega328P) + BH1750 I2C Light sensor module
// IoT Engineering Education @ KMUTNB 
// I2C pins: PC4 = SDA/A4, PC5 = SCL/A5

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
  // wait until START is sent  
  while ( !(TWCR & (1<<TWINT)) ) {} 
  twi_status = (TWSR & 0xf8);
  // TW_START or TW_RESTART
  return (twi_status==0x08 || twi_status==0x10); 
}

void i2c_stop() {
  // send STOP condition
  TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); 
  while( !(TWCR & (1<<TWSTO)) ) {} 
}

uint8_t i2c_write_addr( uint8_t addr, uint8_t read_not_write ) {
  TWDR = (addr << 1) | (read_not_write & 1); // send address + R/W
  TWCR = (1<<TWEN)|(1<<TWINT);
  while ( !(TWCR & (1<<TWINT)) ) {} // wait for addr transmission
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

uint8_t init_bh1750( uint8_t addr, uint8_t mode ) {
  uint8_t err=0;
  if ( !i2c_start() )                      { err++; }
  if ( !err && !i2c_write_addr(addr, 0) )  { err++; }
  if ( !err && !i2c_write_byte(mode) )     { err++; }
  i2c_stop();  
  return (err==0);
}

uint8_t read_bh1750( uint8_t addr, uint16_t *level ) {
  uint8_t err=0, rdata[2];
  uint32_t raw_value;
  
  if ( !i2c_start() )                           { err++; }
  if ( !err && !i2c_write_addr( addr, 1) )      { err++; }
  if ( !err && !i2c_read_byte( &rdata[0], 1 ) ) { err++; }
  if ( !err && !i2c_read_byte( &rdata[1], 0 ) ) { err++; }  
  i2c_stop();
  raw_value = rdata[0];
  raw_value = (raw_value << 8) | rdata[1];
  raw_value = (5*raw_value)/6;
  *level = (uint16_t)raw_value;
  return (err==0);
}

//------------------------------------------------------
int main(void) { 
  char sbuf[32];
  uint16_t light_level;

  init_uart(); // initialize UART
  init_twi();  // initialize TWI

  // use BH1750 address = 0x23, mode=0x10 (continuous, H-resolution)
  if ( !init_bh1750( 0x23, 0x10 ) ) {
    sprintf( sbuf, "error: init_bh1750()" );
    send_string( sbuf ); 
  }
  
  while (1) {
     if ( read_bh1750( 0x23, &light_level ) ) {
       sprintf( sbuf, "ok: %u lx\r\n", light_level );
     } else {
       sprintf( sbuf, "error: read_bh1750()\r\n" );
     }
     send_string( sbuf ); 
     _delay_ms(200);
  }
}

////////////////////////////////////////////////////////////////////////////
