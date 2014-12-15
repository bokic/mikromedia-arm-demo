#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>

/* SPI select pin */ 
#define SPI_SEL      (1 << 11) 

void SPI_Init( void ); 
void SPI_Send( uint8_t *Buf, uint32_t Length ); 
void SSP_SendRecvByte( uint8_t *Buf, uint32_t Length ); 
uint8_t SSP_SendRecvByteByte( void ); 

#endif //_SPI_H_
