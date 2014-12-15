#ifndef __I2C_H__
#define __I2C_H__

#include <stdbool.h>
#include <stdint.h>


void I2C_Init(void);
bool I2C_Read(char address, unsigned char *data, uint8_t num);
bool I2C_Send(char address, unsigned char *data, uint8_t num);

#endif
