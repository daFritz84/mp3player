/*
 * mp3 player controls (start, ...)
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "task_playercontrol.h"
#include "event_userio.h"
#include "event_decoder.h"

/* module variables */
static uint8_t __nSineTestToggle;
static uint8_t __nPlayToggle;

/*
 * wait for event
 */
void playercontrol_work( void ) {
	event_userio_t tempEvent;
	
	if( event_userio_count() > 0 ) {
		event_userio_get( &tempEvent );
		
		switch( tempEvent.nKey ) {
			case KEY_PLAY:
				if( __nPlayToggle == PLAYERCONTROL_PAUSE ) {
					/* pass event to mp3decoder task */
					event_decoder_t playEvent;
					playEvent.nEventID = EVENT_RESUME;
					playEvent.nStepCnt = 0;
										
					event_decoder_put( &playEvent );

					__nSineTestToggle = PLAYERCONTROL_SINETESTOFF;
				}
				else {
					/* pass event to mp3decoder task */
					event_decoder_t pauseEvent;
					pauseEvent.nEventID = EVENT_PAUSE;
					pauseEvent.nStepCnt = 0;					
					
					event_decoder_put( &pauseEvent );
				}
				
				__nPlayToggle = (__nPlayToggle + 1) & PLAYERCONTROL_PLAY;
				break;
			
			case KEY_RESUMEPROM:
				{
					/* pass event to mp3decoder task */
					event_decoder_t playEvent;
					playEvent.nEventID = EVENT_PLAY;
					playEvent.nStepCnt = 0;
										
					event_decoder_put( &playEvent );
					__nPlayToggle = PLAYERCONTROL_PLAY;
				}
				break;
			
			case KEY_FORWARD:
				{
					/* pass event to mp3decoder task */				
					event_decoder_t forwardEvent;
					forwardEvent.nEventID = EVENT_FORWARD;
					forwardEvent.nStepCnt = 0;

					event_decoder_put( &forwardEvent );				
					__nPlayToggle = PLAYERCONTROL_PLAY;
				}
				break;
			case KEY_REVERSE:
				{
					/* pass event to mp3decoder task */
					event_decoder_t reverseEvent;
					reverseEvent.nEventID = EVENT_BACKWARD;
					reverseEvent.nStepCnt = 0;

					event_decoder_put( &reverseEvent );								
					__nPlayToggle = PLAYERCONTROL_PLAY;
				}
				break;
			case KEY_VOLUME:
				{	
					/* pass event to mp3decoder task */	
					event_decoder_t volumeEvent;
					volumeEvent.nEventID = EVENT_VOLUME;
					volumeEvent.nStepCnt = tempEvent.nValue;

					event_decoder_put( &volumeEvent );
				}
				break;
			case KEY_SINETEST:
				if( __nSineTestToggle == PLAYERCONTROL_SINETESTOFF ) {
					/* pass event to mp3decoder task */	
					event_decoder_t sineTestOnEvent;
					sineTestOnEvent.nEventID = EVENT_SINETESTON;
					sineTestOnEvent.nStepCnt = 0;

					event_decoder_put( &sineTestOnEvent );

					__nPlayToggle = PLAYERCONTROL_PAUSE;					
				}
				else {
					/* pass event to mp3decoder task */			
					event_decoder_t sineTestOffEvent;
					sineTestOffEvent.nEventID = EVENT_SINETESTOFF;
					sineTestOffEvent.nStepCnt = 0;

					event_decoder_put( &sineTestOffEvent );	
				}
				
				__nSineTestToggle = (__nSineTestToggle + 1) & PLAYERCONTROL_SINETESTON;
				break;
			default:
				printf_P( PSTR("DEBUG: UNKNOWN_EVENT\n") );
				break;
		}	
	}	
	return;
}

/*
 * get player state
 */
uint8_t playercontrol_getstate( void ) {
	return __nPlayToggle;
}

