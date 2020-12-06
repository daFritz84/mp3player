/*
 * software spi, assembler prototype header
 * author: 	Stefan Seifried
 * date:	29.05.2011
 * matr.nr.:0925401
 */

#ifndef _ASM_SPI_H_
#define _ASM_SPI_H_

/* functions */
extern void asm_spi_init( void );

extern void asm_spi_send_pol0phas1( uint8_t _cData );
extern uint8_t asm_spi_receive_pol0phas1( void );

#endif /* _ASM_SPI_H_ */
