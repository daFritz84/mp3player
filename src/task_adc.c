/*
 * adc converter for volume control
 * author: 	Stefan Seifried
 * date:	28.05.2011
 * matr.nr.:0925401
 */

/* includes */
#ifdef __AVR_VERSION_H_EXISTS__
	#include <avr/interrupt.h>
#else
	#include <avr/interrupt.h>
	#include <avr/signal.h>
#endif

#include "task_adc.h"
#include "event_userio.h"


/*
 * initialize adc
 */
void adc_init( void ) {
	/* setup ADC */
	ADMUX &= ~( _BV(REFS1) | _BV(MUX0) | _BV(MUX1) | _BV(MUX2) | _BV(MUX3) | _BV(MUX4) );
	ADMUX |= ( _BV(REFS0) | _BV(ADLAR) );	
	ADCSRA |= ( _BV(ADEN) | _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) );
	return;
}	

/* 
 * start adc conversion
 */
void adc_convert( void ) {
	/* start conversion */
	ADCSRA |= ( _BV(ADSC) );	
	return;
}

/*********************************************************
 * ISR's
 *********************************************************/
SIGNAL(SIG_ADC){
	event_userio_t tempEvent;
	uint8_t newADC = ADCH;
	static uint8_t nOldADC;	
	
	/* add result to event queue, and do some basic noise cancalation */
	if( (nOldADC + ADC_TOLERANCE) < newADC || (nOldADC - ADC_TOLERANCE) > newADC ) {
		nOldADC = newADC;
	
		tempEvent.nKey = KEY_VOLUME;
		tempEvent.nValue = newADC;		
		event_userio_put( &tempEvent );
	}
}
