//
// ------------------------------------------------------------------------
// RTC.h
// ------------------------------------------------------------------------


#ifndef __RTC_H__
#define __RTC_H__

#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*--------------------------------------------------------------------------+
| Type Definition & Macro                                                   |
+--------------------------------------------------------------------------*/
struct _time_t
{
	u8 Hour;		// 0 - 23
	u8 Minute;		// 0 - 59
	u8 Second;		// 0 - 59
};

struct _date_t
{
	u8  Date;		// 1-31
	u8  Month;		// 1-12
	u16 Year;		// ��Ԫ��ݡ�//[1980 - 2099]
	u8  DayOfWeek;	// 0 - 7 (0=7=Sunday)
};

// ����ʱ��ṹ�壬ʱ����BCD���ʾ
typedef struct _Rtc_Time
{
	u8  Second;
	u8  Minute;
	u8  Hour;
	u8  DayOfWeek;
	u8  DayOfMonth;
	u8  Month;
	u16 Year;
} Rtc_Time;

//ʱ��ṹ��
typedef struct 
{
	vu8 hour;
	vu8 min;
	vu8 sec;			
	//������������
	vu16 w_year;
	vu8  w_month;
	vu8  w_date;
	vu8  week;		 
}_calendar_obj;

extern _calendar_obj calendar;	//�����ṹ��

// �����·ݽṹ��
struct t_Month
{
    u32  Days;			// ����µ�����
    char *Name;			// ����µ�����
    u8  Val;			// �����������ڼ�, Ϊĳ���ض����(2001)��ÿ��1�ŵ�������
};

#define TIME_SET_OK				0		// ʱ�����óɹ�
#define TIME_SET_WRONG_TIME		1		// ����ʱ��ʱ�Ĳ�������

#define Rtc_ReadTime()			RTC_GetTime(NULL)
#define RtcTime					time

#define SYS_TIMER_INC()			{SysRunTimer ++; SysPowerTimer ++;}

/*--------------------------------------------------------------------------+
| Global Variables                                                          |
+--------------------------------------------------------------------------*/
extern Rtc_Time time;
extern const struct t_Month MonthTable[];

extern u32 SysRunTimer;
extern u32 SysPowerTimer;

/*--------------------------------------------------------------------------+
| Function Prototype                                                        |
+--------------------------------------------------------------------------*/
void RTC_Configuration(void);
void RTC_Init(void);

u32 RTC_GetTime(Rtc_Time *tm);
u32 _mktime(Rtc_Time *tm);

u32 GPS_mktime(_calendar_obj *tm);

int RTC_SetTime(u8 hour, u8 minute, u8 second);
int RTC_SetDate(u16 year, u8 month, u8 date);

u32 RTC_GetFatTime(void);

u8  RTC_GetDayOfWeek(u16 year, u8 month, u8 date);
u8 IsLeapYear(u16 year);

void RTC_FormatDateStr(char *pbuf);
void RTC_FormatTimeStr(char *pbuf);
int RTC_GetTimeString(u32 sec, char *pbuf);
u8 Is_Leap_Year(u16 year);//ƽ��,�����ж�
extern u8 RTC_Get(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	// __RTC_H__
/*--------------------------------------------------------------------------+
| End of source file                                                        |
+--------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line ------------------------*/
