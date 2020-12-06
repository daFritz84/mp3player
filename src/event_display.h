/*
 * event queue for display control
 * author: 	Stefan Seifried
 * date:	02.06.2011
 * matr.nr.:0925401
 */

#ifndef _EVENT_DISPLAY_H_
#define _EVENT_DISPLAY_H_


/* constants */
#define EVENT_DISPLAY_BUFFERSIZE	4


struct event_display {
	uint8_t nCmdID;
};
typedef struct event_display event_display_t;


/* functions */
uint8_t event_display_get( event_display_t* _pEvent );
uint8_t event_display_put( event_display_t* _pEvent );
inline uint8_t event_display_count( void );

#endif /* _EVENT_DISPLAY_H_ */
