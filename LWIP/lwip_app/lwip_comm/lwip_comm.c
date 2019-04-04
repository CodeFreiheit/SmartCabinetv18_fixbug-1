#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "delay.h"
#include "usart.h"  
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "webserver.h"
#include "minIni.h"
#include "message.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "iwdg.h"

//TaskHandle_t LWIPTask_Handler;//


#define DHCP_THREAD_PRIORITY		tskIDLE_PRIORITY + 2 
#define DHCP_THREAD_STACK_SIZE		128
void dhcp_thread(void *pv);

#define TCPIP_THREAD_PRIORITY		  tskIDLE_PRIORITY + 3
#define TCPIP_THREAD_STACK_SIZE		512//512
TaskHandle_t TcpipThread_Handler;
void tcpip_thread(void *pv);

#define WEB_THREAD_PRIORITY			1
#define WEB_THREAD_STACK_SIZE		320
TaskHandle_t WebThread_Handler;

__lwip_dev lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�

extern u32 memp_get_memorysize(void);	//��memp.c���涨��
extern u8_t *memp_memory;				//��memp.c���涨��.
extern u8_t *ram_heap;					//��mem.c���涨��.
extern xQueueHandle MainTaskQueue;

u32 TCPTimer=0;			//TCP��ѯ��ʱ��
u32 ARPTimer=0;			//ARP��ѯ��ʱ��
u32 lwip_localtime;		//lwip����ʱ�������,��λ:ms

u8 linkState;
char COM_MAC_BUF[10][18];

#if LWIP_DHCP
u32 DHCPfineTimer=0;	//DHCP��ϸ�����ʱ��
u32 DHCPcoarseTimer=0;	//DHCP�ֲڴ����ʱ��
#endif

// lwip��mem��memp���ڴ�����
// ����ֵ:	0,�ɹ�;
//			����,ʧ��
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();
//	memp_memory=mymalloc(SRAMIN,mempsize);
	memp_memory = (u8_t *)pvPortMalloc(mempsize);
	
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;
//	ram_heap=mymalloc(SRAMIN,ramheapsize);
	ram_heap = (u8_t *)pvPortMalloc(ramheapsize);
	
	if(!memp_memory||!ram_heap)
	{
//		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip��mem��memp�ڴ��ͷ�
void lwip_comm_mem_free(void)
{ 	
//	myfree(SRAMIN,memp_memory);
//	myfree(SRAMIN,ram_heap);
}

void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	char inifile[] = "1:cfg.ini";
	char delim[] = " .";
	char *token;
	char buf[16];
	u8 i;
	
	for(i=0; i<6; i++)lwipx->mac[i]=dm9000cfg.mac_addr[i];
	
	ini_gets("local", "ip", "172.28.86.153", buf, 16, inifile);
	for(token=strtok((char*)buf,delim),i=0; (token!=NULL)&&(i<4); token=strtok(NULL, delim),i++)
		lwipx->ip[i]= atol(token);
	
	ini_gets("local", "mask", "255.255.255.240", buf, 16, inifile);
	for(token=strtok((char*)buf,delim),i=0; (token!=NULL)&&(i<4); token=strtok(NULL, delim),i++)
		lwipx->netmask[i]= atol(token);
	
	ini_gets("local", "gw", "172.28.86.145", buf, 16, inifile);
	for(token=strtok((char*)buf,delim),i=0; (token!=NULL)&&(i<4); token=strtok(NULL, delim),i++)
		lwipx->gateway[i]= atol(token);
	
	ini_gets("mqtt", "ip", "120.210.134.49", buf, 16, inifile);
	for(token=strtok((char*)buf,delim),i=0; (token!=NULL)&&(i<4); token=strtok(NULL, delim),i++)
		lwipx->mqttip[i]= atol(token);
	lwipx->mqttport =ini_getl("mqtt", "port", 1883, inifile);
	
	ini_gets("snmp", "ip", "192.168.1.100", buf, 16, inifile);
	for(token=strtok((char*)buf,delim),i=0; (token!=NULL)&&(i<4); token=strtok(NULL, delim),i++)
		lwipx->snmpip[i]= atol(token);
	lwipx->snmpport =ini_getl("snmp", "port", 1883, inifile);
	
	
	ini_gets("serv", "ip", "192.168.1.142", buf, 16, inifile);
	for(token=strtok((char*)buf,delim),i=0; (token!=NULL)&&(i<4); token=strtok(NULL, delim),i++)
		lwipx->servip[i]= atol(token);
	lwipx->servport =ini_getl("serv", "port", 8087, inifile);
}


//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
#if 0
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	char inifile[] = "1:cfg.ini";
	//Ĭ��Զ��IPΪ:192.168.1.100
	lwipx->mqttip[0]=ini_getl("mqtt",	"ip0",	120,	inifile);
	lwipx->mqttip[1]=ini_getl("mqtt",	"ip1",	210,	inifile);
	lwipx->mqttip[2]=ini_getl("mqtt",	"ip2",	134,	inifile);
	lwipx->mqttip[3]=ini_getl("mqtt",	"ip3",	49,		inifile);
	lwipx->mqttport =ini_getl("mqtt",	"port",	1883,	inifile);
	
	ini_gets("mac", "mac1",  "0", COM_MAC_BUF[0], 18, inifile);
	ini_gets("mac", "mac2",  "0", COM_MAC_BUF[1], 18, inifile);
	ini_gets("mac", "mac3",  "0", COM_MAC_BUF[2], 18, inifile);
	ini_gets("mac", "mac4",  "0", COM_MAC_BUF[3], 18, inifile);
	ini_gets("mac", "mac5",  "0", COM_MAC_BUF[4], 18, inifile);
	ini_gets("mac", "mac6",  "0", COM_MAC_BUF[5], 18, inifile);
	ini_gets("mac", "mac7",  "0", COM_MAC_BUF[6], 18, inifile);
	ini_gets("mac", "mac8",  "0", COM_MAC_BUF[7], 18, inifile);
	ini_gets("mac", "mac9",  "0", COM_MAC_BUF[8], 18, inifile);
	ini_gets("mac", "mac10", "0", COM_MAC_BUF[9], 18, inifile);
	
	lwipx->snmpip[0]=ini_getl("snmp",	"ip0",	192,	inifile);
	lwipx->snmpip[1]=ini_getl("snmp",	"ip1",	168,	inifile);
	lwipx->snmpip[2]=ini_getl("snmp",	"ip2",	1,		inifile);
	lwipx->snmpip[3]=ini_getl("snmp",	"ip3",	100,	inifile);
	lwipx->snmpport =ini_getl("snmp",	"port",	1883,	inifile);
	//MAC��ַ����(�����ֽڹ̶�Ϊ:s/2*2.s.i,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//Ĭ�ϱ���IPΪ:192.168.1.55
	lwipx->ip[0]=ini_getl("local",	"ip0",	192,	inifile);	
	lwipx->ip[1]=ini_getl("local",	"ip1",	168,	inifile);
	lwipx->ip[2]=ini_getl("local",	"ip2",	0,		inifile);
	lwipx->ip[3]=ini_getl("local",	"ip3",	55,		inifile);
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=ini_getl("local",	"mask0",	255,	inifile);	
	lwipx->netmask[1]=ini_getl("local",	"mask1",	255,	inifile);
	lwipx->netmask[2]=ini_getl("local",	"mask2",	255,	inifile);
	lwipx->netmask[3]=ini_getl("local",	"mask3",	0,		inifile);
	//Ĭ������:192.168.1.1
	lwipx->gateway[0]=ini_getl("local",	"gw0",	192,	inifile);	
	lwipx->gateway[1]=ini_getl("local",	"gw1",	168,	inifile);
	lwipx->gateway[2]=ini_getl("local",	"gw2",	0,		inifile);
	lwipx->gateway[3]=ini_getl("local",	"gw3",	1,		inifile);	
	lwipx->dhcpstatus=0;//û��DHCP
}
#endif

void Enc_LinkCallbackFunc(struct netif *netif)
{
	MSG msg;
  printf("...... Enc_LinkCallbackFunc......\r\n");
//	msg.Msg = MSG_LINKSTATE_CHANGE;
//	msg.wParam = NETIF_FLAG_LINK_UP;
//	msg.lParam = (u32)netif;
//	xQueueSend(MainTaskQueue, &msg, 0);
}

void Enc_StatusCallbackFunc(struct netif *netif)
{
	MSG msg;
  printf("...... Enc_StatusCallbackFunc......\r\n");
//	msg.Msg = MSG_LINKSTATE_CHANGE;
//	msg.wParam = NETIF_FLAG_UP;
//	msg.lParam = (u32)netif;
//	xQueueSend(MainTaskQueue, &msg, 0);
}

//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,�ڴ����
//      2,DM9000��ʼ��ʧ��
//      3,�������ʧ��.
u8 lwip_comm_init(void)
{
//	u32 cpu_sr;
//	u8 err;
	struct netif *Netif_Init_Flag;
	struct ip_addr ipaddr; 
	struct ip_addr netmask;
	struct ip_addr gw;
	if(lwip_comm_mem_malloc())		return 1;
	
	DM_bSem = xSemaphoreCreateBinary();
	DM_xSem = xSemaphoreCreateMutex();
	if(DM9000_Init())return 2;
	tcpip_init(NULL,NULL);
	lwip_comm_default_ip_set(&lwipdev);

#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//ʹ�þ�̬IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&ethernet_input);
	if(Netif_Init_Flag != NULL)
	{	
/* init netif link callback function */
		//netif_set_link_callback(&lwip_netif, Enc_LinkCallbackFunc);
	  //netif_set_status_callback(&lwip_netif, Enc_StatusCallbackFunc);	
	
		netif_set_default(&lwip_netif);
		netif_set_up(&lwip_netif);		
	}
	taskENTER_CRITICAL();

	sys_thread_new("TCPIP",	tcpip_thread,	NULL,	TCPIP_THREAD_STACK_SIZE,	TCPIP_THREAD_PRIORITY);
//	sys_thread_new("WEB",	web_thread,		NULL,	WEB_THREAD_STACK_SIZE,		WEB_THREAD_PRIORITY);
	
	taskENTER_CRITICAL();
#if	LWIP_DHCP
		lwip_comm_dhcp_creat();		//����DHCP����
#endif
	return 0;//����OK.
}
//���ʹ����DHCP


//�DM9000���ݽ��մ�������
void tcpip_thread(void *pv)
{
	ethernetif_input(&lwip_netif);	

}

//���ʹ����DHCP
#if LWIP_DHCP

//����DHCP����
void lwip_comm_dhcp_creat(void)
{
	taskENTER_CRITICAL();//rtos�����ٽ���
	xTaskCreate((TaskFunction_t )lwip_dhcp_task,     	
                (const char*    )"lwip_dhcp_task",   	
                (uint16_t       )LWIP_DHCP_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )LWIP_DHCP_TASK_PRIO,	
                (TaskHandle_t*  )&LWIP_DHCP_INPUTTask_Handler); 
	taskENTER_CRITICAL();//rtos�˳��ٽ���
}
//ɾ��DHCP����
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&lwip_netif); 		//�ر�DHCP
	//OSTaskDel(LWIP_DHCP_TASK_PRIO);	//ɾ��DHCP����
	vTaskDelete(LWIP_DHCP_INPUTTask_Handler);//ɾ��DHCP����
}
//DHCP��������
void lwip_dhcp_task(void *pdata)
{
	u32 ip=0,netmask=0,gw=0;
	dhcp_start(&lwip_netif);//����DHCP 
	lwipdev.dhcpstatus=0;	//����DHCP
	printf("���ڲ���DHCP������,���Ե�...........\r\n");   
	while(1)
	{ 
		printf("���ڻ�ȡ��ַ...\r\n");
		ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
		netmask=lwip_netif.netmask.addr;//��ȡ��������
		gw=lwip_netif.gw.addr;			//��ȡĬ������ 
		if(ip!=0)   					//����ȷ��ȡ��IP��ַ��ʱ��
		{
			lwipdev.dhcpstatus=2;	//DHCP�ɹ�
 			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//������ͨ��DHCP��ȡ����IP��ַ
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//����ͨ��DHCP��ȡ�������������ַ
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//������ͨ��DHCP��ȡ����Ĭ������
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
		{  
			lwipdev.dhcpstatus=0XFF;//DHCPʧ��.
			//ʹ�þ�̬IP��ַ
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
		delay_ms(250); //��ʱ250ms
	}
	lwip_comm_dhcp_delete();//ɾ��DHCP���� 
}
#endif


//���ʹ����DHCP
#if  LWIP_DHCP

//DHCP��������
void lwip_dhcp_process_handle(void)
{
	u32 ip=0,netmask=0,gw=0;
	switch(lwipdev.dhcpstatus)
	{
		case 0: 	//����DHCP
			dhcp_start(&lwip_netif);
			lwipdev.dhcpstatus = 1;		//�ȴ�ͨ��DHCP��ȡ���ĵ�ַ
			printf("���ڲ���DHCP������,���Ե�...........\r\n");  
			break;
		case 1:		//�ȴ���ȡ��IP��ַ
		{
			ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
			netmask=lwip_netif.netmask.addr;//��ȡ��������
			gw=lwip_netif.gw.addr;			//��ȡĬ������ 
			
			if(ip!=0)			//��ȷ��ȡ��IP��ַ��ʱ��
			{
				lwipdev.dhcpstatus=2;	//DHCP�ɹ�
				printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				//������ͨ��DHCP��ȡ����IP��ַ
				lwipdev.ip[3]=(uint8_t)(ip>>24); 
				lwipdev.ip[2]=(uint8_t)(ip>>16);
				lwipdev.ip[1]=(uint8_t)(ip>>8);
				lwipdev.ip[0]=(uint8_t)(ip);
				printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				//����ͨ��DHCP��ȡ�������������ַ
				lwipdev.netmask[3]=(uint8_t)(netmask>>24);
				lwipdev.netmask[2]=(uint8_t)(netmask>>16);
				lwipdev.netmask[1]=(uint8_t)(netmask>>8);
				lwipdev.netmask[0]=(uint8_t)(netmask);
				printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				//������ͨ��DHCP��ȡ����Ĭ������
				lwipdev.gateway[3]=(uint8_t)(gw>>24);
				lwipdev.gateway[2]=(uint8_t)(gw>>16);
				lwipdev.gateway[1]=(uint8_t)(gw>>8);
				lwipdev.gateway[0]=(uint8_t)(gw);
				printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
			{
				lwipdev.dhcpstatus=0XFF;//DHCP��ʱʧ��.
				//ʹ�þ�̬IP��ַ
				IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
				printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			}
		}
		break;
		default : break;
	}
}
#endif 
