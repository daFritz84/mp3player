/*
 * atmega16 lcd  driver
 * author: 	Stefan Seifried
 * date:	30.03.2011
 * matr.nr.:0925401
 *
 * implementation of a 4bit HD44780 like interface
 * see WH1602B.pdf for further information
 */

/* cpu frequency used by delay functions */
#ifndef F_CPU
	#define		F_CPU	16000000UL	/*Hz*/
#endif

/* includes */
#ifdef __AVR_VERSION_H_EXISTS__
	#include <util/delay.h>
#else
	#include <avr/delay.h>
#endif

#include <avr/pgmspace.h>

#include "lcd.h"
#include "stringtable.h"

/*
 * enable pulse
 */
static void lcd_internal_enablepulse( void ) {
	LCD_PORT |= _BV(LCD_EN);			// set 'enable' to '1'
	
	/* we need to last at least 140ns
	 * since one nop takes ~62,5ns @ 16MHz we need 3 nop's to
	 * achieve 187,5ns delay
	 */
	asm volatile (
		"nop\n"
		"nop\n"
		"nop"
		:
	);	

	LCD_PORT &=	~_BV(LCD_EN);			// set 'enable' to '0'
	return;
}

/*
 * check busy flag until it returns to '0'
 */
static void lcd_internal_waitbusyflag( void ) {
	uint8_t nData;
	
	// prepare port RS, RW write & data bits read
	LCD_PORT &= ~( _BV(LCD_RS) | _BV(LCD_RW) | _BV(LCD_EN) | (0xF<<LCD_DB4));
	LCD_PORT |= _BV(LCD_RW);
	LCD_DDR |= _BV(LCD_RW) | _BV(LCD_RS) | _BV(LCD_EN);
	LCD_DDR &= ~(0xF<<LCD_DB4);
	
	
	do
	{	
		// get 8-bit value
		nData = 0;					// reset data
		
		// read low nibble
		LCD_PORT |= _BV(LCD_EN);			// set 'enable' to '1'
		asm volatile (					// allow data to get stable
			"nop\n"
			"nop"
			:
		);
		nData |= ((LCD_PIN<<LCD_DB4)&0x0F);
		LCD_PORT &= ~(_BV(LCD_EN));			// set 'enable' to '0'
		
		_delay_us(1.0);					// cycle delay	

		// read high nibble
		LCD_PORT |= _BV(LCD_EN);			// set 'enable' to '1'
		asm volatile (					// allow data to get stable
			"nop\n"					
			"nop"
			:
		);
		nData |= ((LCD_PIN<<LCD_DB4)&0x0F)<<4;
		LCD_PORT &= ~(_BV(LCD_EN));			// set 'enable' to '0'
	}
	while( (nData & (1<<(LCD_DB4+3))) != 0); // mask busy flag & check if set*/
	return;
}

/*
 * prepare port for writing a command
 */
static inline void lcd_internal_portwritecmd( void ) {
	LCD_PORT &= ~(_BV(LCD_RS) | _BV(LCD_RW) | _BV(LCD_EN) | (0x0F<<LCD_DB4));
	LCD_DDR |= ((0x0F<<LCD_DB4)| _BV(LCD_RW) | _BV(LCD_RS) | _BV(LCD_EN));
	return;
}

/*
 * prepare port for writing data
 */
static inline void lcd_internal_portwritedata( void ) {
	LCD_PORT &= ~(_BV(LCD_RS) | _BV(LCD_RW) | _BV(LCD_EN) | (0x0F<<LCD_DB4));
	LCD_PORT |= _BV(LCD_RS);
	LCD_DDR |= ((0x0F<<LCD_DB4)| _BV(LCD_RW) | _BV(LCD_RS) | _BV(LCD_EN));
	return;
}

/*
 * soft reset
 */
static inline void lcd_internal_reset( void ) {
	lcd_internal_portwritecmd();
	LCD_PORT &= ~(0x0F<<LCD_DB4);
	LCD_PORT |= (0x03<<LCD_DB4);
	lcd_internal_enablepulse();
	_delay_us( LCD_DELAY_COMMAND );		// wait for 39 us function set delay	
}

/*
 * write byte to lcd
 * make sure that proper port write mode is selected
 * before calling this function
 */
static void lcd_internal_write( uint8_t _nData ) {
	// send high nibble
	LCD_PORT &= ~(0x0F<<LCD_DB4);
	LCD_PORT |= ( ((_nData & 0xF0)>>4) << LCD_DB4 );
	lcd_internal_enablepulse();	

	// send low nibble
	LCD_PORT &= ~(0x0F<<LCD_DB4);
	LCD_PORT |= ((_nData & 0x0F) << LCD_DB4);
	lcd_internal_enablepulse();

	return;
}

/*
 * send 'function set' command
 * you can use the following flags to specify exact behaviour
 * - LCD_PARAM_DATA4BIT or LCD_PARAM_DATA8BIT to select between 4bit & 8bit mode
 * - LCD_PARAM_DISP1LINE or LCD_PARAM_DISP2LINE to select between a one lined display
 * or a 2 lined.
 * - LCD_PARAM_FONT5X11 or LCD_PARAM_FONT5X8 select between wide character size or 
 * narrow character size
 */
static inline void lcd_internal_functionset( uint8_t _nFlags ) {
	lcd_internal_portwritecmd();
	lcd_internal_write( LCD_COMMAND_FUNCTION | _nFlags );
	_delay_us( LCD_DELAY_COMMAND );				// wait for 39 us function set delay
	return;
}

/*
 * send 'display control on/off control' command
 * you can use the following flags to specify exact behaviour
 * - LCD_PARAM_DISPLAYON or LCD_PARAM_DISPLAYOFF switches the entire display on/off
 * - LCD_PARAM_CURSORON	or LCD_PARAM_CURSOROFF display cursor or not
 * - LCD_PARAM_BLINKINGON or LCD_PARAM_BLINKINGOFF blinking cursor
 */
static void lcd_internal_displaycontrol( uint8_t _nFlags) {
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( LCD_COMMAND_DISPLAYCONTROL | _nFlags );
	return;
}

/*
 * return cursor to home position
 */
static inline void lcd_internal_returnhome( void ) {
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( LCD_COMMAND_RETURNHOME );
	return;
}

/*
 * shift entire display
 * - LCD_PARAM_INCREMENTCURSOR or LCD_PARAM_DECREMENTCURSOR set cursor movement
 * - LCD_PARAM_SHIFT shift entire display
 */
void lcd_internal_entrymodeset( uint8_t _nFlags ) {
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( LCD_COMMAND_ENTRYMODESET | _nFlags );
	return;
}

/*
 * clear display
 */
static void lcd_internal_cleardisplay( void ) {
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( LCD_COMMAND_CLEARDISPLAY );								
	return;
}

/*
 * clear display, user space
 */
void lcd_clear( void ) {
	lcd_gotopos(0, 0);
	lcd_puts_P( STRING_LCDEMPTY );
	lcd_gotopos(0, 1);
	lcd_puts_P( STRING_LCDEMPTY );
	return;
}

/*
 * goto position on lcd
 */
void lcd_gotopos( uint8_t _nX, uint8_t _nY ) {
	uint8_t nAddress = LCD_COMMAND_SETDDRAMADDRESS;

	// validate parameter, fail silent if wrong parameters are provided
	if( _nX > 15 || _nY > 1) {
		return;
	}		

	// calculate address
	switch( _nY ) {
		case 0:
			nAddress += _nX + 0x00;
			break;
		case 1:
			nAddress += _nX + 0x40;
			break;
		default:
			// should never happen
			return;
			break;
	}		
	
	// write cmd
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( nAddress );

	return;
}


/*
 * initialize lcd
 * see page 17 of WH1602B.pdf for a detailed description of this procedure
 * note: this is not the standard HD44780 procedure since
 * this would require a switching to 8 bit mode twice and then 4bit
 */
void lcd_init( void ) {	
	// start initialization sequence
	_delay_ms( LCD_DELAY_BOOTUP );							// wait boot up time of 40ms

	// first setup display in 8bit mode
	lcd_internal_reset();
	lcd_internal_reset();
	lcd_internal_reset();

	lcd_internal_functionset( LCD_PARAM_DATA4BIT | 
		LCD_PARAM_DISP2LINE | LCD_PARAM_FONT5X11 );			// do function set
	lcd_internal_functionset( LCD_PARAM_DATA4BIT | 
		LCD_PARAM_DISP2LINE | LCD_PARAM_FONT5X11 );			// do function set

	lcd_internal_displaycontrol( LCD_PARAM_DISPLAYON );		// displayonoff
	lcd_internal_cleardisplay();									// clear display
	lcd_internal_entrymodeset( LCD_PARAM_INCREMENTCURSOR );			// set entry mode
	lcd_internal_returnhome();
	return;
}

/*
 * write character to lcd
 */
void lcd_putc( char _cData ) {
	lcd_internal_waitbusyflag();
	lcd_internal_portwritedata();
	lcd_internal_write( (uint8_t) _cData );
	return;
}

/* 
 * write string to lcd
 * max 16. chars
 */
void lcd_puts( const char* _sData ) {
	uint8_t nCharCount = 0;

	while( *_sData != '\0' && nCharCount < 16 ) {
		lcd_putc( *_sData );
		_sData++;
		nCharCount++;
	}
	return;
}

/*
 * write string from program memory to lcd
 * max 16. chars
 */
void lcd_puts_P( const char* _sData ) {
	uint8_t nCharCount = 0;	
	
	while( pgm_read_byte(_sData) != '\0' && nCharCount < 16 ) {
		lcd_putc( pgm_read_byte(_sData) );
		_sData++;
		nCharCount++;
	}
	return;
}

/*
 * write cgram character
 * pattern array must be 8 characters wide
 */ 
void lcd_cgram( uint8_t _nLocation, const uint8_t* _aPattern ) {
	uint8_t i;	
	
	/* check bounds, fail silent if wrong parameters are provided */	
	if( _nLocation > 7 ) {
		return;	
	}	
	
	// write initial address
	_nLocation = LCD_COMMAND_SETCGRAMADDRESS + (_nLocation<<3);
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( _nLocation );
	
	for( i=0; i<8; i++) {
		lcd_internal_waitbusyflag();
		lcd_internal_portwritedata();
		lcd_internal_write( *_aPattern );
		_aPattern++;			
	}
}

/*
 * write cgram character from program memory to lcd
 * pattern array must be 8 characters wide
 */
void lcd_cgram_P( uint8_t _nLocation, const uint8_t* _aPattern ) {
	uint8_t i;	
	
	/* check bounds, fail silent if wrong parameters are provided */	
	if( _nLocation > 7 ) {
		return;	
	}	
	
	// write initial address
	_nLocation = LCD_COMMAND_SETCGRAMADDRESS + (_nLocation<<3);
	lcd_internal_waitbusyflag();
	lcd_internal_portwritecmd();
	lcd_internal_write( _nLocation );
	
	for( i=0; i<8; i++) {
		lcd_internal_waitbusyflag();
		lcd_internal_portwritedata();
		lcd_internal_write( pgm_read_byte(_aPattern) );
		_aPattern++;			
	}
}
