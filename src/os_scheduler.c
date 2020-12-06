/*
 * operating system scheduler
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <avr/sleep.h>
#include <string.h>
#ifdef __AVR_VERSION_H_EXISTS__ 	/* needed due to compability reasons */
	#include <avr/interrupt.h>
#else
	#include <avr/interrupt.h>
	#include <avr/signal.h>
#endif

#include "os_scheduler.h"
#include "os_task.h"
#include "common.h"


/* struct's */
struct os_scheduler_taskbuf {
	uint8_t nHead;
	os_task_t aStore[OS_TASK_QUEUESIZE];
};
typedef struct os_scheduler_taskbuf os_scheduler_taskbuf_t;


/* module var's */
static os_scheduler_taskbuf_t __aTaskList; 


/*
 * initializer timer 0 for use as scheduler
 * it will trigger exactly every 10ms
 */
void os_scheduler_init( void ) {
	OCR0 = OS_TIMER_RELOAD_VAL;

	TIMSK |= ( _BV(OCIE0) );	

	TCCR0 &= ~( _BV(WGM00) | _BV(CS01) );	
	TCCR0 |= ( _BV(WGM01) | _BV(CS02) | _BV(CS00) );	

	return;
}

/*
 * add task to scheduler list
 */
uint8_t os_scheduler_addtask( os_task_t* _pTask ) {
	if( __aTaskList.nHead < OS_TASK_QUEUESIZE ) {
		memcpy( &__aTaskList.aStore[__aTaskList.nHead], _pTask, sizeof(os_task_t) );		
		++__aTaskList.nHead;		
		return SUCCESS;	
	}
	else {
		return FAIL;	
	}
}

/*
 * scheduler loop
 * manages executes all the tasks
 */
void os_scheduler_loop( void ) {
	uint8_t	i;	
		
	for(;;){
		/* loop all tasks */
		for(i=0; i < OS_TASK_QUEUESIZE; ++i ){
			if( __aTaskList.aStore[i].nTaskExecutionCount == 0 ) {
				if( __aTaskList.aStore[i].fpTaskHandler != NULL ) {
					__aTaskList.aStore[i].fpTaskHandler();
				}
				/* reload execution timer */
				__aTaskList.aStore[i].nTaskExecutionCount = __aTaskList.aStore[i].nTaskExecutionReload;			
			}		
		}

		sleep_mode();	
	}

	return;
}

/***************************************************
 * ISR's
 ***************************************************/
/*
 * signal task's
 */
SIGNAL( SIG_OUTPUT_COMPARE0 ) {
	uint8_t i;
	
	for(i=0; i < OS_TASK_QUEUESIZE; ++i ){
		if( __aTaskList.aStore[i].nTaskExecutionCount > 0 ) {
			--__aTaskList.aStore[i].nTaskExecutionCount;	
		}
	}	
}
