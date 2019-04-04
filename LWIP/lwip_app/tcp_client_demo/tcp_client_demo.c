
#include "tcp_client_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "stdio.h"
#include "string.h" 

#include "FreeRTOS.h"
#include "semphr.h"
#include "netif/etharp.h"
#include "MQTTGPRSClient.h"
#include "minIni.h"
#include "camera.h"

#include "lwip_comm.h" 
#include "message.h"
#include "rtc.h"
#include <cstdlib>
#include <cstdio>
#include "lwip/api.h"
#include "rtc.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ս�������� V3
//TCP Client ���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/3/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   

char  timestamp_str[20];

u8  Msg_Buf[512];	   //DMA���մ������ݻ�����
 
//TCP Client�������ݻ�����
u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	
//TCP������������������
//const u8 *tcp_client_sendbuf="WarShip STM32F103 TCP Client send data\r\n";//
//const u8 *tcp_client_sendbuf=DMA_Rece_Buf;
//TCP Client ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�������Ϸ�����;1,�����Ϸ�������.
//bit4~0:����

u8 tcp_client_init;	//
u8 tcp_client_flag;	//
u8 tcp_client_test_flag=0;
u8 timestamp_flag;//ʱ��� 

u8 tcp_close_flag;	//
 xQueueHandle timestamp_Queue;
 
extern  TaskHandle_t USBTask_Handler;

//����Զ��IP��ַ
void tcp_client_set_remoteip(void)
{
	u8 *tbuf;
	u16 xoff;
	u8 key;	
	tbuf=(u8_t *)pvPortMalloc(100);
	if(tbuf==NULL)return;
	//ǰ����IP���ֺ�DHCP�õ���IPһ��	
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 	
	//lwipdev.remoteip[3]=ini_getl("server",	"ip3",	142,inifile);	
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//Զ��IP
	xoff=strlen((char*)tbuf)*8+30; 
	while(1)
	{
		 break;
	}
}

//TCP Client ����
void tcp_client_test(void)
{
 	struct tcp_pcb *tcppcb;  	//����һ��TCP���������ƿ�
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
  char inifile[] = "1:cfg.ini";  
	MSG msg;
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
	u8 connflag=0;		//���ӱ��	
	int data_len;
	static int Count_packet=0;	
	tcp_client_set_remoteip();//��ѡ��IP
	tbuf=(u8_t *)pvPortMalloc(200);
	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
	
	//lwipdev.remoteip[3]=ini_getl("server",	"ip3",	142,inifile);	
	//printf("lwipdev.remoteip[3]=%d\n",lwipdev.remoteip[3]);
	
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],142);//Զ��IP
  //TCP_CLIENT_PORT=ini_getl("PORT",	"port0",	8090,	inifile);	
	sprintf((char*)tbuf,"Remotewo Port:%d",TCP_CLIENT_PORT);//�ͻ��˶˿ں�
	tcppcb=tcp_new();	//����һ���µ�pcb
	if(tcppcb)			//�����ɹ�
	{
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],142); 
		tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
 	}else res=1;
	while(res==0)
	{
		if(key==4)break;
     if(tcp_close_flag&(1<<7))
		{
		 tcp_close_flag&=~(1<<7);
//     tcp_client_connection_close(tcppcb,0);//�ر�TCP Client���� 				
     tcp_close(tcppcb);
     vTaskResume(cameraTask_Handler);//�ָ�camera����	
     break;			
		}				
		if(tcp_client_flag&1<<6)//�Ƿ��յ�����?		
		{			
			printf("recv data *\n");	
			tcp_client_flag&=~(1<<6);//��������Ѿ���������.
		}		
		if(tcp_client_flag&1<<5)//�Ƿ�������?
		{
			if(connflag==0)
			{ 								
				connflag=1;//���������
			} 
		}else if(connflag)
		{
			connflag=0;	//������ӶϿ���
		} 
		delay_ms(2);
		t++;
		if(t==200)
		{
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//δ������,��������
			{ 
				tcp_client_connection_close(tcppcb,0);//�ر�����
				tcppcb=tcp_new();	//����һ���µ�pcb
				if(tcppcb)			//�����ɹ�
				{ 
					tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
				}
			}
			t=0;		
		}		
	}
	//tcp_client_connection_close(tcppcb,0);//�ر�TCP Client����
} 

//lwIP TCP���ӽ�������ûص�����
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //�����ڴ�
		if(es) //�ڴ�����ɹ�
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
			es->pcb=tpcb;  
			es->p=NULL; 
			tcp_arg(tpcb,es);        			//ʹ��es����tpcb��callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//��ʼ��LwIP��tcp_recv�ص�����   
			tcp_err(tpcb,tcp_client_error); 	//��ʼ��tcp_err()�ص�����
			tcp_sent(tpcb,tcp_client_sent);		//��ʼ��LwIP��tcp_sent�ص�����
		  tcp_poll(tpcb,tcp_client_poll,1); 	//��ʼ��LwIP��tcp_poll�ص����� 			
 			tcp_client_flag|=1<<5; 				//������ӵ���������
			err=ERR_OK;
		}else
		{ 
			tcp_client_connection_close(tpcb,es);//�ر�����
			err=ERR_MEM;	//�����ڴ�������
		}
	}else
	{
		tcp_client_connection_close(tpcb,0);//�ر�����
	}
	return err;
}
    
//lwIP tcp_recv()�����Ļص�����
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len = 0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
	if(p==NULL)//����ӷ��������յ��յ�����֡�͹ر�����
	{
		es->state=ES_TCPCLIENT_CLOSING;//��Ҫ�ر�TCP ������ 
 		es->p=p; 
		ret_err=ERR_OK;
	}else if(err!= ERR_OK)//�����յ�һ���ǿյ�����֡,����err!=ERR_OK
	{ 
		if(p)pbuf_free(p);//�ͷŽ���pbuf
		ret_err=err;
	}else if(es->state==ES_TCPCLIENT_CONNECTED)	//����������״̬ʱ
	{
		if(p!=NULL)//����������״̬���ҽ��յ������ݲ�Ϊ��ʱ
		{
			memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
				//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
				else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
			}
			tcp_client_flag|=1<<6;		//��ǽ��յ�������
 			tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
			pbuf_free(p);  	//�ͷ��ڴ�
			ret_err=ERR_OK;
		}
	}else  //���յ����ݵ��������Ѿ��ر�,
	{ 
		tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
		es->p=NULL;
		pbuf_free(p); //�ͷ��ڴ�
		ret_err=ERR_OK;
	}
	return ret_err;
}
//lwIP tcp_err�����Ļص�����
void tcp_client_error(void *arg,err_t err)
{  
	//�������ǲ����κδ���
} 

//lwIP tcp_poll�Ļص�����
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	int data_len;
	static int count_tcp;
	static int recv_meg;
	struct tcp_client_struct *es;	
	MSG msg;
	es=(struct tcp_client_struct*)arg;
	if(es!=NULL)  //���Ӵ��ڿ��п��Է�������
	{
		if(timestamp_flag&(1<<7))  //����ʱ���
		{
			static u32 RtcCounter = 0;
			u32 timestamp_buf[10];
			char  string[25];
			char  str[25];
			char topic[20];
			timestamp_buf[0]='@';
			timestamp_buf[1]='@';
		  timestamp_buf[2]=GPS_mktime(&calendar);//RTC_GetTime(NULL);	
      timestamp_buf[3]='*';	
      timestamp_buf[4]='*';	
			sprintf(topic, "%02X%02X%02X", STM32ID2, STM32ID1, STM32ID0);//����ID
			snprintf(str,11, "%d",  timestamp_buf[2]);
			memset(timestamp_str,0,20);
			snprintf(timestamp_str,18, "%d:%d.%s",0,timestamp_buf[2],"jpeg");//·��+ʱ���
			memset(string,0,25);
      snprintf(string,22, "%c%c%s,%s%c%c", timestamp_buf[0],timestamp_buf[1],str,topic,timestamp_buf[3],timestamp_buf[4]);					
			es->p=pbuf_alloc(PBUF_TRANSPORT, 21,PBUF_POOL);	//�����ڴ� 
			pbuf_take(es->p,string,21);	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��
			tcp_client_senddata(tpcb,es);//��tcp_client_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
			timestamp_flag&=~(1<<7);	//������ݷ��ͱ�־			 
			if(es->p)pbuf_free(es->p);	//�ͷ��ڴ�
			
			write_sd_test(0);//������ʱ���������ͼƬ��		
			vTaskResume(cameraTask_Handler);	
		} 
		if(tcp_client_flag&(1<<7))	//�ж��Ƿ�������Ҫ����
		{			
		  data_len=(DMA_Rece_Buf[3]<<8)|(DMA_Rece_Buf[2]&0x00ff);
			//write_sd_test(10);//ͼƬд��SD��	
			es->p=pbuf_alloc(PBUF_TRANSPORT, data_len,PBUF_POOL);	//�����ڴ� 
			pbuf_take(es->p,(char*)&DMA_Rece_Buf[4],data_len);	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��
			tcp_client_senddata(tpcb,es);//��tcp_client_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
 
			tcp_client_flag&=~(1<<7);	//������ݷ��ͱ�־
			Uart4_Over=0;			
			
			write_sd_test(10);//ͼƬд��SD��  �ŵ�����  TCP���Դ�  ����д������SD����	
      vTaskResume(cameraTask_Handler);			
			printf("TCP  transport over\n");
			if(es->p)pbuf_free(es->p);	//�ͷ��ڴ�			
		}      	  
		
		else if(es->state==ES_TCPCLIENT_CLOSING)
		{ 
 			tcp_client_connection_close(tpcb,es);//�ر�TCP����
		} 
		ret_err=ERR_OK;
	}else
	{ 
		tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
		ret_err=ERR_ABRT;
	}
	return ret_err;
} 
//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//��������
	return ERR_OK;
}
//�˺���������������
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb)))
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1); //��Ҫ���͵����ݼ��뵽���ͻ��������
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//ָ����һ��pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf��ref��һ
			pbuf_free(ptr);				//�ͷ�ptr 
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//�����ͻ�������е������������ͳ�ȥ
	} 	
} 
//�ر��������������
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	//�Ƴ��ص�
	tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
	tcp_arg(tpcb,NULL);  
	tcp_recv(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);  
	if(es)mem_free(es); 
	tcp_client_flag&=~(1<<5);//������ӶϿ���
		 
//  vTaskResume(USBTask_Handler);	
//  vTaskResume(TickTask_Handler);
//  vTaskResume(MainTask_Handler);
	
	vTaskResume(cameraTask_Handler);//�ָ�camera����
}





