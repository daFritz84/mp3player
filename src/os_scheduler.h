/*
 * operating system scheduler
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

#ifndef _OS_SCHEDULER_H_
#define _OS_SCHEDULER_H_

/* constants */
#define OS_TIMER_RELOAD_VAL		156		/* provides 10ms resolution */

#define OS_TASK_QUEUESIZE		7		/* max number of tasks */


/* headers */
#include "os_task.h"


/* functions */
void os_scheduler_init( void );
uint8_t os_scheduler_addtask( os_task_t* _pTask );
void os_scheduler_loop( void );

#endif /* _OS_SCHEDULER_H_ */

