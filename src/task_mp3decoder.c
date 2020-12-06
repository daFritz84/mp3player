/*
 * mp3 decoder task
 * author: 	Stefan Seifried
 * date:	02.06.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>

#include "task_mp3decoder.h"
#include "event_decoder.h"
#include "mp3decoder.h"


void mp3decoder_work( void ) {
	event_decoder_t tempEvent;
	
	if( event_decoder_count() > 0 ) {
		event_decoder_get( &tempEvent );
	
		switch( tempEvent.nEventID ) {
			case EVENT_SINETESTON:
				mp3decoder_sinetest_on( tempEvent.nStepCnt );
				break;

			case EVENT_SINETESTOFF:
				mp3decoder_sinetest_off( tempEvent.nStepCnt );
				break;
			
			case EVENT_PLAY:
				mp3decoder_play( tempEvent.nStepCnt );
				break;			
			
			case EVENT_RESUME:
				mp3decoder_resume();
				break;			

			case EVENT_PAUSE:
				mp3decoder_pause();
				break;			
			
			case EVENT_FORWARD:
				mp3decoder_playnext();	
				break;
			
			case EVENT_BACKWARD:
				mp3decoder_playprev();
				break;

			case EVENT_VOLUME:
				mp3decoder_setvolume( tempEvent.nStepCnt );			

			default:
				break;		
		}

	}

	return;
}
