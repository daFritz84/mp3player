/*
 * mp3 decoder driver
 * author: 	Stefan Seifried
 * date:	31.05.2011
 * matr.nr.:0925401
 */

#ifndef _MP3DECODER_H_
#define _MP3DECODER_H_

/* includes */
#include <avr/io.h>


/* constants */
#define MP3DECODER_PORT			PORTC
#define MP3DECODER_DDR			DDRC
#define MP3DECODER_REGCS		PC4		/* MP3_CS */
#define MP3DECODER_DATACS		PC5		/* BSYNC */
#define MP3DECODER_RESET		PC6		/* MP3 RESET */

#define MP3DECODER_DREQ_PORT	PORTD
#define MP3DECODER_DREQ_PIND	PIND
#define MP3DECODER_DREQ_DDR		DDRD
#define MP3DECODER_DREQ_PIN		PD2		/* DREQ */


#define MP3DECODER_CLOCK		12500
#define MP3DECODER_MAXWRITE		32


/* opcodes for decoder operation */
#define MP3DECODER_OPCODE_WRITE			0x2
#define MP3DECODER_OPCODE_READ			0x3


/*possible mp3 decoder registers
  see page 29 of datasheet */
#define MP3DECODER_REG_MODE				0x0
#define MP3DECODER_REG_STATUS			0x1
#define MP3DECODER_REG_BASS				0x2
#define MP3DECODER_REG_CLOCKF			0x3
#define MP3DECODER_REG_DECODE_TIME		0x4
#define MP3DECODER_REG_AUDATA			0x5
#define MP3DECODER_REG_WRAM				0x6
#define MP3DECODER_REG_WRAMADDR			0x7
#define MP3DECODER_REG_HDAT0			0x8
#define MP3DECODER_REG_HDAT1			0x9
#define MP3DECODER_REG_AIADDR			0xA
#define MP3DECODER_REG_VOL				0xB
#define MP3DECODER_REG_AICTRL0			0xC
#define MP3DECODER_REG_AICTRL1			0xD
#define MP3DECODER_REG_AICTRL2			0xE
#define MP3DECODER_REG_AICTRL3			0xF


/*possible bit values for mode register, 
  see page 30 of datasheet */
#define MODE_DIFF						0
#define MODE_LAYER12					1
#define MODE_RESET						2
#define MODE_OUTOFWAV					3
#define MODE_SETTOZERO1					4
#define MODE_TESTS						5
#define MODE_STREAM						6
#define MODE_SETTOZERO2					7
#define MODE_DACT						8
#define MODE_SDIORD						9
#define MODE_SDISHARE					10
#define MODE_SDINEW						11
#define MODE_SETTOZERO3					12
#define MODE_SETTOZERO4					13


/*mp3 status register */
#define REG_VOLCHANGED					0

/* callback function's */
typedef void (*mp3decoder_sendspi_t)( uint8_t _cData );
typedef uint8_t (*mp3decoder_recvspi_t)( void );


/* functions */
void mp3decoder_init( mp3decoder_sendspi_t _fpSend, mp3decoder_recvspi_t _fpRecv );
void mp3decoder_registerwrite( uint8_t _nRegisterID, uint16_t _nValue );
void mp3decoder_registerread( uint8_t _nRegisterID, uint16_t *_pValue );
void mp3decoder_datawrite( uint8_t *_pData, uint8_t _nSize );
void mp3decoder_sinetest_on( uint8_t _nStep );
void mp3decoder_sinetest_off( uint8_t _nStep );
void mp3decoder_play( uint8_t _nStep );
void mp3decoder_pause( void );
void mp3decoder_resume( void );
void mp3decoder_playnext( void );
void mp3decoder_playprev( void );
void mp3decoder_setvolume( uint8_t _nVolume );
uint16_t mp3decoder_getdecodetime( void );
inline void mp3decoder_reset( void );

#endif /*_MP3DECODER_H_*/
