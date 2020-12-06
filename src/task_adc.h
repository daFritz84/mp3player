/*
 * adc converter for volume control
 * author: 	Stefan Seifried
 * date:	28.05.2011
 * matr.nr.:0925401
 */

#ifndef _TASK_ADC_H_
#define _TASK_ADC_H_

/* includes */
#include <avr/io.h>


/* constants */
#define ADC_DDR			DDRA
#define ADC_PORT		PORTA
#define ADC_CHANNEL 	ADC0
#define ADC_TOLERANCE	(1)	


/* functions */
void adc_init( void );
void adc_convert( void );


#endif /*_TASK_ADC_H_*/ 

