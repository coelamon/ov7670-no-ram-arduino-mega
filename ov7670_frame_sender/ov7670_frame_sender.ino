/* code taken from ComputerNerd/ov7670-no-ram-arduino-uno
 * and changed a little
 *
 * OV7670 NO FIFO
 * Arduino Mega 2560
 * 
 */

#include "ov7670.h"

/* Configuration: this lets you easily change between different resolutions
 * You must only uncomment one
 * no more no less*/
//#define useVga
//#define useQvga
#define useQqvga

static inline void serialWrB(uint8_t dat){
	while(!( UCSR0A & (1<<UDRE0))); // wait for byte to transmit
	UDR0=dat;
	while(!( UCSR0A & (1<<UDRE0))); // wait for byte to transmit
}
static void StringPgm(const char * str){
	if (!pgm_read_byte_near(str))
		return;
	do{
		serialWrB(pgm_read_byte_near(str));
	}while(pgm_read_byte_near(++str));
}
static void captureImg(uint16_t wg,uint16_t hg){
	uint16_t lg2;
#ifdef useQvga
	uint8_t buf[640];
#elif defined(useQqvga)
	uint8_t buf[320];
#endif
	StringPgm(PSTR("RDY"));
	// wait for vsync
	while(!(PINA&_BV(4))); // wait for high
	while((PINA&_BV(4)));  // wait for low
#ifdef useVga
	while(hg--){
		lg2=wg;
		while(lg2--){
			while((PINA&_BV(6)));//wait for low
			UDR0=PINC;
			while(!(PINA&_BV(6)));//wait for high
		}
	}
#elif defined(useQvga)
	/*We send half of the line while reading then half later */
	while(hg--){
		uint8_t*b=buf,*b2=buf;
		lg2=wg/2;
		while(lg2--){
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			while(!(PINA&_BV(6)));//wait for high
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			UDR0=*b2++;
			while(!(PINA&_BV(6)));//wait for high
		}
		/* Finish sending the remainder during blanking */
		lg2=wg/2;
		while(!( UCSR0A & (1<<UDRE0)));//wait for byte to transmit
		while(lg2--){
			UDR0=*b2++;
			while(!( UCSR0A & (1<<UDRE0)));//wait for byte to transmit
		}
	}
#else
	/* This code is very similar to qvga sending code except we have even more blanking time to take advantage of */
	while(hg--){
		uint8_t*b=buf,*b2=buf;
		lg2=wg/5;
		while(lg2--){
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			while(!(PINA&_BV(6)));//wait for high
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			while(!(PINA&_BV(6)));//wait for high
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			while(!(PINA&_BV(6)));//wait for high
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			while(!(PINA&_BV(6)));//wait for high
			while((PINA&_BV(6)));//wait for low
			*b++=PINC;
			UDR0=*b2++;
			while(!(PINA&_BV(6)));//wait for high
		}
		/* Finish sending the remainder during blanking */
		lg2=320-(wg/5);
		while(!( UCSR0A & (1<<UDRE0)));//wait for byte to transmit
		while(lg2--){
			UDR0=*b2++;
			while(!( UCSR0A & (1<<UDRE0)));//wait for byte to transmit
		}
	}
#endif
}
int main(void){
	cli();//disable interrupts
	// Setup the 8mhz PWM clock
	// OV7670 XCLK => (through level shift!) => pin 46 = PL3(OC5A), output
	DDRL |= _BV(3);
	TCCR5A = _BV(COM5A0) | _BV(WGM51) | _BV(WGM50);
	TCCR5B = _BV(WGM53) | _BV(WGM52) | _BV(CS50);
	OCR5A = 0; // (F_CPU)/(2*(X+1))
	// OV7670 VSYNC => pin 26 = PA4, input
	DDRA &= ~ _BV(4);
	// OV7670 PCLK => pin 28 = PA6, input
	DDRA &= ~ _BV(6);
	// OV7670 D7-D0 => pin 30-37 = PC7-PC0, input
	DDRC = 0;
	_delay_ms(3000);
	// set up twi for 100khz
	TWSR&=~3; // disable prescaler for TWI
	TWBR=72;  // set to 100khz
	// enable serial
	UBRR0H=0;
	UBRR0L=3;  // 0 = 2M baud rate. 1 = 1M baud. 3 = 0.5M. 7 = 250k 207 is 9600 baud rate.
	UCSR0A|=2; // double speed aysnc
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); // enable receiver and transmitter
	UCSR0C=6; // async USART, 1 stop bit, 8-bit character size, no parity bits
	camInit();
#ifdef useVga
	setResolution(VGA);
	setColorSpace(BAYER_RGB);
	wrReg(REG_CLKRC,63);
#elif defined(useQvga)
	setResolution(QVGA);
	setColorSpace(YUV422);
	wrReg(REG_CLKRC,20);
#else
	setResolution(QQVGA);
	setColorSpace(YUV422);
	wrReg(REG_CLKRC,4);
#endif
	/* If you are not sure what value to use here for the divider (register 0x11)
	 * Values I have found to work raw vga 25 qqvga yuv422 12 qvga yuv422 21
	 * run the commented out test below and pick the smallest value that gets a correct image */
	while (1){
		/* captureImg operates in bytes not pixels in some cases pixels are two bytes per pixel
		 * So for the width (if you were reading 640x480) you would put 1280 if you are reading yuv422 or rgb565 */
		/*uint8_t x=63;//Uncomment this block to test divider settings note the other line you need to uncomment
		  do{
		  wrReg(REG_CLKRC,x);
		  _delay_ms(1000);*/
#ifdef useVga
		captureImg(640,480);
#elif defined(useQvga)
		captureImg(320*2,240);
#else
		captureImg(160*2,120);
#endif
		//}while(--x);//Uncomment this line to test divider settings
	}
}
