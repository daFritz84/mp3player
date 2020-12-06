/*
 * stdout output redirection to usart
 * author: 	Stefan Seifried
 * date:	30.03.2011
 * matr.nr.:0925401
 * 
 * redirect output from stdout to usart so printf(), and
 * all the standard output stream functions become usable
 */

// includes
/*
 * this file is only present in newer builds of avr_libc
 * make sets this define if it is present
 */
#ifdef __AVR_VERSION_H_EXISTS__
#include <avr/version.h>
#endif

#include "usart_stdout.h"
#include "usart.h"

/*
 * wrapper for newer versions of avr_libc
 * Since the old avrlibc does not provide any
 * header to switch between newer version and older
 * ones there is still a method needed which provides
 * compability between those version
 */
#ifdef __AVR_LIBC_VERSION__
int usart_stdout_putc( char _cByte, FILE* _pStream ) {
	usart_putc( _cByte );
	return 0;
}

static FILE __usart_stdout = FDEV_SETUP_STREAM(&usart_stdout_putc, NULL,_FDEV_SETUP_WRITE);
#endif

/*
 * redirect stdout to the usart driver
 */
void usart_stdout_redirect( void ) {
	#ifdef __AVR_LIBC_VERSION__
	stdout = &__usart_stdout;
	#else
	/* the first stream opened with write functionality will be automatically
	 * assigned to stdout & stderr
	 */ 
	fdevopen( &usart_putc, NULL, 0 );
	#endif
	return;
}
