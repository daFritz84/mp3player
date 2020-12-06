/*
 * handles keypad input and debouncing
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <stdio.h>

#include "task_keypad.h"
#include "event_userio.h"


/* module variables */
static uint8_t __nDebouncedState;
static uint8_t __nClock0;
static uint8_t __nClock1;


/*
 * initialize keypad port
 */
void keypad_init( void ) {
	DDR_KEYPAD &= ~( PIN_KEYPADX );
	PORT_KEYPAD |= ( PIN_KEYPADX ); 	
	return;
}


/*
 * check & debounce key
 * any changes are written into the event user io event queue
 */
void keypad_debounce( void ) {
	uint8_t nDelta;
	uint8_t nSample;
	uint8_t	nChanges;
	event_userio_t tempEvent;	
	
	nSample = (PIN_KEYPAD & PIN_KEYPADX );		/* get new sample */	
	nDelta = nSample ^ __nDebouncedState;		/* find all changes */	
	
	__nClock0 ^= __nClock1;						/* increment counters */
	__nClock1 = ~__nClock1;

	__nClock0 &= nDelta;						/* reset counters if no changes */
	__nClock1 &= nDelta;						/* were detected */
	
	nChanges = ~( ~nDelta | __nClock0 | __nClock1 );
	__nDebouncedState ^= nChanges;
	
	/* put changes into event queue */
	if( (nChanges & _BV(PIN_KEYPAD0)) && (~__nDebouncedState & _BV(PIN_KEYPAD0)) ) {
		tempEvent.nKey = KEY_PLAY;		
		event_userio_put( &tempEvent );
	}		
	
	if( (nChanges & _BV(PIN_KEYPAD1)) && (~__nDebouncedState & _BV(PIN_KEYPAD1)) ) {
		tempEvent.nKey = KEY_FORWARD;		
		event_userio_put( &tempEvent );	
	}

	if( (nChanges & _BV(PIN_KEYPAD2)) && (~__nDebouncedState & _BV(PIN_KEYPAD2)) ) {
		tempEvent.nKey = KEY_REVERSE;		
		event_userio_put( &tempEvent );
	}

	return;
}
