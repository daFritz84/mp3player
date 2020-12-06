/*
 * atmega16 lcd  driver
 * author: 	Stefan Seifried
 * date:	30.03.2011
 * matr.nr.:0925401
 *
 * implementation of a 4bit HD44780 like interface
 */

#ifndef _LCD_H_
#define _LCD_H_

/* includes */
#include <avr/io.h>


/* macros */
/* specifiy used port here */
#define LCD_PORT		PORTA
#define LCD_DDR			DDRA
#define LCD_PIN			PINA

/* specify used data pin's here */
#define LCD_DB4			PA4		// use lower 4 bit's
#define LCD_RS			PA1
#define LCD_RW			PA2
#define LCD_EN			PA3


/* lcd timing */
#define LCD_DELAY_BOOTUP		40 	// wait for more than 40.0 ms
#define LCD_DELAY_COMMAND		39 	// wait for more than 39.0 us

/* lcd commands */
/* - function set */
#define LCD_COMMAND_FUNCTION 		(0x20)
#define LCD_PARAM_DATA4BIT			(0)
#define LCD_PARAM_DATA8BIT			_BV(4)
#define LCD_PARAM_DISP1LINE			(0)
#define LCD_PARAM_DISP2LINE			_BV(3)
#define LCD_PARAM_FONT5X8			(0)
#define	LCD_PARAM_FONT5X11			_BV(2)

/* - display control */
#define LCD_COMMAND_DISPLAYCONTROL	(0x08)
#define LCD_PARAM_DISPLAYON			_BV(2)
#define LCD_PARAM_DISPLAYOFF		(0)
#define LCD_PARAM_CURSORON			_BV(1)
#define LCD_PARAM_CURSOROFF			(0)
#define LCD_PARAM_BLINKINGON		(1)
#define LCD_PARAM_BLINKINGOFF		(0)

/* - clear display */
#define LCD_COMMAND_CLEARDISPLAY	(0x01)

/* - set entry mode */
#define LCD_COMMAND_ENTRYMODESET	(0x10)
#define LCD_PARAM_INCREMENTCURSOR	_BV(1)
#define LCD_PARAM_DECREMENTCURSOR	(0)
#define LCD_PARAM_SHIFT				(1)

/* - return cursor home */
#define LCD_COMMAND_RETURNHOME		(0x02)

/* - lcd goto command / set ddram address */
#define LCD_COMMAND_SETDDRAMADDRESS	(0x80)

/* - lcd cgram command */
#define LCD_COMMAND_SETCGRAMADDRESS (0x40)


/* functions */
void lcd_init( void );

/* commands */
void lcd_clear( void );
void lcd_gotopos( uint8_t _nX, uint8_t _nY );

/* i/o */
void lcd_putc( char _cData );
void lcd_puts( const char* _sData );
void lcd_puts_P( const char* _sData );

/* custom characters */
void lcd_cgram( uint8_t _nLocation, const uint8_t* _aPattern );
void lcd_cgram_P( uint8_t _nLocation, const uint8_t* _aPattern );

#endif /* _LCD_H_ */
