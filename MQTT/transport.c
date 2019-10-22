#include "transport.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "string.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "system_stm32f10x.h"
#include "stdint.h"
#include "print.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

int mysock;
//char *recv_data;

//extern struct sockaddr_in; 
	

/************************************************************************
** ��������: transport_sendPacketBuffer									
** ��������: ��TCP��ʽ��������
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/

s32 transport_sendPacketBuffer( u8* buf, s32 buflen)
{
	s32 rc;
//	printf("buf=%s\n",buf);
	rc = lwip_write(mysock, buf, buflen);
	printf("sendPacketBuffer_rc=%d\n",rc);
	return rc;
}

/************************************************************************
** ��������: transport_getdata									
** ��������: �������ķ�ʽ����TCP����
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int count�����ݳ���
** ���ڲ���: <=0��������ʧ��									
************************************************************************/
 s32 transport_getdata(u8* buf, s32 count)
{
	s32 rc;
	//������������ﲻ����
  rc = lwip_recv(mysock, buf, count, 0);
//	vPortFree(recv_data);
	return rc;
}


/************************************************************************
** ��������: transport_open									
** ��������: ��һ���ӿڣ����Һͷ����� ��������
** ��ڲ���: char* servip:����������
**           int   port:�˿ں�
** ���ڲ���: <0������ʧ��										
************************************************************************/
s32 transport_open(s8* servip, s32 port)
{	
	s32 ret;
	//s32 opt;
	
//	char *recv_data;
	int bytes_received;
	struct sockaddr_in server_addr;
	
	struct sockaddr_in addr;
	s32 *sock = &mysock;

//	/* �������ڴ�����ݵĻ��� */
//	recv_data = (char *)pvPortMalloc(512);
//	if (recv_data == NULL)
//	{
//		Printf(("\nclient_request: no memory"));
//		return -6;
//	}


	
	
	
	
	//��ʼ����������Ϣ
	memset(&addr,0,sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	//��д�������˿ں�
	addr.sin_port = PP_HTONS(port);
	//��д������IP��ַ
	addr.sin_addr.s_addr = inet_addr((const char*)servip);
	
	//����SOCK
	*sock = lwip_socket(AF_INET,SOCK_STREAM,0);
	 if(*sock < 0)
	 {
	  Printf("[ERROR] Create socket failed\n");
	 	/* �ͷ����� */
//		vPortFree(recv_data);
	 }
	 
	 

	 
   
	//���ӷ����� 
	ret = lwip_connect(*sock,(struct sockaddr*)&addr,sizeof(addr));
	if(ret != 0)
	{
		 Printf("\nclient_request: socket connect error");
		 //�ر�����
		 lwip_close(*sock);		
	 	/* �ͷ����� */
//		vPortFree(recv_data);
		 //����ʧ��
		 return -1;
	}
	//���ӳɹ�,���ó�ʱʱ��1000ms
	//opt = 1000;
	//setsockopt(*sock,SOL_SOCKET,SO_RCVTIMEO,&opt,sizeof(int));
	
	//�����׽���
	return *sock;
}


/************************************************************************
** ��������: transport_close									
** ��������: �ر��׽���
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
int transport_close(void)
{
	int rc;
	rc = lwip_close(mysock);
	return rc;
}
