#include "lpc214x.h"
#include "display.h"
#include "utils.h"
#include "font8x8_basic.h"

#include "fat_filelib.h"

#include <stdbool.h>
#include <stdint.h>
#include <math.h>


#define DCX_RS 0x00000100 // P0.8
#define RDX    0x00000200 // P0.9
#define CS     0x00000400 // P0.10
#define RESET  0x00000800 // P0.11
#define WRX    0x00001000 // P0.12
#define POWER  0x00002000 // P0.13
#define D0     0x00008000 // P0.15
#define D1     0x00010000 // P0.16
#define D2     0x00020000 // P0.17
#define D3     0x00040000 // P0.18
#define D4     0x00080000 // P0.19
#define D5     0x00100000 // P0.20
#define D6     0x00200000 // P0.21
#define D7     0x00400000 // P0.22
#define D8     0x00010000 // P1.16
#define D9     0x00020000 // P1.17
#define D10    0x00040000 // P1.18
#define D11    0x00080000 // P1.19
#define D12    0x00100000 // P1.20
#define D13    0x00200000 // P1.21
#define D14    0x00400000 // P1.22
#define D15    0x00800000 // P1.23

#define SET_CTRLS(x) IO0SET=x
#define CLEAR_CTRLS(x) IO0CLR=x
#define SET_D0_D7(x) IO0SET=x
#define CLEAR_D0_D7(x) IO0CLR=x
#define SET_D8_D15(x) IO1SET=x
#define CLEAR_D8_D15(x) IO1CLR=x
#define READ_D0_D7(x) IO0PIN & x
#define READ_D8_D15(x) IO1PIN & x

static unsigned short PenColor = 0x0000;
static unsigned short row_data[320];
//static unsigned char PenWidth = 1;


__inline void TFT_Set8BitData(uint8_t data)
{
	SET_D0_D7(data << 15);
	data = ~data;
	CLEAR_D0_D7(data << 15);
}

__inline void TFT_Set16BitData(uint16_t data)
{
	uint8_t low, high;
	
	low = (data >> 0);
	SET_D0_D7(low << 15);
	low = ~low;
	CLEAR_D0_D7(low << 15);
	
	high = (data >> 8);
	SET_D8_D15(high << 16);
	high = ~high;
	CLEAR_D8_D15(high << 16);
}

__inline unsigned char TFT_Get8BitData()
{
	unsigned char ret = 0;
	
	// TODO: Optimize TFT_Get8BitData()
	if (READ_D0_D7(D0)) ret |= 0x01;
	if (READ_D0_D7(D1)) ret |= 0x02;
	if (READ_D0_D7(D2)) ret |= 0x04;
	if (READ_D0_D7(D3)) ret |= 0x08;
	if (READ_D0_D7(D4)) ret |= 0x10;
	if (READ_D0_D7(D5)) ret |= 0x20;
	if (READ_D0_D7(D6)) ret |= 0x40;
	if (READ_D0_D7(D7)) ret |= 0x80;
	
	return ret;
}

__inline unsigned short TFT_Get16BitData()
{
	unsigned short ret = 0;
	
	// TODO: Optimize TFT_Get16BitData()
	if (READ_D0_D7(D0)) ret |= 0x01;
	if (READ_D0_D7(D1)) ret |= 0x02;
	if (READ_D0_D7(D2)) ret |= 0x04;
	if (READ_D0_D7(D3)) ret |= 0x08;
	if (READ_D0_D7(D4)) ret |= 0x10;
	if (READ_D0_D7(D5)) ret |= 0x20;
	if (READ_D0_D7(D6)) ret |= 0x40;
	if (READ_D0_D7(D7)) ret |= 0x80;
	if (READ_D8_D15(D8)) ret |= 0x0100;
	if (READ_D8_D15(D9)) ret |= 0x0200;
	if (READ_D8_D15(D10)) ret |= 0x0400;
	if (READ_D8_D15(D11)) ret |= 0x0800;
	if (READ_D8_D15(D12)) ret |= 0x1000;
	if (READ_D8_D15(D13)) ret |= 0x2000;
	if (READ_D8_D15(D14)) ret |= 0x4000;
	if (READ_D8_D15(D15)) ret |= 0x8000;
	
	return ret;
}

__inline void TFT_Write_Strobe(void)
{
	CLEAR_CTRLS(WRX);
	SET_CTRLS(WRX);
}

__inline void TFT_Read_Strobe(void)
{
	CLEAR_CTRLS(RDX);
	SET_CTRLS(RDX);
}

void TFT_Set_Index16(char index)
{
	CLEAR_CTRLS(DCX_RS);
	
	TFT_Set8BitData(index);
	
	TFT_Write_Strobe();
}

void TFT_Write_Command16(unsigned char command)
{
	SET_CTRLS(DCX_RS);
	
	TFT_Set8BitData(command);
	
	TFT_Write_Strobe();	
}

unsigned char TFT_Read_Command16(void)
{
	SET_CTRLS(DCX_RS);
		
	TFT_Read_Strobe();	

	return 	TFT_Get8BitData();
}

void TFT_Write_Data_16(unsigned short data)
{
	SET_CTRLS(DCX_RS);

	TFT_Set16BitData(data);
	
	TFT_Write_Strobe();
}

unsigned char TFT_ReadRegister(unsigned char index)
{
	unsigned char ret;
	
	SET_CTRLS(CS|DCX_RS|RDX|WRX);
	CLEAR_CTRLS(CS);
	
	TFT_Set_Index16(index);
	
	IO0DIR=DCX_RS|RDX|CS|RESET|WRX; // Set D0-7 as input pins.
	
	ret = TFT_Read_Command16();
	
	SET_CTRLS(CS);
	
	IO0DIR=DCX_RS|RDX|CS|RESET|WRX|D0|D1|D2|D3|D4|D5|D6|D7; // Set all pins as output pins

	return ret;
}

void TFT_Set_Address(unsigned short x, unsigned short y)
{
	TFT_Set_Index16(0x02);
	TFT_Write_Command16(x  >> 8);
	
	TFT_Set_Index16(0x03);
	TFT_Write_Command16(x);

	TFT_Set_Index16(0x06);
	TFT_Write_Command16(y  >> 8);

	TFT_Set_Index16(0x07);
	TFT_Write_Command16(y);	

	TFT_Set_Index16(0x22);
}

void TFT_WriteRegister(unsigned char index, unsigned char value)
{
	SET_CTRLS(CS|DCX_RS|RDX|WRX);
	CLEAR_CTRLS(CS);
	
	TFT_Set_Index16(index);
	
	TFT_Write_Command16(value);
	
	SET_CTRLS(CS);
}

void TFT_Fill_Screen(unsigned short color)
{
	int c;
	
	SET_CTRLS(CS|DCX_RS|RDX|WRX);
	CLEAR_CTRLS(CS);

	TFT_Set_Address(0, 0);

	SET_CTRLS(DCX_RS);

	TFT_Set16BitData(color);

	for(c = 0; c < 320 * 256; c++)
	{	
		TFT_Write_Strobe();
	}
	
	SET_CTRLS(CS);	
}

void TFT_Circle(short x_center, short y_center, short radius)
{
	int x, y;
	int l;
	int r2, y2;
	int y2_new;
	int ty;

	/* cos pi/4 = 185363 / 2^18 (approx) */
	l = ((radius * 185363) >> 18) + 1;

	/* At x=0, y=radius */
	y = radius;

	r2 = y2 = y * y;
	ty = (2 * y) - 1;
	y2_new = r2 + 3;

	for (x = 0; x <= l; x++) {
		y2_new -= (2 * x) - 3;

		if ((y2 - y2_new) >= ty) {
			y2 -= ty;
			y -= 1;
			ty -= 2;
		}

		TFT_Dot(x_center + x, y_center + y, PenColor);
		TFT_Dot(x_center + x, y_center - y, PenColor);
		TFT_Dot(x_center - x, y_center + y, PenColor);
		TFT_Dot(x_center - x, y_center - y, PenColor);

		TFT_Dot(x_center + y, y_center + x, PenColor);
		TFT_Dot(x_center + y, y_center - x, PenColor);
		TFT_Dot(x_center - y, y_center + x, PenColor);
		TFT_Dot(x_center - y, y_center - x, PenColor);
	}
}

/// commands-end
void TFT_Init(void)
{	
	IO0CLR=DCX_RS|RDX|CS|RESET|POWER|WRX|D0|D1|D2|D3|D4|D5|D6|D7; // Clear all io pins
	IO1CLR=D8|D9|D10|D11|D12|D13|D14|D15; // Clear all io pins

	IO0DIR|=DCX_RS|RDX|CS|RESET|POWER|WRX|D0|D1|D2|D3|D4|D5|D6|D7; // Set all as output pin
	IO1DIR|=D8|D9|D10|D11|D12|D13|D14|D15; // Set P1.16-P1.23 as output pins

	SET_CTRLS(DCX_RS|RDX|WRX|CS);

	DelayMS(10);

	SET_CTRLS(RESET);
	
	// Driving ability setting
	TFT_WriteRegister(0x2E, 0x89); //write_data(0x00,); //GDOFF
	TFT_WriteRegister(0x29, 0X8F); //write_data(0x00); //RTN
	TFT_WriteRegister(0x2B, 0X02); //write_data(0x00,); //DUM
	TFT_WriteRegister(0xE2, 0X00); //write_data(0x00,); //VREF
	TFT_WriteRegister(0xE4, 0X01); //write_data(0x00,); //EQ
	TFT_WriteRegister(0xE5, 0X10); //write_data(0x00,); //EQ
	TFT_WriteRegister(0xE6, 0X01); //write_data(0x00,); //EQ
	TFT_WriteRegister(0xE7, 0X10); //write_data(0x00,); //EQ
	TFT_WriteRegister(0xE8, 0X70); //write_data(0x00,); //OPON
	TFT_WriteRegister(0xF2, 0X00); //write_data(0x00,); //GEN
	TFT_WriteRegister(0xEA, 0X00); //write_data(0x00,); //PTBA
	TFT_WriteRegister(0xEB, 0X20); //write_data(0x00,); //PTBA
	TFT_WriteRegister(0xEC, 0X3C); //write_data(0x00,); //STBA
	TFT_WriteRegister(0xED, 0XC8); //write_data(0x00,); //STBA
	TFT_WriteRegister(0xE9, 0X38); //write_data(0x00,); //OPON1
	TFT_WriteRegister(0xF1, 0X01); //write_data(0x00,); //OTPS1B

	// Gamma 2.8 setting 
	TFT_WriteRegister(0x40, 0X00); //write_data(0x00,); //
	TFT_WriteRegister(0x41, 0X00); //write_data(0x00,); //
	TFT_WriteRegister(0x42, 0X00); //write_data(0x00,); //
	TFT_WriteRegister(0x43, 0X15); //write_data(0x00,); //
	TFT_WriteRegister(0x44, 0X13); //write_data(0x00,); //
	TFT_WriteRegister(0x45, 0X3f); //write_data(0x00,); //
	TFT_WriteRegister(0x47, 0X55); //write_data(0x00,); //
	TFT_WriteRegister(0x48, 0X00); //write_data(0x00,); //
	TFT_WriteRegister(0x49, 0X12); //write_data(0x00,); //
	TFT_WriteRegister(0x4A, 0X19); //write_data(0x00,); //
	TFT_WriteRegister(0x4B, 0X19); //write_data(0x00,); //
	TFT_WriteRegister(0x4C, 0X16); //write_data(0x00,); //
	TFT_WriteRegister(0x50, 0X00); //write_data(0x00,); //
	TFT_WriteRegister(0x51, 0X2c); //write_data(0x00,); //
	TFT_WriteRegister(0x52, 0X2a); //write_data(0x00,); //
	TFT_WriteRegister(0x53, 0X3F); //write_data(0x00,); //
	TFT_WriteRegister(0x54, 0X3F); //write_data(0x00,); //
	TFT_WriteRegister(0x55, 0X3F); //write_data(0x00,); //
	TFT_WriteRegister(0x56, 0X2a); //write_data(0x00,); //
	TFT_WriteRegister(0x57, 0X7e); //write_data(0x00,); //
	TFT_WriteRegister(0x58, 0X09); //write_data(0x00,); //
	TFT_WriteRegister(0x59, 0X06); //write_data(0x00,); //
	TFT_WriteRegister(0x5A, 0X06); //write_data(0x00,); //
	TFT_WriteRegister(0x5B, 0X0d); //write_data(0x00,); //
	TFT_WriteRegister(0x5C, 0X1F); //write_data(0x00,); //
	TFT_WriteRegister(0x5D, 0XFF); //write_data(0x00,); //
	
	// Power Voltage Setting
	TFT_WriteRegister(0x1B, 0X1A); //write_data(0x00);
	TFT_WriteRegister(0x1A, 0X02); //write_data(0x00);
	TFT_WriteRegister(0x24, 0X61); //write_data(0x00);
	TFT_WriteRegister(0x25, 0X5C); //write_data(0x00);

	// Vcom offset
	//   TFT_WriteRegister(0x23,0x8D);   // FLICKER ADJUST
	TFT_WriteRegister(0x23, 0x62); //write_data(0x00,0X62);

	// Power ON Setting
	TFT_WriteRegister(0x18, 0X36); //write_data(0x00); //RADJ 70Hz
	DelayMS(5);
	TFT_WriteRegister(0x19, 0X01); //write_data(0x00,0X01); //OSC_EN=1
	DelayMS(5);
	TFT_WriteRegister(0x1F, 0X88); //write_data(0x00,0X88); // GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
	DelayMS(5);
	TFT_WriteRegister(0x1F, 0X80); //write_data(0x00,0X80); // GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
	DelayMS(5);
	TFT_WriteRegister(0x1F, 0X90); //write_data(0x00,0X90); // GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
	DelayMS(5);
	TFT_WriteRegister(0x1F, 0XD4); //write_data(0x00,0XD4); // GAS=1, VOMG=10, PON=1, DK=0, XDK=1, DDVDH_TRI=0, STB=0
	DelayMS(5);
	//262k/65k color selection
	TFT_WriteRegister(0x17, 0X05); //write_data(0x00,0X05); //default 0x06 262k color // 0x05 65k color
	DelayMS(5);

	//SET PANEL
	TFT_WriteRegister(0x36, 0X09); //write_data(0x00,0X09); //SS_P, GS_P,REV_P,BGR_P
	DelayMS(5);

	//Display ON Setting
	TFT_WriteRegister(0x28, 0X38); //write_data(0x00,0X38); //GON=1, DTE=1, D=1000
	DelayMS(40);
	TFT_WriteRegister(0x28, 0X3C); //write_data(0x00,0X3C); //GON=1, DTE=1, D=1100
	DelayMS(5);

	// 0x140 = 320, 0xF0 = 240
	//Set GRAM Area
	TFT_WriteRegister(0x16, 0x20);
	TFT_WriteRegister(0x02, 0x00);
	TFT_WriteRegister(0x03, 0x00);
	TFT_WriteRegister(0x04, 0x01);
	TFT_WriteRegister(0x05, 0x3F);
	TFT_WriteRegister(0x06, 0x00);
	TFT_WriteRegister(0x07, 0x00);
	TFT_WriteRegister(0x08, 0x00);
	TFT_WriteRegister(0x09, 0xF0);
	
	TFT_Fill_Screen(TFT_COLOR_BLACK);
	
	TFT_PowerDisplayLED(true);
	
	TFT_Set_Pen(TFT_COLOR_WHITE, 1);
}

void TFT_HardReset(void)
{
	CLEAR_CTRLS(RESET); // Clear P0.11(RESET)

	DelayMS(10);

	SET_CTRLS(RESET); // Set P0.11(RESET)
}

void TFT_PowerDisplayLED(bool power)
{
	if (power == true)
	{
		IO0SET=POWER;
	}
	else
	{
		IO0CLR=POWER;
	}
}

void TFT_Dot(short x, short y, uint16_t color)
{
	SET_CTRLS(CS|DCX_RS|RDX|WRX);
	CLEAR_CTRLS(CS);

	TFT_Set_Address(x, y);

	TFT_Write_Data_16(color);
	
	SET_CTRLS(CS);
}

void TFT_H_Line(short x_start, short x_end, short y_pos)
{
	short tmp;
	
	if (x_start > x_end)
	{
		tmp = x_start;
		x_start = x_end;
		x_end = tmp;
	}
	
	if ((y_pos < 0)||(y_pos >= 240)||(x_start >= 320)||(x_end < 0))
		return;

	if (x_start < 0) x_start = 0;
	if (x_end >= 320) x_end = 319;

	SET_CTRLS(CS|DCX_RS|RDX|WRX);
	CLEAR_CTRLS(CS);
	
	TFT_Set_Address(x_start, y_pos);
	
	SET_CTRLS(DCX_RS);
	TFT_Set16BitData(PenColor);	
	
	for(tmp = x_start; tmp <= x_end; tmp++)
	{
		TFT_Write_Strobe();
	}
	
	SET_CTRLS(CS);
}

void TFT_V_Line(short y_start, short y_end, short x_pos)
{
	short tmp;
	
	if (y_start > y_end)
	{
		tmp = y_start;
		y_start = y_end;
		y_end = tmp;
	}
	
	if ((x_pos < 0)||(x_pos >= 320)||(y_start >= 240)||(y_end < 0))
		return;

	if (y_start < 0) y_start = 0;
	if (y_end >= 240) y_end = 239;

	
	SET_CTRLS(CS|DCX_RS|RDX|WRX);
	CLEAR_CTRLS(CS);

	TFT_Set_Address(x_pos, y_start);

	// TODO: Optimize TFT_V_Line()
	for(tmp = y_start; tmp <= y_end; tmp++)
	{
		TFT_Dot(x_pos, tmp, PenColor);
	}

	SET_CTRLS(CS);	
}

void TFT_Line(short x1, short y1, short x2, short y2)
{
	int dx, dy;
	short tmp, x, y;
	
	if (x1 == x2)
	{
		TFT_V_Line(y1, y2, x1);
		return;
	}
	
	if (y1 == y2)
	{
		TFT_H_Line(x1, x2, y1);
		return;
	}
	
	if (x1 > x2)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	
	if (y1 > y2)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	
	dx = x2 - x1;
	dy = y2 - y1;
	
	for(x = x1; x <= x2; x++)
	{
		y = y1 + (dy) * (x - x1)/(dx);
		TFT_Dot(x, y, PenColor);
	}
}

void TFT_Rectangle(short x_upper_left, short y_upper_left, short x_bottom_right, short y_bottom_right)
{
    short y;
	
    for(y = y_upper_left; y <= y_bottom_right; y++)
	{
        TFT_H_Line(x_upper_left, x_bottom_right, y);
	}
}

void TFT_Write_Char(unsigned char ch, unsigned short x, unsigned short y)
{
	char data;
	unsigned int col;
	
	if (ch > 127)
		return;
	
	for(col = 0; col < 8; col++)
	{
		data = font8x8_basic[ch][col];

        if (data == 0)
            continue;
		
		if (data & 0x01) TFT_Dot(x + 0, y + col, PenColor);
		if (data & 0x02) TFT_Dot(x + 1, y + col, PenColor);
		if (data & 0x04) TFT_Dot(x + 2, y + col, PenColor);
		if (data & 0x08) TFT_Dot(x + 3, y + col, PenColor);
		if (data & 0x10) TFT_Dot(x + 4, y + col, PenColor);
		if (data & 0x20) TFT_Dot(x + 5, y + col, PenColor);
		if (data & 0x40) TFT_Dot(x + 6, y + col, PenColor);
		if (data & 0x80) TFT_Dot(x + 7, y + col, PenColor);
	}
}

void TFT_Write_Text(char *text, unsigned short x, unsigned short y)
{
	if (y >= 240)
		return;
	
	while((*text!=0)&&(x < 320))
	{
		TFT_Write_Char(*text, x, y);
		text++;
		x += 8;
	}
}

void TFT_Image(unsigned short left, unsigned short top, const char *image)
{
	unsigned short w, h, xpos, ypos, col, last_col;
	void *handle;
	int fsize;
	int first_col = 1;
	
	handle = fl_fopen(image, "r");
	
	if (handle)
	{
		fl_fseek(handle, 0, SEEK_END);
		fsize = fl_ftell(handle);
		fl_fseek(handle, 0, SEEK_SET);

		fl_fread(&w, 2, 1, handle);
		
		if ((w > 0)&&(w <= 320))
		{
			h = ((fsize - 2) / 2) / w;
			
			SET_CTRLS(CS|DCX_RS|RDX|WRX);
			CLEAR_CTRLS(CS);

			for(ypos = top; ypos < (h + top); ypos++)
			{
				if (ypos >= 240)
					break;
				
				fl_fread(row_data, 2, w, handle);
				
				TFT_Set_Address(left, ypos);
				first_col = 1;
				last_col = 0;
				
				SET_CTRLS(DCX_RS);
				
				for(xpos = left; xpos < (w + left); xpos++)
				{					
					if (xpos < 320)
					{
						col = row_data[xpos - left];
						
						if ((first_col)||(last_col != col))
						{
							TFT_Set16BitData(col);
							last_col = col;
							first_col = 0;
						}
						
						TFT_Write_Strobe();
					}
				}
			}
			
			SET_CTRLS(CS);
		}
				
		fl_fclose(handle);
	}	
}

void TFT_Set_Pen(unsigned short color, unsigned char width)
{
	PenColor = color;
	
	//PenWidth = width;
}
