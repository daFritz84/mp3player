/*
 * stdout output redirection to usart
 * author: 	Stefan Seifried
 * date:	30.03.2011
 * matr.nr.:0925401
 * 
 * redirect output from stdout to usart so printf(), and
 * all the standard output stream functions become usable
 */

#ifndef _USART_STDOUT_H_
#define _USART_STDOUT_H_

#include <stdio.h>

// functions
void usart_stdout_redirect( void );

#endif /*_USART_STDOUT_H_*/
