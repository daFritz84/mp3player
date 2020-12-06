/*
 * handles keypad input and debouncing
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* constants */
#define PORT_KEYPAD		PORTD
#define DDR_KEYPAD		DDRD
#define PIN_KEYPAD		PIND

#define PIN_KEYPAD0		PIND4				/* should be three coherent pins */
#define PIN_KEYPAD1		(PIN_KEYPAD0 + 1)
#define PIN_KEYPAD2		(PIN_KEYPAD0 + 2)

#define PIN_KEYPADX		(_BV(PIN_KEYPAD0) | _BV(PIN_KEYPAD1) | _BV(PIN_KEYPAD2))


/* functions */
void keypad_init( void );
void keypad_debounce( void );

