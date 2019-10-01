#include "gsm.h"
#include "usart.h"		
#include "delay.h"	 
#include "string.h" 
#include "usart.h" 
#include "math.h"
#include "stdio.h"
#include "dma.h"
#include "FreeRTOS.h"
#include "task.h"
#include "print.h"

u8 Flag_Rec_Message = 0;
u8 SIM_CON_OK;

// usmart͸��
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA & 0X8000)
	{
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;
		printf("%s",USART2_RX_BUF);
		if(mode)USART2_RX_STA = 0;
	} 
}

u8* sim800c_check_cmd(u8 *str)
{
	char *strx = 0;
	if(USART2_RX_STA & 0X8000)
	{
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;	// ��ӽ�����
		strx = strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}

u8 sim800c_send_cmd(u8 *cmd, u8 *ack, u16 waittime)
{
	u8 res = 0;
	
	USART2_RX_STA = 0;

	vTaskSuspendAll();
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR != 0);			// �ȴ�ͨ��7�������   
		USART2->DR=(u32)cmd;
	}
	else
	{
		if(cmd[0]=='+'&&cmd[1]=='+')
			USART2_vspf("%s",cmd);
		else USART2_vspf("%s\r\n",cmd);				// ��������
	}
	xTaskResumeAll();
	
	// �ȴ�Ӧ��
	if(ack && waittime)
	{
		while(--waittime)
		{
			vTaskDelay(20);
			if(USART2_RX_STA & 0X8000)
			{
			//	printf("USART2_RX_BUF receiv:%s\r\n",USART2_RX_BUF);
				if(sim800c_check_cmd(ack))break;	// �õ���Ч���� 
				USART2_RX_STA = 0;
			}
		}
		if(waittime == 0)res = 1;					// ��ʱ
	}	
	UART_DMA_Enable(DMA1_Channel6, USART2_MAX_LEN);
	USART2_RX_STA = 0;
	return res;
} 

u8 sim800c_work_test(void)
{
	vTaskDelay(1000/portTICK_RATE_MS);  
	sim800c_send_cmd((u8 *)"+++",(u8 *)"OK",100);  //AT+CPIN?
	vTaskDelay(1000/portTICK_RATE_MS);
	
//	
//	sim800c_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",100);//
//  vTaskDelay(1000/portTICK_RATE_MS);	
//	
//	sim800c_send_cmd((u8 *)"AT+CREG?",(u8 *)"OK",100);//
//  vTaskDelay(1000/portTICK_RATE_MS);	
//	
//	sim800c_send_cmd((u8 *)"AT+COPS?",(u8 *)"OK",100);//
//  vTaskDelay(1000/portTICK_RATE_MS);	
//	
//	sim800c_send_cmd((u8 *)"AT+CGATT=1",(u8 *)"OK",100);//
//  vTaskDelay(1000/portTICK_RATE_MS);	
	
	if(sim800c_send_cmd((u8 *)"AT",(u8 *)"OK",100))
	{
		if(sim800c_send_cmd((u8 *)"AT",(u8 *)"OK",100))return SIM_COMMUNTION_ERR;	// ͨ�Ų���
	}		
	if(sim800c_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",400))return SIM_CPIN_ERR;	// û��SIM��
	if(sim800c_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",400)){}
	else return SIM_CREG_FAIL;														// ע��ʧ��
	
	return SIM_OK;
}

u8 gsm_dect(void)
{
	u8 res;
	
	res = sim800c_work_test();	
	switch(res)
	{
		case SIM_OK:
			Printf("GSMģ���Լ�ɹ�\r\n");
			break;
		case SIM_COMMUNTION_ERR:
			SIM_CON_OK = 0;
			Printf("��GSMģ��δͨѶ�ɹ�����ȴ�\r\n");
			break;
		case SIM_CPIN_ERR:
			SIM_CON_OK = 0;
			Printf("û��⵽SIM��\r\n");
			break;
		default:
			break;
	}
	return res;
}

void gsm_reset(void)
{
//	Printf("GSMģ������...\r\n");
//	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
//	vTaskDelay(2000/portTICK_RATE_MS); // ����2��������¿��ػ�
//	GPIO_SetBits(GPIOC,GPIO_Pin_9);
//	vTaskDelay(2000/portTICK_RATE_MS);
}

u8 SIM800C_CONNECT_SERVER(u8 *servip,u8 *port)
{
	u8 dtbufa[50];
	if(sim800c_send_cmd((u8 *)"AT+CGATT?",(u8 *)"+CGATT: 1",200))
	if(sim800c_send_cmd((u8 *)"AT+CIPMUX=0",(u8 *)"OK",400))	return 3;	// ��·����
	if(sim800c_send_cmd((u8 *)"AT+CGATT=1",(u8 *)"OK",400))		return 4;	// ��MT����GPRSҵ��
	if(sim800c_send_cmd((u8 *)"AT+CIPSHUT",(u8 *)"OK",400))		return 5;	// �ر��ƶ�����
	if(sim800c_send_cmd((u8 *)"AT+CIPMODE=1",(u8 *)"OK",200))	return 6;	// ͸��ģʽ
	// �ش�����5,ʱ����2*100ms,�����ֽ�1024,����ת������1,Rxmode=0,Rxsize=1460,Rxtime=50
	if(sim800c_send_cmd((u8 *)"AT+CIPCCFG=5,2,1024,1,0,1460,50",(u8 *)"OK",500))		return 7;
	// ��ͨ���������uninet	�ƶ�cmnet
	if(sim800c_send_cmd((u8 *)"AT+CSTT=\"uninet\",\"NULL\",\"NULL\"",(u8 *)"OK",200))	return 8;
	if(sim800c_send_cmd((u8 *)"AT+CIICR",(u8 *)"OK",2000))		return 9;	// �����ƶ�����
	if(!sim800c_send_cmd((u8 *)"AT+CIFSR",(u8 *)"ERROR",200))	return 10;	// ��ȡ����IP
	
	sprintf((char*)dtbufa,"AT+CIPSTART=TCP,%s,%s\r\n",servip,port);			// TCP����,IP,�˿�
	
	if(sim800c_send_cmd((u8 *)dtbufa,(u8 *)"CONNECT",200))	return 12;
	
	return 0;
}

//  SIM800C_CONNECT_SERVER  by Wired test 
//u8 SIM800C_CONNECT_SERVER(u8 *servip,u8 *port)
//{
//  struct tcp_pcb *tcppcb;  	//����һ��TCP���������ƿ�
//	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
//  char inifile[] = "1:cfg.ini";  
//	MSG msg;
//	u8 *tbuf;
// 	u8 key;
//	u8 res=0;		
//	u8 t=0; 
//	u8 connflag=0;		//���ӱ��	
//	int data_len;
//	static int Count_packet=0;	
//	tcp_client_set_remoteip();//��ѡ��IP 
//	tbuf=(u8_t *)pvPortMalloc(200);
//	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
//	
//	//lwipdev.remoteip[3]=ini_getl("server",	"ip3",	142,inifile);	
//	//printf("lwipdev.remoteip[3]=%d\n",lwipdev.remoteip[3]);
//	
//	
//	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
//	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],200);//Զ��IP
//  //TCP_CLIENT_PORT=ini_getl("PORT",	"port0",	8090,	inifile);	
//	sprintf((char*)tbuf,"Remotewo Port:%d",TCP_CLIENT_PORT);//�ͻ��˶˿ں�
//	tcppcb=tcp_new();	//����һ���µ�pcb
//	if(tcppcb)			//�����ɹ�
//	{
//		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],200); 
//		tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
// 	}else res=1;
//	while(res==0)
//	{
//		if(key==4)break;
//     if(tcp_close_flag&(1<<7))
//		{
//		 tcp_close_flag&=~(1<<7);
////     tcp_client_connection_close(tcppcb,0);//�ر�TCP Client���� 				
//     tcp_close(tcppcb);
//     vTaskResume(cameraTask_Handler);//�ָ�camera����	
//     break;			
//		}				
//		if(tcp_client_flag&1<<6)//�Ƿ��յ�����?		
//		{			
//			printf("recv data *\n");	
//			tcp_client_flag&=~(1<<6);//��������Ѿ���������.
//		}		
//		if(tcp_client_flag&1<<5)//�Ƿ�������?
//		{
//			if(connflag==0)
//			{ 								
//				connflag=1;//���������
//			} 
//		}else if(connflag)
//		{
//			connflag=0;	//������ӶϿ���
//		} 
//		delay_ms(2);
//		t++;
//		if(t==200)
//		{
//			if(connflag==0&&(tcp_client_flag&1<<5)==0)//δ������,��������
//			{ 
//				tcp_client_connection_close(tcppcb,0);//�ر�����
//				tcppcb=tcp_new();	//����һ���µ�pcb
//				if(tcppcb)			//�����ɹ�
//				{ 
//					tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
//				}
//			}
//			t=0;		
//		}		
//	}
//}
