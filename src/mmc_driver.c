/*!
 * \file    mmc_driver.h
 * \author  Michael Birner
 * \date	25.09.2010
 * \modified by Andreas Hagmann on 22.05.2011
 * \modified by Andreas Hagmann on 25.05.2011
 * \version 1.2
 */

/*-------------------------------------------------------------------------------*/
/*                                INCLUDES                                       */
/*-------------------------------------------------------------------------------*/

#include "mmc_driver.h"
#include <stdlib.h>
#include <avr/pgmspace.h>

/*-------------------------------------------------------------------------------*/
/*                              DECLARATIONS                                     */
/*-------------------------------------------------------------------------------*/

#define MMC_SPI_SEL_LOW()     PORTC &= ~((1<<PC3))
#define MMC_SPI_SEL_HIGH()    PORTC |= ((1<<PC3))

#define BLOCK_SIZE	32

#define MMC_NO_CHIPCARD_INSIDE  0
#define MMC_CHIPCARD_INSIDE     1

#define READ_TIMEOUT	1000UL

static inline uint8_t mmc_check_card(void);
static void mmc_send_command(const uint8_t *);

static void (*sw_spi_send_data)(uint8_t);
static uint8_t (*sw_spi_receive_data)(void);
static void (*output_string_function)(const char *string);

#define COMMAND_LEN	6

static const uint8_t reset_command[] 		= {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
static const uint8_t get_css_command[]		= {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const uint8_t app_command[]			= {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const uint8_t send_op_cond_command[]	= {0x69, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const uint8_t set_blocklen_command[]	= {0x50, 0x00, 0x00, 0x00, 0x20, 0xFF};

/* temp buffer for progmem strings */
#define MAX_STRING	50
static char string_buffer[MAX_STRING];

void debug(PGM_P string) {
	strncpy_P(string_buffer, string, MAX_STRING);
    output_string_function(string_buffer);
}

/*-------------------------------------------------------------------------------*/
/*                              SUBROUTINES                                      */
/*-------------------------------------------------------------------------------*/

#define CHECK_TIMEOUT(_counter, time, str)				\
        timeout++;										\
														\
		if (_counter == time) {							\
       	    if (output_string_function != NULL)			\
	        {											\
                debug(PSTR(str));						\
            }											\
		    MMC_SPI_SEL_HIGH();							\
            return MMC_ERROR;							\
        }

/*!
 * \brief Init chip card driver - Must be called after UART has been init !
 * \return MMC_FAIL in case of an error, MMC_SUCCESS else
 */
mmc_status_t init_mmc_driver(void (*output_string)(const char *string), void (*spi_send)(uint8_t data), uint8_t (*spi_receive)(void))
{
    uint8_t i = 0;
    uint8_t resp = 255;
    uint8_t init_resp = 255;
	uint32_t timeout;
    uint8_t timeout2;
    
    /* install callback function */
    output_string_function = output_string;
	sw_spi_send_data = spi_send;
	sw_spi_receive_data = spi_receive;

    /* init ports */
    /* set mmc_chipsel to output */
    PORTC |= ((1<<PC3));
    DDRC |= ((1<<DDC3));

    /* set mmc_cd to input */
    PORTC |= ((1<<PC7));
    DDRC &= ~((1<<PC7));
    
    /* check if card is inserted */
    if(mmc_check_card() == MMC_NO_CHIPCARD_INSIDE)
    {
		if (output_string_function != NULL)
		{
        	debug(PSTR("\n\rMMC ERROR: NO SD card"));
		}
		MMC_SPI_SEL_HIGH();
        return MMC_NO_CARD;
    }

    MMC_SPI_SEL_LOW();

    /* sending 74+ clock cycles dummy packets to mmc */
    for(i = 0; i < 200; i++)
    {
        sw_spi_send_data(0xFF);
    }

    /* send reset command */
    mmc_send_command(reset_command);
	timeout = 0;
    while((resp = sw_spi_receive_data()) == 255) {
   	    CHECK_TIMEOUT(timeout, READ_TIMEOUT, "\n\rMMC_DRIVER : Reset timed out");
	}

    if(resp != 1)
    {
		if (output_string_function != NULL)
		{
        	debug(PSTR("\n\rMMC ERROR during RESET"));
    		debug(PSTR("\n\rERROR CODE: "));
        	itoa(resp, string_buffer, 10);
        	output_string_function(string_buffer);
		}
		MMC_SPI_SEL_HIGH();
        return MMC_ERROR;
    }
    
    /* init card */
    timeout2 = 0;
    while(init_resp != 0)
    {
	    timeout2++;
        mmc_send_command(app_command);
        timeout = 0;
        while((resp = sw_spi_receive_data()) == 255) {
            CHECK_TIMEOUT(timeout, READ_TIMEOUT, "\n\rMMC_DRIVER : App command response timeout");
		}
        
        if(resp != 1)
        {
			if (output_string_function != NULL)
			{
                debug(PSTR("\n\rMMC ERROR during APP_CMD"));
                debug(PSTR("\n\rERROR CODE: "));
                output_string_function(string_buffer);
                itoa(resp, string_buffer, 10);
                output_string_function(string_buffer);
            }
			MMC_SPI_SEL_HIGH();
            return MMC_ERROR;
        }
                
        mmc_send_command(send_op_cond_command);
        timeout = 0;
        while((init_resp = sw_spi_receive_data()) == 255) {
            CHECK_TIMEOUT(timeout, READ_TIMEOUT, "\n\rMMC_DRIVER : Send Op response timeout");	       
		}

        CHECK_TIMEOUT(timeout2, 10, "\n\rMMC_DRIVER : Send Op response timeout");
    }
    
    if(init_resp != 0)
    {
        if (output_string_function != NULL)
        {
    	    debug(PSTR("\n\rMMC ERROR during SEND_OP_COND"));
            debug(PSTR("\n\rERROR CODE: "));
            itoa(init_resp, string_buffer, 10);
            output_string_function(string_buffer);
        }
		MMC_SPI_SEL_HIGH();
        return MMC_ERROR;
    }
    
    mmc_send_command(get_css_command);
    timeout = 0;
    while((resp = sw_spi_receive_data()) == 255) {
        CHECK_TIMEOUT(timeout, READ_TIMEOUT, "\n\rMMC_DRIVER : Get CSS timeout");
    }
    
    if(resp != 0)
    {
        if (output_string_function != NULL)
	    {
            debug(PSTR("\n\rMMC ERROR during GET_CSS"));
            debug(PSTR("\n\rERROR CODE: "));
            itoa(resp, string_buffer, 10);
            output_string_function(string_buffer);
        }
		MMC_SPI_SEL_HIGH();
        return MMC_ERROR;
    }
    resp = sw_spi_receive_data() & 0x40;
    
    if (output_string_function != NULL)
	{
        if(resp == 0x40)
        {
            debug(PSTR("\n\rMMC: Found high capacity or extended capacity memory card"));
        }
        else
        {
            debug(PSTR("\n\rMMC: Found standard capacity memory card"));
        }
    }
             
    /* get other response tokens */
    sw_spi_receive_data();
    sw_spi_receive_data();
    sw_spi_receive_data();

    /* set block length */
    mmc_send_command(set_blocklen_command);
    timeout = 0;
    while((resp = sw_spi_receive_data()) == 255) {
        CHECK_TIMEOUT(timeout, READ_TIMEOUT, "\n\rMMC_DRIVER : Set blocklen timeout");
    }

   	if(resp != 0)
	{
        if (output_string_function != NULL)
	    {
    	    debug(PSTR("\n\rMMC ERROR during SET_BLOCK_LENGTH"));
            debug(PSTR("\n\rERROR CODE: "));
            itoa(resp, string_buffer, 10);
            output_string_function(string_buffer);
        }
		MMC_SPI_SEL_HIGH();
        return MMC_ERROR;
    }

    MMC_SPI_SEL_HIGH();
    
    return MMC_SUCCESS;
}

/*!
 * \brief Checks if card is inserted
 * \return MMC_NO_CHIPCARD_INSIDE or MMC_CHIPCARD_INSERTED
 */
static uint8_t mmc_check_card(void)
{
    /* mmc_cd = 1 */
    if(bit_is_clear(PINC, 7) != 0)
        return MMC_CHIPCARD_INSIDE;
    else
        return MMC_NO_CHIPCARD_INSIDE;
}

/*!
 * \brief Read one block of data from the sd card
 * \param address Block address (is mapped to byte address)
 * \param *mmc_buf MMC_buffer where data is written into
 */
mmc_status_t mmc_read_single_block(uint32_t address, mmc_block_t *mmc_buf)
{
	uint8_t command[COMMAND_LEN];

    uint8_t resp = 0xFF;
    uint8_t i = 0;
    uint32_t timeout;

	/* fucking, shit, driver */
	for(i = 0; i < 4; i++)
    {
        sw_spi_send_data(0xFF);
    }

	MMC_SPI_SEL_LOW();

    if(mmc_check_card() == MMC_NO_CHIPCARD_INSIDE)
    {
		if (output_string_function != NULL)
		{
        	debug(PSTR("\n\rMMC ERROR: NO SD card"));
		}
		MMC_SPI_SEL_HIGH();
        return MMC_NO_CARD;
    }

	address <<= 5;

    /* CMD17 */
    command[0] = 0x51;
    command[1] = (address >> 24) & 0xFF;
    command[2] = (address >> 16) & 0xFF;
    command[3] = (address >> 8) & 0xFF;
    command[4] = (address >> 0) & 0xFF;
    command[5] = 0xFF;

    mmc_send_command(command);
    timeout = 0;
    while((resp = sw_spi_receive_data()) == 255) {
        CHECK_TIMEOUT(timeout, READ_TIMEOUT, "\n\rMMC_DRIVER : Start read timeout");
    }
   
    /* wait for start byte */
    timeout = 0;
    while(((resp = sw_spi_receive_data()) != 254)) {
    	timeout++;

		if (timeout == READ_TIMEOUT) {
       	    if (output_string_function != NULL)
	        {
                debug(PSTR("\n\rMMC_DRIVER : Start read timeout!"));
            }
		    MMC_SPI_SEL_HIGH();
            return MMC_ERROR;	
        }
    }

    /* read in data */
    for(i = 0; i < BLOCK_SIZE; i++)
    {
        mmc_buf->data[i] = sw_spi_receive_data();
    }
    /* get crc data */
    mmc_buf->crc[0] = sw_spi_receive_data();
    mmc_buf->crc[1] = sw_spi_receive_data();

    MMC_SPI_SEL_HIGH();
    
    return MMC_SUCCESS;
}

/*!
 * \brief Sends one command to sd card
 */
void mmc_send_command(const uint8_t *command)
{
	uint8_t i;
    for(i = 0; i < COMMAND_LEN; i++)
    {
        sw_spi_send_data(command[i]);
    }
}

/* EOF */
