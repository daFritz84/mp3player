/*
 * event queue for display control
 * author: 	Stefan Seifried
 * date:	02.06.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <string.h>

#include "event_display.h"
#include "common.h"


/* struct's */
struct event_display_buf {
	uint8_t nHead;
	uint8_t nTail;
	uint8_t nCount;
	event_display_t aStore[EVENT_DISPLAY_BUFFERSIZE];
};
typedef struct event_display_buf event_display_buf_t;


/* module var's */
static event_display_buf_t __aEventDisplayBuffer; 


/*
 * get and remove event from event queue
 */
uint8_t event_display_get( event_display_t* _pEvent ) {
	if( __aEventDisplayBuffer.nCount != 0 ) {
		--__aEventDisplayBuffer.nCount;
		memcpy( _pEvent, &__aEventDisplayBuffer.aStore[__aEventDisplayBuffer.nTail], sizeof(event_display_t) );		
		__aEventDisplayBuffer.nTail = ( __aEventDisplayBuffer.nTail + 1 ) & ( EVENT_DISPLAY_BUFFERSIZE - 1 );
		return SUCCESS;	
	}
	else {
		return FAIL;
	}
}

/*
 * put event into event queue
 */
uint8_t event_display_put( event_display_t* _pEvent ) {
	if( __aEventDisplayBuffer.nCount < EVENT_DISPLAY_BUFFERSIZE ) {
		++__aEventDisplayBuffer.nCount;
		memcpy( &__aEventDisplayBuffer.aStore[__aEventDisplayBuffer.nHead], _pEvent, sizeof(event_display_t) );
		__aEventDisplayBuffer.nHead = ( __aEventDisplayBuffer.nHead + 1 ) & ( EVENT_DISPLAY_BUFFERSIZE - 1 );		
		return SUCCESS;	
	}
	else {
		return FAIL;	
	}
}

/*
 * get number of events in buffer
 */
inline uint8_t event_display_count( void ) {
	return __aEventDisplayBuffer.nCount;
}

