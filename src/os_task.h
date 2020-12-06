/*
 * operating basic task implementation
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

#ifndef _OS_TASK_H_
#define _OS_TASK_H_


typedef void (*os_task_callback_t)(void);

struct os_task {
	volatile uint8_t	nTaskExecutionCount;	/* execution counter */
	uint8_t				nTaskExecutionReload;	/* execution reload value */
	os_task_callback_t 	fpTaskHandler;			/* task callback */
};

typedef struct os_task os_task_t;

#endif /*_OS_TASK_H_*/
