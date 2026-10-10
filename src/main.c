#include <lpc214x.h>
#include <accel.h>
#include <i2c.h>
#include <mmc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include "spi.h"
#include "mp3.h"
#include <fs.h>
#include <display.h>
#include <touchscreen.h>

/* Button / UI Region bounds */
#define BTN_CAL_X1   10
#define BTN_CAL_Y1   195
#define BTN_CAL_X2   100
#define BTN_CAL_Y2   230

#define BTN_CLR_X1   115
#define BTN_CLR_Y1   195
#define BTN_CLR_X2   205
#define BTN_CLR_Y2   230

#define BTN_SND_X1   220
#define BTN_SND_Y1   195
#define BTN_SND_X2   310
#define BTN_SND_Y2   230

/* Draw static UI layout */
static void draw_dashboard(bool sd_ok, bool accel_ok)
{
    TFT_Fill_Screen(TFT_COLOR_BLACK);

    /* Header Bar */
    TFT_Set_Pen(TFT_COLOR_BLUE, 1);
    TFT_Rectangle(0, 0, 319, 24);
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_Write_Text("mikromedia ARM (LPC2148) Demo", 20, 8);

    /* Hardware Status Line */
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_Write_Text("HW:", 10, 32);

    if (sd_ok) {
        TFT_Set_Pen(TFT_COLOR_GREEN, 1);
        TFT_Write_Text("SD:OK", 42, 32);
    } else {
        TFT_Set_Pen(TFT_COLOR_RED, 1);
        TFT_Write_Text("SD:NO", 42, 32);
    }

    if (accel_ok) {
        TFT_Set_Pen(TFT_COLOR_GREEN, 1);
        TFT_Write_Text("ACCEL:OK", 95, 32);
    } else {
        TFT_Set_Pen(TFT_COLOR_RED, 1);
        TFT_Write_Text("ACCEL:NO", 95, 32);
    }

    TFT_Set_Pen(TFT_COLOR_GREEN, 1);
    TFT_Write_Text("MP3:VS1053", 175, 32);

    /* Divider */
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_H_Line(0, 319, 46);

    /* Accelerometer Area */
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_Write_Text("Accelerometer ADXL345 (I2C):", 10, 52);

    /* Touch Canvas Area Header */
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_H_Line(0, 319, 98);
    TFT_Write_Text("Touch Canvas (Draw Here):", 10, 104);

    /* Touch drawing box */
    TFT_Set_Pen(0x39E7, 1); /* Dark grey border */
    TFT_Rectangle(10, 118, 310, 185);

    /* Buttons */
    /* Button 1: Calibrate */
    TFT_Set_Pen(0x001F, 1); /* Blue fill */
    TFT_Rectangle(BTN_CAL_X1, BTN_CAL_Y1, BTN_CAL_X2, BTN_CAL_Y2);
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_Write_Text("Calibrate", BTN_CAL_X1 + 10, BTN_CAL_Y1 + 10);

    /* Button 2: Clear Canvas */
    TFT_Set_Pen(0x4208, 1); /* Grey fill */
    TFT_Rectangle(BTN_CLR_X1, BTN_CLR_Y1, BTN_CLR_X2, BTN_CLR_Y2);
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_Write_Text("Clear", BTN_CLR_X1 + 25, BTN_CLR_Y1 + 10);

    /* Button 3: Sound Test */
    TFT_Set_Pen(0x03E0, 1); /* Green fill */
    TFT_Rectangle(BTN_SND_X1, BTN_SND_Y1, BTN_SND_X2, BTN_SND_Y2);
    TFT_Set_Pen(TFT_COLOR_BLACK, 1);
    TFT_Write_Text("Beep", BTN_SND_X1 + 25, BTN_SND_Y1 + 10);
}

/* Play a short tone via VS1053 sine test */
static void play_demo_tone(void)
{
    /* Enable Sine Test on VS1053 */
    MP3_Init();
    MP3_SCI_Write(0x00, 0x0820); /* Set SM_RESET | SM_TESTS */
    DelayMS(10);

    /* VS1053 Sine test command sequence: 0x53 0xEF 0x6E n 0 0 0 0 */
    char sine_cmd[8] = { 0x53, 0xEF, 0x6E, 0x7E, 0x00, 0x00, 0x00, 0x00 };
    for (int i = 0; i < 8; i++) {
        MP3_SDI_Write(sine_cmd[i]);
    }

    DelayMS(300); /* Tone duration */

    /* Stop Sine test sequence: 0x45 0x78 0x69 0x74 0 0 0 0 */
    char exit_cmd[8] = { 0x45, 0x78, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00 };
    for (int i = 0; i < 8; i++) {
        MP3_SDI_Write(exit_cmd[i]);
    }

    DelayMS(20);
    MP3_SCI_Write(0x00, 0x0800); /* Normal mode */
}

/**********************************************************
                       MAIN
**********************************************************/
int main (void)
{
    /* Initialize clock to 60 MHz */
    setMCUSpeed60MHz();
    DelayMS(100);

    /* Initialize general and peripheral pin routing */
    PinSetup();
    PeripherialPinSetup();

    /* Initialize TFT display */
    TFT_Init();
    TFT_Fill_Screen(TFT_COLOR_BLACK);
    TFT_Set_Pen(TFT_COLOR_WHITE, 1);
    TFT_Write_Text("Booting mikromedia ARM...", 20, 100);

    /* Initialize SPI0 bus (for SD card and MP3) */
    SPI0_init(0xfe, true, false, false, false);

    /* Initialize touchscreen */
    touch_init();

    /* Initialize SD Card & Filesystem */
    bool sd_ok = Mmc_Init(&IO1PIN, 24);
    if (sd_ok) {
        fs_init();
    }

    /* Initialize I2C and Accelerometer */
    I2C_Init();
    bool accel_ok = Accel_Init();

    /* Initialize MP3 Codec */
    MP3_Init();

    /* Draw UI Dashboard */
    draw_dashboard(sd_ok, accel_ok);

    short prev_x = 0, prev_y = 0, prev_z = 0;
    unsigned int accel_refresh = 0;
    char text_buf[32];

    while(1)
    {
        /* 1. Accelerometer reading and display update (~every 100 iterations) */
        if (accel_ok && (++accel_refresh >= 100))
        {
            accel_refresh = 0;
            short ax = Accel_ReadX();
            short ay = Accel_ReadY();
            short az = Accel_ReadZ();

            if (ax != prev_x || ay != prev_y || az != prev_z)
            {
                prev_x = ax;
                prev_y = ay;
                prev_z = az;

                /* Overwrite previous text region with black */
                TFT_Set_Pen(TFT_COLOR_BLACK, 1);
                TFT_Rectangle(10, 68, 310, 85);

                /* Write current accelerometer coordinates */
                snprintf(text_buf, sizeof(text_buf), "X:%-5d Y:%-5d Z:%-5d", ax, ay, az);
                TFT_Set_Pen(TFT_COLOR_GREEN, 1);
                TFT_Write_Text(text_buf, 10, 70);

                /* Small visual spirit-level bubble */
                int bubble_x = 160 + (ax / 8);
                int bubble_y = 75 - (ay / 8);
                if (bubble_x >= 230 && bubble_x <= 300 && bubble_y >= 55 && bubble_y <= 90) {
                    TFT_Set_Pen(TFT_COLOR_RED, 1);
                    TFT_Circle((short)bubble_x, (short)bubble_y, 4);
                }
            }
        }

        /* 2. Touch screen handling */
        unsigned short tx = 0, ty = 0;
        if (touch_get(&tx, &ty))
        {
            /* Touch within drawing canvas */
            if (tx >= 12 && tx <= 308 && ty >= 120 && ty <= 183)
            {
                TFT_Set_Pen(TFT_COLOR_WHITE, 1);
                TFT_Circle((short)tx, (short)ty, 2);
            }
            /* Button 1: Calibrate */
            else if (tx >= BTN_CAL_X1 && tx <= BTN_CAL_X2 && ty >= BTN_CAL_Y1 && ty <= BTN_CAL_Y2)
            {
                touch_calibrate();
                draw_dashboard(sd_ok, accel_ok);
            }
            /* Button 2: Clear Canvas */
            else if (tx >= BTN_CLR_X1 && tx <= BTN_CLR_X2 && ty >= BTN_CLR_Y1 && ty <= BTN_CLR_Y2)
            {
                TFT_Set_Pen(TFT_COLOR_BLACK, 1);
                TFT_Rectangle(11, 119, 309, 184);
            }
            /* Button 3: Sound Test */
            else if (tx >= BTN_SND_X1 && tx <= BTN_SND_X2 && ty >= BTN_SND_Y1 && ty <= BTN_SND_Y2)
            {
                /* Audio feedback */
                TFT_Set_Pen(TFT_COLOR_RED, 1);
                TFT_Write_Text("BEEPING...", BTN_SND_X1 - 70, BTN_SND_Y1 - 12);
                play_demo_tone();
                TFT_Set_Pen(TFT_COLOR_BLACK, 1);
                TFT_Rectangle(BTN_SND_X1 - 70, BTN_SND_Y1 - 12, BTN_SND_X1 + 60, BTN_SND_Y1 - 2);
            }
        }

        DelayMS(5);
    }

    return 0;
}

/* Stubs for various interrupts */
void IRQ_Routine (void)   { while (1); }
void FIQ_Routine (void)   { while (1); }
void SWI_Routine (void)   { while (1); }
void UNDEF_Routine (void) { while (1); }
