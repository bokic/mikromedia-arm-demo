#include <lpc214x.h>

#include "i2c.h"
#include "accel.h"


// ADXL345 Register Definition
#define _POWER_CTL      0x2D
#define _DATA_FORMAT    0x31
#define _BW_RATE        0x2C
#define _DATAX0         0x32
#define _DATAX1         0x33
#define _DATAY0         0x34
#define _DATAY1         0x35
#define _DATAZ0         0x36
#define _DATAZ1         0x37
#define _FIFO_CTL       0x38
#define _SPEED          0x0F                       // Buffer Speed - 3200Hz
#define _SPEED_800      0x0D                       // Buffer Speed - 800Hz
#define _SPI_READ       0x80
#define _SPI_WRITE      0x00
#define _ACCEL_ERROR    0x02

static void ADXL345_Write(unsigned char address, unsigned char data1)
{
  unsigned char data[2];
  data[0] = address;
  data[1] = data1;

  I2C_Send(0x3A, data, 2);
}

static unsigned char ADXL345_Read(unsigned char address)
{
  unsigned char data[1];
  unsigned char tmp = 0;

  data[0] = address;
  I2C_Send(0x3A, data, 1);

  I2C_Read(0x3B, data, 1);
  tmp =	data[0];

  return tmp;
}

bool Accel_Init(void)
{
  unsigned char id;

  // Go into standby mode to configure the device.
  ADXL345_Write(0x2D, 0x00);

  id = ADXL345_Read(0x00);
  if (id != 0xE5) {
    return false;
  }

  ADXL345_Write(_POWER_CTL, 0x00);         // Go into standby mode to configure the device
  ADXL345_Write(_DATA_FORMAT, 0x08);       // Full resolution, +/-2g, 4mg/LSB, right justified
  ADXL345_Write(_BW_RATE, 0x0A);           // Set 100 Hz data rate
  ADXL345_Write(_FIFO_CTL, 0x80);          // stream mode
  ADXL345_Write(_POWER_CTL, 0x08);         // POWER_CTL reg: measurement mode*/

  return 0x00;
}

// Read X Axis
short Accel_ReadX(void)
{
  unsigned short high_byte, low_byte, X_tmp;
  short X;

  high_byte = ADXL345_Read(_DATAX1);
  low_byte  = ADXL345_Read(_DATAX0);

  X_tmp = (high_byte & 0x03) << 8;
  X_tmp = X_tmp | low_byte;

  X = X_tmp;
  if (X_tmp > 511) {
           X_tmp = 1024 - X_tmp;
		   X = (signed short)(0 - X_tmp);
		  }

  return X;
}

// Read Y Axis
short Accel_ReadY(void)
{
  unsigned short high_byte, low_byte;
  short Y;

  high_byte = ADXL345_Read(_DATAY1);
  low_byte  = ADXL345_Read(_DATAY0);

  Y = (high_byte & 0x03) << 8;
  Y = Y | low_byte;
  if (Y > 511) Y = Y - 1024;

  return Y;
}

// Read Z Axis
short Accel_ReadZ(void)
{
  unsigned short high_byte, low_byte;
  short Z;

  high_byte = ADXL345_Read(_DATAZ1);
  low_byte  = ADXL345_Read(_DATAZ0);

  Z = (high_byte & 0x03) << 8;
  Z = Z | low_byte;
  if (Z > 511) Z = Z - 1024;

  return Z;
}
