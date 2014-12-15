#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdbool.h>
#include <stdint.h>

#define TFT_COLOR_RED 0xF800
#define TFT_COLOR_GREEN 0x07E0
#define TFT_COLOR_BLUE 0x001F
#define TFT_COLOR_WHITE 0xFFFF
#define TFT_COLOR_BLACK 0x0000


extern void TFT_Init(void);
extern void TFT_HardReset(void);
extern void TFT_PowerDisplayLED(bool power);
extern void TFT_Dot(short x, short y, uint16_t color); 
extern void TFT_H_Line(short x_start, short x_end, short y_pos); 
extern void TFT_V_Line(short y_start, short y_end, short x_pos); 
extern void TFT_Line(short x1, short y1, short x2, short y2);
extern void TFT_Fill_Screen(unsigned short color);
extern void TFT_Circle(short x_center, short y_center, short radius);
extern void TFT_Rectangle(short x_upper_left, short y_upper_left, short x_bottom_right, short y_bottom_right);
//extern void TFT_Rectangle_Round_Edges(unsigned short x_upper_left, unsigned short y_upper_left,
//                                      unsigned short x_bottom_right, unsigned short y_bottom_right,
//                                      unsigned short round_radius);
extern void TFT_Write_Char(unsigned char ch, unsigned short x, unsigned short y);
extern void TFT_Write_Text(char *text, unsigned short x, unsigned short y);
extern void TFT_Image(unsigned short left, unsigned short top, const char *image);

extern void TFT_Set_Pen(unsigned short color, unsigned char width);

#endif // __DISPLAY_H__
