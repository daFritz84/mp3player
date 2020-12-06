/*
 * string table
 * author: 	Stefan Seifried
 * date:	24.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/pgmspace.h>

#include "stringtable.h"

const char STRING_LCDEMPTY[] PROGMEM = "                ";
const char STRING_SDCARDERROR1[] PROGMEM = "NO/INCOMPATIBLE";
const char STRING_SDCARDERROR2[] PROGMEM = "SD-CARD";
const char STRING_SDCARDERROR_UART[] PROGMEM = "No or incompatible SD-CARD";

