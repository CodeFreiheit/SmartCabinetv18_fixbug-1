
/*--------------------------------------------------------------------------+
| Include files                                                             |
+--------------------------------------------------------------------------*/
#include <stdio.h>

#include "FreeRTOS.h"

#include "print.h"
#include "rtc.h"
#include "semphr.h"

const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

_calendar_obj calendar;//ʱ�ӽṹ�� 

/*--------------------------------------------------------------------------+
| Type Definition & Macro                                                   |
+--------------------------------------------------------------------------*/

#define SECONDS_PER_YEAR	(365*24*60*60)ul
#define SECONDS_PER_DAY		(24*60*60)ul
#define DAYS_PER_YEAR		(365)
#define HOURS_PER_DAY		(24)
#define MINUTES_PER_HOUR	(60)
#define SECONDS_PER_MINUTE	(60)

#define RTCClockOutput_Enable	0
#define RTCInterrupt_Enable		1

/*--------------------------------------------------------------------------+
| Global Variables                                                          |
+--------------------------------------------------------------------------*/
Rtc_Time time;
extern xSemaphoreHandle RTC_Sem;

/* ����Ӹ�λ֮���ϵͳ���е�ʱ�䣬����Ϊ��λ��32λ�����Լ���136�� */
u32 SysRunTimer;
/* ������ϵ�֮���ϵͳ����ʱ�䣬�����λ������ոü����� */
 u32 SysPowerTimer;

/*--------------------------------------------------------------------------+
| Internal Variables                                                        |
+--------------------------------------------------------------------------*/
const struct t_Month MonthTable[] =
{
    {0,  "",    0},				// Invalid month
    {31, "Jan", 1},				// January
    {28, "Feb", 4},				// February (note leap years are handled by code)
    {31, "Mar", 4},				// March
    {30, "Apr", 7},				// April
    {31, "May", 2},				// May
    {30, "Jun", 5},				// June
    {31, "Jul", 7},				// July
    {31, "Aug", 3},				// August
    {30, "Sep", 6},				// September
    {31, "Oct", 1},				// October
    {30, "Nov", 4},				// November
    {31, "Dec", 6}				// December
};

/*--------------------------------------------------------------------------+
| Function Prototype                                                        |
+--------------------------------------------------------------------------*/
u32 _mktime(Rtc_Time *tm);
int _localtime(u32 timereg, Rtc_Time *tm);
 void _WriteTime(u16 year, u8 month, u8 date, u8 hour, u8 minute, u8 second);

/*--------------------------------------------------------------------------+
| System Initialization Routines                                            |
+--------------------------------------------------------------------------*/
/****************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/
void RTC_Configuration(void)
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	
	/* Reset Backup Domain */
	BKP_DeInit();
	
	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) ;
	
	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Change the current time */
//	_WriteTime(2018, 2, 11, 8, 30, 0);// ��2018��2��11������8��30��0�뿪ʼ����
	_WriteTime(1982, 6, 25, 11, 0, 0);// 
}

void RTC_Init(void)
{
#if RTCInterrupt_Enable
	NVIC_InitTypeDef NVIC_InitStructure;
#endif // RTCInterrupt_Enable

	/* Check if the Power On Reset flag is set */
	if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
	{
		// �ϵ縴λ Power On Reset
//		PrintS("\n\nPower On Reset");
		/* �ϵ�󣬳�ʼ��SysPowerTimer��������ֵ */
		SysPowerTimer = 0;
	}
	/* Check if the Pin Reset flag is set */
	else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
	{
		// �ⲿRST�ܽŸ�λ
//		PrintS("\n\nExternal Reset");
	}
	/* Check if the system has resumed from IWDG reset */
	else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{
		// IWDG���Ź���λ
//		PrintS("\n\nIWDG Reset");
	}
	else
	{
	}

	SysRunTimer = 0;

	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
//		PrintS("\nRTC not yet configured,");
		RTC_Configuration();
//		PrintS(" configured!");
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
//		PrintS("\nNo need to configure RTC");
		
		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	    /* Wait for RTC registers synchronization */
	    RTC_WaitForSynchro();
	}

	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Disable the Tamper Pin */
	BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
								 functionality must be disabled */

#if RTCClockOutput_Enable
	/* Enable RTC Clock Output on Tamper Pin */
	BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
#endif // RTCClockOutput_Enable

#if RTCInterrupt_Enable
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_SYS_INTERRUPT_PRIORITY + 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif // RTCInterrupt_Enable

	/* Clear reset flags */
	RCC_ClearFlag();
}

/*--------------------------------------------------------------------------+
| General Subroutines                                                       |
+--------------------------------------------------------------------------*/

/*
��RTC�������ж�ȡ��ǰʱ�䣬��ת��Ϊ�ꡢ�¡��ո�ʽ��ʱ��
*/
u32 RTC_GetTime(Rtc_Time *tm)
{
//	xSemaphoreTake(RTC_Sem, portMAX_DELAY);
	u32 counter = RTC_GetCounter();
	if (tm == NULL)
	{
		_localtime(counter, &time);
	}
	else
	{
		_localtime(counter, tm);
	}
//	xSemaphoreGive(RTC_Sem);
	return counter;
}

// ����FAT�ļ�ϵͳ����ʶ������ʱ���ʽ
// Ϊ��ֹ�жϸı�ʱ���ֵ����ʹ��ϵͳ��time��������ʹ�ôӼ������ж�ȡ��ֵ����ֵ���ᱻ��Ľ������ı�
u32 RTC_GetFatTime(void)
{
	u32 rt;
	u32 counter;
	Rtc_Time tm;

	counter = RTC_GetCounter();
	_localtime(counter, &tm);
	
	rt = ((tm.Year - 1980) << 25)		// 31 - 25
		| ((u32)tm.Month << 21)			// 24 - 21
		| ((u32)tm.DayOfMonth << 16)	// 20 - 16
		| ((u32)tm.Hour << 11)			// 15 - 11
		| ((u32)tm.Minute << 5)			// 10 -  5
		| ((u32)tm.Second >> 1);		//  4 -  0�����32��������Ҫ����������һλ
	return rt;
}

// һ����д�����ں�ʱ�䣬�����������ڲ�ʹ��
void _WriteTime(u16 year, u8 month, u8 date, u8 hour, u8 minute, u8 second)
{
	u32 counter;

	time.Year   = year;
	time.Month  = month;
	time.DayOfMonth = date;
	time.DayOfWeek  = RTC_GetDayOfWeek(year, month, date);
	time.Hour   = hour;
	time.Minute = minute;
	time.Second = second;
	counter = _mktime(&time);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(counter);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

//void RTC_SetTime(Rtc_Time *tm)
int RTC_SetTime(u8 hour, u8 minute, u8 second)
{
	u32 counter;
	Rtc_Time tm;

	if ((hour >= HOURS_PER_DAY) || (minute >= MINUTES_PER_HOUR) || (second >= SECONDS_PER_MINUTE))
	{
		return TIME_SET_WRONG_TIME;
	}
	counter = RTC_GetCounter();
	_localtime(counter, &tm);
	tm.Hour   = hour;
	tm.Minute = minute;
	tm.Second = second;
	counter = _mktime(&tm);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(counter);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	return TIME_SET_OK;
}

// ����ϵͳ��ǰ���� (�꣭�£���)
// ���أ��ɹ���TIME_SET_OK��
//       ���󣺴������
int RTC_SetDate(u16 year, u8 month, u8 date)
{
	u32 counter;
	Rtc_Time tm;

	// �������
	if ((year  > 2099)
	 || (month > 12)
	 || (date  > 31))
	{
		return TIME_SET_WRONG_TIME;
	}
	if (date > MonthTable[month].Days)
	{
		if (month == 2)	// ���������Ķ���, ���ֶ��һ��
		{
			if (IsLeapYear(year))
			{
				if (date > 29)	// ����Ķ���Ϊ29��
				{
					return TIME_SET_WRONG_TIME;
				}
			}
			else
			{
				return TIME_SET_WRONG_TIME;
			}
		}
		else
		{
			return TIME_SET_WRONG_TIME;
		}
	}

	counter = RTC_GetCounter();
	_localtime(counter, &tm);
	tm.Year  = year;
	tm.Month = month;
	tm.DayOfMonth = date;
	counter = _mktime(&tm);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(counter);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	return TIME_SET_OK;
}

//
// �ж��Ƿ�������, ����2��Ϊ29��, ��һ��Ϊ366��
// year: ��Ԫ���
u8 IsLeapYear(u16 year)
{
	if ((!(year % 4) && (year % 100)) || !(year % 400))
	{
		return 1;
	}
	return 0;
}


//
// ���㵱ǰΪ���ڼ�, ��2001��1��1��Ϊ����һΪ��׼
// ���ؼ���õ���������
u8 RTC_GetDayOfWeek(u16 year, u8 month, u8 date)
{
	u16 temp;

	// ÿ����1��, ��������2��
	if (year >= 2001)					// �����2001���Ժ������ֱ�Ӽ���
	{
		temp  = (year - 2001);
		// �ټ���ÿ�������������1��
		temp += (year - 2000)/4;
		temp -= (year - 2000)/100;
		temp += (year - 2000)/400;

		// ��ȥ��Ϊ����������һ��
		if (month <= 2)
		{
			if (IsLeapYear(year))
			{
				temp --;
			}
		}
	}
	else								// �����2001����ǰ�����ȼ����2001�������������ٲ�����
	{
		temp = year - 2001;				// ��ʱtempΪ������
		// �ټ�ȥÿ����������ٵ���1��
		temp -= (2000 - year)/4;
		temp += (2000 - year)/100;
		temp -= (2000 - year)/400;

		// ��ȥ��Ϊ����������1��
		temp --;
		// ��������Ķ��·�֮����·�û�ж��㣬����Ҫ�ӻظղż�ȥ��1��
		if (month > 2)
		{
			if (IsLeapYear(year))
			{
				temp ++;
			}
		}
	}
	// ����ÿ���µ�1�Ŷ�Ӧ��������
	temp += MonthTable[month].Val;
	temp += (date - 1);

	// ��������Ϊ����
	while (temp > 0x7FFF)
	{
		temp += 7;						// ÿ�μ�7��ֱ�������Ϊ����
	}

	// ���������ٺ�7����͵õ���������
	temp %= 7;
	if (temp == 0)
		temp = 7;
	return temp;
}

// ��STM32�Ĵ�����ʽ��ʱ����Ϣת��Ϊ�ɶ���ʱ����Ϣ
int _localtime(u32 timereg, Rtc_Time *tm)
{
	u32 temp;
//	u8 bLeap = 0;
	
	tm->Second = timereg % SECONDS_PER_MINUTE;	// ��
	timereg   /= SECONDS_PER_MINUTE;			// ������
	tm->Minute = timereg % MINUTES_PER_HOUR;	// ��
	timereg   /= MINUTES_PER_HOUR;				// Сʱ��
	tm->Hour   = timereg % HOURS_PER_DAY;		// ʱ
	timereg   /= HOURS_PER_DAY;					// ����1970��01��01�յ�����

	/* ���¸������������㵱ǰ����, ��1968����һ���꿪ʼ���� */
	temp = 1968;
	timereg += DAYS_PER_YEAR*2 + 1;
	while (timereg >= (DAYS_PER_YEAR*4 + 1))	// ÿ��4�꣬����һ������
	{
		timereg -= (DAYS_PER_YEAR*4 + 1);
		temp += 4;
	}
	while (timereg > DAYS_PER_YEAR)
	{
		timereg -= DAYS_PER_YEAR;
		temp += 1;
	}
	tm->Year = temp;					// ��
	temp = 1;
	/* ����������1�º�2�·ݣ��� */
	if (IsLeapYear(tm->Year) &&
		(timereg <= (MonthTable[1].Days + MonthTable[2].Days)))
	{
		timereg += 1;					// �������꣬ǰ������1�����ڼӻ���
		/* ���ǵ�����Ķ��·���29�죬��Ҫ���⴦�� */
		/* �������Ϊ֪���Ƕ��·�֮ǰ������ֻ��ȥ��һ���µ����� */
		if (timereg > MonthTable[1].Days)
		{
			timereg -= MonthTable[1].Days;
			temp += 1;
		}
//		bLeap = 1;
	}
	else
	{
		while (timereg > MonthTable[temp].Days)
		{
			timereg -= MonthTable[temp].Days;
			temp += 1;
		}
	}
	tm->Month  = temp;
	tm->DayOfMonth = timereg;
	tm->DayOfWeek  = RTC_GetDayOfWeek(tm->Year, tm->Month, tm->DayOfMonth);

	return 0;
}

// ��������Ϥ���ꡢ�¡��ո�ʽ��ʱ��ת����STM32��Ӳ����������ֵ
u32 _mktime(Rtc_Time *tm)
{
	u32 temp;
	int i;

	temp = tm->Year - 1970;				// �����1970�������������
	temp *= DAYS_PER_YEAR;				// ����ÿ�������
	temp += (tm->Year - 1968)/4;		// �ټ���ÿ�������������1�죬����ֻ��Ҫ����
										// 1970-2099��֮�䣬��2000�������꣬���Բ�����
										// 100�������겻�������400�������������������
	for (i=1; i<tm->Month; i++)		// ���ϸ��·��ڸ����е�����
	{
		temp += (u32)(MonthTable[i].Days);
	}
	if (tm->Month <= 2)					// ��������꣬���·�С��3�·ݣ�����Ҫ��ȥ֮ǰ��ӵ�1
	{
		if (IsLeapYear(tm->Year))
		{
			temp -= 1;
		}
	}
	temp += (tm->DayOfMonth - 1);		// ���ϵ��µ��������õ�����1970��01��01������ȥ������
	temp *= HOURS_PER_DAY;				// ����ÿ���Сʱ����
	temp += tm->Hour;					// ����Сʱ��
	temp *= MINUTES_PER_HOUR;			// ����ÿСʱ60��
	temp += tm->Minute;					// ���Ϸ�����
	temp *= SECONDS_PER_MINUTE;			// ����ÿ����60��
	temp += tm->Second;					// �����������õ���ǰʱ����1970��01��01��00ʱ00��00������ȥ������
										// ��������ǽ�Ҫд�뵽STM322��Ӳ��ʱ�Ӽ����������ֵ

	return temp;
}

// ��������Ϥ���ꡢ�¡��ո�ʽ��ʱ��ת����STM32��Ӳ����������ֵ GPSʱ��ת����ʱ���
u32 GPS_mktime(_calendar_obj *tm)
{
	u32 temp;
	int i;

	temp = tm->w_year - 1970;				// �����1970�������������
	temp *= DAYS_PER_YEAR;				// ����ÿ�������
	temp += (tm->w_year - 1968)/4;		// �ټ���ÿ�������������1�죬����ֻ��Ҫ����
										// 1970-2099��ּ䣬��2000�������꣬���Բ�����
										// 100�������겻�������400�������������������
	for (i=1; i<tm->w_month; i++)		// ���ϸ��·��ڸ����е�����
	{
		temp += (u32)(MonthTable[i].Days);
	}
	if (tm->w_month <= 2)					// ��������꣬���·�С��3�·ݣ�����Ҫ��ȥ�ǰ��ӵ�1
	{
		if (IsLeapYear(tm->w_year))
		{
			temp -= 1;
		}
	}
	temp += (tm->w_date - 1);		// ���ϵ��µ��������õ�����1970��01��01������ȥ������
	temp *= HOURS_PER_DAY;				// ����ÿ���Сʱ����
	temp += tm->hour;					// ����Сʱ��
	temp *= MINUTES_PER_HOUR;			// ����ÿСʱ60��
	temp += tm->min;					// ���Ϸ�����
	temp *= SECONDS_PER_MINUTE;			// ����ÿ����60��
	temp += tm->sec;					// �����������õ���ǰʱ����1970��01��01��00ʱ00��00������ȥ������
										// ��������ǽ�Ҫд�뵽STM322��Ӳ��ʱ�Ӽ����������ֵ

	return temp;
}


void RTC_FormatDateStr(char *pbuf)
{
	sprintf(pbuf, "%04d-%02d-%02d ", time.Year, time.Month, time.DayOfMonth);
}

void RTC_FormatTimeStr(char *pbuf)
{
	sprintf(pbuf, "%02d:%02d:%02d ", time.Hour, time.Minute, time.Second);
}

// �����������ֵת��Ϊ����������Сʱ�����Ӻ�����ַ�����������ʾ
int RTC_GetTimeString(u32 sec, char *pbuf)
{
	u8 second, minute, hour;
	u16 days;
	
	second = sec % SECONDS_PER_MINUTE;	// ��
	sec   /= SECONDS_PER_MINUTE;		// ������
	minute = sec % MINUTES_PER_HOUR;	// ��
	sec   /= MINUTES_PER_HOUR;			// Сʱ��
	hour   = sec % HOURS_PER_DAY;		// ʱ
	days   = sec / HOURS_PER_DAY;		// ����

	sprintf(pbuf, "%d �� %02d:%02d:%02d ", days, hour, minute, second);

	return 0;
}




/*
 * ���������õ���ǰ��ʱ��
 * ����  ��
 * ����  ����
 * ���  ����	
 */
u8 RTC_Get(void)
{
	static u16 daycnt=0;
	u32 timecount=0; 
	u32 temp=0;
	u16 temp1=0;	  
    timecount=RTC_GetCounter();	 
 	temp=timecount/86400;   //�õ�����(��������Ӧ��)
	if(daycnt!=temp)//����һ����
	{	  
		daycnt=temp;
		temp1=1970;	//��1970�꿪ʼ
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//������
			{
				if(temp>=366)temp-=366;//�����������
				else {temp1++;break;}  
			}
			else temp-=365;	  //ƽ�� 
			temp1++;  
		}   
		calendar.w_year=temp1;//�õ����
		temp1=0;
		while(temp>=28)//������һ����
		{
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(temp>=29)temp-=29;//�����������
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		calendar.w_month=temp1+1;	//�õ��·�
		calendar.w_date=temp+1;  	//�õ����� 
	}
	temp=timecount%86400;     		//�õ�������   	   
	calendar.hour=temp/3600+8;     	//Сʱ
	calendar.min=(temp%3600)/60; 	//����	
	calendar.sec=(temp%3600)%60; 	//����
	//calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//��ȡ����   
	return 0;
}	 

//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   

/*--------------------------------------------------------------------------+
| Interrupt Service Routines                                                |
+--------------------------------------------------------------------------*/
void RTC_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}
	/* �ۼ�ϵͳ����ʱ������� */
	SYS_TIMER_INC();
}

/*--------------------------------------------------------------------------+
| End of source file                                                        |
+--------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line ------------------------*/
