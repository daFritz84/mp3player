/*
 * event queue for decoder control
 * author: 	Stefan Seifried
 * date:	02.06.2011
 * matr.nr.:0925401
 */

#ifndef _EVENT_DECODER_H_
#define _EVENT_DECODER_H_


/* event types */
#define EVENT_SINETESTON		0
#define EVENT_SINETESTOFF		1
#define EVENT_PLAY				2
#define EVENT_PAUSE				3
#define EVENT_FORWARD			4
#define EVENT_BACKWARD			5
#define EVENT_VOLUME			6
#define EVENT_RESUME			7


struct event_decoder {
	uint8_t nEventID;		/* event id */
	uint8_t	nStepCnt;		/* optional, step count for devided event's */
};
typedef struct event_decoder event_decoder_t;


/* functions */
uint8_t event_decoder_get( event_decoder_t* _pEvent );
uint8_t event_decoder_put( event_decoder_t* _pEvent );
inline uint8_t event_decoder_count( void );

#endif /* _EVENT_DECODER_H_ */

