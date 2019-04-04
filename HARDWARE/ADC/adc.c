#include "adc.h"
#include "delay.h"
#include "sensor.h"
#include "minini.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "camera.h"

float VOL, CUR;
extern TaskHandle_t USBTask_Handler;

void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1, ENABLE );

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);									// ����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	// PC1-��ѹ��PA0����           
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;						// ģ����������  ��ѹ
	GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;						// ģ����������  ����
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
		
	

	ADC_DeInit(ADC1);  // ��λADC1 

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						// ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					// ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;								// ˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);									// ����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

  
	ADC_Cmd(ADC1, ENABLE);						// ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);					// ʹ�ܸ�λУ׼  

	while(ADC_GetResetCalibrationStatus(ADC1));	// �ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);					//����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));		//�ȴ�У׼����
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}

//���ADCֵ��ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)
{
  	// ����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	// ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);								// ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));						// �ȴ�ת������

	return ADC_GetConversionValue(ADC1);								// �������һ��ADC1�������ת�����
}

u16 Get_Adc_Average(u8 ch,u8 times)
{ 
	u32 temp_val=0;
	u16 temp,i;
	for(i=0;i<1000;i++)
	{
//		temp_val+=Get_Adc(ch);
		if(Get_Adc(ch)>temp_val) temp_val=Get_Adc(ch);
		//delay_ms(1);
	}
	return temp_val;
} 	 

void Get_Vol(void)
{
	u16 adcx;	
	u16 temp;
	//adcx = Get_Adc_Average(ADC_Channel_11, 20); //PC1
	//temp= Get_Adc_Average(ADC_Channel_11, 20); //PC1
	temp = Get_Adc_Average(ADC_Channel_0, 20);//PA0
	VOL = (float)temp*3.3/4096*180;
	if((VOL>600)||(VOL<80))VOL = 0;	
}

void Get_Cur(void)
{
	u16 adcx;
	
	//adcx = Get_Adc_Average(ADC_Channel_0, 20);//PA0
	adcx = Get_Adc_Average(ADC_Channel_11, 20); //PC1
	//CUR = ((float)adcx*3.3/4096-0.02)*1000;
	//CUR = ((float)adcx*3.3/4096)*3.75;
	CUR = ((float)adcx*3.3/4096)*4.545;//330�� ����
	if((CUR<0) || (VOL==0))CUR = 0;
}
/*
 * ��������void output_control_default(void)
 * ����  ��Ĭ���������
 * ����  ����
 * ���  ����	
 */
void output_control_default(void)
{
	char inifile[] = "1:cfg.ini";
	
	if((ini_getl("ctr",	"L1",	0,	inifile))) 
	{		
		OUT_AC1_220V_ON();
		AC1_STAT=1;
	}			
	if(!(ini_getl("ctr","L1",	0,	inifile)))
	{		
	  OUT_AC1_220V_OFF();
		AC1_STAT=0;
	}    
  if((ini_getl("ctr",	"L1",	0,	inifile))==2) {};		

	if((ini_getl("ctr",	"L2",	0,	inifile))) 
	{	 
		OUT_AC2_220V_ON();
		AC2_STAT=1;
	}		
	if(!(ini_getl("ctr","L2",	0,	inifile)))
	{	  
		OUT_AC2_220V_OFF();
    AC2_STAT=0;		
	}    
	if((ini_getl("ctr",	"L2",	0,	inifile))==2) {};	

	if((ini_getl("ctr",	"L3",	0,	inifile))) 
	{	 
		OUT_AC3_220V_ON();
		AC3_STAT=1;
	}			
	if(!(ini_getl("ctr","L3",	0,	inifile))) 
	{	
	 OUT_AC3_220V_OFF();
	 AC3_STAT=0;
	}			
	if((ini_getl("ctr",	"L3",	0,	inifile))==2) {};	

	if((ini_getl("ctr",	"V1",	0,	inifile)))
	{	
	 DC1_ON();
	 DC1_STAT=1;
	}    	
	if(!(ini_getl("ctr","V1",	0,	inifile))) 
	{
		DC1_STAT=0;
	  DC1_OFF();
	}		
  if((ini_getl("ctr",	"V1",	0,	inifile))==2) {};			

	if((ini_getl("ctr",	"V2",	0,	inifile)))
	{
		DC2_STAT=1;
		DC2_ON();	
	} 	
	if(!(ini_getl("ctr","V2",	0,	inifile)))
	{
	  DC2_STAT=0;
	  DC2_OFF();
	}
	if((ini_getl("ctr",	"V2",	0,	inifile))==2) {};	

	if((ini_getl("ctr",	"V3",	0,	inifile)))
	{
	 DC3_STAT=1;
	 DC3_ON();	
	}   
	if(!(ini_getl("ctr","V3",	0,	inifile)))
	{
	 DC3_STAT=0;
	 DC3_OFF();		
	}   
  if((ini_getl("ctr",	"V3",	0,	inifile))==2) {};	

		
	//DC4	
  if((ini_getl("ctr",	"V4",	0,	inifile)))
	{
	 DC4_STAT=1;
	 DC4_ON();	
	}   
	if(!(ini_getl("ctr","V4",	0,	inifile)))
	{
	 DC4_STAT=0;
	 DC4_OFF();		
	}   
  if((ini_getl("ctr",	"V4",	0,	inifile))==2) {};			

//	if((ini_getl("ctr","light",	0,	inifile)))  light_OFF();
//	if(!(ini_getl("ctr","light",0,	inifile)))light_ON();	
//	if((ini_getl("ctr",	"light",2,	inifile))==2) {};	

//	if(ini_getl("ctr",	"fan",	2,	inifile))   out_fan_OFF();
//	if(!(ini_getl("ctr","fan",	2,	inifile)))  out_fan_ON();
//	if((ini_getl("ctr",	"fan",	2,	inifile))==2) {};	
}

/*
 * ��������void read_Flooding(void)
 * ����  ��ˮ������
 * ����  ����
 * ���  ����	
 */
void read_Flooding(void)
{
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)==0) //ˮ��
	{
	 GPIO_SetBits(GPIOE,GPIO_Pin_6);//��L1
   GPIO_SetBits(GPIOF,GPIO_Pin_7);//��L2					 
	 GPIO_SetBits(GPIOF,GPIO_Pin_9);//��L3 
	}
}

























