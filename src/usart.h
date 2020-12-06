/*
 * atmega16 usart driver
 * author: 	Stefan Seifried
 * date:	30.03.2011
 * matr.nr.:0925401
 *
 * basic initialization and low level functions of the internal
 * usart
 *
 * this driver only supports "Normal Asynchronous Mode" but should
 * be written in a manner that allows easy implementation of other
 * operating modes as well.
 *
 */

#ifndef _USART_H_
#define _USART_H_

/* includes */
#include <avr/io.h>


/* cpu frequency used by baudrate calculation */
#ifndef F_CPU
	#define		F_CPU	16000000UL
#endif

/* baudrate calculation */
#define		UBRRCALC( __BAUD__ ) \
	((F_CPU)/(16*__BAUD__) - 1)

/* beware! calculation cannot be done without some error
 * for std. baudrates. if the error exceeds +/-1% it is
 * not acceptable any more so we only allow some of them.
 *
 * see precalculated values from http://www.gjlay.de/helferlein/avr-uart-rechner.html
 * Baud Rate(bps)	UBRR			Error (%)	
 * 300				3332=0xd04		0	
 * 1200				832=0x340		0	
 * 2400				416=0x1a0		-0.1	
 * 4800				207=0xcf		0.2	
 * 9600				103=0x67		0.2	
 * 14.4k			68=0x44			0.6	
 * 19.2k			51=0x33			0.2	
 * 28.8k			34=0x22			-0.8	
 * 38.4k			25=0x19			0.2	
 * 57.6k			16=0x10			2.1	
 * 76.8k			12=0xc			0.2	
 * 115.2k			8				-3.5	
 */
#define BAUD300			UBRRCALC( 300UL )
#define BAUD1200		UBRRCALC( 1200UL )
#define BAUD4800		UBRRCALC( 4800UL )
#define	BAUD9600		UBRRCALC( 9600UL )
#define BAUD19200		UBRRCALC( 19200UL )
#define	BAUD38400		UBRRCALC( 38400UL )


/* buffer sizes, must be a power of 2! */
#define USART_QUEUESIZE			8	


/* struct's */
struct usart_ring_buf {
	volatile uint8_t nHead;
	volatile uint8_t nTail;
	uint8_t aQueue[ USART_QUEUESIZE ];
};

typedef struct usart_ring_buf usart_ring_buf_t;


/* initialization */
void usart_init( uint16_t _BaudVal );

/* receive & transmit functions */
uint8_t usart_getc( char* _pByte );
int usart_putc( char _cByte );

#endif /* _USART_H_ */

