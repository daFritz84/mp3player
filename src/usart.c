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

/* includes */
#ifdef __AVR_VERSION_H_EXISTS__
	#include <avr/interrupt.h>
#else
	#include <avr/interrupt.h>
	#include <avr/signal.h>
#endif
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>

#include "usart.h"
#include "common.h"


/* global variables */
static usart_ring_buf_t __recv_buffer;
static usart_ring_buf_t __trans_buffer;

/*
 * get character from usart
 */
uint8_t usart_getc( char* _pByte ) {	
	/* wait for characters to arrive, we should be awakend by an
	 * arriving recv interrupt
	 */
	while( __recv_buffer.nHead == __recv_buffer.nTail ) {
		return FAIL;
    	}
	
	(*_pByte) = __recv_buffer.aQueue[ __recv_buffer.nTail ];
	__recv_buffer.nTail = ( __recv_buffer.nTail + 1 ) & ( USART_QUEUESIZE -1 );
	return SUCCESS;
}

/*
 * put character to usart
 * if character could be written then '0' is returned
 * else '1'
 */
int usart_putc( char _cByte ) {
	uint8_t nTempHead;
	nTempHead = ( __trans_buffer.nHead + 1 ) & ( USART_QUEUESIZE - 1 );
	
	/* wait for transmit queue to get some free space. Sleep is not applicable. When we get
	 * stuck in the busy wait loop. There are hopefully some transmit isr's pushing
	 * out data, so we wouldn't stay asleep due to the recurring interrupts.
     *
	 * for god's sake I just put the sleepmode here to to allow to sleep at very low data rates
     */
	while( __trans_buffer.nTail == nTempHead ) {
		sleep_mode();	
	}

	__trans_buffer.aQueue[ __trans_buffer.nHead ] = _cByte;
	__trans_buffer.nHead = nTempHead;

	// trigger transmit interrupt
	UCSRB |= (1<<UDRIE);	
	return 0;	
}

/*
 * initialize usart for interrupt based communication
 *
 * _BaudVal:	desired baudrate !be aware that not every baud rate
 *				is acceptable!
 */
void usart_init( uint16_t _BaudVal )  {
	
	// initialize UBBR
	UBRRH = (uint8_t) (_BaudVal>>8);
	UBRRL = (uint8_t) _BaudVal;
	
	// enable receive, transmit & according interrupts
	// and set frame format to 8N1 per default
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); 

	return;
}

/*********************************************************
 * ISR's
 *********************************************************/

/*
 * receive interrupt handler
 */
SIGNAL( SIG_USART_RECV ) {
	/*
	 * HACK: increment buffer index, avoid modulo with clever trick
	 * in case the queue size is a power of 2 it will look something like this 0b01000000, this minus 1
	 * = 0b00111111. Which makes a perfect mask. Everytime it is overtaken the header resets to 0x0
	 * thus restarting from the beginning!
	 */
	__recv_buffer.aQueue[ __recv_buffer.nHead ] = UDR;
	__recv_buffer.nHead = ( __recv_buffer.nHead + 1 ) & ( USART_QUEUESIZE - 1 );
}

/*
 * transmit interrupt handler
 */
SIGNAL( SIG_USART_DATA ) {
	if( __trans_buffer.nHead != __trans_buffer.nTail ) {
		/*
	 	 * HACK: same hack as in receive interrupt handler
	 	 */		
		UDR = __trans_buffer.aQueue[ __trans_buffer.nTail ];		
		__trans_buffer.nTail = ( __trans_buffer.nTail + 1 ) & ( USART_QUEUESIZE - 1 );
	}
	else {
		// clear interrupt and signal that we're finished sending
		UCSRB &= ~(1<<UDRIE);
	}
}
