#include "mmc.h"
#include "spi.h"
#include "lpc214x.h"
#include "utils.h"
#include <stdbool.h>


/* Common command set */
#define SD_CMD0_GO_IDLE_STATE            0x00
#define SD_CMD1_SEND_OPCOND              0x01
#define SD_CMD9_SEND_CSD                 0x09
#define SD_CMD10_SEND_CID                0x0a
#define SD_CMD12_STOP_TRANSMISSION       0x0b
#define SD_CMD13_SEND_STATUS             0x0c
#define SD_CMD16_SET_BLOCKLEN            0x10
#define SD_CMD17_READ_BLOCK              0x11
#define SD_CMD18_READ_MULTIPLE_BLOCK     0x12
#define SD_CMD24_WRITE_BLOCK             0x18
#define SD_CMD25_WRITE_MULTIPLE_BLOCK    0x19
#define SD_CMD27_PROGRAM_CSD             0x1b
#define SD_CMD28_SET_WRITE_PROT          0x1c
#define SD_CMD29_CLR_WRITE_PROT          0x1d
#define SD_CMD30_SEND_WRITE_PROT         0x1e
#define SD_CMD32_ERASE_WR_BLK_START_ADDR 0x20
#define SD_CMD33_ERASE_WR_BLK_END_ADDR   0x21
#define SD_CMD38_ERASE                   0x26
#define SD_ACMD41_INIT                   0x29
#define SD_CMD55_APP_CMD                 0x37
#define SD_CMD56_GEN_CMD                 0x38
#define SD_CMD58_READ_OCR                0x3a
#define SD_CMD59_CRC_ON_OFF              0x3b

#define SD_START_TOKEN    0xFE

/* R1 format responses (ORed together as a bit-field) */
#define SD_R1_NOERROR     0x00
#define SD_R1_IDLE        0x01
#define SD_R1_ERASE       0x02
#define SD_R1_ILLEGAL     0x04
#define SD_R1_CRC_ERR     0x08
#define SD_R1_ERASE_SEQ   0x10
#define SD_R1_ADDR_ERR    0x20
#define SD_R1_PARAM_ERR   0x40

/* R2 format responses - second byte only, first is identical to R1 */
#define SD_R2_LOCKED      0x01
#define SD_R2_WP_FAILED   0x02
#define SD_R2_ERROR       0x04
#define SD_R2_CTRL_ERR    0x08
#define SD_R2_ECC_FAIL    0x10
#define SD_R2_WP_VIOL     0x20
#define SD_R2_ERASE_PARAM 0x40
#define SD_R2_RANGE_ERR   0x80

/* Error mask for debug (used with sdGetError) */
#define SD_CARD_ERROR_INIT_1      0x1100
#define SD_CARD_ERROR_INIT_2      0x1200
#define SD_CARD_ERROR_WRITE_1     0x2100
#define SD_CARD_ERROR_WRITE_2     0x2200
#define SD_CARD_ERROR_WRITE_3     0x2300
#define SD_CARD_ERROR_READ_1      0x3100
#define SD_CARD_ERROR_READ_2      0x3200
#define SD_CARD_ERROR_REGISTER_1  0x4100
#define SD_CARD_ERROR_REGISTER_2  0x4200
#define SD_CARD_ERROR_REGISTER_3  0x4300
#define SD_CARD_ERROR_OCR_1       0x5100
#define SD_CARD_ERROR_OCR_2       0x5200


static volatile unsigned long *SPIcsSET;
static volatile unsigned long *SPIcsCLR;
static unsigned int CSmask;

static bool mmc_initialized = false;

static void DisableCS(void)
{
	*SPIcsSET = CSmask;
}

static void EnableCS(void)
{
	*SPIcsCLR = CSmask;
	DelayUS(10);
}

unsigned char Mmc_Command(unsigned char command, unsigned long param, unsigned char crc)
{
	unsigned char ret;
	unsigned int c = 0;
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.

	SPI0_send(0xFF);
	SPI0_send((command & 0x3F) | 0x40);
	SPI0_send(param >> 24);
	SPI0_send(param >> 16);
	SPI0_send(param >> 8);
	SPI0_send(param >> 0);
	SPI0_send(crc);
	ret = SPI0_sendrecieve(0xFF);
	
	while ((ret & 0x80)&&(c < 10000))
	{
		ret = SPI0_sendrecieve(0xFF);
		c++;
	}
	
	return ret;
}

unsigned char Mmc_Send_Command(unsigned char command, unsigned long param, unsigned char crc)
{
	unsigned char ret;
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.
	
	EnableCS();
	
	ret = Mmc_Command(command, param, crc);

	SPI_char(0xFF);
	
	DisableCS();

	SPI_char(0xFF);
	
	return ret;
}

unsigned char Mmc_Init(volatile unsigned long *port, unsigned char cspin)
{
	unsigned char tmp;
	int c;
	
	mmc_initialized = false;
	
	if (((*port != IO0PIN)&&(*port != IO1PIN))||(cspin > 31))
	{
		return false;
	}
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.

	SPIcsSET = port + 0x01;
	SPIcsCLR = port + 0x03;	
	CSmask = 1 << cspin;

	DisableCS();	

	// Clock out dummy data.
	for(c = 0; c < 10; c++)
	{
		SPI0_send(0xFF);
	}
	
	if (Mmc_Send_Command(SD_CMD0_GO_IDLE_STATE, 0, 0x95) != 0x01)
	{
		return 0;
	}
	
	do // TODO: Limit number of retries.
	{
		tmp = Mmc_Send_Command(SD_CMD1_SEND_OPCOND, 0, 0xFF);
	} while (tmp != 0);
	
	mmc_initialized = true;
	
	return 1;
}

unsigned char Mmc_Read_Sector(unsigned long sector, unsigned char *dbuff)
{
	unsigned char tmp;
	int c;
		
	if(mmc_initialized == false)
	{
		return 1;
	}
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.

	S0SPCCR = 0x08; // Set SPI clock to 7.5MHz.
	
	EnableCS();
	tmp = Mmc_Command(SD_CMD17_READ_BLOCK, sector * 512, 0xFF);
	if(tmp != 0)
	{
		SPI_char(0xFF);
		DisableCS();
		SPI_char(0xFF);
		
		return 1;
	}
	
	do // TODO: Limit number of retries.
	{
		tmp = SPI_char(0xFF);
	} while(tmp != 0xFE);
	
	for(c = 0; c < 512; c++)
	{
		dbuff[c] = SPI_char(0xFF);
	}
	
	SPI_char(0xFF);
	SPI_char(0xFF);
	SPI_char(0xFF);
	
	DisableCS();
	
	SPI_char(0xFF);

	return 0;
}

unsigned char Mmc_Write_Sector(unsigned long sector, unsigned char *dbuff)
{
	unsigned char tmp;
	int c;

	if(mmc_initialized == false)
	{
		return 1;
	}
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.

	S0SPCCR = 0x08; // Set SPI clock to 7.5MHz.
	
	EnableCS();
	tmp = Mmc_Command(SD_CMD24_WRITE_BLOCK, sector * 512, 0xFF);
	if(tmp != 0)
	{
		SPI_char(0xFF);
		DisableCS();
		SPI_char(0xFF);
		
		return 1;
	}
	
	SPI_char(0xFF);
	SPI_char(0xFF);
	SPI_char(0xFE);

	for(c = 0; c < 512; c++)
	{
		SPI_char(dbuff[c]);
	}

	SPI_char(0xFF);
	SPI_char(0xFF);
	tmp = SPI_char(0xFF);
	if ((tmp & 0x1F) == 0x05)
	{
		SPI_char(0xFF);
		DisableCS();
		SPI_char(0xFF);

		return 2;
	}
	
	do // TODO: Limit number of retries.
	{
		tmp = SPI_char(0xFF);
	} while(tmp != 0xFF);

	SPI_char(0xFF);
	DisableCS();
	SPI_char(0xFF);
	
	return 0;
}

unsigned char Mmc_Read_Cid(unsigned char *data_cid)
{
	unsigned char tmp;
	int c;
	
	if(mmc_initialized == false)
	{
		return false;
	}
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.

	EnableCS();
	
	tmp = Mmc_Command(SD_CMD10_SEND_CID,0, 0xFF);
	if(tmp != 0)
	{
		SPI_char(0xFF);
		DisableCS();
		SPI_char(0xFF);
		return false;
	}
	
	do // TODO: Limit number of retries.
	{
		tmp = SPI_char(0xFF);
	} while(tmp != 0xFE);
	
	for(c = 0; c < 0x10; c++)
	{
		data_cid[c] = SPI_char(0xFF);
	}
	
	SPI_char(0xFF);
	
	DisableCS();
	
	SPI_char(0xFF);
	
	return true;
}

unsigned char Mmc_Read_Csd(unsigned char *data_csd)
{
	unsigned char tmp;
	int c;
	
	if(mmc_initialized == false)
	{
		return false;
	}
	
	// play nice with mp3 ic.
	IODIR0|= 0x00400000; // Set P0.22 as output
	IOSET0 = 0x00400000; // Disable VS1053 XDCS pin.
	
	EnableCS();
	
	tmp = Mmc_Command(SD_CMD9_SEND_CSD,0, 0xFF);
	if(tmp != 0)
	{
		SPI_char(0xFF);
		DisableCS();
		SPI_char(0xFF);
		return false;
	}
	
	do // TODO: Limit number of retries.
	{
		tmp = SPI_char(0xFF);
	} while(tmp != 0xFE);
	
	for(c = 0; c < 0x10; c++)
	{
		data_csd[c] = SPI_char(0xFF);
	}
	
	SPI_char(0xFF);
	
	DisableCS();
	
	SPI_char(0xFF);
	
	return true;
}

int media_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
{
    unsigned long i;

    for (i=0;i<sector_count;i++)
    {
        if (Mmc_Read_Sector(sector, buffer) != 0)
				{
					return 0;
				}

        sector ++;
        buffer += 512;
    }

    return 1;
}

int media_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
{
    unsigned long i;

    for (i=0;i<sector_count;i++)
    {
        if (Mmc_Write_Sector(sector, buffer) != 0)
				{
					return 0;
				}

        sector ++;
        buffer += 512;
    }

    return 1;
}
