/*
 * event queue for decoder control
 * author: 	Stefan Seifried
 * date:	02.06.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <string.h>

#include "event_decoder.h"
#include "common.h"


/* struct's */
struct event_decoder_buf {
	uint8_t nCount;				/* serves as 'are you there' flag */
	event_decoder_t aStore;
};
typedef struct event_decoder_buf event_decoder_buf_t;


/* module var's */
static event_decoder_buf_t __aEventDecoderBuffer; 


/*
 * get and remove event from event queue
 */
uint8_t event_decoder_get( event_decoder_t* _pEvent ) {
	if( __aEventDecoderBuffer.nCount != 0 ) {
		__aEventDecoderBuffer.nCount = 0;
		memcpy( _pEvent, &__aEventDecoderBuffer.aStore, sizeof(event_decoder_t) );		
		return SUCCESS;	
	}
	else {
		return FAIL;
	}
}

/*
 * put event into event queue
 * we always succeed here, hence we allow interrupting of pending commands with user io!
 */
uint8_t event_decoder_put( event_decoder_t* _pEvent ) {
	__aEventDecoderBuffer.nCount = 1;
	memcpy( &__aEventDecoderBuffer.aStore, _pEvent, sizeof(event_decoder_t) );
	return SUCCESS;	
}

/*
 * get number of events in buffer
 */
inline uint8_t event_decoder_count( void ) {
	return __aEventDecoderBuffer.nCount;
}

