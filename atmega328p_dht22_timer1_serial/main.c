// Date: 2019-11-26
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 
// Note: use the PD2 pin to connect the DATA pin of DHT22 sensor module

#define F_CPU 16000000UL   // 16MHz Crystal Frequency

#include <avr/io.h>        // for accessing the SFRs
#include <util/delay.h>    // for delay functions
#include <avr/interrupt.h> // for sei() and ISRs
#include <stdio.h>         // for sprintf()

void initTimer1() { // 4 usec resolution
  TCCR1A = TCCR1B = TCCR1C = 0;
  TCNT1  = 0;
  // Start Timer1, Fcpu/64, no interrupt
  TCCR1B |= (1 << CS11)|(1 << CS10); 
}

#define BAUD         115200UL
#define UBRR_VALUE   ((F_CPU/8/BAUD) - 1)

void initUSART() {
  UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
  UBRR0L = (uint8_t) (UBRR_VALUE);
  UCSR0A = (1 << U2X0); // Asynchronous double speed
  // Async., no parity, 1-stop bit, 8-bit data
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); 
  UCSR0B = (1<<TXEN0) |(1<<RXEN0); // Enable Tx & Rx  
}

void sendChar( uint8_t ch ) {
  while ( !(UCSR0A & (1<<UDRE0)) ){ }
  UDR0 = ch;
}

void sendString( const char *str ) {
  const char *p = str;
  while ( *p ) {
    sendChar( (uint8_t) *p++ );
  }
}

// Global variable
uint8_t dbuf[5]; // 5-byte data buffer

void data_output( uint8_t _bit ) {
  DDRD |= (1 << DDD2);
  if ( _bit ) {
    PORTD |= (1 << PORTD2);
  } else {
    PORTD &= ~(1 << PORTD2);  
  }
}

void data_input() {
  DDRD &= ~(1 << DDD2); 
}

uint8_t read_data() {
  return ((PIND >> PIND2) & 1);
}

void readSensor() {
  int i;
  uint8_t _data = 0x00;
  uint16_t ts;

  // Send the START signal 
  data_output(0); // output 0
  _delay_ms(1);   // delay 1 msec (min.)
  data_output(1); // output 1
  data_input();   // change to input

  // Wait for the RESPONSE signal
  while ( read_data()) {} // wait until DATA goes 0
  while (!read_data()) {} // wait until DATA goes 1
  while ( read_data()) {} // wait until DATA goes 0
  
  // Read 40-bit data
  for ( i=0; i < 40; i++ ) {
    while (!read_data()) {} // wait until DATA goes 1
    TCNT1 = 0;              // reset Timer1 counter
    while ( read_data()) {} // wait until DATA goes 0
    ts = TCNT1;             // read Timer1 counter
    _data = (_data << 1) | ((ts > 8) ? 1 : 0);    
    if ( i % 8 == 7 ) {     // 8 bits complete
       dbuf[i/8] = _data;   // save the data byte
       _data = 0x00;
    }
  }
  while ( !read_data() ) {}   // wait until DATA goes 1
}

void showTemperature() {
  int16_t t;
  char sbuf[32];
  char sign;

  // 16-bit signed integer for temperature
  t = (dbuf[2] << 8) | dbuf[3]; 
  if ( t < 0 ) { 
    t = -t; 
    sign = '-'; 
  } else {
    sign = '+';
  }
  sprintf( sbuf, "T = %c%d.%d deg.C\r\n", sign, t/10, t%10 );
  sendString( sbuf );
}

void showHumidity() {
  int16_t h;
  char sbuf[32];

  // 16-bit signed integer for relative humidity
  h = (dbuf[0] << 8) | dbuf[1];
  sprintf( sbuf, "H = %d.%d %%RH\r\n", h/10, h%10 );
  sendString( sbuf );
}

void showChecksum() {
  uint8_t x = (dbuf[0]+dbuf[1]+dbuf[2]+dbuf[3]);

  if ( dbuf[4] == x ) {
    sendString( "Checksum: OK\r\n" );
  } else {
    sendString( "Checksum: FAILED\r\n" );
  }  
}

int main(void) {
  initTimer1();
  initUSART();

  sendString( "Start reading DHT22 sensor...\r\n" );

  while (1) {
    readSensor();
    showTemperature();
    showHumidity();
    showChecksum();
    _delay_ms(2000);
  }
}

