/*
 * software spi assmbler implementation
 * author: 	Stefan Seifried
 * date:	29.05.2011
 * matr.nr.:0925401
 */

/* header */
#include <avr/io.h>

/* used ports */
#define SPI_DDR		DDRC
#define SPI_PORT	PORTC
#define SPI_PIN		PINC

#define SCK			PC1
#define MOSI		PC0	/* needs to be 1. bit!!! */
#define MISO		PC2


/* define used registers */
#define sreg_save	18	/* save sreg register */
#define spi_byte	24	/* return val, asm_spi_receive */
#define spi_lclock	19	/* spi low phase clock */
#define	temp		20	/* temp register */
#define _cData		24	/* param val,  asm_spi_send */


/* macro's */
/* set clock low */
.macro sck_low
	cbi		_SFR_IO_ADDR( SPI_PORT ), SCK
.endm 

/* set clock high */
.macro sck_high
	sbi		_SFR_IO_ADDR( SPI_PORT ), SCK
.endm

/* set mosi low */
.macro mosi_low
	cbi		_SFR_IO_ADDR( SPI_PORT ), MOSI
.endm

/* set mosi high */
.macro mosi_high
	sbi		_SFR_IO_ADDR( SPI_PORT ), MOSI
.endm


/* functions */
/*
 * initialize spi
 */
.global asm_spi_init
asm_spi_init:
	/* initialize SCK */	
	sck_low
	sbi		_SFR_IO_ADDR( SPI_DDR ), SCK
	
	/* according to dataheet, all lines have to be high */
	/* initialize MOSI, data output, */
	mosi_high
	sbi		_SFR_IO_ADDR( SPI_DDR ), MOSI

	/* initialize MISO, data input */
	/* according to  http://www.mikrocontroller.net/articles/MMC-_und_SD-Karten, 
       it is necessary to set a pullup */	
	sbi		_SFR_IO_ADDR( SPI_PORT ), MISO
	cbi		_SFR_IO_ADDR( SPI_DDR ), MISO
	
	ret

/*
 * send spi data @2Mhz clock
 * uses spi mode 1, (CPOL 0, CPH 1)
 */
.global asm_spi_send_pol0phas1
asm_spi_send_pol0phas1:
	in      sreg_save, _SFR_IO_ADDR(SREG)	
	cli										/* atomic! */
	
	/* prepare port for low clock state */
	in		spi_lclock, _SFR_IO_ADDR(SPI_PORT)
	andi	spi_lclock, ~_BV(MOSI)	

	/* 1. bit */
	rol		_cData
	rol		_cData	/* move MSB into first bit */
	mov		temp, _cData
	andi	temp, 0x1
	or		temp, spi_lclock
	out		_SFR_IO_ADDR(SPI_PORT), temp

	/* 2. bit */
	rol		_cData
	mov		temp, _cData	
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp

	/* 3. bit */
	rol		_cData
	mov		temp, _cData
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp


	/* 4. bit */
	rol		_cData
	mov		temp, _cData
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp	


	/* 5. bit */
	rol		_cData
	mov		temp, _cData
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp


	/* 6. bit */
	rol		_cData
	mov		temp, _cData
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp


	/* 7. bit */
	rol		_cData
	mov		temp, _cData
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp
	
	/* 8. bit */
	rol		_cData
	mov		temp, _cData
	sck_high
	andi	temp, 0x1
	or		temp, spi_lclock
	nop
	out		_SFR_IO_ADDR(SPI_PORT), temp


	/* finished, ensure one last clock pulse and we are done */				
	nop
	nop
	sck_high
	nop
	nop
	mosi_high	
	sck_low

	out	_SFR_IO_ADDR(SREG), sreg_save
	ret

/*
 * receive spi data @2Mhz clock
 * uses spi mode 1, (CPOL 0, CPH 1)
 */
.global asm_spi_receive_pol0phas1
asm_spi_receive_pol0phas1:
	in      sreg_save, _SFR_IO_ADDR(SREG)	
	cli				/* atomic! */
	
	/* enhanced through loop unrolling */
	/* 1. bit */	
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte

	/* 2. bit */
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* 3. bit */
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* 4. bit */
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* 5. bit */
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* 6. bit */
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* 7. bit */
	sck_low
	lsl		spi_byte
	nop
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* 8. bit */
	sck_low
	lsl		spi_byte
	nop	
	sck_high
	sbic 	_SFR_IO_ADDR( SPI_PIN ), MISO
	inc		spi_byte
	
	/* clean up */
	sck_low

	out	_SFR_IO_ADDR(SREG), sreg_save
	ret

