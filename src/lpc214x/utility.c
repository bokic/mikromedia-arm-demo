/******************************************************************************/
/*                                                                            */
/*  Utility.C:                                                                */
/*                                                                            */
/******************************************************************************/

#include "lpc214x.h"


//-------------- Return Byte 0 (LSB) of DWORD
unsigned char Lo(unsigned long val)
{
	return (val & 0x000000FF);
}

//-------------- Return Byte 1 of DWORD
unsigned char Hi(unsigned long val)
{
	return ((val >> 8) & 0x000000FF);
}

//-------------- Return Byte 2 of DWORD
unsigned char Higher(unsigned long val)
{
	return ((val >> 16) & 0x000000FF);
}

//-------------- Return Byte 3 (MSB) of DWORD
unsigned char Highest(unsigned long val)
{
	return ((val >> 24) & 0x000000FF);
}



//-------------- Convert DWORD into string
void hex2str (unsigned char *p, unsigned long intnum, unsigned char len)
{
	unsigned long tmp, rem;
	unsigned char HEXtmp[8];
	unsigned char n;

	tmp = intnum;

	n = 8;
	while (n > 0)
	{
		rem = tmp % 16;
		tmp /= 16;
		if (rem >= 10)
			rem += 55;
		else
			rem += 48;
		HEXtmp[--n] = rem;
	}

	n = 8-len;
	while (n < 8)
		*p++ = HEXtmp[n++];

	*p = 0;
}

//-------------- Convert INTEGER into string
unsigned char int2str (unsigned char *p, signed long intnum)
{
	unsigned long tmp, rem;
	char n, len;
	unsigned char DECtmp[10];		// 2^32 - max. 10 digits

	tmp = intnum;
	if (intnum < 0)
		tmp = -(intnum);

	n = 10;
	while (n > 0)
	{
		rem = tmp % 10;
		tmp /= 10;
		if (rem >= 10)
			rem += 55;
        DECtmp[(int)--n] = rem + 0x30;
	}

	*p = '+';
	if (intnum < 0)
		*p = '-';
	p++;

	n = 0;
    while ((DECtmp[(int)n] == 0x30) && (n < 9))
		n++;

	len = 1;
	while (n < 10)
	{
        *p++ = DECtmp[(int)n++];
		len++;
	}

	*p = 0;

	return (len);
}


//-------------- Delay Procedure
void DelayProc(unsigned int Cyc)
{
	unsigned int n;
	
	n = Cyc;
	while (n--);
}

//-------------- Right justified, fixed length = 5+1 (digits + string term 0)
void WordToStr(unsigned short input, unsigned char * output) {
  unsigned char len;


  //--- the result is right justified - so we go from bottom to top
  for(len=0;len < 5; len++)
    output[len] = ' ';
  output[len--] = 0;

  while(1) {
    output[len] = input % 10u + 48;
    input = input / 10u;
    if (input == 0)
      break;
    len--;
  }
}


//-------------- Right justified, fixed length = 6+1 (digits + string term 0)
void ShortToStr(short input, unsigned char *output) {
  unsigned short              // It is assumed that input is in range
   i, negative;              //  -32768..32767.
  unsigned short                  // Promenljiva len izbacena jer se uopste ne koristi u kodu
   inword;                   //   06.08.2008. Mx

  negative = 0;                 // negarive = FALSE;
  inword = (unsigned)(input);
  if (input < 0) {
      negative = 1;             // negative = TRUE;
      inword = (unsigned)(0 - input);
  }
  WordToStr(inword, output);

  i = 6;
  while (i > 0) {
    output[i] = output[i-1];
    i--;
  }
  output[0] = ' ';
    if(negative){
       i = 0;
       while (output[ i ] == ' ') i++;
          i--;
       output[ i ] = '-';}
}
