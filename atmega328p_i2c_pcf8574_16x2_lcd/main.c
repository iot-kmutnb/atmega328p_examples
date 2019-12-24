// Date: 2019-12-05
// MCU Board: Arduino Nano v3 (ATmega328P) + PCF8574(A) Adapter for 16x2 LCD
// IoT Engineering Education @ KMUTNB 
// I2C pins: PC4 = SDA/A4, PC5 = SCL/A5

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>
#include <stdio.h>         // for sprintf()

#define I2C_ADDR 0x3f      // define the address for PCF8574 here

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

#define I2C_FREQ     (100000UL)
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

// P7..P0 = D7 | D6 | D5 | D4 | BL | EN | RW | RS
int pcf8574_write( uint8_t wdata ) { 
  uint8_t err = 0;
  wdata |= 0x08; // turn on backlight
  if ( !err && !i2c_start() )                   { err++; }
  if ( !err && !i2c_write_addr( I2C_ADDR, 0 ) ) { err++; }
  if ( !err && !i2c_write_byte( wdata ) )       { err++; }
  i2c_stop();
  return (err==0);
}

void write4bits( uint8_t wdata ) {
  pcf8574_write( wdata | (1<<2) ); // EN=1
  pcf8574_write( wdata );          // EN=0
  _delay_ms(2);
}

void lcd_write( uint8_t wdata, uint8_t cmd_mode ) {
  uint8_t high_nibble = wdata & 0xf0;
  uint8_t low_nibble = (wdata << 4) & 0xf0;
  write4bits( high_nibble | cmd_mode );
  write4bits( low_nibble  | cmd_mode ); 
}

void lcd_data( uint8_t wdata ) {
  lcd_write( wdata, (1 << 0) ); // RS=1
}

void lcd_cmd( uint8_t wdata ) {
  lcd_write( wdata, 0 );        // RS=0
}

void lcd_init() { 
  _delay_ms(40);   // wait at least 40 msec after LCD power on
  // change from 8-bit interface to 4-bit interface
  write4bits( 0x03 << 4 ); // 1. set 8-bit data interface
  _delay_us(4500); 
  write4bits( 0x03 << 4 ); // 2. set 8-bit data interface
  _delay_us(120);
  write4bits( 0x03 << 4 ); // 3. set 8-bit data interface
  write4bits( 0x02 << 4 ); // 4. change to 4-bit data interface 
  lcd_cmd( 0x28 ); // function set: 2-line, 5x8 dots
  lcd_cmd( 0x0c ); // display ctrl: display on, cursor off
  lcd_cmd( 0x80 );
}

void lcd_clear() {
  lcd_cmd( 0x01 );
}

void lcd_return_home() {
  lcd_cmd( 0x02 );
}

void lcd_goto_line( int line ) {
  lcd_cmd( 0x80 | ((line==0) ? 0x00 : 0x40) ); 
}

void lcd_print( const char *s ) {
  uint8_t i, len = strlen(s);
  for ( i=0 ; i < len; i++ ) {
     lcd_data( s[i] );
  }
}

int main(void) { 
  char sbuf[32];
  uint16_t cnt=0;
  
  init_uart();
  init_twi();
  scan_slaves( 0x20, 0x3f );
  
  lcd_init();
  lcd_goto_line(0);
  lcd_print("Hello World!" );

  while (1) {
    lcd_goto_line(1);
    sprintf( sbuf, "COUNT:  %04u", cnt );
    cnt = (cnt+1) % 10000;
    lcd_print( sbuf );
    _delay_ms(1000);
  }
}
////////////////////////////////////////////////////////////////////////////
