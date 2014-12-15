#include "lpc214x.h"
#include "utils.h"

static int mhz = 12;

void DelayMS(unsigned int value)
{
    DelayUS(value * 1000);
}

void DelayUS(unsigned int value)
{
	unsigned int cycles;
	
	T0TCR = 0x00000002; // Reset timer0 value
	T0TCR = 0x00000001; // Start timer0

    cycles = mhz * value / 4; // 4 because VPB bus clock is one fourth of the processor clock.
	
	while(T0TC < cycles);
	
	T0TCR = 0x00000002; // Reset timer0 value
}

void setMCUSpeed12MHz(void)
{
    PLL0CON  = 0x00000001; // Set PLLE

    PLL0CFG  = 0x20; // MSEL=1;PSEL=4
    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;

    while((PLL0STAT & 0x00000400) == 0); // wait until PLL0CON.PLOCK

    PLL0CON |= 0x00000003; // Set PLLE and PLLC

    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;

    VPBDIV   = 0x00; // VPB bus clock is one fourth of the processor clock.

    mhz = 12;
}

void setMCUSpeed60MHz(void)
{
    PLL0CON  = 0x00000001; // Set PLLE

    PLL0CFG  = 0x24; // MSEL=5;PSEL=4
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

    while((PLL0STAT & 0x00000400) == 0); // wait until PLL0CON.PLOCK

    PLL0CON |= 0x00000003; // Set PLLE and PLLC

    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;

    VPBDIV   = 0x00; // VPB bus clock is one fourth of the processor clock.

    mhz = 60;
}

void PinSetup(void)
{
    // Set all pins as input.
    IO0DIR = 0x00000000;
    IO1DIR = 0x00000000;

    PINSEL2 &= 0xFFFFFFF7; // Set P1.25-16 as GPIO(disable TRACE)

    // Common SPI start.
    IO1DIR |= 0x01000000;  // Set SDCard CS pin(P1.24) as output.
    IO1SET = 0x01000000;   // Set SDCard CS pin(P1.24).
    IO1CLR = 0x01000000;   // Set SDCard CS pin(P1.24).

    //PINSEL1 &= 0xFFFFCFFF; // Set MP3 XDCS pin(P0.22) as GPIO
    IO0DIR |= 0x00400000;  // Set MP3 XDCS pin(P0.22) as output.
    IO0SET = 0x00400000;   // Set MP3 XDCS pin(P0.22).

    IO1DIR |= 0x02000000;  // Set MP3 XCS pin(P1.25) as output.
    IO1SET = 0x02000000;   // Set MP3 XCS pin(P1.25).
}

void PeripherialPinSetup(void)
{
    // Set P0.2 as I2C SCL0
    PINSEL0 &= 0xFFFFFFCF;
    PINSEL0 |= 0x00000010;

    // Set P0.3 as I2C SDA0
    PINSEL0 &= 0xFFFFFF3F;
    PINSEL0 |= 0x00000040;
}
