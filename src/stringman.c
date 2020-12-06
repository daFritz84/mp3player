/*
 * string manipulation functions
 * author: 	Stefan Seifried
 * date:	26.06.2011
 * matr.nr.:0925401
 */

/* includes */
#include <ctype.h>
#include <string.h>


/*
 * trim trailing whitespaces
 */
void string_rtrim( char* _sValue ) {
	char* _pWalker = _sValue + strlen(_sValue) - 1;
	
	while( _pWalker > _sValue && isspace(*_pWalker)) {
		--_pWalker;	
	}	
	
	*(_pWalker+1) = '\0';
	return;
}

