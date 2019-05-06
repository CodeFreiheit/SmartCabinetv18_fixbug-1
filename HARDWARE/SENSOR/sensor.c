#include "stm32f10x.h"
#include "sys.h" 
#include "delay.h"
#include "usart.h"
#include "sensor.h"

#include "FreeRTOS.h"
#include "task.h"
#include "camera.h"
#include "semphr.h"

#include "mmc_sd.h"
#include "diskio.h"
#include "message.h"
#include "ff.h"
#include "mass_mal.h"
#include "diskio.h"	
#include "exfuns.h"
#include "usb_prop.h"
#include "oled.h"
#include "mmc_sd.h"	
#include "spi1.h"
#include "stm32f10x_iwdg.h"
#include "tcp_client_demo.h" 
#include "hw_config.h"
#include "memory.h"
#include "usb_init.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "STM32F10x_IWDG.h"
#include "iwdg.h"
#include "httpd.h"
#include <string.h>
#include <stdio.h>
#include "usb_regs.h"
#include "mqtt.h"
#include "math.h"
#include "trap.h"

#include "stdlib.h"


#define	USB_EP_NUM	4

#define EP_BUF_ADDR (sizeof(EP_BUF_DSCR)*USB_EP_NUM)  

#define Change_TEM  5//10
#define Change_VOL  10//10

//EP_BUF_DSCR *EP0_DSCR = (EP_BUF_DSCR *) 0x40006000;

/*USB??????????????,????*/  
EP_BUF_DSCR * pBUF_DSCR = (EP_BUF_DSCR *) 0x40006000; 

float TEM = 0, HUM = 0;
u16 temp=0;
//float  HUM = 0;
u8 DOOR_STAT=1, WATER_STAT, SYS12_STAT, BAK12_STAT, BAT12_STAT,UPS_STAT,AC24_STAT;
u8 AC1_STAT, AC2_STAT, AC3_STAT,fan_STAT, alarm_STAT, light_STAT,heat_STAT,DC1_STAT,DC2_STAT,DC3_STAT,DC4_STAT,out_special_STAT;
u8 NET_STAT;
u8 IS_EQU_SYS12V, IS_EQU_UPS12V;
u8 stat_changed = 0;
u8 tem_stat_changed=0,vol_stat_changed= 0,sys12_stat_changed=0;

u8 snmp_tem_changed=0,snmp_vol_changed= 0;

u8 bak12_stat_changed=0,ups_stat_changed= 0,ac24_stat_changed=0;
u32 check_stat_times;
u8 TEM_STAT=0;
u8 VOL_STAT=0;

int SD_STAT;

//union data_TEM 
//{
// char c[4];
// float TEM;
//} union_TEM;

void USB_CTR_Handler(void);
extern xQueueHandle MainTaskQueue;
extern xQueueHandle USBTaskQueue;
extern TaskHandle_t USBTask_Handler;



// ��������IO���Ƴ�ʼ��
void SENSOR_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOE| RCC_APB2Periph_GPIOF, ENABLE);
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);
	
	// PC3	SD��SD_IN �������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;  //��������  insert-0   no-insert--1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //
	GPIO_Init(GPIOC,&GPIO_InitStructure); 
	// PC7	�ⲿ�ж�����
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8;
//	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;  //��������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //
//	GPIO_Init(GPIOC,&GPIO_InitStructure);  
	
	// PD2	�Ž�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;  //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //
	GPIO_Init(GPIOD,&GPIO_InitStructure);  
//	
	// PA8	Զ�̿���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		// ����
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	

  // PB7	ˮ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		// ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PC12	��ʪ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	// ��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	// PG12	����Դ ��Դ1 ֱ��12V
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		// ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	// PD3	��Դ2
	// PD6	UPS״̬���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		// ��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	// PB9	�����Դ״̬���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		// ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	

	
	// PB6	�����ԴV4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	// �������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
//	// PB7	ˮ��
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		// ��������
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PE2	V3
	// PE3	V2
	// PE4	V1
		
	// PE6 C4	 fan
	// PE5 C5  alarm
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;	    		 //LED1-->PE.5 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	GPIO_SetBits(GPIOE,GPIO_Pin_2);//V3  Ĭ�Ͽ�	
	GPIO_SetBits(GPIOE,GPIO_Pin_3);//V2  Ĭ�Ͽ�	
	GPIO_SetBits(GPIOE,GPIO_Pin_4);//V1  Ĭ�Ͽ�	
	GPIO_SetBits(GPIOE,GPIO_Pin_6);//�ط���
	
	
	GPIO_SetBits(GPIOE,GPIO_Pin_5);						 //������  Ĭ�Ϲ�

	
	// PF6 C1	AC1
	// PF7 C2	AC2
	// PF9 C3 AC3
	// PF8	����
	
	// PF10	���ȿ���
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	// �������
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	
	GPIO_ResetBits(GPIOF,GPIO_Pin_6);						 //AC1  Ĭ�Ͽ�					 
	GPIO_ResetBits(GPIOF,GPIO_Pin_7);						 //AC2  Ĭ�Ͽ�	
	GPIO_ResetBits(GPIOF,GPIO_Pin_9);						 //AC3  Ĭ�Ͽ�					 
	
  GPIO_SetBits(GPIOF,GPIO_Pin_8);						 //F8  ����Ĭ�Ϲ�
	
	
	
	
	
	//��������
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	// �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
 // PA12  D+����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	// �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
			
	// PA11  D-����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	// �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
			
}

u8 Read_SensorData(void)
{
	u8 i, count, dat, Rev_Bit;
	
	for(i=0; i<8; i++)
	{
		count = 1;
		while(TEM_HUM_SENSOR&&(count++))delay_us(1);	// �ȴ��źŸߵ�ƽ����
		count = 1;
		while(!TEM_HUM_SENSOR&&(count++))delay_us(1);	// �źŵ͵�ƽʱ��48-55us
		delay_us(50);									// �źŸߵ�ƽʱ��"0"22-30us,"1"68-75us
		if(TEM_HUM_SENSOR)Rev_Bit = 1;
		else Rev_Bit = 0;
		dat <<= 1;
		dat |= Rev_Bit;
	}
	return (dat);
}

void GET_AM2301_Data(void)
{
	u8 count, i, Sensor_Check = 0;
	u8 Sensor_Data[4];
	
	AM2301_Write_0();delay_us(1000);	// ������ʼ�ź�����ʱ��800-2000us
	AM2301_Write_1();delay_us(30);		// �����ͷ�����ʱ��20-200us
	if(!TEM_HUM_SENSOR)					// �յ�Ӧ���ź�˵��ͨ�ųɹ�
	{
		count = 1;
		while(!TEM_HUM_SENSOR&&(count++))delay_us(1);	// ��Ӧ�͵�ƽʱ��75-85us
		count = 1;
		while(TEM_HUM_SENSOR&&(count++))delay_us(1);	// ��Ӧ�ߵ�ƽʱ��75-85us
		
		// ���ݽ���
		for(i=0; i<4; i++)
		{
			Sensor_Data[i] = Read_SensorData();
			Sensor_Check += Sensor_Data[i];
		}
		if(Sensor_Check == Read_SensorData())
		{
			HUM = Sensor_Data[0]*256 + Sensor_Data[1];
			HUM = HUM/10;
			TEM = Sensor_Data[2]*256 + Sensor_Data[3];
      if(TEM >= 0x8000)TEM = 0-(TEM-0x8000)/10;
			else TEM = TEM/10;			
			if(TEM > 42)
			{
				out_fan_ON();
				fan_STAT=1;
			}
			if(TEM < 40)
			{
				out_fan_OFF();
				fan_STAT=0;
			}
			
			if(TEM > 2)
			{
			  heat_OFF();//
			  heat_STAT=0;
			}
			if(TEM < 0)
			{
				heat_ON();
				heat_STAT=1;
			}
		}
	}
}

void STAT_CHECK(void)
{	


//  objid[0]=objid_vol;
//	changed_trap(&objid[0],VOL);
//	
//	objid[1]=objid_cur;
//	changed_trap(&objid[0],CUR);
	
	if(WATER_STAT != WATER_SENSOR)  //ˮ��״̬�ı�����
	{
		delay_ms(10);
		if(WATER_STAT != WATER_SENSOR)
		{
			WATER_STAT = WATER_SENSOR;
			stat_changed = 1;
		}
	}		
	
	if(SYS12_STAT != SYS12_SENSOR) //��Դ1 ֱ��12V  
	{
		delay_ms(10);
		if(SYS12_STAT != SYS12_SENSOR)
		{
			SYS12_STAT = SYS12_SENSOR;
			stat_changed = 1;
			sys12_stat_changed=1;
		}
	}
	
	if(BAK12_STAT != BAK12_SENSOR) //��Դ2 
	{
		delay_ms(10);
		if(BAK12_STAT != BAK12_SENSOR)
		{
			BAK12_STAT = BAK12_SENSOR;			
			stat_changed = 1;
			bak12_stat_changed=1;
		}
	}
	
	if(UPS_STAT != UPS12_SENSOR)  //UPS_STA
	{
		delay_ms(10);
		if(UPS_STAT != UPS12_SENSOR)
		{
			UPS_STAT = UPS12_SENSOR;
			stat_changed = 1;
			ups_stat_changed=1;
		}
	}	
	if(AC24_STAT != AC24_SENSOR)  //�����Դ
	{
		delay_ms(10);
		if(AC24_STAT != AC24_SENSOR)
		{
			AC24_STAT = AC24_SENSOR;
			stat_changed = 1;
			ac24_stat_changed=1;
		}
	}		


	vTaskSuspendAll();
	GET_AM2301_Data();
	xTaskResumeAll();

  if((abs)(TEM_STAT-TEM)>=Change_TEM)  //�¶� �仯 20% 5�� 
	{
		delay_ms(10);
		if((abs)(TEM_STAT-TEM)>Change_TEM)
		{
			TEM_STAT = TEM;
			tem_stat_changed = 1;
			snmp_tem_changed=1;
		}
	}	
	if((abs)(VOL_STAT-VOL)>=Change_VOL)  //��ѹ �仯 10V 20V
	{
		delay_ms(10);
		if((abs)(VOL_STAT-VOL)>Change_VOL)
		{
			VOL_STAT = VOL;
			vol_stat_changed = 1;
		  snmp_vol_changed=1;
		}
	}	
}

/*
 * ��������void DOOR_SENSOR_CHECK(void)
 * ����  ���ſ��ض�ʱ���
 * ����  ����
 * ���  ����	
 */
void DOOR_SENSOR_CHECK(void) //�ſ��ؽӿ�ɨ�躯��
{
	if(DOOR_STAT != DOOR_SENSOR)
	{
		delay_ms(10);
		if(DOOR_STAT != DOOR_SENSOR)
		{
			DOOR_STAT = DOOR_SENSOR;					
			stat_changed = 1;
			if(DOOR_SENSOR==1)//����
			{					
				light_ON();
				light_STAT=1;
				xSemaphoreGive(BinarySemaphore_photo_command);		
			}
			else
			{
			 light_OFF();
			 light_STAT=0;
			}						
		}
	}
}

/*
 * ��������void LIGHT_SENSOR_CHECK(void)
 * ����  ���ſ��ض�ʱ���
 * ����  ����
 * ���  ����	
 */
void LIGHT_SENSOR_CHECK(void)
{
	if(DOOR_SENSOR==0)//������ǿ��ŵ�
	{					
		light_ON();
		light_STAT=1;
	}
	if(DOOR_SENSOR==1)//������ǹ��ŵ�
	{					
		light_OFF();
		light_STAT=0;
	}
}
     


void sd_out_config(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
  // PA4	SD��CS����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;  //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //
	GPIO_Init(GPIOA,&GPIO_InitStructure);  
}


void sd_ipd_config(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
  // PA4	SD��CS����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPD;  //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //
	GPIO_Init(GPIOA,&GPIO_InitStructure);  
}





/*
 * ��������main ������ 
 * ����  ��
 * ����  ����
 * ���  ����	
 */
void tcp_task(void *pvParameters)
{ 	
	for(;;)
	{	
	 if(timestamp_flag&(1<<7))
   {     	 
		tcp_client_test();		 
	 }		 	 	 
	 if(tcp_client_flag&(1<<7))  //����ʱ���
	 {		  
	  tcp_client_test();  
	 }
	 if(tcp_close_flag&(1<<7))  //�ر�TCP����
	 {
	  tcp_client_test(); 
	 }
   vTaskDelay(10);
	}
}


/*
 * ��������void sd_task(void *pvParameters)
 * ����  ��Sd���������
 * ����  ����
 * ���  ����	
 */
void sd_task(void *pvParameters)
{	
	for(;;)
	{	
	 SD_SENSOR_CHECK();		 		 	 	  		
	 vTaskDelay(500);     	
	}
}
/*
 * ��������void ping_task(void *pvParameters)
 * ����  ��ping�����
 * ����  ����
 * ���  ����	
 */
void ping_task(void *pvParameters)
{
	u8 pingFailedTimes[10];//++;
	static u8 counts,Ping_frequency;
	while(1)
	{	
		for(Ping_frequency=0;Ping_frequency<4;Ping_frequency++)
		{
		 for(counts=0;counts<MAX_IP;counts++)
		 {
			if(pingCmdSend(counts))
			{
				if(xSemaphoreTake(ping_signal, 400)==pdTRUE)
				{
				 memcpy(IP_STAT[counts],CHECK_IP[counts],strlen(CHECK_IP[counts]));
				 //printf("%d:Ping sucess\n",counts);		 		
				}
				else
				{
				 //printf("%d:ping Timeout\n",counts);
				 pingFailedTimes[counts]++;
				 if(pingFailedTimes[counts]==4)
				 {
				 //printf("%d:ping Timeout  ErrCode 4!\n",counts);			
				 strcpy(IP_STAT[counts], "0");
				 pingFailedTimes[counts]=0;
				 }						
				}
		  }
			else
			{
			 //printf("ping Timeout\n");
			 pingFailedTimes[counts]++;
			 if(pingFailedTimes[counts]==4)
			 {
				//printf("%d:ping Timeout  ErrCode 4!\n",counts);			
				strcpy(IP_STAT[counts], "0");
				pingFailedTimes[counts]=0;
			 }		
			}
      vTaskDelay(10);	 			
		 }
		}
    vTaskDelay(2000);	 
	}
}


/*
 * ��������void SD_SENSOR_CHECK(void)
 * ����  ��Sd����⺯��
 * ����  ����
 * ���  ����	
 */
void SD_SENSOR_CHECK(void) //SD����ѯ���
{	
	u16 temp=0;
	//temp=SD_Initialize();
	temp=GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);
	if(SD_STAT!= temp)
	{
		delay_ms(10);
		//temp=SD_Initialize();
		temp=GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);
		if(SD_STAT != temp)
		{
			SD_STAT = temp;					
			stat_changed = 1;
			if(temp==0)//SD����λ
			{	
       Max_Lun=1;
       SD_Initialize();				
			 if(f_mount(fs[0],"0:",1) == 0X0D) //����SD	
			 f_mkfs("0:",0,4096);
			 Mass_Block_Size[1] =512;						
			 Mass_Block_Count[1]=0xE6D000;
			 PAout(12)=0;
			 GPIO_ResetBits(GPIOA,GPIO_Pin_12);		                                            
			 delay_ms(65);   
			 GPIO_SetBits(GPIOA,GPIO_Pin_12);	
			 delay_ms(65);
	     USB_Init();
			}
			else 
			{
			 Max_Lun=0;
			 f_mount(NULL, "0:", 0);        // ж��SD��
			 Mass_Block_Size[1] =0;							
			 Mass_Block_Count[1]=0;	
			}						
		}
	}
}


/*
 * ��������void SD_state_machine_CHECK(void) //SD����ѯ���
 * ����  ��״̬�����
 * ����  ����
 * ���  ����	
 */
void SD_state_machine_CHECK(void) //SD����ѯ���
{
  u16 cur_state=0;
	u16 temp;
	temp=SD_Initialize();
	cur_state = SD_STAT;   
	switch(cur_state) //
	{            
	case 0: 
			if(temp==255) //
			{		
			 SD_STAT=255;  //
			 Max_Lun=0;
			 f_mount(NULL, "0", 0);        // ж��SD��
			 Mass_Block_Size[1] =0;							
			 Mass_Block_Count[1]=0;	
			} 
			else if(temp==170) //
			{   
			 SD_STAT=170;
			 Max_Lun=0;
			 f_mount(NULL, "0", 0);        // ж��SD��
			 Mass_Block_Size[1] =0;							
			 Mass_Block_Count[1]=0;	
			}    
      else if(temp==0) //
			{   
			 SD_STAT =0;
			 Max_Lun=1;
       SD_Initialize();				
			 if(f_mount(fs[0],"0:",1) == 0X0D) //����SD	
			 f_mkfs("0:",0,4096);
			 Mass_Block_Size[1] =512;						
			 Mass_Block_Count[1]=0xE6D000;
			}  			
	case 255: 
			if(temp==0) //
			{                
			 SD_STAT =0;
			 Max_Lun=1;
       SD_Initialize();				
			 if(f_mount(fs[0],"0:",1) == 0X0D) //����SD	
			 f_mkfs("0:",0,4096);
			 Mass_Block_Size[1] =512;						
			 Mass_Block_Count[1]=0xE6D000;
			}           
			else if(temp==170)
			{
			 SD_STAT=170;
			 Max_Lun=0;
			 f_mount(NULL, "0", 0);        // ж��SD��
			 Mass_Block_Size[1] =0;							
			 Mass_Block_Count[1]=0;	
			}
	case 170: 
			if(temp==170)  
			{          
			 SD_STAT=170;
			 Max_Lun=0;
			 f_mount(NULL, "0", 0);        // ж��SD��
			 Mass_Block_Size[1] =0;							
			 Mass_Block_Count[1]=0;	
			}
			else if(temp==255) //
			{   
			 SD_STAT=255;
			 Max_Lun=0;
			 f_mount(NULL, "0", 0);        // ж��SD��
			 Mass_Block_Size[1] =0;							
			 Mass_Block_Count[1]=0;	
			}  
			else if(temp==0) //
			{   
			 SD_STAT=0;
			 Max_Lun=1;
       SD_Initialize();				
			 if(f_mount(fs[0],"0:",1) == 0X0D) //����SD	
			 f_mkfs("0:",0,4096);
			 Mass_Block_Size[1] =512;						
			 Mass_Block_Count[1]=0xE6D000;
			} 
	}
}



void Init_Iwdg(void)
{
	/* IWDG timeout equal to 200 ms (the timeout may varies due to LSI frequency dispersion) */

	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
  //	IWDG_SetPrescaler(IWDG_Prescaler_32);
	
	/* IWDG counter clock: 40KHz(LSI) / 256 = 0.15625 KHz */
  	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/* Set counter reload value to 1249: (2499+1)/1.25=2000ms */
	//IWDG_SetReload(2499);
	
	/* Set counter reload value to 124999: (1561+1)/0.15625=10000ms */
	IWDG_SetReload(1561);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
  
	/* Configures the IWDG clock mode when the MCU under Debug mode */
	DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);
}


/*
 * ��������void test_detect(void)  
 * ����  ��״̬�����
 * ����  ����
 * ���  ����	
 */
void test_detect(void) //SD����ѯ���
{
//  u16 cur_state=0;
//	u16 temp;
//	temp=SD_Initialize();
//	cur_state = SD_STAT; 
//	if(temp==0)
//	{
//	 e0_event_function(SD_STAT);
//	}


}






/*
 * ��������void UsbTask(void *pParameters)
 * ����  ��USB����
 * ����  ����
 * ���  ����	
 */
void UsbTask(void *pParameters)
{		
	USBTaskQueue = xQueueCreate((unsigned portBASE_TYPE)1, semSEMAPHORE_QUEUE_ITEM_LENGTH);
	xSemaphoreTake(USBTaskQueue,  0);

	Data_Buffer = pvPortMalloc(BULK_MAX_PACKET_SIZE*2*4*4);
	Bulk_Data_Buff = pvPortMalloc(BULK_MAX_PACKET_SIZE);
	
	vTaskDelay(1800/portTICK_RATE_MS);
	USB_Port_Set(0);
	vTaskDelay(700/portTICK_RATE_MS);
	USB_Port_Set(1);
	USB_HwInit();
	USB_Init();
  
	while(1)
	{		
		if(xSemaphoreTake(USBTaskQueue, portMAX_DELAY) == pdTRUE)
		{					
			USB_CTR_Handler();
		}
	}
}



