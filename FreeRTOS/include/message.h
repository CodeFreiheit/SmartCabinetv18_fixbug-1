//
// --------------------------------------------------------------------------
// Message.h
// --------------------------------------------------------------------------

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*----------------------------------------------------------------------------+
| Include files                                                               |
+----------------------------------------------------------------------------*/
#include "sys.h" 

/*----------------------------------------------------------------------------+
| Type Definition & Macro                                                     |
+----------------------------------------------------------------------------*/

#define MSG_NULL				0x00	// û����Ϣ
#define MSG_TICK_1_SECOND		0x01	// ϵͳÿ��һ������
#define MSG_SYS_TICK			0x02	// ϵͳÿ10msһ��
#define MSG_TICK_3_SECOND		0x03	// ����ÿ3��һ������
#define MSG_TICK_5_SECOND		0x05
#define MSG_TICK_30_SECOND		30
#define MSG_TICK_60_SECOND		60
	
#define MSG_TICK_10ms_SECOND		0x08	// 10msһ������

// ���źſ�����ص���Ϣ����
#define MSG_POWER_ON  0x0f 
#define MSG_EVENT_SWITCH 0x10

// �밴����ص���Ϣ����
#define MSG_KEY_DOWN			0x11
#define MSG_KEY_UP				0x12
#define MSG_KEY_PRESS			0x13

#define MSG_KEY_FUNC_PRESS		0x14	// ���ܼ����£�ִ���ض��Ĺ���

// ��USART1��ص���Ϣ����
#define MSG_UART1_RX_BYTE		0x21
#define MSG_UART1_TX_BYTE		0x22
#define MSG_UART1_RX_FRAME		0x23	// UART0���յ�һ������֡
#define MSG_UART1_TX_FRAME		0x24	//

#define MSG_camera_finish		0x25	//

// ��USART2��ص���Ϣ����
#define MSG_UART2_RX_BYTE		0x31
#define MSG_UART2_TX_BYTE		0x32
#define MSG_UART2_RX_FRAME		0x33	// UART1���յ�һ������֡
#define MSG_UART2_TX_FRAME		0x34	//

// ��UART5��ص���Ϣ����
#define MSG_UART3_RX_BYTE		0x36
#define MSG_UART3_TX_BYTE		0x37
#define MSG_UART3_RX_FRAME		0x38	// ���յ�һ������֡
#define MSG_UART3_TX_FRAME		0x39	//  

// ��SD����ص���Ϣ����
#define MSG_SD_INSERT			0x41	// SD������
#define MSG_SD_DESERT			0x42	// SD���γ�

#define MSG_SD_DETECT		0x43	// SD������

// ��USB��ص���Ϣ����
#define MSG_USB_INT				0x51	// USB�����ж�
#define MSG_USB_SETUP_PACKET	0x52	// ���յ�����������SetupPacket
#define MSG_USB_CTR				0x53	// Endpoint Correct Transfer Request

#define MSG_LINKSTATE_CHANGE	0x54	// ����״̬�ı�

#define MSG_TEMPERATURE_CHANGE	0x55	// �¶�״̬�ı�

// Message structure
typedef struct t_MSG
{
	u16 Msg;
	u16 wParam;
	u32 lParam;
} MSG, *PMSG;


typedef struct MSG
{
	u16 USB;
	u16 SD;
	u32 MQTT;
} Task_MSG;





typedef struct t2_MSG
{
	u16 Msg;
	u16 wParam;
	u32 lParam;
} tcp_MSG;

/*----------------------------------------------------------------------------+
| Function Prototype                                                          |
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
| End of source file                                                          |
+----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MESSAGE_H__
/*------------------------ Nothing Below This Line --------------------------*/
