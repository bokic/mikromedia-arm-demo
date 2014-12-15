#ifndef __SERIAL_H_
#define __SERIAL_H_



#define	length_5_bit			0
#define	length_6_bit			1
#define	length_7_bit			2
#define	length_8_bit			3

#define	stop_bit_1				0
#define	stop_bit_2				1

#define	parity_disable		0
#define	parity_enable			1

#define	parity_odd				0
#define	parity_even				1
#define	parity_forced_1		2
#define	parity_forced_0		3



void UART0_init (unsigned long bps, unsigned long VPBclk, unsigned char len, unsigned char stop, unsigned char parity, unsigned char parity_sel);
void UART0_sendchar (char ch);
void UART0_sendstring (const unsigned char *p);
char UART0_getchar (void);
void UART0_WriteNum (unsigned char *p, signed long n);
void UART0_WriteLnNum (unsigned char *p, signed long n);

void UART1_init (unsigned int bps, unsigned int VPBclk, unsigned char len, unsigned char stop, unsigned char parity, unsigned char parity_sel);
void UART1_sendchar (char ch);
void UART1_sendstring (unsigned char *p);
char UART1_getchar (void);
void UART1_WriteNum (unsigned char *p, signed long n);
void UART1_WriteLnNum (unsigned char *p, signed long n);


#endif
