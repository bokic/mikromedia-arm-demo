#ifndef __MP3_H__
#define __MP3_H__


extern void MP3_Init(void);
extern void MP3_play(char *filename);
extern void MP3_SCI_Write(char address, unsigned short data_in);
extern void MP3_SDI_Write(char data_);
extern void MP3_SDI_Write_32(unsigned char *data_);

#endif // __UTILS_H__
