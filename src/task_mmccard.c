/*
 * lcd display task
 * author: 	Stefan Seifried
 * date:	07.06.2011
 * matr.nr.:0925401
 */

/* includes */
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#ifdef __AVR_VERSION_H_EXISTS__
	#include <avr/interrupt.h>
#else
	#include <avr/interrupt.h>
	#include <avr/signal.h>
#endif


#include "task_mmccard.h"
#include "asm_spi.h"
#include "mmc_driver.h"
#include "common.h"
#include "fat.h"
#include "stringman.h"
#include "event_userio.h"
#include "main.h"


/* helper macro's */
#define CHECKREAD( __read__ ) \
	if( (__read__) == FAIL ) { \
		return FAIL; \
	}



/* module constants */
static mmccard_sendspi_t __fpSend;
static mmccard_recvspi_t __fpRecv;

static FAT_Handle_t	__oMusicHandle;
static FAT_Handle_t __oInfoHandle;
static FAT_Stream_t __oStream;
static uint8_t __nState = STATE_INITIALIZING;
static uint8_t __nEepromStatus = EEPROM_NOTWRITTEN;

static mp3info_t __structMP3Info;


#ifdef DEBUG
/*
 * debug output via USART
 */
static void mmccard_debugout( const char* _sMessage ) {
	printf("%s\n", _sMessage );	
	return;
}
#endif

/*
 * mmc card wrapper to support fat read operations
 */
static uint8_t mmccard_read_wrapper( uint32_t _nAddress, uint8_t* _pBuffer ) {
	mmc_block_t aBuffer;

	/* TODO: add crc check here */
	if( mmc_read_single_block( _nAddress, &aBuffer ) == MMC_SUCCESS ) {
		memcpy( _pBuffer, aBuffer.data, 32 );
		return SUCCESS;
	}	
	else {
		mmccard_reset();
		return FAIL;
	}
}

static uint8_t mmccard_readentry( mp3info_t* _pInfo ) {
	uint8_t aBuffer[32];

	/* read first 32 byte */	
	CHECKREAD( fat_fread(aBuffer, &__oStream, &__oInfoHandle ) );	

	memcpy( _pInfo->sFileName, &aBuffer[0], 8 );
	_pInfo->sFileName[8] = '\0';				/* don't forget 0 term */	
	
	memcpy( _pInfo->sInfo, &aBuffer[8], 24 );

	/* read second 32 byte */
	CHECKREAD( fat_fread(aBuffer, &__oStream, &__oInfoHandle ) );	
	memcpy( &_pInfo->sInfo[24], aBuffer, 32 );	
	_pInfo->sInfo[55] = '\0';					/* don't forget 0 term */	
	
	/* trim trailing white spaces from info */
	string_rtrim( _pInfo->sInfo );

	return SUCCESS;
}

/*
 * initialize mmc card task
 */
void mmccard_init( mmccard_sendspi_t _fpSend, mmccard_recvspi_t _fpRecv ) {
	__fpSend = _fpSend;
	__fpRecv = _fpRecv;	
	return;
}


/*
 * mmc card worker
 * implements actual state machine which drives mmc card
 */
void mmccard_worker( void ){
	switch( __nState ) {
		case STATE_INITIALIZING:
			/* try initializing card */
			#ifdef DEBUG
			if( init_mmc_driver( mmccard_debugout, __fpSend, __fpRecv ) == MMC_SUCCESS ) {
			#else
			if( init_mmc_driver( NULL, __fpSend, __fpRecv ) == MMC_SUCCESS ) {
			#endif				
				/* advance to next state */
				++__nState;
			}
			else {
				/* do nothing, try next time slice */
			}			
			break;

		case STATE_INITIALIZEFAT:
			/* initialize fat file system stream*/
			if( fat_ctor( &__oStream, mmccard_read_wrapper ) == SUCCESS ) {
				++__nState;	
			}
			else {
				/* could not initialize fat file system, reset */
				__nState = STATE_INITIALIZING;
			}
			break;
	
		case STATE_OPENMUSICINFO:
			/* get id3info.txt for music */
			if( fat_fopen( "ID3INFO ", &__oStream, &__oInfoHandle ) == SUCCESS ) {
				if( (main_getresetreg() & _BV(PORF)) > 0 ) {
					main_clearresetreg();
						
					/* load stored mp3_info */
					mmccard_load_info();				

					if( mmccard_findmp3() != SUCCESS ) {
						__nState = STATE_INITIALIZING;
					}					

					if( mmccard_openmp3() != SUCCESS ) {
						__nState = STATE_INITIALIZING;
					}
					
					event_userio_t tempEvent;
					tempEvent.nKey = KEY_RESUMEPROM;
					event_userio_put( &tempEvent );
				}
				else {
					event_userio_t tempEvent;
					tempEvent.nKey = KEY_FORWARD;
					event_userio_put( &tempEvent );
				}
				++__nState;			
			}
			else {
				/* could not initialize fat file system, reset */
				__nState = STATE_INITIALIZING;
			}
			break;		
		
		case STATE_READY:
			/* check if we need to play next song */
			if( __oMusicHandle.nFilePos >= __oMusicHandle.nFileSize ) {
				event_userio_t tempEvent;
				tempEvent.nKey = KEY_FORWARD;
				event_userio_put( &tempEvent );
			}
			
			if( __nEepromStatus == EEPROM_NOTWRITTEN ) {
				if( mmccard_save_info() == SUCCESS ) {
					__nEepromStatus = EEPROM_WRITTEN;
				}
			}

			break;		

		default:
			printf_P( PSTR("DEBUG: MMC_UNDEFINED_STATE") );
			break;	
	}

	return;
}

/*
 * reset mmc card 
 */
void mmccard_reset( void ) {
	__nState = STATE_INITIALIZING;
	return;
}

/*
 * reset eeprom state
 */
void eeprom_reset( void ) {
	__nEepromStatus = EEPROM_NOTWRITTEN;
}

/*
 * get actual mmc card state
 */
uint8_t mmccard_getstate( void ) {
	return __nState;
}

/*
 * read next mp3 title info
 */
uint8_t mmccard_readnextmp3( void ) {
	//uint8_t aBuffer[32];	
	
	/* check if we have at least one entry, and are 64 byte aligned */
	if( (__oInfoHandle.nFileSize & 63) > 0 || __oInfoHandle.nFileSize < 64 ) {
		return FAIL;
	}

	/* check if we have exceeded file size, so we need to start over */	
	if( __oInfoHandle.nFilePos >= __oInfoHandle.nFileSize ) {
		CHECKREAD( fat_seek( 0, &__oStream, &__oInfoHandle ) );	
	}
	
	CHECKREAD( mmccard_readentry(&__structMP3Info) );

	return SUCCESS;
}

/*
 * read previous mp3 title info
 */
uint8_t mmccard_readprevmp3( void ) {
	//uint8_t aBuffer[32];	
	
	/* check if we have at least one entry, and are 64 byte aligned */
	if( (__oInfoHandle.nFileSize & 63) > 0 || __oInfoHandle.nFileSize < 64 ) {
		return FAIL;
	}

	/* check if we have reached 0, so we need to start over */
	if( __oMusicHandle.nFilePos > REVERSE_THRESHOLD ) {
		CHECKREAD( fat_seek( (__oInfoHandle.nFilePos - 64), &__oStream, &__oInfoHandle ) );
	}	
	else if( __oInfoHandle.nFilePos >= 128 ) {
		CHECKREAD( fat_seek( (__oInfoHandle.nFilePos - 128), &__oStream, &__oInfoHandle ) );	
	}
	else{
		CHECKREAD( fat_seek( (__oInfoHandle.nFileSize - 64), &__oStream, &__oInfoHandle ) );	
	}	
	
	CHECKREAD( mmccard_readentry(&__structMP3Info) );	
	
	return SUCCESS;
}

/*
 * find mp3 file in index file
 */
uint8_t mmccard_findmp3( void ) {
	mp3info_t __tempInfo;
	//uint8_t aBuffer[32];	

	/* check if we have at least one entry, and are 64 byte aligned */
	if( (__oInfoHandle.nFileSize & 63) > 0 || __oInfoHandle.nFileSize < 64 ) {
		return FAIL;
	}

	/* we assume that we are already at the beginning, thus FilePos must be 0 */
	else if( __oInfoHandle.nFilePos != 0 ) {
		return FAIL;
	}
	
	/* search given mp3 info */

	while( __oInfoHandle.nFilePos < __oInfoHandle.nFileSize ) {
			
			CHECKREAD( mmccard_readentry(&__tempInfo) );			

			/* check if we have found our entry, therfore file name and id3 info must exactly match */
			if( memcmp( &__tempInfo, &__structMP3Info, sizeof(mp3info_t) ) == 0 ) {
				return SUCCESS;
			}	
	}	

	return FAIL;
}

/*
 * open mp3 file
 */
uint8_t mmccard_openmp3( void ) {
	CHECKREAD( fat_fopen( __structMP3Info.sFileName, &__oStream, &__oMusicHandle ) );	
	return SUCCESS;
}

/*
 * get music from current mp3
 */
uint8_t mmccard_get_music( uint8_t* _aBuffer ) {
	return fat_fread( _aBuffer, &__oStream, &__oMusicHandle );
}

/*
 * save music info to eeprom
 */
uint8_t mmccard_save_info( void ) {
	if( eeprom_is_ready() ) {
		eeprom_write_block( &__structMP3Info, 0x0, sizeof(__structMP3Info) );
		return SUCCESS;
	}
	return FAIL;
}

/*
 * load music info from eeprom
 */
void mmccard_load_info( void ) {
	/* busy waiting seems acceptable here, since we call this function only
	 * once during fat boot up
     */
	eeprom_busy_wait();
	eeprom_read_block( &__structMP3Info, 0x0, sizeof(__structMP3Info) );
	return;
}

/*
 * get current mp3 info string
 */
char* mmccard_get_info( void ) {
	return __structMP3Info.sInfo;
}

/*
 * get progress in 45 segment steps
 */
uint8_t mmccard_get_progress( void ) {
	uint8_t nProgress;
	nProgress = ((uint32_t)__oMusicHandle.nFilePos * 45LU) / (uint32_t)__oMusicHandle.nFileSize;
	return nProgress;
}
