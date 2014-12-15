#ifndef __UTILS_H__
#define __UTILS_H__

extern void DelayMS(unsigned int value);
extern void DelayUS(unsigned int value);
extern void setMCUSpeed12MHz(void);
extern void setMCUSpeed60MHz(void);
extern void PinSetup(void);
extern void PeripherialPinSetup(void);

#endif // __UTILS_H__
