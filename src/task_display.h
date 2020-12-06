/*
 * lcd display task
 * author: 	Stefan Seifried
 * date:	07.06.2011
 * matr.nr.:0925401
 */

#ifndef _TASK_DISPLAY_H_
#define _TASK_DISPLAY_H_

/* constants */
#define DISPLAY_UNKNOWN 0
#define DISPLAY_SDERROR	1
#define DISPLAY_INFO 	2


/* functions */
void display_work( void );
void display_reset( void );

#endif /* _TASK_DISPLAY_H_ */

