#include <lpc214x.h>
#include <accel.h>
#include <i2c.h>
#include <mmc.h>
#include <stdbool.h>
#include <string.h>
#include <utils.h>
#include "spi.h"
#include "mp3.h"
#include <fs.h>

#include <display.h>
#include <touchscreen.h>



void Initialize(void);
void feed(void);

void IRQ_Routine (void)   __attribute__ ((interrupt("IRQ")));
void FIQ_Routine (void)   __attribute__ ((interrupt("FIQ")));
void SWI_Routine (void)   __attribute__ ((interrupt("SWI")));
void UNDEF_Routine (void) __attribute__ ((interrupt("UNDEF")));

/**********************************************************
                       MAIN
**********************************************************/
int main (void)
{
    //unsigned char tmp[512];

    setMCUSpeed60MHz();
    //setMCUSpeed12MHz();

    DelayMS(100);

    PinSetup();
    PeripherialPinSetup();

    TFT_Init();
    TFT_Fill_Screen(0);

    TFT_Set_Pen(0xFFFF, 1);
    TFT_Write_Text("Initializing SPI.", 0, 0);
    SPI0_init(0xfe, true, false, false, false);
    TFT_Fill_Screen(0);

    // Common SPI end.
    //touch_init();
    //touch_calibrate();

    TFT_Set_Pen(0xFFFF, 1);
    TFT_Write_Text("Initializing SDCard.", 0, 0);
    //memset(tmp, 0, sizeof(tmp));

    if (Mmc_Init(&IO1PIN, 24) == false)
    {
    	TFT_Fill_Screen(0);
    	TFT_Set_Pen(0xFFFF, 1);
    	TFT_Write_Text("SDCard Initialization FAILED!", 0, 0);

    	return 0;
    }

    /*Mmc_Read_Sector(0, tmp);
    if ((tmp[510] != 85)||(tmp[511] != 170))
    {
    	TFT_Fill_Screen(0);
    	TFT_Set_Pen(0xFFFF, 1);
    	TFT_Write_Text("SDCard is not formated correctly!", 0, 0);

    	return 0;
    }*/


    /*fs_init();
    MP3_Init();
    MP3_play("/mp3/everlast.mp3");

    TFT_Fill_Screen(0);
	for(;;)
	{
		TFT_Image(0, 0, "/img/img1.img");
		DelayMS(3000);

		TFT_Image(0, 0, "/img/img2.img");
		DelayMS(3000);

		TFT_Image(0, 0, "/img/img3.img");
		DelayMS(3000);

		TFT_Image(0, 0, "/img/img4.img");
		DelayMS(3000);

		TFT_Image(0, 0, "/img/img5.img");
		DelayMS(3000);

		TFT_Image(0, 0, "/img/img6.img");
		DelayMS(3000);

		TFT_Image(0, 0, "/img/img7.img");
		DelayMS(3000);
	}*/

    /*TFT_Set_Pen(0xFFFF, 1);
    TFT_Circle(100, 100, 80);*/

    /*while(1) {
        unsigned int x, y;
        while (touch_get(&x, &y) == false);

        TFT_Set_Pen(0x00FF, 1);
        TFT_Rectangle(30, 30, 150, 150);
        TFT_Set_Pen(0xFF00, 1);
        TFT_Rectangle(30, 30, 150, 150);
    }*/

    I2C_Init();
    Accel_Init();


    short x, y, z;
    int tmp1 = 0;

    while(1)
    {
        x = Accel_ReadX();
        y = Accel_ReadY();
        z = Accel_ReadZ();

        tmp1++;
    }

    while(1);

    return 0;
}


/*  Stubs for various interrupts (may be replaced later)  */
/*  ----------------------------------------------------  */

void IRQ_Routine (void)
{
	while (1);
}

void FIQ_Routine (void)
{
	while (1);
}


void SWI_Routine (void)
{
	while (1);
}


void UNDEF_Routine (void)
{
	while (1);
}









