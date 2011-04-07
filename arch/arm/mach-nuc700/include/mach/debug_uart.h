/*
 * FileName : debug.h
 *
 * Description : for debug porting using
 *
 * History :	
 *	2005/09/20	Created by Qfu
 *	2010/03/30	Modified by zswan
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define WRITE_BYTE(val, addr)	(*((volatile unsigned char *)addr) = val)
#define READ_BYTE(addr)			(*((volatile unsigned char *)addr))
#define WRITE_DWORD(val, addr)	(*((volatile unsigned int *)addr) = val)
#define READ_DWORD(addr)		(*((volatile unsigned int *)addr))

#define _UART_			0xfff80000

#define CLKSEL			0xfff0000c
#define UART_RBR		(_UART_ | 0x00)
#define UART_THR		(_UART_ | 0x00)
#define UART_IER		(_UART_ | 0x04)
#define UART_FCR		(_UART_ | 0x08)
#define UART_LCR		(_UART_ | 0x0c)

#define UART_LSR		(_UART_ | 0x14)

#define UART_DLL		(_UART_ | 0x00)
#define UART_DLM		(_UART_ | 0x04)

#define Uart_Init()	do{WRITE_BYTE(0x80, UART_LCR);\
			WRITE_DWORD(((READ_DWORD(CLKSEL)) | 0x20),CLKSEL); \
			WRITE_DWORD(0x06, UART_DLL);\
			WRITE_DWORD(0x00, UART_DLM);\
			WRITE_BYTE(0x03, UART_LCR);\
			WRITE_BYTE(0x07, UART_FCR);}while(0)

#define Uart_Put_Byte(c)	do{while(!(READ_BYTE(UART_LSR) & 0x20));\
					WRITE_BYTE(c, UART_THR);}while(0)
#define Uart_Put_String(str)	do{int _i_tmp = 0; while((str)[_i_tmp])\
					Uart_Put_Byte((str)[_i_tmp++]);}while(0)


#endif	/*  _DEBUG_H_ */
