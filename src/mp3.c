#include "mp3.h"
#include "lpc214x.h"
#include "spi.h"
#include "utils.h"
#include "fat_filelib.h"

#include <string.h>


//--- 8-bit
#define Lo(param) ((char *)&param)[0]
#define Hi(param) ((char *)&param)[1]
#define Higher(param) ((char *)&param)[2]
#define Highest(param) ((char *)&param)[3]

#define lo(param) ((char *)&param)[0]
#define hi(param) ((char *)&param)[1]
#define higher(param) ((char *)&param)[2]
#define highest(param) ((char *)&param)[3]


//--- 16-bit
#define LoWord(param) ((unsigned *)&param)[0]
#define HiWord(param) ((unsigned *)&param)[1]

#define loword(param) ((unsigned *)&param)[0]
#define hiword(param) ((unsigned *)&param)[1]


//--- 32-bit
#define HigherWord(param)  ((unsigned *)&param)[2]
#define HighestWord(param) ((unsigned *)&param)[3]

#define higherword(param)  ((unsigned *)&param)[2]
#define highestword(param) ((unsigned *)&param)[3]

#define LoDWord(param) ((unsigned long*)&param)[0]
#define HiDWord(param) ((unsigned long*)&param)[1]

#define lodword(param) ((unsigned long*)&param)[0]
#define hidword(param) ((unsigned long*)&param)[1]

typedef struct {
      volatile unsigned B0:1;
      volatile unsigned B1:1;
      volatile unsigned B2:1;
      volatile unsigned B3:1;
      volatile unsigned B4:1;
      volatile unsigned B5:1;
      volatile unsigned B6:1;
      volatile unsigned B7:1;
      volatile unsigned B8:1;
      volatile unsigned B9:1;
      volatile unsigned B10:1;
      volatile unsigned B11:1;
      volatile unsigned B12:1;
      volatile unsigned B13:1;
      volatile unsigned B14:1;
      volatile unsigned B15:1;
      volatile unsigned B16:1;
      volatile unsigned B17:1;
      volatile unsigned B18:1;
      volatile unsigned B19:1;
      volatile unsigned B20:1;
      volatile unsigned B21:1;
      volatile unsigned B22:1;
      volatile unsigned B23:1;
      volatile unsigned B24:1;
      volatile unsigned B25:1;
      volatile unsigned B26:1;
      volatile unsigned B27:1;
      volatile unsigned B28:1;
      volatile unsigned B29:1;
      volatile unsigned B30:1;
      volatile unsigned B31:1;
} volatile typeBit32;


//sbit Mmc_Chip_Select           at PORTF_OUT.B5;
//#define MP3_CS   ((typeBit32 *)&IOPIN1)->B25  // P1.25
//#define MP3_RST  ((typeBit32 *)&IOPIN0)->B18  // P0.18
//#define BSYNC    ((typeBit32 *)&IOPIN0)->B22  // P0.22
//#define DREQ     ((typeBit32 *)&IOPIN0)->B19  // P0.19

#define MP3_CS_SET   ((typeBit32 *)&IOSET1)->B25 = 1 // P1.25
#define MP3_CS_CLR   ((typeBit32 *)&IOCLR1)->B25 = 1 // P1.25
#define MP3_RST_SET  ((typeBit32 *)&IOSET0)->B18 = 1 // P0.18
#define MP3_RST_CLR  ((typeBit32 *)&IOCLR0)->B18 = 1 // P0.18
#define BSYNC_SET    ((typeBit32 *)&IOSET0)->B22 = 1 // P0.22
#define BSYNC_CLR    ((typeBit32 *)&IOCLR0)->B22 = 1 // P0.22
#define DREQ_SET     ((typeBit32 *)&IOSET0)->B19 = 1 // P0.19
#define DREQ_CLR     ((typeBit32 *)&IOCLR0)->B19 = 1 // P0.19
#define DREQ         ((typeBit32 *)&IOPIN0)->B19     // P0.19


//sbit Mmc_Chip_Select_Direction at PORTF_DIR.B5;
#define MP3_CS_Direction    ((typeBit32 *)&IODIR1)->B25
#define MP3_RST_Direction   ((typeBit32 *)&IODIR0)->B18
#define BSYNC_Direction     ((typeBit32 *)&IODIR0)->B22
#define DREQ_Direction      ((typeBit32 *)&IODIR0)->B19


const char WRITE_CODE  = 0x02;
const char READ_CODE   = 0x03;
const char SCI_MODE_ADDR   = 0x00;
const char SCI_CLOCKF_ADDR = 0x03;
const char SCI_VOL_ADDR    = 0x0B;
const char SCI_BASS_ADDR = 0x02;


// Writes one word to MP3 SCI
void MP3_SCI_Write(char address, unsigned short data_in) {
	
	// max spi speed is CLKI/4
	S0SPCCR = 0x14; // Set SPI clock to 3MHz.
	
  BSYNC_SET;
  
  MP3_CS_CLR;                             // select MP3 SCI
  SPI0_send(WRITE_CODE);
  SPI0_send(address);
  SPI0_send(Hi(data_in));                 // high byte
  SPI0_send(Lo(data_in));                 // low byte
  MP3_CS_SET;                             // deselect MP3 SCI

  while (DREQ == 0);                      // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI
}

// Reads words_count words from MP3 SCI
void MP3_SCI_Read(char start_address, char words_count, unsigned short *data_buffer) {
 
	unsigned short temp;
		
	// max spi speed is CLKI/7 or 1.5MHz(to be safe)
	S0SPCCR = 0x28; // Set SPI clock to 1.5MHz.

  BSYNC_SET;

  MP3_CS_CLR;                             // select MP3 SCI
  SPI0_send(READ_CODE);
  SPI0_send(start_address);

  while (words_count--) {                 // read words_count words byte per byte
    temp = SPI0_sendrecieve(0);
    temp <<= 8;
    temp += SPI0_sendrecieve(0);
    *(data_buffer++) = temp;
		while (DREQ == 0);                    // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI
  }
  MP3_CS_SET;                             // deselect MP3 SCI
}

// Write one byte to MP3 SDI
void MP3_SDI_Write(char data_) {
	
	// max spi speed is CLKI/4 or 3MHz
	S0SPCCR = 0x14; // Set SPI clock to 3MHz.

  MP3_CS_SET;
  BSYNC_CLR;

  while (DREQ == 0);                      // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI

  SPI0_send(data_);
  BSYNC_SET;
}

// Write 32 bytes to MP3 SDI
void MP3_SDI_Write_32(char *data_) {
	
  char i;

	// max spi speed is CLKI/4 or 3MHz
	S0SPCCR = 0x14; // Set SPI clock to 3MHz.

  MP3_CS_SET;
  BSYNC_CLR;

  while (DREQ == 0);                      // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI

  for (i=0; i<32; i++)
		SPI0_send(data_[i]);
	
  BSYNC_SET;
}

// Initialize VS1011E
void MP3_Init(void) {
	
  MP3_CS_Direction  = 1;                  // Configure MP3_CS as output
  MP3_CS_SET;                             // Deselect MP3_CS
  MP3_RST_Direction = 1;                  // Configure MP3_RST as output
  MP3_RST_SET;                            // Set MP3_RST pin

  DREQ_Direction  = 0;                    // Configure DREQ as input
  BSYNC_Direction = 1;                    // Configure BSYNC as output
  BSYNC_SET;                              // Set BSYNC

  BSYNC_SET;
  MP3_CS_SET;

  // Hardware reset
  MP3_RST_CLR;
  DelayUS(200); 
  MP3_RST_SET;

  while (DREQ == 0);

  MP3_SCI_Write(SCI_MODE_ADDR, 0x0800);
  MP3_SCI_Write(SCI_BASS_ADDR, 0x7A00);
  MP3_SCI_Write(SCI_VOL_ADDR, 0x4040);	
  MP3_SCI_Write(SCI_CLOCKF_ADDR, 0x2000); // default 12 288 000 Hz

  BSYNC_SET;
  MP3_CS_SET;
}

void MP3_play(char *filename)
{
	char buf[32];
	void *handle;
	int readed;
	int c;
	
	handle = fl_fopen(filename, "r");
	
	if(handle)
	{
		MP3_Init();

		for(;;)
		{
			readed = fl_fread(buf, 32, 1, handle);
			
			if (readed != 32)
				break;

			MP3_SDI_Write_32(buf);
		}
		
		fl_fclose(handle);

		memset(buf, 0, sizeof(buf));
		
		// clockout dummy data so the mp3 will finish the playback.
		for(c = 0; c <= 64; c++)
		{
			MP3_SDI_Write_32(buf);
		}
		
		// Configure DREQ as output(to be nice to TFT module)
		DREQ_Direction = 1;
		
		// Disable mp3 ic CS lines.
		MP3_CS_SET;
		BSYNC_SET;
	}
}
