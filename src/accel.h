#ifndef __ACCEL_H_
#define __ACCEL_H_

#include <stdbool.h>


bool Accel_Init(void);
short Accel_ReadX(void);
short Accel_ReadY(void);
short Accel_ReadZ(void);

#endif // __ACCEL_H_
