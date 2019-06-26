#include <string.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "usart3.h"
#include "sram.h"
#include "lwip_comm.h"
#include "shell.h"
#include "rtc.h"
#include "dma.h"
#include "mqtt.h"
#include "lwip/netif.h"
#include "print.h"
#include "sensor.h"
#include "gps.h"
#include "oled.h"
#include "adc.h"
#include "led.h"
#include "mmc_sd.h"	
#include "w25qxx.h"
#include "mass_mal.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "memory.h"
#include "usb_bot.h"
#include "usb_int.h"

#include "exfuns.h"
#include "ff.h"

#include "mqtt.h"
#include "MQTTPacket.h"
#include "MQTTGPRSClient.h"

#include "event_groups.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "message.h"


#include "snmp.h"
#include "snmp_msg.h"
#include "private_mib.h"
#include "lwip/snmp_msg.h"

#include "httpd.h"
#include "MD5.h"
#include "camera.h"
#include "usart.h"
#include "MMC_SD.h"
#include "sdio_sdcard.h"
#include "mass_mal.h"

#include "tcp_client_demo.h" 
#include "tcp_server_demo.h" 
#include "minIni.h"
#include "diskio.h"	
#include "mmc_sd.h"	
#include "usb_prop.h"
#include "timer.h"
#include "sensor.h"
#include "iwdg.h"

#include "rtc.h"

#include "dns.h"
//#include "fec.h"
//#include "api_lib.h"

#include "lwip/api.h"

#include "lwip/inet.h"

#include "lwip/ip_addr.h"



#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))
	
extern const char BuildDate[];
extern const char BuildTime[];
extern const char ReleaseVersion[];



extern u16 link_status;
//#define WDG_TASK_PRIORITY			2
//#define WDG_TASK_STACK_SIZE			512
//TaskHandle_t WDGTask_Handler;
//void vTaskWDG(void *pvParameters);

void _WriteTime(u16 year, u8 month, u8 date, u8 hour, u8 minute, u8 second);


//#define MAIN_TASK_PRIORITY			2//8//2//5//2//5//(tskIDLE_PRIORITY + 2)
//#define MAIN_TASK_STACK_SIZE		512
//#define MAIN_QUEUE_SIZE				32
//#define tcp_QUEUE_SIZE				512
//TaskHandle_t MainTask_Handler;
//TaskHandle_t tcpTask_Handler;
//xQueueHandle MainTaskQueue;
//xQueueHandle tcp_client_Queue;
//void MainTask(void *pvParameters);


#define USB_TASK_PRIORITY			4//7//(tskIDLE_PRIORITY + 3)
#define USB_TASK_STACK_SIZE			512
#define USB_QUEUE_SIZE				16
TaskHandle_t USBTask_Handler;
xQueueHandle USBTaskQueue;



void UsbTask(void *pvParameters);

//camera����
#define camera_TASK_PRIO		5//17//16//7//16//15
//�����ջ��С	
 #define camera_STK_SIZE 		1200//512//512//100 
//������
//TaskHandle_t cameraTask_Handler;
//������
void camera_task(void *pvParameters);

//tcp�������ȼ�
#define tcp_TASK_PRIO		6
//�����ջ��С	
#define tcp_STK_SIZE 		2000
//������
TaskHandle_t tcpTask_Handler;
//������
void tcp_task(void *pvParameters);





#define MQTT_TASK_PRIORITY			7//(tskIDLE_PRIORITY + 3)
#define MQTT_TASK_STACK_SIZE		512
#define MQTT_QUEUE_SIZE				32
TaskHandle_t MqttTask_Handler;
xQueueHandle MqttTaskQueue;


#define TICK_TASK_PRIORITY			8//(tskIDLE_PRIORITY + 4)
#define TICK_TASK_STACK_SIZE		128
#define TICK_QUEUE_SIZE				16
TaskHandle_t TickTask_Handler;
xQueueHandle TickTaskQueue;
static void TickTask(void *pParameters);


#define MAIN_TASK_PRIORITY			9//8//2//5//2//5//(tskIDLE_PRIORITY + 2)
#define MAIN_TASK_STACK_SIZE		512
#define MAIN_QUEUE_SIZE				32
#define tcp_QUEUE_SIZE				512
TaskHandle_t MainTask_Handler;
TaskHandle_t tcpTask_Handler;
xQueueHandle MainTaskQueue;
xQueueHandle tcp_client_Queue;
xQueueHandle ServerTaskQueue;
void MainTask(void *pvParameters);



//�������ȼ�
#define IAP_TASK_PRIO		10//12
//�����ջ��С	
#define IAP_STK_SIZE 		15000
//������
//TaskHandle_t IAPTask_Handler;
//������
void iap_task(void *pvParameters);




//tcp server
#define SD_TASK_PRIO		11
//�����ջ��С	
#define SD_STK_SIZE 		512
//������
TaskHandle_t SDTask_Handler;
//������
void sd_task(void *pvParameters);


//tcp server
#define Ping_TASK_PRIO		12
//�����ջ��С	
#define Ping_STK_SIZE 		512
//������
TaskHandle_t PingTask_Handler;
//������
void ping_task(void *pvParameters);


//tcp server  printf task
#define printf_TASK_PRIO		13
//�����ջ��С	
#define printf_STK_SIZE 		512
//������
TaskHandle_t printfTask_Handler;
//������
void server_printf_task(void *pvParameters);



//�������ȼ�
#define trap_TASK_PRIO		13
//�����ջ��С	
#define trap_STK_SIZE 		512//1000//50  
//������
TaskHandle_t trapTask_Handler;
//������
void trap_task(void *pvParameters);


u8 test_sd;
u8 test_w=1;
u8 test_mkfs;	
xSemaphoreHandle USART1_Sem;



int main(void)
{
	char *test_post_bin="1:post.bin";
	char *file_md5_boot="1:file_md5_boot.txt";
  u8 res_sd=10;
	u8 SD_IN=13;
	u8 res_test;	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init();	    				//��ʱ������ʼ��	
	OLED_Init();
 	LED_Init();
	USART1_Init(115200);				//��ʼ������
	USART2_Init(115200);
	USART3_Init(38400);
 	UART4_Init(14400);
	printf("........ SC System Power On .......\r\n");
	printf("........ update v0.6 .......\r\n");
  GPS_Init();	
	SENSOR_Init();	
	Adc_Init();
	Init_Shell();
	FSMC_SRAM_Init();		
	USART1_Sem = xSemaphoreCreateMutex();
	W25QXX_Init();
	exfuns_init();
	
	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)==0) //sd card insert 
  {
	 res_sd=SD_Initialize();
   SD_STAT=0;		
	}
	else
	{
	 SD_STAT=255;//��Ǹտ�ʼSD��״̬
	}

  if(f_mount(fs[1],"1:",1) == 0X0D)f_mkfs("1:",0,4096);//����flash
		
	if(res_sd==0) 
	{
	 res_test=f_mount(fs[0],"0:",1);
	 printf("res_test=%d\n",res_test);
	 if(f_mount(fs[0],"0:",1) == 0X0D)	 
	 f_mkfs("0:",0,4096);//����SD	
	}	
	if(!res_sd)
	{	                                      
	 Mass_Block_Size[1] =512;							//��Ϊ������Init����������SD���Ĳ����ֽ�Ϊ512��,��������һ����512���ֽ�.
	 Mass_Block_Count[1]=0xE6D000;			
	}	
	Mass_Memory_Size[0]=12*1024*1024;		
	Mass_Block_Size[0] =2048;//512;			//����SPI FLASH�Ĳ���������СΪ512
  Mass_Block_Count[0]=1024*6;//0x6000;//24576--512
	
														 	
	lwip_comm_init();
	
	Init_Iwdg();	
	ping_init();//ping ��ʼ��
	
	
	
	httpd_init();  			//Web Serverģʽ
	
 	mf_unlink((u8*)test_post_bin);
  mf_unlink((u8*)file_md5_boot);
	
	DOOR_STAT= DOOR_SENSOR;
	
  output_control_default();	
	
	snmp_init();
	
	dns_init();//dns������ʼ��

	USB_Queue= xQueueCreate(30, sizeof( int16_t ));
	
	MainTaskQueue= xQueueCreate(MAIN_QUEUE_SIZE, sizeof(MSG));
	ServerTaskQueue=xQueueCreate(MAIN_QUEUE_SIZE, sizeof(MSG));
	MsgQueue = xQueueCreate(30, sizeof( int16_t ));
	
	Msg_response= xQueueCreate(30, sizeof( int16_t ));
	
  BinarySemaphore_web=xSemaphoreCreateBinary();//����web�ź���
	BinarySemaphore_camera_Sync=xSemaphoreCreateBinary();
	BinarySemaphore_camera_response=xSemaphoreCreateBinary();//����camera�ź��� 	
	BinarySemaphore_camera=xSemaphoreCreateBinary();//����camera�ź���  
	BinarySemaphore_response=xSemaphoreCreateBinary();//����camera�ź��� 	
	BinarySemaphore_camera_data_response=xSemaphoreCreateBinary();//����camera�ź��� 	
	BinarySemaphore_photo_command=xSemaphoreCreateBinary();
	tcp_command=xSemaphoreCreateBinary();
	mqtt_command=xSemaphoreCreateBinary();//MQTT���ӷ�������־
	
  sd_Sem = xSemaphoreCreateMutex();///* �����ź���  for sd write*/
	test_trap_signal=xSemaphoreCreateBinary();//����test_signal �ź���
	test_tcp_signal=xSemaphoreCreateBinary();//����test_signal �ź���
	
	ping_signal=xSemaphoreCreateBinary();//����test_signal �ź���
	
	printf_signal=xSemaphoreCreateBinary();//���������ӡ �ź���
	
	//Start the main tasks	
	xTaskCreate((TaskFunction_t )UsbTask,
                (const char*    )"USB",
                (uint16_t       )USB_TASK_STACK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )USB_TASK_PRIORITY,
                (TaskHandle_t*  )&USBTask_Handler);
	xTaskCreate((TaskFunction_t )MainTask,				//������
                (const char*    )"MAIN",				//��������
                (uint16_t       )MAIN_TASK_STACK_SIZE,	//�����ջ��С
                (void*          )NULL,					//���ݸ��������Ĳ���
                (UBaseType_t    )MAIN_TASK_PRIORITY,	//�������ȼ�
                (TaskHandle_t*  )&MainTask_Handler);	//������	
	xTaskCreate((TaskFunction_t )TickTask,
                (const char*    )"TICK",
                (uint16_t       )TICK_TASK_STACK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TICK_TASK_PRIORITY,
                (TaskHandle_t*  )&TickTask_Handler);									
	xTaskCreate((TaskFunction_t )MqttTask,
                (const char*    )"MQTT",
                (uint16_t       )MQTT_TASK_STACK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )MQTT_TASK_PRIORITY,
                (TaskHandle_t*  )&MqttTask_Handler);
//   //����IAP���� webǰ����������  
	 xTaskCreate((TaskFunction_t )iap_task,     
                (const char*    )"iap_task",   
                (uint16_t       )IAP_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )IAP_TASK_PRIO,
                (TaskHandle_t*  )&IAPTask_Handler);
   //����camera����  
    xTaskCreate((TaskFunction_t )camera_task,     
                (const char*    )"camera_task",   
                (uint16_t       )camera_STK_SIZE, 
                 (void*          )NULL,
                (UBaseType_t    )camera_TASK_PRIO,								
                (TaskHandle_t*  )&cameraTask_Handler);								 
	   xTaskCreate((TaskFunction_t )tcp_task,     	
                (const char*    )"tcp_task",   	
                (uint16_t       )tcp_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )tcp_TASK_PRIO,	
                (TaskHandle_t*  )&tcpTask_Handler);
     xTaskCreate((TaskFunction_t )sd_task,     	
                (const char*    )"sd_task",   	
                (uint16_t       )SD_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )SD_TASK_PRIO,	
                (TaskHandle_t*  )&SDTask_Handler);
     xTaskCreate((TaskFunction_t )ping_task,     	
                (const char*    )"ping_task",   	
                (uint16_t       )Ping_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )Ping_TASK_PRIO,	
                (TaskHandle_t*  )&PingTask_Handler);
	   xTaskCreate((TaskFunction_t )server_printf_task,     	
                (const char*    )"server_printf_task",   	
                (uint16_t       )printf_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )printf_TASK_PRIO,	
                (TaskHandle_t*  )&printfTask_Handler);
//   //����TAP����  �����ϱ�����֡
//   xTaskCreate((TaskFunction_t )trap_task,     	
//                (const char*    )"trap_task",   	
//                (uint16_t       )trap_STK_SIZE, 
//                (void*          )NULL,				
//                (UBaseType_t    )trap_TASK_PRIO,	
//                (TaskHandle_t*  )&trapTask_Handler);									
	//Start the scheduler
	vTaskStartScheduler();	
	//Will only get here if there was not enough heap space to create the idle task
	while (1);
}



/*
 * ��������void MainTask(void *pParameters)
 * ����  ��
 * ����  ����
 * ���  ����	
 */
void MainTask(void *pParameters)
{
	MSG msg;
	u8 page;
	u8 i;
	u8 Flag_reset=0;
	u8 test_water=10;	
	while (1)
	{				
	 if(xQueueReceive(MainTaskQueue, &msg, portMAX_DELAY) == pdPASS)
	 switch (msg.Msg)
	 {			
		case MSG_TICK_1_SECOND:
		{
			LED_MCU = !LED_MCU;						
			break;
		}
		case MSG_TICK_3_SECOND:
		{
			page++;
			LIGHT_SENSOR_CHECK();			
			break;
		}
		case MSG_TICK_5_SECOND:
		{
			if(USART3_RX_STA & 0X8000)
			{
				if(NMEA_GNRMC_Analysis(&gpsxSC, USART3_RX_BUF)==0)
				if(gpsxSC.latitude!=0&&gpsxSC.longitude!=0)
				USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);				
			  if(calendar.hour==1&&Flag_reset)
				{
				 NVIC_SystemReset();
				}
				if(calendar.hour==2)
				{
				 Flag_reset=1;
				}
				USART3_RX_STA = 0;
			}
      read_Flooding();//ˮ��	
      Get_Vol();		// ��ѹ
			Get_Cur();		// ����			
   		STAT_CHECK();	// ������״̬			
			break;
		}
		case MSG_TICK_30_SECOND:
		{
			Printf("Free Size:     %d\r\n", xPortGetFreeHeapSize());
			Printf("Min Free Size: %d\r\n", xPortGetMinimumEverFreeHeapSize());
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);// �ָ�����GPS
			break;
		} 		
		case MSG_TICK_60_SECOND:
		{ 
			break;
		} 
		case MSG_UART1_RX_BYTE:
		{
			break;
		}
		case MSG_UART2_RX_FRAME:
		{
			break;
		}
		case MSG_LINKSTATE_CHANGE:
		{
			break;
		}
		case MSG_TEMPERATURE_CHANGE:
		{
			break;
		}
		default:
			break;
	 }	 
	}
}



/*
 * ��������static void TickTask(void *pParameters)
 * ����  ����ʱ����
 * ����  ����
 * ���  ����	
 */
static void TickTask(void *pParameters)
{
	portTickType xLastExecutionTime;
	MSG msg;
	u32 counts;
	//u8 test_sd=10;
	static u32 RtcCounter = 0;
	xLastExecutionTime = xTaskGetTickCount();
	while(1)
	{
		//vTaskDelayUntil(&xLastExecutionTime, configTICK_RATE_HZ / SYS_TICK_RATE_HZ);	// 10ms
		vTaskDelayUntil(&xLastExecutionTime, 10);	// 10ms
			
		counts++;	
    //DOOR_SENSOR_CHECK(); 		
		// 1��
		if((counts%92)==0)		// 1s
		{			
			msg.Msg = MSG_TICK_1_SECOND;
			xQueueSend(MainTaskQueue, &msg, 0);
		}		
		if((counts%300)==0)		// 3s
		{			
			msg.Msg = MSG_TICK_3_SECOND;
			xQueueSend(MainTaskQueue, &msg, 0);
		}		
		if((counts%500)==0)		// 5s
		{
			// ι��
      IWDG_ReloadCounter();	
			msg.Msg = MSG_TICK_5_SECOND;
			xQueueSend(MainTaskQueue, &msg, 0);
		}		
		if((counts%3000)==0)	//30s
		{
			msg.Msg = MSG_TICK_30_SECOND;
			xQueueSend(MainTaskQueue, &msg, 0);
		}
		if((counts%6000)==0)	//60s
		{			
			msg.Msg = MSG_TICK_60_SECOND;
			xQueueSend(MainTaskQueue, &msg, 0);
		}		
	}
}




