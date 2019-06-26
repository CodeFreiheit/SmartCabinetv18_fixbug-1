// Copyright (c)2011 by Aaron, All Rights Reserved.
/*--------------------------------------------------------------------------+
|  File Name:  Printf.c, v1.0.0												|
|  Author:     aaron.xu1982@gmail.com                                       |
|  Date:       2006��02��28��												|
+---------------------------------------------------------------------------+
|  Description: ���ڸ�ʽ���������											|
|																			|
+---------------------------------------------------------------------------+
|  Release Notes:                                                           |
|                                                                           |
|  Logs:                                                                    |
|  WHO       WHEN         WHAT                                              |
|  ---       ----------   --------------------------------------------------|
|  Aaron     2011/09/28   born                                              |
|                                                                           |
+--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------+
| Include files                                                             |
+--------------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "usart.h"
#include "print.h"
#include "tcp_server_demo.h" 
#include <stdio.h>
#include <string.h>

#include "sensor.h"

/*--------------------------------------------------------------------------+
| Type Definition & Macro                                                   |
+--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------+
| Global Variables                                                          |
+--------------------------------------------------------------------------*/
const char HexTab[] = "0123456789ABCDEF";		// ASCII-hex table
extern xSemaphoreHandle USART1_Sem;

//void Printf(const char *format, ...);

u8 Printf_Buf[512];	   //����ת����  Buf


/*--------------------------------------------------------------------------+
| Internal Variables                                                        |
+--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------+
| Function Prototype                                                        |
+--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------+
| General Subroutines                                                       |
+--------------------------------------------------------------------------*/
void PrintC(char c)
{
	SerialPutChar(c);
}

// �Ӵ�����ʾ�ַ�����
void PrintS(const char *s)
{
	xSemaphoreTake(USART1_Sem, portMAX_DELAY);

	SerialPutString((const u16 *)s, 0);

	xSemaphoreGive(USART1_Sem);
}

// �Ӵ�����ʾ��ʽ���ַ�������ȿ⺯��printf()�������˻������Ĳ�����ͬʱ��С�˶Զ�ջ������
void Printf(const char *format, ...)
{
#define MAX_TBUF	128							// ע��: ֻ������ոô�С���ַ���.
	char    tbuf[MAX_TBUF];
	va_list v_list;
	char    *ptr;


	va_start(v_list, format);					// Initialize variable arguments.
	vsnprintf(tbuf, MAX_TBUF, format, v_list);
	va_end(v_list);

	xSemaphoreTake(USART1_Sem, portMAX_DELAY);
	ptr= tbuf;
	//tbuf=Printf_Buf;
	memcpy((char*)tcp_server_sendbuf,tbuf,strlen((char*)(tbuf)));	
	//   xQueueSend(printfTaskQueue, &tcp_server_sendbuf, 0);
	//tcp_server_sendbuf=tbuf;
	//xQueueSend(ServerTaskQueue, &server_msg, 0);
	//tcp_server_test(1);
	//xSemaphoreGive(printf_signal);
	while(*ptr != '\0')
	{
		SerialPutChar(*ptr++);
	}
	xSemaphoreGive(USART1_Sem);
	xSemaphoreGive(printf_signal);

#	undef MAX_TBUF
}

/* ��ӡ8bit������ */
void Print8(u8 n)
{
	SerialPutChar(HexTab[(n & 0xF0)>>4]);
	SerialPutChar(HexTab[(n & 0x0F)]);
}

/* ��ӡ16bit������ */
void Print16(u16 n)
{
	SerialPutChar(HexTab[(n & 0xF000)>>12]);
	SerialPutChar(HexTab[(n & 0x0F00)>>8]);
	SerialPutChar(HexTab[(n & 0x00F0)>>4]);
	SerialPutChar(HexTab[(n & 0x000F)]);
}

/* ��ӡ32bit������ */
void Print32(u32 n)
{
	SerialPutChar(HexTab[(n & 0xF0000000)>>28]);
	SerialPutChar(HexTab[(n & 0x0F000000)>>24]);
	SerialPutChar(HexTab[(n & 0x00F00000)>>20]);
	SerialPutChar(HexTab[(n & 0x000F0000)>>16]);
	SerialPutChar(HexTab[(n & 0x0000F000)>>12]);
	SerialPutChar(HexTab[(n & 0x00000F00)>>8]);
	SerialPutChar(HexTab[(n & 0x000000F0)>>4]);
	SerialPutChar(HexTab[(n & 0x0000000F)]);
}

// PrintHex(): ��16������ʽ���һ��������
// s:            Ҫ��ʾ�����ݵ���ʼ��ַ
// nLength:      Ҫ��ʾ�����ݵĳ���
// bShowAddress: �Ƿ����������ʾ��ǰ���ݵ�ַ
// offset: 	     ��������ݵ�ַ�ĳ�ʼֵ�����ڶ�ε��øú�����ʾһ���󻺳����е�����
void PrintHex(u8 *s, u32 nLength, u8 bShowAddress, u32 offset)
{
	u32 i;
	u32 j;

	xSemaphoreTake(USART1_Sem, portMAX_DELAY);

	for (i=0; i<nLength; i++)
	{
		if (bShowAddress)
		{
			if (i == 0)
			{
				SerialPutChar(HexTab[(offset & 0xF000)>>12]);
				SerialPutChar(HexTab[(offset & 0x0F00)>>8]);
				SerialPutChar(HexTab[(offset & 0x00F0)>>4]);
				SerialPutChar(HexTab[(offset & 0x000F)]);
				SerialPutChar(':');
				SerialPutChar(' ');
			}
			else if ((i & 0x000F) == 0)		// 16�ı���
			{
#if 1			// �Ƿ���һ�ж��������ݺ�����ʾ��һ�ж��������ݵ�ASCII�ַ�
				SerialPutChar(' ');
				SerialPutChar(' ');
				SerialPutChar(' ');
				for (j=i-16; j<i; j++)
				{
					if ((s[j] >= 0x20) && (s[j] < 0x80))	// �ɴ�ӡ�ַ�
						SerialPutChar(s[j]);
					else				// ���ɴ�ӡ���ַ�������ʾһ��'.'
						SerialPutChar('.');	
				}
#endif
				SerialPutChar('\n');
				SerialPutChar(HexTab[((i+offset) & 0xF000)>>12]);
				SerialPutChar(HexTab[((i+offset) & 0x0F00)>>8]);
				SerialPutChar(HexTab[((i+offset) & 0x00F0)>>4]);
				SerialPutChar(HexTab[((i+offset) & 0x000F)]);
				SerialPutChar(':');
				SerialPutChar(' ');
			}
			else if ((i & 0x0007) == 0)		// 8�ı���
			{
				SerialPutChar(' ');
				SerialPutChar(' ');
			}
			else
			{
				SerialPutChar(' ');
			}
		}
		else
		{
			if (i == 0)
			{
			}
			else if ((i & 0x000F) == 0)		// 16�ı���
			{
				SerialPutChar('\n');
			}
			else if ((i & 0x0007) == 0)		// 8�ı���
			{
				SerialPutChar(' ');
				SerialPutChar(' ');
			}
			else
			{
				SerialPutChar(' ');
			}
		}
		SerialPutChar(HexTab[(s[i]>>4) & 0x0F]);
		SerialPutChar(HexTab[s[i] & 0x0F]);
	}	
#if 1
	if (bShowAddress)
	{
		if ((i & 0x0F) > 0)
		{
			if ((i & 0x0F) <= 8)
				SerialPutChar(' ');			// ���ÿһ���м��һ���ո�Ŀ��
			for (j=(i&0x0F); j<16; j++)		// ����һ�еĿո���
			{
				SerialPutChar(' ');
				SerialPutChar(' ');
				SerialPutChar(' ');
			}
			SerialPutChar(' ');
			SerialPutChar(' ');
			SerialPutChar(' ');
			for (j=(i & ~0x0F); j<i; j++)
			{
				if ((s[j] >= 0x20) && (s[j] < 0x80))	// �ɴ�ӡ�ַ�
					SerialPutChar(s[j]);
				else						// ���ɴ�ӡ���ַ�������ʾһ��'.'
					SerialPutChar('.');	
			}
		}
		else
		{
			SerialPutChar(' ');
			SerialPutChar(' ');
			SerialPutChar(' ');
			for (j=i-16; j<i; j++)
			{
				if ((s[j] >= 0x20) && (s[j] < 0x80))	// �ɴ�ӡ�ַ�
					SerialPutChar(s[j]);
				else						// ���ɴ�ӡ���ַ�������ʾһ��'.'
					SerialPutChar('.');	
			}
		}
	}
#endif
	SerialPutChar('\n');

	xSemaphoreGive(USART1_Sem);
}






/*--------------------------------------------------------------------------+
| End of source file                                                        |
+--------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line ------------------------*/
