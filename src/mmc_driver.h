/*!
 * \file    mmc_driver.h
 * \author  Michael Birner
 * \date	25.09.2010
 * \modified by Andreas Hagmann on 22.05.2011
 * \modified by Andreas Hagmann on 25.05.2011
 * \version 1.2
 */
 
/*
 * Pin Description
 *	MMC_CD	PC7
 * 	MMC_CS	PC3
 */

#ifndef __MMC_DRIVER_H__
#define __MMC_DRIVER_H__

#include <avr/io.h>

typedef enum {
	MMC_SUCCESS,
	MMC_NO_CARD,
	MMC_ERROR,
} mmc_status_t;

typedef struct mmc_buffer {
    uint8_t data[32];
    uint8_t crc[2];
} mmc_block_t;

/*
 * Init the MMC Driver module
 *
 * \param output_string		Function pointer to any function which accepts a string. (Can be used for debugging. E.g. with LCD or UART)
 *							Can be NULL if no debug output is needed.
 * \param spi_send			Function pointer to a SPI send function.
 * \param spi_receive		Function pointer to a SPI receive function.
 *
 * \returns	status code
 */
mmc_status_t init_mmc_driver(void (*output_string)(const char *string), void (*spi_send)(uint8_t data), uint8_t (*spi_receive)(void));

/*
 * Reads a 32 byte block from a memory card.
 *
 * \param address	Block address
 * \param mmc_buf	Pointer to read buffer.
 *
 * \returns status code
 */
mmc_status_t mmc_read_single_block(uint32_t address, mmc_block_t *mmc_buf);

#endif /* __MMC_DRIVER_H__ */

/* EOF */
