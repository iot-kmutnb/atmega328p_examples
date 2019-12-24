// Date: 2019-12-22
// MCU Board: Arduino Nano v3 (ATmega328P) 
// IoT Engineering Education @ KMUTNB 

#define F_CPU 16000000UL
#include <avr/io.h>

// PINB = 0x03 (0x23), DDRB = 0x04 (0x24), PORTB = 0x05 (0x25)

void test1() {
  asm volatile(
    "sbi 0x04, %0 \n"   // 1C: set DDRB5 to 1  
    "out 0x05, %1 \n"   // 1C: PORTB = (1<<5)
    "L1:          \n"
    "out 0x03, %1 \n"   // 1C: PINB  = (1<<5)
    "rjmp L1      \n"   // 2C
    : : "I" (5), "r" (1<<5)
  );
}

void test2() {
  asm volatile(
    "sbi 0x04, %0 \n"   // 1C: set DDRB5 to 1  
    "sts 0x25, %1 \n"   // 2C: PORTB = (1<<5)
    "L2:          \n"
    "sts 0x23, %1 \n"   // 2C: PINB  = (1<<5)
    "rjmp L2      \n"   // 2C
    : : "I" (5), "r" (1<<5)
  );
}

void test3() {
  asm volatile(
    "sbi 0x04, %0 \n"          // 1C: set DDRB5 to 1  
    "sts 0x25, %1 \n"          // 2C: PORTB = (1<<5)
    "mov __tmp_reg__, %1 \n"   // 1C: t = (1<<5)
    "L3:                 \n"
    "lds %1, 0x25        \n"   // 2C: r = PORTB
    "eor %1, __tmp_reg__ \n"   // 1C: r = r ^ t
    "sts 0x25, %1        \n"   // 2C: PORTB = r
    "rjmp L3             \n"   // 2C
    : : "I" (5), "r" (1<<5)
  ); 
}

void test4() {
  asm volatile(
    "sbi 0x04, %0 \n"          // 1C: set DDRB5 to 1  
    "sts 0x25, %1 \n"          // 2C: PORTB = (1<<5)
    "mov __tmp_reg__, %1 \n"   // 1C: t = (1<<5)
    "L2:                 \n"
    "in  %1, 0x05        \n"   // 1C: r = PORTB
    "eor %1, __tmp_reg__ \n"   // 1C: r = r ^ t
    "out 0x05, %1        \n"   // 1C: PORTB = r
    "rjmp L2             \n"   // 2C
    : : "I" (5), "r" (1<<5)
  ); 
}

int main(void) {
  test1();
}
