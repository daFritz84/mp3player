/*
 * lcd display task
 * author: 	Stefan Seifried
 * date:	07.06.2011
 * matr.nr.:0925401
 */

#ifndef _TASK_MMCCARD_H_
#define _TASK_MMCCARD_H_

/* constants */
#define STATE_INITIALIZING	0
#define STATE_INITIALIZEFAT	1
#define STATE_OPENMUSICINFO	2
#define STATE_READY			3

#define REVERSE_THRESHOLD	131072LU

#define EEPROM_NOTWRITTEN	0
#define EEPROM_WRITTEN		1

/* data structures */
struct mp3info {
	char sFileName[9];
	char sInfo[56];
};
typedef struct mp3info mp3info_t;

/* includes */
#include "mmc_driver.h"


/* callback definitions */
typedef void (*mmccard_sendspi_t)( uint8_t _cData );
typedef uint8_t (*mmccard_recvspi_t)( void );

/* functions */
void mmccard_init( mmccard_sendspi_t _fpSend, mmccard_recvspi_t _fpRecv );
void mmccard_worker( void );
void mmccard_reset( void );
void eeprom_reset( void ); 
uint8_t mmccard_getstate( void );
uint8_t mmccard_readnextmp3( void );
uint8_t mmccard_readprevmp3( void );
uint8_t mmccard_findmp3( void );
uint8_t mmccard_openmp3( void );
uint8_t mmccard_get_music( uint8_t* _aBuffer );

uint8_t mmccard_save_info( void );
void mmccard_load_info( void );

char* mmccard_get_info( void );
uint8_t mmccard_get_progress( void );

#endif /* _TASK_MMCCARD_H_ */
