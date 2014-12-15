#ifndef __TOUCHSCREEN_H__
#define __TOUCHSCREEN_H__

#include <stdbool.h>

extern void touch_init(void);
extern bool touch_get(unsigned short *x, unsigned short *y);
extern void touch_calibrate(void);

#endif // __TOUCHSCREEN_H__
