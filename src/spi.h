#ifndef __SPIHW_H
#define __SPIHW_H

#include <stdbool.h>

#define SPI_char(tx) SPI0_sendrecieve(tx)

extern void SPI0_init(unsigned char pclkdiv, bool MASTER, bool LSB_First, bool CPOL, bool CPHA);
extern void SPI0_send(unsigned char tx);
extern unsigned char SPI0_recieve(void);
extern unsigned char SPI0_sendrecieve(unsigned char tx);

#endif
