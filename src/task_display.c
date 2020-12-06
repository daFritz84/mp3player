/*
 * lcd display task
 * author: 	Stefan Seifried
 * date:	07.06.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>

#include "task_display.h"
#include "task_mmccard.h"
#include "task_playercontrol.h"
#include "mp3decoder.h"
#include "lcd.h"
#include "stringtable.h"
#include "event_display.h"


/* module variables */
static uint8_t __nLastDisplay;
static uint8_t __nScrollOffset;

static char VTANSI_ClrScreen[] PROGMEM = { 0x1B, '[', '2', 'J', '\0' };
static char VTANSI_Home[] PROGMEM = { 0x1B, '[', '0', ';', '0', 'H', '\0' };
static char VTANSI_HomeUpdate[] PROGMEM = { 0x1B, '[', '2', ';', '0', 'H', '\0' };

/* functions */
/*
 * display worker
 */
void display_work( void ) {
	uint8_t i;
	uint8_t nProgress;
	uint16_t nDecodeTime;
	char sTimeBuffer[8];

	/* error screen */
	if( mmccard_getstate() < STATE_READY && __nLastDisplay != DISPLAY_SDERROR ) {
			/* LC DISPLAY */
			__nLastDisplay = DISPLAY_SDERROR;
			lcd_clear();
			lcd_gotopos( 0, 0 );
			lcd_puts_P( STRING_SDCARDERROR1 );
			lcd_gotopos( 0, 1 );
			lcd_puts_P( STRING_SDCARDERROR2 );

			/* UART */
			printf_P( VTANSI_ClrScreen );
			printf_P( VTANSI_Home );
			printf_P( STRING_SDCARDERROR_UART );	
	}
	/* initial info screen */
	else if( mmccard_getstate() == STATE_READY && __nLastDisplay != DISPLAY_INFO ) {
			/* LC DISPLAY */
			__nLastDisplay = DISPLAY_INFO;
			__nScrollOffset = 0;
			
			lcd_clear();
			lcd_gotopos( 0 , 0 );
			lcd_puts( mmccard_get_info() );

			/* UART */
			printf_P( VTANSI_ClrScreen );
			printf_P( VTANSI_Home );
			printf( mmccard_get_info() );
	}
	/* update info screen */
	else if( mmccard_getstate() == STATE_READY && __nLastDisplay == DISPLAY_INFO ) {
			/* LC DISPLAY */
			/* scrolling title info */			
			if( strlen( mmccard_get_info() ) > 16 ) {
				if( __nScrollOffset >= (strlen( mmccard_get_info() ) - 16) ) {
					__nScrollOffset = 0;
				}
				else {
					++__nScrollOffset;
				}
				
				lcd_gotopos( 0, 0 );
				lcd_puts( &mmccard_get_info()[__nScrollOffset] );
			}
			/* non scrolling title info */
			else {
				/* nothing to update */
			}
			
			/* display progress bar */
			lcd_gotopos( 0, 1 );

			/*print out full segments*/
			nProgress = mmccard_get_progress();
			if( playercontrol_getstate() == PLAYERCONTROL_PLAY ) {
				for( i=0; i<nProgress/5; i++) {
					lcd_putc( 0x04 );
				}
				lcd_putc( nProgress%5 );			
				for(i++; i<9; i++) {
					lcd_putc(' ');
				}
			}
			else {
				lcd_puts_P( PSTR("  PAUSE  ") );
			}			

			/* display decoded time */			
			lcd_gotopos( 9, 1 );	
			nDecodeTime = mp3decoder_getdecodetime();
			sprintf_P(sTimeBuffer, PSTR("(%02d:%02d)"), nDecodeTime/60, nDecodeTime%60 );
			lcd_puts(sTimeBuffer);

			/* UART */
			printf_P( VTANSI_HomeUpdate );
			if( playercontrol_getstate() == PLAYERCONTROL_PLAY ) {
				for( i=0; i<nProgress/5; i++) {
					printf_P( PSTR("*") );
				}
				printf_P( PSTR("+") );			
				for(i++; i<9; i++) {
					printf_P( PSTR("-") );
				}
			}
			else {
				printf_P( PSTR("  PAUSE  ") );
			}

			printf_P( PSTR("(%02d:%02d)"), nDecodeTime/60, nDecodeTime%60 );	
	}		
	else {
		/* nothing to do */	
	}	

	return;
}

/*
 * display reset
 */
void display_reset() {
	__nLastDisplay = DISPLAY_UNKNOWN;
	return;
}
