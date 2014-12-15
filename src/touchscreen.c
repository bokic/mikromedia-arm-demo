#include "lpc214x.h"
#include "touchscreen.h"
#include "display.h"
#include "utils.h"


#define XL (1 << 25)
#define YU (1 << 28)
#define XR (1 << 29)
#define YD (1 << 30)

static unsigned short minx = 200;
static unsigned short miny = 200;
static unsigned short maxx = 800;
static unsigned short maxy = 800;


void touch_init(void)
{
	// P0.25(AD0.4) - XL(X Left)
	// P0.28(AD0.1) - YU(Y Up)
	// P0.29(AD0.2) - XR(X Right)
	// P0.30(AD0.3) - YD(Y Down)
	
	IO0DIR |= XL|YU|XR|YD;
	IO0CLR  = XL|YU|XR|YD;
	
	// Disble AD0 interrupts
	AD0INTEN = 0;
}

void touch_getraw(unsigned short *x, unsigned short *y)
{	
	// Get X
	IO0CLR = YU; // clears YU
	IO0SET = YD; // sets YD
	IO0DIR &= ~(XL|XR); // Set XL and XR pin as input(high impedance)
	PINSEL1 = (PINSEL1 & 0xFFF3FFFF) | 0x00040000; // Set P0.25 as (AD0.4) 
	DelayUS(400);
	AD0CR = 0x01200D10; // Starts ADC conversion.
	while((AD0GDR & 0x80000000) == 0); // Wait til conversion finishes.
	*x = (AD0DR4 >> 6) & 0x3FF;
	PINSEL1 = (PINSEL1 & 0xFFF3FFFF); // Set P0.25 as GPIO
	IO0CLR  = XL|YU|XR|YD;
	IO0DIR |= XL|YU|XR|YD; // Set YU and YD pin as output
	
	// Get Y
	IO0CLR = XR; // clears XR
	IO0SET = XL; // sets XL
	IO0DIR &= ~(YD|YU); // Set YD and YU pin as input(high impedance)
	PINSEL1 = (PINSEL1 & 0xCFFFFFFF) | 0x10000000; // // Set P0.30 as (AD0.3)  
	DelayUS(400);
	AD0CR = 0x01200D08; // Starts ADC conversion.
	while((AD0GDR & 0x80000000) == 0); // Wait til conversion finishes.
	*y = (AD0DR3 >> 6) & 0x3FF;	
	PINSEL1 = (PINSEL1 & 0xCFFFFFFF); // Set P0.30 as GPIO
	IO0CLR  = XL|YU|XR|YD;
	IO0DIR |= XL|YU|XR|YD; // Set YU and YD pin as output
}

bool touch_get(unsigned short *x, unsigned short *y)
{
	unsigned short tmpx, tmpy;
	
	touch_getraw(&tmpx, &tmpy);
	
	if ((tmpx < (minx / 2))||(tmpy < (miny / 2)))
	{
		return false;
	}
	
	if (tmpx < minx)
		minx = tmpx;
	else if (tmpx > maxx)
		maxx = tmpx;
	
	if (tmpy < miny)
		miny = tmpy;
	else if (tmpy > maxy)
		maxy = tmpy;

	tmpx -= minx;
	tmpy -= miny;
	
	if ((maxx == 0)||(maxy == 0))
	{
		*x = 0;	
		*y = 0;
	}
	else
	{
		*x = (tmpx * 320) / (maxx - minx);	
		*y = (tmpy * 240) / (maxy - minx);
		
		if (*x >= 320) *x = 319;
		if (*y >= 240) *y = 239;
	}
	
	return true;
}

void touch_calibrate(void)
{
	unsigned short tmpx, tmpy, no_touchx, no_touchy;
	int c;
	
	minx = 0xFFFF;
	miny = 0xFFFF;
	no_touchx = 0xFFFF;
	no_touchy = 0xFFFF;
	
	maxx = 0;
	maxy = 0;

	TFT_Fill_Screen(0x0000);
	TFT_Set_Pen(TFT_COLOR_RED, 1);
	TFT_Write_Text("Do not touch the screen!", 50, 100);
	
	for(c = 0; c < 500; c++)
	{
		touch_getraw(&tmpx, &tmpy);
		
		if (tmpx < no_touchx)
			no_touchx = tmpx;		
		if (tmpy < no_touchy)
			no_touchy = tmpy;		
		
		DelayMS(10);
	}
	
	no_touchx *= 2;
	no_touchy *= 2;	
	
	TFT_Fill_Screen(0x0000);
	DelayMS(2000);
	
	TFT_Set_Pen(TFT_COLOR_WHITE, 1);
	TFT_Write_Text("Press and hold.", 100, 100);
	TFT_Line(0, 0, 0, 20);
	TFT_Line(0, 0, 20, 0);
	TFT_Line(0, 0, 40, 40);
	c = 0;
	do
	{
		touch_getraw(&tmpx, &tmpy);
		
		if((tmpx < no_touchx)||(tmpy < no_touchy))
			continue;		
		
		if(tmpx < minx)
			minx = tmpx;
		if(tmpy < miny)
			miny = tmpy;
		c++;
		DelayMS(10);
	} while (c < 200);
	
	TFT_Set_Pen(TFT_COLOR_BLACK, 1);
	TFT_Rectangle(0, 0, 40, 40);
	
	TFT_Set_Pen(TFT_COLOR_WHITE, 1);
	TFT_Line(319, 239, 319, 219);
	TFT_Line(319, 239, 299, 239);
	TFT_Line(319, 239, 279, 199);
	c = 0;
	do
	{
		touch_getraw(&tmpx, &tmpy);
		
		if((tmpx < no_touchx)||(tmpy < no_touchy))
			continue;		

		if(tmpx > maxx)
			maxx = tmpx;
		if(tmpy > maxy)
			maxy = tmpy;
		c++;
		DelayMS(10);
	} while (c < 200);

    TFT_Fill_Screen(0);
}
