/*
 * event queue for user io
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <string.h>

#include "event_userio.h"
#include "common.h"


/* struct's */
struct event_userio_buf {
	uint8_t nHead;
	uint8_t nTail;
	uint8_t nCount;
	event_userio_t aStore[EVENT_USERIO_BUFFERSIZE];
};
typedef struct event_userio_buf event_userio_buf_t;


/* module var's */
static event_userio_buf_t __aEventUserIOBuffer; 


/*
 * get and remove event from event queue
 */
uint8_t event_userio_get( event_userio_t* _pEvent ) {
	if( __aEventUserIOBuffer.nCount != 0 ) {
		--__aEventUserIOBuffer.nCount;
		memcpy( _pEvent, &__aEventUserIOBuffer.aStore[__aEventUserIOBuffer.nTail], sizeof(event_userio_t) );		
		__aEventUserIOBuffer.nTail = ( __aEventUserIOBuffer.nTail + 1 ) & ( EVENT_USERIO_BUFFERSIZE - 1 );
		return SUCCESS;	
	}
	else {
		return FAIL;
	}
}

/*
 * put event into event queue
 */
uint8_t event_userio_put( event_userio_t* _pEvent ) {
	if( __aEventUserIOBuffer.nCount < EVENT_USERIO_BUFFERSIZE ) {
		++__aEventUserIOBuffer.nCount;
		memcpy( &__aEventUserIOBuffer.aStore[__aEventUserIOBuffer.nHead], _pEvent, sizeof(event_userio_t) );
		__aEventUserIOBuffer.nHead = ( __aEventUserIOBuffer.nHead + 1 ) & ( EVENT_USERIO_BUFFERSIZE - 1 );		
		return SUCCESS;	
	}
	else {
		return FAIL;	
	}
}

/*
 * get number of events in buffer
 */
inline uint8_t event_userio_count( void ) {
	return __aEventUserIOBuffer.nCount;
}

