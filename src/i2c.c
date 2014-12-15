#include "lpc214x.h"
#include "i2c.h"
#include <stdbool.h>
#include <stdint.h>


void I2C_Init(void)
{
	PINSEL0 = (PINSEL0 & 0xFFFFFF0F) | 0x00000050; // Enable I2C pins

	I2C0CONCLR = 0x6C; // Clear Status
	I2C0CONSET = 0x40; // Clear Status

	// Set I2C speed to 400KHz
	I2C0SCLH = 0x4B;
	I2C0SCLL = 0x4B;
}

bool I2C_Read(char address, unsigned char *data, uint8_t num)
{
	uint8_t c = 0;
	volatile uint8_t dummy;

	// I2C interrupt Clear bit.
	I2C0CONCLR = 0x08;

	// Start i2c
	I2C0CONSET |= 0x20;

	while((I2C0CONCLR & 0x08) == 0);
	while(I2C0STAT != 0x08);
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	I2C0CONCLR = 0x20;
	I2C0DAT = address | 0x01;

	while((I2C0CONCLR & 0x08) == 0);
	while(I2C0STAT != 0x40);
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	for(c = 0; c < num; c++)
	{
		I2C0CONSET = 0x04;

		while((I2C0CONCLR & 0x08) == 0);
		while(I2C0STAT != 0x50);
		I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

		data[c] = I2C0DAT;
	}

	I2C0CONCLR = 0x04;

	while((I2C0CONCLR & 0x08) == 0);
	while(I2C0STAT != 0x58);

	dummy = I2C0DAT;
	I2C0CONSET = 0x10;

	return true;
}

bool I2C_Send(char address, unsigned char *data, uint8_t num)
{
	uint8_t c = 0;

	// I2C interrupt Clear bit.
	I2C0CONCLR = 0x08;

	// Start i2c
	I2C0CONSET |= 0x20;

	while((I2C0CONCLR & 0x08) == 0);
	while(I2C0STAT != 0x08);
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	I2C0CONCLR = 0x20;
	I2C0DAT = address & 0xFE;

	while((I2C0CONCLR & 0x08) == 0);
	while(I2C0STAT != 0x18);
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	for(c = 0; c < num; c++)
	{
		I2C0DAT = data[c];

		while((I2C0CONCLR & 0x08) == 0);
		while(I2C0STAT != 0x28);
		I2C0CONCLR = 0x08; // I2C interrupt Clear bit.
	}

	I2C0CONSET = 0x10;

	return true;
}
