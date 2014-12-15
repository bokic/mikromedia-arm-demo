#include "spi.h"
#include "lpc214x.h"

#include <stdbool.h>


void SPI0_init(unsigned char pclkdiv, bool MASTER, bool LSB_First, bool CPOL, bool CPHA)
{
	char spicr = 0x00;
	
	if (MASTER) spicr |= 0x20;
	if (LSB_First) spicr |= 0x40;	
	if (CPOL) spicr |= 0x10;
	if (CPHA) spicr |= 0x08;
	
	PINSEL0 = (PINSEL0 & 0xFFFFC0FF) | 0x00001500;
	
	S0SPCR = spicr;
	S0SPCCR = pclkdiv;	
}

void SPI0_send(unsigned char tx)
{
	volatile unsigned char tmp;
	
	S0SPDR = tx;
	
	while((S0SPSR & 0x80) == 0);
	
	tmp = S0SPDR;	
}

unsigned char SPI0_recieve(void)
{
	S0SPDR = 0;
	
	while((S0SPSR & 0x80) == 0);
	
	return S0SPDR;	
}

unsigned char SPI0_sendrecieve(unsigned char tx)
{
	S0SPDR = tx;
	
	while((S0SPSR & 0x80) == 0);
	
	return S0SPDR;	
}

