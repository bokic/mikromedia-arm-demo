#include "mp3.h"
#include "lpc214x.h"
#include "spi.h"
#include "utils.h"
#include "fat_filelib.h"

#include <stdint.h>
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

// MP3 Pin Bitmasks
// MP3_CS:  P1.25
// MP3_RST: P0.18
// BSYNC:   P0.22 (XDCS)
// DREQ:    P0.19
#define PIN_MP3_CS      (1UL << 25)
#define PIN_MP3_RST     (1UL << 18)
#define PIN_BSYNC       (1UL << 22)
#define PIN_DREQ        (1UL << 19)

#define MP3_CS_SET      (IOSET1 = PIN_MP3_CS)
#define MP3_CS_CLR      (IOCLR1 = PIN_MP3_CS)
#define MP3_RST_SET     (IOSET0 = PIN_MP3_RST)
#define MP3_RST_CLR     (IOCLR0 = PIN_MP3_RST)
#define BSYNC_SET       (IOSET0 = PIN_BSYNC)
#define BSYNC_CLR       (IOCLR0 = PIN_BSYNC)
#define DREQ            ((IOPIN0 & PIN_DREQ) != 0)

#define MP3_CS_DIR_OUT()   (IODIR1 |= PIN_MP3_CS)
#define MP3_RST_DIR_OUT()  (IODIR0 |= PIN_MP3_RST)
#define BSYNC_DIR_OUT()    (IODIR0 |= PIN_BSYNC)
#define DREQ_DIR_IN()      (IODIR0 &= ~PIN_DREQ)
#define DREQ_DIR_OUT()     (IODIR0 |= PIN_DREQ)


const char WRITE_CODE  = 0x02;
const char READ_CODE   = 0x03;
const char SCI_MODE_ADDR   = 0x00;
const char SCI_CLOCKF_ADDR = 0x03;
const char SCI_VOL_ADDR    = 0x0B;
const char SCI_BASS_ADDR = 0x02;


#define MP3_DREQ_TIMEOUT 500000

// Writes one word to MP3 SCI
void MP3_SCI_Write(char address, unsigned short data_in) {
	uint32_t timeout;

	// max spi speed is CLKI/4
	S0SPCCR = 0x14; // Set SPI clock to 3MHz.

  BSYNC_SET;

  MP3_CS_CLR;                             // select MP3 SCI
  SPI0_send(WRITE_CODE);
  SPI0_send(address);
  SPI0_send(Hi(data_in));                 // high byte
  SPI0_send(Lo(data_in));                 // low byte
  MP3_CS_SET;                             // deselect MP3 SCI

  timeout = MP3_DREQ_TIMEOUT;
  while ((DREQ == 0) && --timeout);       // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI
}

// Reads words_count words from MP3 SCI
void MP3_SCI_Read(char start_address, char words_count, unsigned short *data_buffer) {
	uint32_t timeout;
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
    timeout = MP3_DREQ_TIMEOUT;
    while ((DREQ == 0) && --timeout);     // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI
  }
  MP3_CS_SET;                             // deselect MP3 SCI
}

// Write one byte to MP3 SDI
void MP3_SDI_Write(char data_) {
	uint32_t timeout;

	// max spi speed is CLKI/4 or 3MHz
	S0SPCCR = 0x14; // Set SPI clock to 3MHz.

  MP3_CS_SET;
  BSYNC_CLR;

  timeout = MP3_DREQ_TIMEOUT;
  while ((DREQ == 0) && --timeout);       // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI

  SPI0_send(data_);
  BSYNC_SET;
}

// Write 32 bytes to MP3 SDI
void MP3_SDI_Write_32(unsigned char *data_) {
  uint32_t timeout;
  int i;

	// max spi speed is CLKI/4 or 3MHz
	S0SPCCR = 0x14; // Set SPI clock to 3MHz.

  MP3_CS_SET;
  BSYNC_CLR;

  timeout = MP3_DREQ_TIMEOUT;
  while ((DREQ == 0) && --timeout);       // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI

  for (i=0; i<32; i++)
		SPI0_send(data_[i]);

  BSYNC_SET;
}

// Initialize VS1011E
void MP3_Init(void) {
  uint32_t timeout;

  MP3_CS_DIR_OUT();                       // Configure MP3_CS as output
  MP3_CS_SET;                             // Deselect MP3_CS
  MP3_RST_DIR_OUT();                      // Configure MP3_RST as output
  MP3_RST_SET;                            // Set MP3_RST pin

  DREQ_DIR_IN();                          // Configure DREQ as input
  BSYNC_DIR_OUT();                        // Configure BSYNC as output
  BSYNC_SET;                              // Set BSYNC

  BSYNC_SET;
  MP3_CS_SET;

  // Hardware reset
  MP3_RST_CLR;
  DelayUS(200);
  MP3_RST_SET;

  timeout = MP3_DREQ_TIMEOUT;
  while ((DREQ == 0) && --timeout);

  MP3_SCI_Write(SCI_MODE_ADDR, 0x0800);
  MP3_SCI_Write(SCI_BASS_ADDR, 0x7A00);
  MP3_SCI_Write(SCI_VOL_ADDR, 0x4040);
  MP3_SCI_Write(SCI_CLOCKF_ADDR, 0x2000); // default 12 288 000 Hz

  BSYNC_SET;
  MP3_CS_SET;
}

void MP3_play(char *filename)
{
	unsigned char buf[32];
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
		DREQ_DIR_OUT();

		// Disable mp3 ic CS lines.
		MP3_CS_SET;
		BSYNC_SET;
	}
}
