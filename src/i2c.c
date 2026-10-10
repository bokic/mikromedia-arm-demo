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

#define I2C_TIMEOUT 100000

bool I2C_Read(char address, unsigned char *data, uint8_t num)
{
	uint8_t c = 0;
	__attribute__((unused)) volatile uint8_t dummy;
	uint32_t timeout;

	// I2C interrupt Clear bit.
	I2C0CONCLR = 0x08;

	// Start i2c
	I2C0CONSET |= 0x20;

	timeout = I2C_TIMEOUT;
	while(((I2C0CONSET & 0x08) == 0) && --timeout);
	if (!timeout) return false;

	timeout = I2C_TIMEOUT;
	while((I2C0STAT != 0x08) && --timeout);
	if (!timeout) return false;
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	I2C0CONCLR = 0x20;
	I2C0DAT = address | 0x01;

	timeout = I2C_TIMEOUT;
	while(((I2C0CONSET & 0x08) == 0) && --timeout);
	if (!timeout) return false;

	timeout = I2C_TIMEOUT;
	while((I2C0STAT != 0x40) && --timeout);
	if (!timeout) return false;
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	for(c = 0; c < num; c++)
	{
		I2C0CONSET = 0x04;

		timeout = I2C_TIMEOUT;
		while(((I2C0CONSET & 0x08) == 0) && --timeout);
		if (!timeout) return false;

		timeout = I2C_TIMEOUT;
		while((I2C0STAT != 0x50) && --timeout);
		if (!timeout) return false;
		I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

		data[c] = I2C0DAT;
	}

	I2C0CONCLR = 0x04;

	timeout = I2C_TIMEOUT;
	while(((I2C0CONSET & 0x08) == 0) && --timeout);
	if (!timeout) return false;

	timeout = I2C_TIMEOUT;
	while((I2C0STAT != 0x58) && --timeout);
	if (!timeout) return false;

	dummy = I2C0DAT;
	I2C0CONSET = 0x10;

	return true;
}

bool I2C_Send(char address, unsigned char *data, uint8_t num)
{
	uint8_t c = 0;
	uint32_t timeout;

	// I2C interrupt Clear bit.
	I2C0CONCLR = 0x08;

	// Start i2c
	I2C0CONSET |= 0x20;

	timeout = I2C_TIMEOUT;
	while(((I2C0CONSET & 0x08) == 0) && --timeout);
	if (!timeout) return false;

	timeout = I2C_TIMEOUT;
	while((I2C0STAT != 0x08) && --timeout);
	if (!timeout) return false;
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	I2C0CONCLR = 0x20;
	I2C0DAT = address & 0xFE;

	timeout = I2C_TIMEOUT;
	while(((I2C0CONSET & 0x08) == 0) && --timeout);
	if (!timeout) return false;

	timeout = I2C_TIMEOUT;
	while((I2C0STAT != 0x18) && --timeout);
	if (!timeout) return false;
	I2C0CONCLR = 0x08; // I2C interrupt Clear bit.

	for(c = 0; c < num; c++)
	{
		I2C0DAT = data[c];

		timeout = I2C_TIMEOUT;
		while(((I2C0CONSET & 0x08) == 0) && --timeout);
		if (!timeout) return false;

		timeout = I2C_TIMEOUT;
		while((I2C0STAT != 0x28) && --timeout);
		if (!timeout) return false;
		I2C0CONCLR = 0x08; // I2C interrupt Clear bit.
	}

	I2C0CONSET = 0x10;

	return true;
}
