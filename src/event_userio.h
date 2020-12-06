/*
 * event queue for user io
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

#ifndef _EVENT_USERIO_H_
#define _EVENT_USERIO_H_


/* constants */
#define EVENT_USERIO_BUFFERSIZE	4


/* structs */
#define KEY_PLAY		0
#define	KEY_FORWARD 	1
#define KEY_REVERSE 	2
#define KEY_VOLUME		3
#define KEY_SINETEST	4
#define KEY_RESUMEPROM	5 


struct event_userio {
	uint8_t nKey;
	uint8_t nValue;
};
typedef struct event_userio event_userio_t;


/* functions */
uint8_t event_userio_get( event_userio_t* _pEvent );
uint8_t event_userio_put( event_userio_t* _pEvent );
inline uint8_t event_userio_count( void );

#endif /* _EVENT_USERIO_H_ */

