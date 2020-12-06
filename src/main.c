/*
 * main application
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "main.h"
#include "usart.h"
#include "usart_stdout.h"
#include "stringtable.h"
#include "os_scheduler.h"
#include "os_task.h"
#include "asm_spi.h"
#include "mp3decoder.h"
#include "lcd.h"

#include "task_keypad.h"
#include "task_usart.h"
#include "task_playercontrol.h"
#include "task_adc.h"
#include "task_mp3decoder.h"
#include "task_display.h"
#include "task_mmccard.h"


/* cgram patterns */
static uint8_t __aPattern1[] PROGMEM = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
static uint8_t __aPattern2[] PROGMEM = { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 };
static uint8_t __aPattern3[] PROGMEM = { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C };
static uint8_t __aPattern4[] PROGMEM = { 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E };
static uint8_t __aPattern5[] PROGMEM = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F };

/* module variables */
static uint8_t __nMCUCSR;

/*
 * main application entry point
 */
int main( void ) {
	os_task_t tempTask;
	
	/* save reset register */
	__nMCUCSR = MCUCSR;

	/* initialize usart */
	usart_init( BAUD38400 );
	usart_stdout_redirect();

	/* enable interrupts */	
	sei();

	/* add keypad task */
	keypad_init();	
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 0;
	tempTask.fpTaskHandler = keypad_debounce;
	os_scheduler_addtask( &tempTask );
	
	/* add usart task */
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 0;
	tempTask.fpTaskHandler = usart_work;
	os_scheduler_addtask( &tempTask );	
	
	/* add adc task */
	adc_init();
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 50;
	tempTask.fpTaskHandler = adc_convert;
	os_scheduler_addtask( &tempTask );	
	
	/* add player control task */
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 0;
	tempTask.fpTaskHandler = playercontrol_work;
	os_scheduler_addtask( &tempTask );
	
	/* add mp3 decoder task */
	asm_spi_init();	
	mp3decoder_init( asm_spi_send_pol0phas1, asm_spi_receive_pol0phas1 );
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 0;
	tempTask.fpTaskHandler = mp3decoder_work;
	os_scheduler_addtask( &tempTask );
	
	/* add lcd display task */
	lcd_init();
	lcd_cgram_P( 0, __aPattern1 );
	lcd_cgram_P( 0, __aPattern1 );
	lcd_cgram_P( 1, __aPattern2	);
	lcd_cgram_P( 2, __aPattern3 );
	lcd_cgram_P( 3, __aPattern4 );
	lcd_cgram_P( 4, __aPattern5 );
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 50;
	tempTask.fpTaskHandler = display_work;
	os_scheduler_addtask( &tempTask );	
	
	/* add mmc card task */
	mmccard_init( asm_spi_send_pol0phas1, asm_spi_receive_pol0phas1 );
	tempTask.nTaskExecutionCount = 0;
	tempTask.nTaskExecutionReload = 1;
	tempTask.fpTaskHandler = mmccard_worker;
	os_scheduler_addtask( &tempTask );

	os_scheduler_init();
	os_scheduler_loop();

	return 0;
}

/*
 * get saved reset register
 */
uint8_t main_getresetreg( void ) {
	return __nMCUCSR;
}

/*
 * clear reset register
 */
void main_clearresetreg( void ) {
	__nMCUCSR = 0x0;
	return;
}
