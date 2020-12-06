/*
 * mp3 player controls (start, ...)
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

#ifndef _TASK_PLAYERCONTROL_H_
#define _TASK_PLAYERCONTROL_H_

/* constants */
#define PLAYERCONTROL_SINETESTOFF	0
#define PLAYERCONTROL_SINETESTON	1

#define PLAYERCONTROL_PAUSE			0
#define PLAYERCONTROL_PLAY			1


/* functions */
void playercontrol_work( void );
uint8_t playercontrol_getstate( void );

#endif /* _TASK_PLAYCONTROL_H_ */

