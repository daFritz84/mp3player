/*
 * mp3 decoder driver
 * author: 	Stefan Seifried
 * date:	31.05.2011
 * matr.nr.:0925401
 */

/* includes */
#ifdef __AVR_VERSION_H_EXISTS__
	#include <avr/interrupt.h>
#else
	#include <avr/interrupt.h>
	#include <avr/signal.h>
#endif
#include <stdio.h>

#include "mp3decoder.h"
#include "event_decoder.h"
#include "task_mmccard.h"
#include "task_display.h"
#include "common.h"


/* module vars */
static mp3decoder_sendspi_t __fpSend;
static mp3decoder_recvspi_t __fpRecv;
static uint8_t __aMP3Buffer[32];
static uint8_t __nMP3Volume;


/* helper macro's */
#define SEND_SINEONEVENT( __STEP__ ) \
	event_decoder_t tempEvent; \
	tempEvent.nEventID = EVENT_SINETESTON; \
	tempEvent.nStepCnt = (__STEP__); \
	event_decoder_put( &tempEvent );	

#define SEND_SINEOFFEVENT( __STEP__ ) \
	event_decoder_t tempEvent; \
	tempEvent.nEventID = EVENT_SINETESTOFF; \
	tempEvent.nStepCnt = (__STEP__); \
	event_decoder_put( &tempEvent );	

#define SEND_PLAYEVENT( __STEP__ ) \
	event_decoder_t tempEvent; \
	tempEvent.nEventID = EVENT_PLAY; \
	tempEvent.nStepCnt = (__STEP__); \
	event_decoder_put( &tempEvent );

#define SEND_VOLUMEEVENT( __VOLUME__ ) \
	event_decoder_t tempEvent; \
	tempEvent.nEventID = EVENT_VOLUME; \
	tempEvent.nStepCnt = (__VOLUME__); \
	event_decoder_put( &tempEvent );

/*
 * debug output helper
 */
void debug_print( const char* _pData ) {
	printf( "%s\n", _pData );	
	return;
}

/*
 * initialize mp3 decoder
 */
void mp3decoder_init( mp3decoder_sendspi_t _fpSend, mp3decoder_recvspi_t _fpRecv ) {
	__fpSend = _fpSend;
	__fpRecv = _fpRecv;

	/* initialize ports */
	MP3DECODER_PORT |= ( _BV(MP3DECODER_REGCS) | _BV(MP3DECODER_DATACS) | _BV(MP3DECODER_RESET) );
	MP3DECODER_DDR |= ( _BV(MP3DECODER_REGCS) | _BV(MP3DECODER_DATACS) | _BV(MP3DECODER_RESET) );
	
	MP3DECODER_DREQ_PORT |= ( _BV(MP3DECODER_DREQ_PIN) );
	MP3DECODER_DREQ_DDR	&= ~( _BV(MP3DECODER_DREQ_PIN) );

	/* rising edge triggers data transmission */	
	MCUCR |= ( _BV(ISC00) | _BV(ISC01) );					
	
	mp3decoder_reset();
	return;
}

/*
 * write mp3 decoder register
 * atomic function
 */
void mp3decoder_registerwrite( uint8_t _nRegisterID, uint16_t _nValue ) {

	MP3DECODER_PORT &= ~( _BV(MP3DECODER_REGCS) );
	
	/* according to datasheet, section 7.4, we do not need to check DREQ 
       during operations that send less than 32 bytes!*/
	__fpSend( MP3DECODER_OPCODE_WRITE );	/* write opcode */
	__fpSend( _nRegisterID );				/* register value */
	__fpSend( (uint8_t)(_nValue>>8) );		/* high byte */
	__fpSend( (uint8_t)(_nValue) );			/* low byte */
	
	MP3DECODER_PORT |= ( _BV(MP3DECODER_REGCS) );
	
	return;
}

/*
 * read mp3 decoder register
 * atomic function
 */
void mp3decoder_registerread( uint8_t _nRegisterID, uint16_t *_pValue ) {
	
	MP3DECODER_PORT &= ~( _BV(MP3DECODER_REGCS) );
	
	__fpSend( MP3DECODER_OPCODE_READ );		/* read opcode */
	__fpSend( _nRegisterID );				/* register value */
	(*_pValue) = (uint16_t)__fpRecv()<<8;	/* high byte */
	(*_pValue) |= __fpRecv();				/* low byte */
	
	MP3DECODER_PORT |= ( _BV(MP3DECODER_REGCS) );
	
	return;
}

/*
 * write data to mp3 decoder
 * data must be aligned by two bytes! Since the receiving chip is 16bit!
 * see new_mode spec. in datasheet
 * atomic function
 *
 * returns number of bytes written
 */
void mp3decoder_datawrite( uint8_t *_pData, uint8_t _nSize ) {

	MP3DECODER_PORT &= ~( _BV(MP3DECODER_DATACS) );		
	while(_nSize--) {
		__fpSend( *_pData++ );
	}
	MP3DECODER_PORT |= ( _BV(MP3DECODER_DATACS) );	
	
	return;
}

/*
 * mp3 decoder sine test on
 */
void mp3decoder_sinetest_on( uint8_t _nStep ) {
	uint8_t aSineOn[] = {0x53, 0xEF, 0x6E, 0xCC, 0x00, 0x00, 0x00, 0x00};		
	
	/* stop playback! */
	mp3decoder_pause();	

	switch( _nStep ) {
		case 0:
			/* hard reset */
			mp3decoder_reset();
				
		case 1:
			/* set clock */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {
				mp3decoder_registerwrite( MP3DECODER_REG_CLOCKF, MP3DECODER_CLOCK );
			}
			else {
				SEND_SINEONEVENT(1);
				return;
			}

		case 2:
			/* set modes */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {
				mp3decoder_registerwrite( MP3DECODER_REG_MODE, _BV( MODE_SDINEW ) | _BV( MODE_TESTS ) );
			}
			else {
				SEND_SINEONEVENT(2);
				return;			
			}
		
		case 3:
			/* set volume */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {
				mp3decoder_registerwrite( MP3DECODER_REG_VOL, ((~__nMP3Volume)<<8) );
			}
			else {
				SEND_SINEONEVENT(3);
				return;			
			}
		
		case 4:
			/* set sine on sequence */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {			
				mp3decoder_datawrite( aSineOn, sizeof(aSineOn) );	
			}
			else {
				SEND_SINEONEVENT(4);
				return;		
			}
			break;
			
		default:
			break;
	}
	return;
}

/*
 * mp3 decoder sine test off
 */
void mp3decoder_sinetest_off( uint8_t _nStep ) {
	uint8_t aSineOff[] = {0x45, 0x78, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00};	
	
	/* disable playback */
	mp3decoder_pause();

	switch( _nStep ) {
		case 0:
			/* send stop sequence */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {						
				mp3decoder_datawrite( aSineOff, sizeof(aSineOff) );				
			}
			else {
				SEND_SINEOFFEVENT(0);
				return;
			}
		
		case 1:
			/* send mode */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {	
				mp3decoder_registerwrite( MP3DECODER_REG_MODE, _BV( MODE_SDINEW ) );
			}
			else {
				SEND_SINEOFFEVENT(1);
				return;
			}
			break;

		default:
			break;	
	}
	
	return;
}

/*
 * initialize mp3 decoder for playback
 */
void mp3decoder_play( uint8_t _nStep ) {

	switch( _nStep ) {
		case 0:
			/* soft reset */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {	
				mp3decoder_registerwrite( MP3DECODER_REG_MODE, _BV(MODE_RESET) );
			}
			else {
				SEND_PLAYEVENT(1);
				return;
			}			
					
		case 1:
			/* set clock */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {	
				mp3decoder_registerwrite( MP3DECODER_REG_CLOCKF, MP3DECODER_CLOCK );
			}
			else {
				SEND_PLAYEVENT(1);
				return;
			}
		
		case 2:
			/* set mode */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {
				mp3decoder_registerwrite( MP3DECODER_REG_MODE, _BV( MODE_SDINEW ) );
			}
			else {
				SEND_PLAYEVENT(2);
				return;			
			}
		
		case 3:
			/* set volume */
			if( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {
				mp3decoder_registerwrite( MP3DECODER_REG_VOL, ((~__nMP3Volume)<<8) );
			}
			else {
				SEND_PLAYEVENT(3);
				return;			
			}

		case 4:
			mp3decoder_resume();
			break;
	}

	return;
}

/*
 * play next title
 */
void mp3decoder_playnext( void ) {
	mp3decoder_pause();
	
	if( mmccard_readnextmp3() != SUCCESS ) {
		mmccard_reset();	
	}
	
	if( mmccard_openmp3() != SUCCESS ) {
		mmccard_reset();
	}
	
	display_reset();
	eeprom_reset();
	mp3decoder_play( 0 );
	return;
}

/*
 * play previous title
 */
void mp3decoder_playprev( void ) {
	mp3decoder_pause();
	
	if( mmccard_readprevmp3() != SUCCESS ) {
		mmccard_reset();	
	}
	
	if( mmccard_openmp3() != SUCCESS ) {
		mmccard_reset();
	}
	
	display_reset();
	eeprom_reset();
	mp3decoder_play( 0 );
	return;
}

/*
 * pause mp3 playback
 */
void mp3decoder_pause( void ) {
	/* disable play interrupt */
	GICR &= ~_BV(INT0);
	return;
}

/*
 * resume mp3 playback
 */
void mp3decoder_resume( void ) {
	/* enable interrupt */
	GICR |= _BV(INT0);	

	/* send first few bytes if dreq is already high, so we start the interrupt chain for sure */
	while( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {
		if( mmccard_get_music( __aMP3Buffer ) == SUCCESS ) {					
			mp3decoder_datawrite( __aMP3Buffer, sizeof(__aMP3Buffer) );
		}
		else {
			mp3decoder_pause();
			break;
		}		
	}

	return;
}

/*
 * set mp3 volume
 */
void mp3decoder_setvolume( uint8_t _nVolume ) {
	uint8_t nSREGsave = SREG;
	if( __nMP3Volume != _nVolume ) {
		__nMP3Volume = _nVolume;
		cli();
		mp3decoder_registerwrite( MP3DECODER_REG_VOL, ((~__nMP3Volume)<<8) );
		SREG = nSREGsave;
	}
	return;
}

/*
 * get mp3 decoder time
 */
uint16_t mp3decoder_getdecodetime( void ) {
	uint16_t nValue;
	uint8_t nSREGsave = SREG;

	cli();
	mp3decoder_registerread( MP3DECODER_REG_DECODE_TIME, &nValue );
	SREG = nSREGsave;

	return nValue;
}

/*
 * mp3 decoder reset
 */
inline void mp3decoder_reset( void ) {
	/* create reset pulse */	
	MP3DECODER_PORT &= ~( _BV(MP3DECODER_RESET) );
	asm volatile(
		"nop\n"
		"nop\n"
		"nop\n"
		"nop"		
		:
	);
	MP3DECODER_PORT |= ( _BV(MP3DECODER_RESET) );
	return;
}


/*********************************************************
 * ISR's
 *********************************************************/

/*
 * int0, does actual data copy
 */
SIGNAL( SIG_INTERRUPT0 ) {
	uint8_t nStatus;
	while( (MP3DECODER_DREQ_PIND & _BV(MP3DECODER_DREQ_PIN)) > 0) {	
		if( (nStatus = mmccard_get_music( __aMP3Buffer )) == SUCCESS ) {					
			mp3decoder_datawrite( __aMP3Buffer, sizeof(__aMP3Buffer) );
		}
		else {
			mp3decoder_pause();
			break;
		}	
	}
}

