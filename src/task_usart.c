/*
 * usart input control
 * author: 	Stefan Seifried
 * date:	27.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include "task_usart.h"
#include "usart.h"
#include "event_userio.h"
#include "common.h"


/*
 * scan usart for character's
 */
void usart_work( void ) {
	char cTemp;	
	event_userio_t tempEvent;	
	
	if( usart_getc( &cTemp ) == SUCCESS ) {
		switch( cTemp ) {
			case 'P':
			case 'p':
				tempEvent.nKey = KEY_PLAY;
				event_userio_put( &tempEvent );
				break;
			
			case 'F':
			case 'f':				
				tempEvent.nKey = KEY_FORWARD;
				event_userio_put( &tempEvent );
				break;
			
			case 'R':
			case 'r':				
				tempEvent.nKey = KEY_REVERSE;
				event_userio_put( &tempEvent );
				break;
			
			case 'S':
			case 's':
				tempEvent.nKey = KEY_SINETEST;
				event_userio_put( &tempEvent );		
		}	
	}
 
	return;
}
