#include "serial.h"
#include "lpc214x.h"
#include "utility.h"

/* Universal Asynchronous Receiver Transmitter Bits */
/// IER register
#define UART_IER_RBRIE          0x00000001
#define UART_IER_THREIE         0x00000002
#define UART_IER_RXLSIE         0x00000004
#define UART_IER_ABEOIE         0x00000100
#define UART_IER_ABTOIE         0x00000200
// IIR register
#define UART_IIR_IP             0x00000001
#define UART_IIR_II_RLS         0x00000006
#define UART_IIR_II_RDA         0x00000004
#define UART_IIR_II_CTI         0x0000000C
#define UART_IIR_II_THRE        0x00000002
#define UART_IIR_FIFO_EN_MASK   0x000000C0
#define UART_IIR_ABEO_INT       0x00000100
#define UART_IIR_ABTO_INT       0x00000200

// FCR register
#define UART_FCR_FIFO_EN       0x00000001
#define UART_FCR_RX_FIFO_RES   0x00000002
#define UART_FCR_TX_FIFO_RES   0x00000004
#define UART_FCR_RX_TRIG_LEV_0 0x00000000
#define UART_FCR_RX_TRIG_LEV_1 0x00000040
#define UART_FCR_RX_TRIG_LEV_2 0x00000080
#define UART_FCR_RX_TRIG_LEV_3 0x000000C0

// LCR register
#define UART_LCR_5BIT_DATA     0x00000000
#define UART_LCR_6BIT_DATA     0x00000001
#define UART_LCR_7BIT_DATA     0x00000002
#define UART_LCR_8BIT_DATA     0x00000003
#define UART_LCR_8BIT_DATA     0x00000003
#define UART_LCR_1BIT_STOP     0x00000000
#define UART_LCR_2BIT_STOP     0x00000004
#define UART_LCR_PARITY_EN     0x00000008
#define UART_LCR_PARITY_ODD    0x00000000
#define UART_LCR_PARITY_EVEN   0x00000010
#define UART_LCR_PARITY_1      0x00000020
#define UART_LCR_PARITY_0      0x00000030
#define UART_LCR_BREAK         0x00000040
#define UART_LCR_DLAB          0x00000080

// LSR register
#define UART_LSR_RDR           0x00000001
#define UART_LSR_OE            0x00000002
#define UART_LSR_PE            0x00000004
#define UART_LSR_FE            0x00000008
#define UART_LSR_BI            0x00000010
#define UART_LSR_THRE          0x00000020
#define UART_LSR_TEMT          0x00000040
#define UART_LSR_RXFE          0x00000080

// ACR register
#define UART_ACR_START         0x00000001
#define UART_ACR_MODE          0x00000002
#define UART_ACR_AUTORESTART   0x00000004
#define UART_ACR_ABEOINTCLR    0x00000100
#define UART_ACR_ABTOINTCLR    0x00000200
// FDR register
#define UART_FDR_DIVADDVAL     0x0000000F
#define UART_FDR_MULVAL        0x000000F0

// TER register
#define UART_TER_TXEN          0x00000080

static unsigned char DECnum[256];


void UART0_init (unsigned long bps, unsigned long VPBclk, unsigned char len, unsigned char stop, unsigned char parity, unsigned char parity_sel)
{
	unsigned long temp = 0;
	unsigned int lcr = 0;
	
	temp = (VPBclk / bps) >> 4;
	
	switch(len)
	{
	case length_5_bit:
		lcr = UART_LCR_5BIT_DATA;
		break;
	case length_6_bit:
		lcr = UART_LCR_6BIT_DATA;
		break;
	case length_7_bit:
		lcr = UART_LCR_7BIT_DATA;
		break;
	case length_8_bit:
		lcr = UART_LCR_8BIT_DATA;
		break;
	default:
		while(1) {}
		break;
	}
	
	switch(stop)
	{
	case stop_bit_1:
		lcr |= UART_LCR_1BIT_STOP;
		break;
	case stop_bit_2:
		lcr |= UART_LCR_2BIT_STOP;
		break;
	default:
		while(1) {}
		break;
	}
	
	switch(parity)
	{
	case parity_disable:
		break;
	case parity_enable:
		lcr |= UART_LCR_PARITY_EN;
		break;
	default:
		while(1) {}
		break;
	}
	
	switch(parity_sel)
	{
	case parity_odd:
		lcr |= UART_LCR_PARITY_ODD;
		break;
	case parity_even:
		lcr |= UART_LCR_PARITY_EVEN;
		break;
	case parity_forced_1:
		lcr |= UART_LCR_PARITY_1;
		break;
	case parity_forced_0:
		lcr |= UART_LCR_PARITY_0;
		break;
	default:
		while(1) {}
		break;
	}

	PINSEL0 = PINSEL0 & 0xFFFFFFF0; // Set P0.0 as digital input, and P0.1 as digital input

	U0DLL = temp % 16;
	U0DLM = temp / 16;
	U0IER = 0; // Disable all interrupts

	U0FCR = UART_FCR_FIFO_EN | UART_FCR_RX_FIFO_RES | UART_FCR_TX_FIFO_RES;
	U0LCR = 0;
	
	U0TER = UART_TER_TXEN;
	
	PINSEL0 = (PINSEL0 & 0xFFFFFFF0) | 0x00000005; // Set P0.0 as TXD (UART0), and P0.1 as RxD (UART0)
}

void UART0_sendchar(char ch)
{
    while((U0LSR & UART_LSR_THRE) == 0) {}

	U0THR = ch;
}

void UART0_sendstring(const unsigned char *p)
{
	while(*p)
	{
		UART0_sendchar((char)*p);

		p++;
	}
}

char UART0_getchar(void)
{
    while((U0LSR & UART_LSR_RDR) == 0) {}

	return (char)U0RBR;
}

void UART0_WriteNum(unsigned char *p, signed long n)
{
	if (p)
	{
		UART0_sendstring(p);
	}
	
	hex2str(DECnum, n, 2);	
	UART0_sendstring(DECnum);
}

void UART0_WriteLnNum(unsigned char *p, signed long n)
{
	UART0_WriteNum(p, n);

	UART0_sendchar('\r');
	UART0_sendchar('\n');
}


void UART1_init(unsigned int bps, unsigned int VPBclk, unsigned char len, unsigned char stop, unsigned char parity, unsigned char parity_sel)
{
	unsigned long temp = 0;
	unsigned int lcr = 0;
	
	temp = (VPBclk / bps) >> 4;
	
	switch(len)
	{
	case length_5_bit:
		lcr = UART_LCR_5BIT_DATA;
		break;
	case length_6_bit:
		lcr = UART_LCR_6BIT_DATA;
		break;
	case length_7_bit:
		lcr = UART_LCR_7BIT_DATA;
		break;
	case length_8_bit:
		lcr = UART_LCR_8BIT_DATA;
		break;
	default:
		while(1) {}
		break;
	}
	
	switch(stop)
	{
	case stop_bit_1:
		lcr |= UART_LCR_1BIT_STOP;
		break;
	case stop_bit_2:
		lcr |= UART_LCR_2BIT_STOP;
		break;
	default:
		while(1) {}
		break;
	}
	
	switch(parity)
	{
	case parity_disable:
		break;
	case parity_enable:
		lcr |= UART_LCR_PARITY_EN;
		break;
	default:
		while(1) {}
		break;
	}
	
	switch(parity_sel)
	{
	case parity_odd:
		lcr |= UART_LCR_PARITY_ODD;
		break;
	case parity_even:
		lcr |= UART_LCR_PARITY_EVEN;
		break;
	case parity_forced_1:
		lcr |= UART_LCR_PARITY_1;
		break;
	case parity_forced_0:
		lcr |= UART_LCR_PARITY_0;
		break;
	default:
		while(1) {}
		break;
	}

	PINSEL0 = PINSEL0 & 0xFFF0FFFF; // Set P0.8 as GPIO, and P0.9 as GPIO

	U1DLL = temp % 16;
	U1DLM = temp / 16;
	U1IER = 0; // Disable all interrupts

	U1FCR = UART_FCR_FIFO_EN | UART_FCR_RX_FIFO_RES | UART_FCR_TX_FIFO_RES;
	U1LCR = 0;
	U1MCR = 0;

	U1TER = UART_TER_TXEN;
	
	PINSEL0 = (PINSEL0 & 0xFFF0FFFF) | 0x00050000; // Set P0.8 as TXD (UART1), and P0.9 as RxD (UART1)
}

void UART1_sendchar(char ch)
{
    while((U1LSR & UART_LSR_THRE) == 0) {}

	U1THR = ch;
}

void UART1_sendstring(unsigned char *p)
{
	while(*p)
	{
		UART1_sendchar((char)*p);

		p++;
	}
}

char UART1_getchar(void)
{
    while((U1LSR & UART_LSR_RDR) == 0) {}

	return (char)U1RBR;
}

void UART1_WriteNum(unsigned char *p, signed long n)
{
	if (p)
	{
		UART1_sendstring(p);
	}
	
	hex2str(DECnum, n, 2);
	UART1_sendstring(DECnum);
}

void UART1_WriteLnNum(unsigned char *p, signed long n)
{
	UART1_WriteNum(p, n);

	UART1_sendchar('\r');
	UART1_sendchar('\n');
}

