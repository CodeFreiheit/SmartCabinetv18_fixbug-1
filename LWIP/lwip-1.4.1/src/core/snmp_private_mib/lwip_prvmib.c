/**
 * @file
 * lwip Private MIB 
 *
 * @todo create MIB file for this example
 * @note the lwip enterprise tree root (26381) is owned by the lwIP project.
 * It is NOT allowed to allocate new objects under this ID (26381) without our,
 * the lwip developers, permission!
 *
 * Please apply for your own ID with IANA: http://www.iana.org/numbers.html
 *  
 * lwip        OBJECT IDENTIFIER ::= { enterprises 26381 }
 * example     OBJECT IDENTIFIER ::= { lwip 1 }
 */
 
/*
 * Copyright (c) 2006 Axon Digital Design B.V., The Netherlands.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * Author: Christiaan Simons <christiaan.simons@axon.tv>
 */

#include "private_mib.h"

#if LWIP_SNMP

/** Directory where the sensor files are */
#define SENSORS_DIR           "w:\\sensors"
/** Set to 1 to read sensor values from files (in directory defined by SENSORS_DIR) */
#define SENSORS_USE_FILES     0//0
/** Set to 1 to search sensor files at startup (in directory defined by SENSORS_DIR) */
#define SENSORS_SEARCH_FILES  0//0

#if SENSORS_SEARCH_FILES
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#endif /* SENSORS_SEARCH_FILES */

#include <string.h>
#include <stdio.h>

#include "lwip/snmp_msg.h"
#include "lwip/snmp_asn1.h"
#include "snmp_structs.h"
#include "adc.h"
#include "string.h"
#include "sensor.h"
void test(void);
void ocstrncpy2(u8_t *dst, u8_t *src, u16_t n);
static void emmp_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t emmp_set_test(struct obj_def *od, u16_t len, void *value);
static void emmp_set_value(struct obj_def *od, u16_t len, void *value);
static void emmp_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);


static char* char_arrays_to_strings(u8_t *buff,u8 len);

#if !SENSORS_USE_FILES || !SENSORS_SEARCH_FILES
/** When not using & searching files, defines the number of sensors */
#define SENSOR_COUNT 2
#endif /* !SENSORS_USE_FILES || !SENSORS_SEARCH_FILES */
#if !SENSORS_USE_FILES
/** When not using files, contains the values of the sensors */
s32_t sensor_values[SENSOR_COUNT];
#endif /* !SENSORS_USE_FILES */

/*
  This example presents a table for a few (at most 10) sensors.
  Sensor detection takes place at initialization (once only).
  Sensors may and can not be added or removed after agent
  has started. Note this is only a limitation of this crude example,
  the agent does support dynamic object insertions and removals.
   
  You'll need to manually create a directory called "sensors" and
  a few single line text files with an integer temperature value.
  The files must be called [0..9].txt. 
   
  ./sensors/0.txt [content: 20]
  ./sensors/3.txt [content: 75]
    
  The sensor values may be changed in runtime by editing the 
  text files in the "sensors" directory.
*/


#define SENSOR_MAX 10
#define SENSOR_NAME_LEN 20

u8_t  len_test;
//u8_t  sys_default[] = "1,2,3,4,9,8,,10.13*,12";
u8_t  sys_default[10];// = "S,31,W,117*8,9,10.3,8,0,i";
//u8_t  sys_len_default =19;//22;//4;
u8_t  sys_len_default;//=sizeof(sys_default)-1;
//len_test=sizeof(sys_default);
u8_t* sys_ptr=(u8_t*)&sys_default[0];
u8_t* sys_len_ptr = (u8_t*)&sys_len_default;


//u8_t  sys_default2[] = "nihao";
u8_t  sys_default2[10];
u8_t  sys_len_default2;
//u8_t  sys_len_default2=sizeof(sys_default2)-1;
u8_t* sys_ptr2=(u8_t*)&sys_default2[0];
u8_t* sys_len_ptr2 = (u8_t*)&sys_len_default2;

//TEM
u8_t  sys_default3[10];
u8_t  sys_len_default3;//=14;//sizeof(sys_default3)-1;
//u8_t  sys_len_default3;//=strlen(sys_default3);
u8_t* sys_ptr3=(u8_t*)&sys_default3[0];
u8_t* sys_len_ptr3 = (u8_t*)&sys_len_default3;

//HUM
u8_t  sys_default4[10];
u8_t  sys_len_default4;
u8_t* sys_ptr4=(u8_t*)&sys_default4[0];
u8_t* sys_len_ptr4 = (u8_t*)&sys_len_default4;


//ˮ��
u8_t  sys_default5[5];
u8_t  sys_len_default5;
u8_t* sys_ptr5=(u8_t*)&sys_default5[0];
u8_t* sys_len_ptr5 =(u8_t*)&sys_len_default5;

//�Ž�
u8_t  sys_default6[5];
u8_t  sys_len_default6;
u8_t* sys_ptr6=(u8_t*)&sys_default6[0];
u8_t* sys_len_ptr6 =(u8_t*)&sys_len_default6;

//ֱ����Դ
u8_t  sys_default7[5];
u8_t  sys_len_default7;
u8_t* sys_ptr7=(u8_t*)&sys_default7[0];
u8_t* sys_len_ptr7 =(u8_t*)&sys_len_default7;

//���õ�Դ
u8_t  sys_default8[5];
u8_t  sys_len_default8;
u8_t* sys_ptr8=(u8_t*)&sys_default8[0];
u8_t* sys_len_ptr8 =(u8_t*)&sys_len_default8;

//UPS��Դ
u8_t  sys_default9[5];
u8_t  sys_len_default9;
u8_t* sys_ptr9=(u8_t*)&sys_default9[0];
u8_t* sys_len_ptr9 =(u8_t*)&sys_len_default9;

//�����Դ
u8_t  sys_default10[5];
u8_t  sys_len_default10;
u8_t* sys_ptr10=(u8_t*)&sys_default10[0];
u8_t* sys_len_ptr10=(u8_t*)&sys_len_default10;

//���AC1
u8_t  sys_default_ac1[5];
u8_t  sys_len_default_ac1;
u8_t* sys_ptr_ac1=(u8_t*)&sys_default_ac1[0];
u8_t* sys_len_ptr_ac1=(u8_t*)&sys_len_default_ac1;

//���AC2
u8_t  sys_default_ac2[5];
u8_t  sys_len_default_ac2;
u8_t* sys_ptr_ac2=(u8_t*)&sys_default_ac2[0];
u8_t* sys_len_ptr_ac2=(u8_t*)&sys_len_default_ac2;

//���AC3
u8_t  sys_default_ac3[5];
u8_t  sys_len_default_ac3;
u8_t* sys_ptr_ac3=(u8_t*)&sys_default_ac3[0];
u8_t* sys_len_ptr_ac3=(u8_t*)&sys_len_default_ac3;

//����
u8_t  sys_default_fan[5];
u8_t  sys_len_default_fan;
u8_t* sys_ptr_fan=(u8_t*)&sys_default_fan[0];
u8_t* sys_len_ptr_fan=(u8_t*)&sys_len_default_fan;

//����
u8_t  sys_default_alarm[5];
u8_t  sys_len_default_alarm;
u8_t* sys_ptr_alarm=(u8_t*)&sys_default_alarm[0];
u8_t* sys_len_ptr_alarm=(u8_t*)&sys_len_default_alarm;

//����
u8_t  sys_default_light[5];
u8_t  sys_len_default_light;
u8_t* sys_ptr_light=(u8_t*)&sys_default_light[0];
u8_t* sys_len_ptr_light=(u8_t*)&sys_len_default_light;

//����
u8_t  sys_default_heat[5];
u8_t  sys_len_default_heat;
u8_t* sys_ptr_heat=(u8_t*)&sys_default_heat[0];
u8_t* sys_len_ptr_heat=(u8_t*)&sys_len_default_heat;

//���DC1
u8_t  sys_default_dc1[5];
u8_t  sys_len_default_dc1;
u8_t* sys_ptr_dc1=(u8_t*)&sys_default_dc1[0];
u8_t* sys_len_ptr_dc1=(u8_t*)&sys_len_default_dc1;

//���DC2
u8_t  sys_default_dc2[5];
u8_t  sys_len_default_dc2;
u8_t* sys_ptr_dc2=(u8_t*)&sys_default_dc2[0];
u8_t* sys_len_ptr_dc2=(u8_t*)&sys_len_default_dc2;

//���DC3
u8_t  sys_default_dc3[5];
u8_t  sys_len_default_dc3;
u8_t* sys_ptr_dc3=(u8_t*)&sys_default_dc3[0];
u8_t* sys_len_ptr_dc3=(u8_t*)&sys_len_default_dc3;

//������� DC4
u8_t  sys_default_dc4[5];
u8_t  sys_len_default_dc4;
u8_t* sys_ptr_dc4=(u8_t*)&sys_default_dc4[0];
u8_t* sys_len_ptr_dc4=(u8_t*)&sys_len_default_dc4;


/*
 * �������h����豸�ڵ�
 * ����  ��
 * ����  ����
 * ���  ����	
 */
static void emmp_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;
  char *strings;
  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
	
	printf("get_object_def private*********** \n");
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;
 
    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private emmp.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* VOL  */
			  sprintf((char*)sys_default, "%.2f", VOL);
			  strings=char_arrays_to_strings(sys_default,strlen((char*)sys_default));
			  sys_len_default=strlen(strings);	
			
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_SEQ);  			
			  od->v_len =*sys_len_ptr;//sizeof(u32_t);			  
        break;
      case 2:    /* cur  */
				sprintf((char*)sys_default2, "%.2f", CUR);
			  strings=char_arrays_to_strings(sys_default2,strlen((char*)sys_default2));
			  sys_len_default2=strlen(strings);
			
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr2;
        break;
			case 3:    /* TEM  */					
			  sprintf((char*)sys_default3, "%.1f",TEM);
			  strings=char_arrays_to_strings(sys_default3,strlen((char*)sys_default3));
			  sys_len_default3=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
          od->v_len = *sys_len_ptr3;
					printf("case 3\n");
        break;
			case 4:    /* HUM  */				
				sprintf((char*)sys_default4, "%.1f",HUM);
			  strings=char_arrays_to_strings(sys_default4,strlen((char*)sys_default4));
			  sys_len_default4=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
          od->v_len = *sys_len_ptr4;
					printf("case 4\n");
        break;
			case 5:    /* ˮ��  */				
				sprintf((char*)sys_default5,  "%d",!WATER_STAT);
			  strings=char_arrays_to_strings(sys_default5,strlen((char*)sys_default5));
			  sys_len_default5=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr5;
        break;
			case 6:    /* �Ž�  */				
				sprintf((char*)sys_default6, "%d",DOOR_STAT);
			  strings=char_arrays_to_strings(sys_default6,strlen((char*)sys_default6));
			  sys_len_default6=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr6;
				printf("case 6\n");
        break;
			case 7:    /* ֱ����Դ */				
				sprintf((char*)sys_default7, "%d",!SYS12_STAT);
			  strings=char_arrays_to_strings(sys_default7,strlen((char*)sys_default7));
			  sys_len_default7=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr7;
        break;
			case 8:   /*���õ�Դ*/			
				sprintf((char*)sys_default8, "%d",!BAK12_STAT);
			  strings=char_arrays_to_strings(sys_default8,strlen((char*)sys_default8));
			  sys_len_default8=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr8;
        break;
			case 9:   /*ups��Դ*/		
				sprintf((char*)sys_default9, "%d",!UPS_STAT);
			  strings=char_arrays_to_strings(sys_default9,strlen((char*)sys_default9));
			  sys_len_default9=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr9;
        break;
			case 10:   /*�����Դ*/		
				sprintf((char*)sys_default10, "%d",!AC24_STAT);
			  strings=char_arrays_to_strings(sys_default10,strlen((char*)sys_default10));
			  sys_len_default10=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr10;
        break;
			case 11:  /*AC1*/	
				sprintf((char*)sys_default_ac1, "%d",AC1_STAT);
			  strings=char_arrays_to_strings(sys_default_ac1,strlen((char*)sys_default_ac1));
			  sys_len_default_ac1=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_ac1;
        break;
			case 12:  /*AC2*/	
				sprintf((char*)sys_default_ac2, "%d",AC2_STAT);
			  strings=char_arrays_to_strings(sys_default_ac2,strlen((char*)sys_default_ac2));
			  sys_len_default_ac2=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_ac2;
        break;
			case 13:  /*AC3*/	
				sprintf((char*)sys_default_ac3, "%d",AC3_STAT);
			  strings=char_arrays_to_strings(sys_default_ac3,strlen((char*)sys_default_ac3));
			  sys_len_default_ac3=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_ac3;
        break;
			case 14:  /*fan*/	
				sprintf((char*)sys_default_fan, "%d",fan_STAT);
			  strings=char_arrays_to_strings(sys_default_fan,strlen((char*)sys_default_fan));
			  sys_len_default_fan=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_fan;
        break;
			case 15:  /*alarm*/	
				sprintf((char*)sys_default_alarm, "%d",alarm_STAT);
			  strings=char_arrays_to_strings(sys_default_alarm,strlen((char*)sys_default_alarm));
			  sys_len_default_alarm=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_alarm;
        break;
			case 16:  /*light*/	
				sprintf((char*)sys_default_light, "%d",light_STAT);
			  strings=char_arrays_to_strings(sys_default_light,strlen((char*)sys_default_light));
			  sys_len_default_light=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_light;
        break;
			case 17:  /*heat*/	
				sprintf((char*)sys_default_heat, "%d",heat_STAT);
			  strings=char_arrays_to_strings(sys_default_heat,strlen((char*)sys_default_heat));
			  sys_len_default_heat=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_heat;
        break;
			case 18:  /*DC1*/	
				sprintf((char*)sys_default_dc1, "%d",DC1_STAT);
			  strings=char_arrays_to_strings(sys_default_dc1,strlen((char*)sys_default_dc1));
			  sys_len_default_dc1=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_dc1;
        break;
			case 19:  /*DC2*/	
				sprintf((char*)sys_default_dc2, "%d",DC2_STAT);
			  strings=char_arrays_to_strings(sys_default_dc2,strlen((char*)sys_default_dc2));
			  sys_len_default_dc2=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_dc2;
        break;
			case 20:  /*DC3*/	
				sprintf((char*)sys_default_dc3, "%d",DC3_STAT);
			  strings=char_arrays_to_strings(sys_default_dc3,strlen((char*)sys_default_dc3));
			  sys_len_default_dc3=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_dc3;
        break;
			case 21:  /*DC4*/	
				sprintf((char*)sys_default_dc4, "%d",0);
			  strings=char_arrays_to_strings(sys_default_dc4,strlen((char*)sys_default_dc4));
			  sys_len_default_dc4=strlen(strings);
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
			  od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR); 
        //od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);			  
        od->v_len = *sys_len_ptr_dc4;
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("emmp_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private emmp_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}
/*
 * �����������豸�ڵ�ֵ
 * ����  ��
 * ����  ����
 * ���  ����	
 */
static void emmp_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
 
  id = od->id_inst_ptr[0];
	  printf("id1=%d\n",id);
  switch (id)
  {
    case 1:    /* VOL  */
      {
				s32_t *sint_ptr =value;
				printf("len=%d\n",len);
				ocstrncpy2((u8_t*)sint_ptr, sys_ptr, len);
      }
      break;
    case 2:    /* CUR  */
      {
        s32_t *sint_ptr = value;
        printf("id2=%d\n",id);
				ocstrncpy2((u8_t*)sint_ptr, sys_ptr2, len);
      }
			break;
		case 3:    
			{
				
				s32_t *sint_ptr = value;
				printf("id3=%d\n",id);
				ocstrncpy2((u8_t*)sint_ptr, sys_ptr3, len);
			}
		break;
		case 4:    /*HUM*/
		{
			s32_t *sint_ptr = value;
			printf("id2=%d\n",id);
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr4, len);
		}
      break;
		case 5:    /*ˮ��*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr5, len);
		}
      break;
		case 6:    /*�Ž�*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr6, len);
		}
      break;
		case 7:    /*ֱ����Դ*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr7, len);
		}
      break;
		case 8:    /*���õ�Դ*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr8, len);
		}
      break;
		case 9:   /*ups��Դ*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr9, len);
		}
      break;
		case 10:   /*�����Դ*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr10, len);
		}
      break;
		case 11:   /*AC1*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_ac1, len);
		}
      break;
		case 12:   /*AC2*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_ac2, len);
		}
      break;
		case 13:   /*AC3*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_ac3, len);
		}
      break;
		case 14:   /*fan*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_fan, len);
		}
      break;
		case 15:   /*alarm*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_alarm, len);
		}
      break;
		case 16:   /*light*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_light, len);
		}
      break;
		case 17:   /*heat*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_heat, len);
		}
      break;
		case 18:   /*DC1*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_dc1, len);
		}
      break;
		case 19:   /*DC2*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_dc2, len);
		}
      break;
		case 20:   /*DC3*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_dc3, len);
		}
      break;
		case 21:   /*DC4*/
		{
			s32_t *sint_ptr = value;
			ocstrncpy2((u8_t*)sint_ptr, sys_ptr_dc4, len);
		}
      break;
  };
}
/*
 * �������vvoid ocstrncpy2(u8_t *dst, u8_t *src, u16_t n)�
 * ����  ��
 * ����  ����
 * ���  ����	
 */
void ocstrncpy2(u8_t *dst, u8_t *src, u16_t n)
{
  u16_t i = n;
  while (i > 0) {
    i--;
    *dst++ = *src++;
  }
}
/*
 * ��������д�豸�ڵ�test
 * ����  ��emmp_set_test
 * ����  ����
 * ���  ����	
 */
static u8_t emmp_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;
 
  set_ok = 0;
  id = od->id_inst_ptr[0];
	printf("emmp_set_test\n");
  switch (id)
  {
    case 1:    /* restart  */
		    {
		     s32_t *sint_ptr = value;
				 printf("*sint_ptr=%ld\n",*sint_ptr);
		    }
		     
  /* validate the value argument and set ok  */
      break;
    case 2:    /* reset  */
  /* validate the value argument and set ok  */
      break;
  };
  return set_ok;
}
/*
 * ��������д�豸�ڵ�
 * ����  ��emmp_set_value
 * ����  ����
 * ���  ����	
 */
static void emmp_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
 
  id = od->id_inst_ptr[0];
	
  switch (id)
  {
    case 1:    /* restart  */
      {
        s32_t *sint_ptr = value;
				printf("*sint_ptr222=%ld\n",*sint_ptr);
         //= *sint_ptr;  /* do something with the value */
      }
      break;
    case 2:    /* reset  */
      {
        s32_t *sint_ptr = value;
         //= *sint_ptr;  /* do something with the value */
      }
      break;
  };
}


struct sensor_inf
{
  char sensor_files[SENSOR_MAX][SENSOR_NAME_LEN + 1];
  /* (Sparse) list of sensors (table index),
   the actual "hot insertion" is done in lwip_privmib_init() */
  struct mib_list_rootnode sensor_list_rn;
};

struct sensor_inf sensor_addr_inf =
{
  {{0}},
  {
    NULL,
    NULL,
    NULL,
    NULL,
    MIB_NODE_LR,
    0,
    NULL,
    NULL,
    0
  }
};


static u16_t sensorentry_length(void* addr_inf, u8_t level);
static s32_t sensorentry_idcmp(void* addr_inf, u8_t level, u16_t idx, s32_t sub_id);
static void sensorentry_get_subid(void* addr_inf, u8_t level, u16_t idx, s32_t *sub_id);

static void sensorentry_get_object_def_q(void* addr_inf, u8_t rid, u8_t ident_len, s32_t *ident);
static void sensorentry_get_object_def_a(u8_t rid, u8_t ident_len, s32_t *ident, struct obj_def *od);
static void sensorentry_get_object_def_pc(u8_t rid, u8_t ident_len, s32_t *ident);
static void sensorentry_get_value_q(u8_t rid, struct obj_def *od);
static void sensorentry_get_value_a(u8_t rid, struct obj_def *od, u16_t len, void *value);
static void sensorentry_get_value_pc(u8_t rid, struct obj_def *od);
static void sensorentry_set_test_q(u8_t rid, struct obj_def *od);
static u8_t sensorentry_set_test_a(u8_t rid, struct obj_def *od, u16_t len, void *value);
static void sensorentry_set_test_pc(u8_t rid, struct obj_def *od);
static void sensorentry_set_value_q(u8_t rid, struct obj_def *od, u16_t len, void *value);
static void sensorentry_set_value_a(u8_t rid, struct obj_def *od, u16_t len, void *value);
static void sensorentry_set_value_pc(u8_t rid, struct obj_def *od);

/* sensorentry .1.3.6.1.4.1.26381.1.1.1 (.level0.level1)
   where level 0 is the object identifier (temperature) and level 1 the index */

 struct mib_external_node  sensorentry = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_EX,
  0,
  &sensor_addr_inf,
  /* 0 tree_levels (empty table) at power-up,
     2 when one or more sensors are detected */
  0,
  &sensorentry_length,
  &sensorentry_idcmp,
  &sensorentry_get_subid,
  
  &sensorentry_get_object_def_q,
  &sensorentry_get_value_q,
  &sensorentry_set_test_q,
  &sensorentry_set_value_q,

  &sensorentry_get_object_def_a,
  &sensorentry_get_value_a,
  &sensorentry_set_test_a,
  &sensorentry_set_value_a,

  &sensorentry_get_object_def_pc,
  &sensorentry_get_value_pc,
  &sensorentry_set_test_pc,
  &sensorentry_set_value_pc
};



//s32_t iftable_id = 1;
//struct mib_node* iftable_node = (struct mib_node*)&ifentry;
//struct mib_ram_array_node iftable2 = {
//  &noleafs_get_object_def,
//  &noleafs_get_value,
//  &noleafs_set_test,
//  &noleafs_set_value,
//  MIB_NODE_RA,
//  0,
//  &iftable_id,
//  &iftable_node
//};

///* interfaces .1.3.6.1.2.1.2 */
const mib_scalar_node interfaces_scalar2 = {
  &emmp_get_object_def,
  &emmp_get_value,
  &emmp_set_test,
  &emmp_set_value,
  MIB_NODE_SC,
  0
};

  const s32_t interfaces_ids2[1] = {2 };
  struct mib_node* const interfaces_nodes2[1] = {
  (struct mib_node*)&interfaces_scalar2, 
  };
  const struct mib_array_node interfaces2 = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  2,
  interfaces_ids2,
  interfaces_nodes2
};




/*             0 1 2 3 4 5 6 */
/* system .1.3.6.1.2.1.1 */
const mib_scalar_node sys_tem_scalar2 = {
  &emmp_get_object_def,
  &emmp_get_value,
  &emmp_set_test,
  &emmp_set_value,
  MIB_NODE_SC,
  0
};
const s32_t sys_tem_ids22[21] ={ 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14, \
15,16,17,18,19,20,21
};
struct mib_node* const sys_tem_nodes22[21] = {
  (struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
  (struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
  (struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
  (struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
	(struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
	(struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
	(struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
	(struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
	(struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
	(struct mib_node*)&sys_tem_scalar2, (struct mib_node*)&sys_tem_scalar2,
  (struct mib_node*)&sys_tem_scalar2
};


//struct mib_node* const sys_tem_nodes22[1] = {(struct mib_node*)&sys_tem_scalar22};
/* work around name issue with 'sys_tem', some compiler(s?) seem to reserve 'system' */


const struct mib_array_node sys_tem2 = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  21,
  sys_tem_ids22,
  sys_tem_nodes22
};

// mqtt״̬  ���Խڵ�

const s32_t test_ids[7] ={ 1, 2, 3, 4, 5, 6, 7 };
struct mib_node* const test_nodes[7] = {
  (struct mib_node*)&sys_tem_scalar2,(struct mib_node*)&sys_tem_scalar2
};

const struct mib_array_node test_node = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  7,
  test_ids,
  test_nodes
};

/* mib-2 .1.3.6.1.2.1 */
#if LWIP_TCP
#define MIB2_GROUPS 8
#else
#define MIB2_GROUPS 7
#endif
const s32_t mib2_ids2[MIB2_GROUPS] =
{
  1,
  2,
  3,
  4,
  5,
#if LWIP_TCP
  6,
#endif
  7,
  11
};

//struct mib_node* const mib2_nodes2[8] = {
//  (struct mib_node*)&sys_tem2,
//};
//  .1.3.6.1.4.1.(1,2,3,4,5,6...11)  �ڵ�1,2,3
struct mib_node* const mib2_nodes2[8] = {
  (struct mib_node*)&sys_tem2,(struct mib_node*)&sys_tem2,(struct mib_node*)&test_node
};


/* mgmt .1.3.6.1.4.1.(1,2,3,4,5,6...11)  */
const struct mib_array_node mib22 = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  8,
  mib2_ids2,
  mib2_nodes2
};







/* private .1.3.6.1.4.1 */
const s32_t private_ids[1] = { 1 };
struct mib_node* const private_nodes[1] = { (struct mib_node* const)&mib22 };
//static struct mib_node* const private_nodes[1] = { (struct mib_node* const)&sensorentry };
const struct mib_array_node mib_private = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  private_ids,
  private_nodes
};



struct mib_list_rootnode moduleEntry_root = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_LR,
  0,
  NULL,
  NULL,  0,
};

/* moduleEntry  .1.3.6.1.4.1.26381.1.3.1    */
const s32_t moduleEntry_ids[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
struct mib_node* const moduleEntry_nodes[8] = {
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root,
  (struct mib_node* const)&moduleEntry_root
};
 
const struct mib_array_node moduleEntry = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  8,
  moduleEntry_ids,
  moduleEntry_nodes
};
 
/* modulesTable  .1.3.6.1.4.1.26381.1.3    */
s32_t modulesTable_ids[1] = { 1 };
struct mib_node* modulesTable_nodes[1] = {
  (struct mib_node* const)&moduleEntry
};
 
struct mib_ram_array_node modulesTable = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_RA,
  0,
  modulesTable_ids,
  modulesTable_nodes
};






const mib_scalar_node emmp_scalar2= {  
  &emmp_get_object_def,
  &emmp_get_value,
  &emmp_set_test,
  &emmp_set_value,
  MIB_NODE_SC,
  0
};
 
/* emmp  .1.3.6.1.4.1.26381.1    */
const s32_t emmp_ids[1] = {1};
struct mib_node* const emmp_nodes[3] = {
  (struct mib_node* const)&emmp_scalar2,
  (struct mib_node* const)&emmp_scalar2,
  (struct mib_node* const)&modulesTable
};
 
const struct mib_array_node emmp = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  3,
  emmp_ids,
  emmp_nodes
};
 
/* lwip  .1.3.6.1.4.1.26381    */
const s32_t lwip_ids2[1] = { 1 };
struct mib_node* const lwip_nodes2[1] = {
  (struct mib_node* const)&emmp
};
 
const struct mib_array_node lwip2 = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  lwip_ids2,
  lwip_nodes2
};
 
/* enterprises  .1.3.6.1.4.1    */
const s32_t enterprises_ids2[1] = { 200};
struct mib_node* const enterprises_nodes2[1] = {
  (struct mib_node* const)&lwip2
};
 
const struct mib_array_node enterprises2 = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  enterprises_ids2,
  enterprises_nodes2
};
 
/* private  .1.3.6.1.4    */
const s32_t private_ids2[1] = { 1 };
struct mib_node* const private_nodes2[1] = {
  (struct mib_node* const)&enterprises2
};
 
const struct mib_array_node private = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  private_ids2,
  private_nodes
};

#endif
/**
 * Initialises this private MIB before use.
 * @see main.c
 */
void
lwip_privmib_init(void)
{
#if SENSORS_USE_FILES && SENSORS_SEARCH_FILES
  char *buf, *ebuf, *cp;
  size_t bufsize;
  int nbytes;
  struct stat sb;
  struct dirent *dp;
  int fd;
#else /* SENSORS_USE_FILES && SENSORS_SEARCH_FILES */
  int i;
#endif /* SENSORS_USE_FILES && SENSORS_SEARCH_FILES */

  printf("SNMP private MIB start, detecting sensors.\n");

#if SENSORS_USE_FILES && SENSORS_SEARCH_FILES
  /* look for sensors in sensors directory */
  fd = open(SENSORS_DIR, O_RDONLY);
  if (fd > -1)
  {
    fstat(fd, &sb);
    bufsize = sb.st_size;
    if (bufsize < sb.st_blksize)
    {
      bufsize = sb.st_blksize;
    }
    buf = malloc(bufsize);
    if (buf != NULL)
    {
      do
      {
        long base;
        
        nbytes = getdirentries(fd, buf, bufsize, &base);
        if (nbytes > 0)
        {
          ebuf = buf + nbytes;
          cp = buf;
          while (cp < ebuf)
          {
            dp = (struct dirent *)cp;
            if (isdigit(dp->d_name[0]))
            {
              struct mib_list_node *dummy;
              unsigned char index;
              
              index = dp->d_name[0] - '0';
              snmp_mib_node_insert(&sensor_addr_inf.sensor_list_rn,index,&dummy);
							//snmp_mib_node_insert(&exEntry_root, exEntry_root.count + 1, &if_node);//

              strncpy(&sensor_addr_inf.sensor_files[index][0],dp->d_name,SENSOR_NAME_LEN);
              printf("%s\n", sensor_addr_inf.sensor_files[index]);
            }
            cp += dp->d_reclen;
          }
        } 
      }
      while (nbytes > 0);
    
      free(buf);
    }
    close(fd);
  }
#else /* SENSORS_USE_FILES && SENSORS_SEARCH_FILES */
  for (i = 0; i < SENSOR_COUNT; i++) {
    struct mib_list_node *dummy;
		struct mib_list_node *if_node = NULL;	
    s32_t index = i;
    char name[256];
    sprintf(name, "%d.txt", i); 
   // printf("%d\n",index);	
    printf("before insert\n");		
	  snmp_mib_node_insert(&sensor_addr_inf.sensor_list_rn,index,&dummy);
		printf("after insert\n");
   //snmp_mib_node_insert(&moduleEntry_root,moduleEntry_root.count + 1, &if_node); //snmp_scalar		
		//snmp_mib_node_insert(&emmp_scalar2,index, &if_node);
		//test();		
    strncpy(&sensor_addr_inf.sensor_files[index][0], name, SENSOR_NAME_LEN);//snmp_scalar
    printf("%s\n", sensor_addr_inf.sensor_files[index]);
#if !SENSORS_USE_FILES
    /* initialize sensor value to != zero */
    sensor_values[i] = 11 * (i+1);
#endif /* !SENSORS_USE_FILES */
  }
#endif /* SENSORS_USE_FILE && SENSORS_SEARCH_FILES */
  if (sensor_addr_inf.sensor_list_rn.count != 0)
  {
    /* enable sensor table, 2 tree_levels under this node
       one for the registers and one for the index */
    sensorentry.tree_levels = 2;
   printf("%d\n", sensorentry.tree_levels);			
  }
}


static u16_t
sensorentry_length(void* addr_inf, u8_t level)
{
  struct sensor_inf *sensors = (struct sensor_inf *)addr_inf;

  if (level == 0)
  {
    /* one object (temperature) */
    return 1;
  }
  else if (level == 1)
  {
    /* number of sensor indexes */
    return sensors->sensor_list_rn.count;
		printf("%d\n", sensorentry.tree_levels);
  }
  else
  {
    return 0;
  }
}


static s32_t
sensorentry_idcmp(void* addr_inf, u8_t level, u16_t idx, s32_t sub_id)
{
  struct sensor_inf *sensors = (struct sensor_inf *)addr_inf;
  
  if (level == 0)
  {
    return ((s32_t)(idx + 1) - sub_id);
  }
  else if (level == 1)
  {
    struct mib_list_node *ln;
    u16_t i;
  
    i = 0;
    ln = sensors->sensor_list_rn.head;
    while (i < idx)
    {
      i++;
      ln = ln->next;
    }
    LWIP_ASSERT("ln != NULL", ln != NULL);
    return (ln->objid - sub_id);
  }
  else
  {
    return -1;
  }
}

static void
sensorentry_get_subid(void* addr_inf, u8_t level, u16_t idx, s32_t *sub_id)
{
  struct sensor_inf *sensors = (struct sensor_inf *)addr_inf;

  if (level == 0)
  {
    *sub_id = idx + 1;
  }
  else if (level == 1)
  {
    struct mib_list_node *ln;
    u16_t i;

    i = 0;
    ln = sensors->sensor_list_rn.head;
    while (i < idx)
    {
      i++;
      ln = ln->next;
    }
    LWIP_ASSERT("ln != NULL", ln != NULL);
    *sub_id = ln->objid;
  }
}

/**
 * Async question for object definition
 */
static void
sensorentry_get_object_def_q(void* addr_inf, u8_t rid, u8_t ident_len, s32_t *ident)
{
  s32_t sensor_register, sensor_address;

  LWIP_UNUSED_ARG(addr_inf);
  LWIP_UNUSED_ARG(rid);

  ident_len += 1;
  ident -= 1;

  /* send request */
  sensor_register = ident[0];
  sensor_address = ident[1];
  LWIP_DEBUGF(SNMP_MIB_DEBUG,("sensor_request reg=%"S32_F" addr=%"S32_F"\n",
                              sensor_register, sensor_address));
  /* fake async quesion/answer */
  snmp_msg_event(rid);//request id
}

static void
sensorentry_get_object_def_a(u8_t rid, u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  LWIP_UNUSED_ARG(rid);

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if(1)// (ident_len == 2)
  { 
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    od->instance = MIB_OBJECT_TAB;
    od->access = MIB_OBJECT_READ_WRITE;
    od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
    od->v_len = sizeof(s32_t);
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("sensorentry_get_object_def_a: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void
sensorentry_get_object_def_pc(u8_t rid, u8_t ident_len, s32_t *ident)
{
  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(ident_len);
  LWIP_UNUSED_ARG(ident);
  /* nop */
}

static void
sensorentry_get_value_q(u8_t rid, struct obj_def *od)
{
  LWIP_UNUSED_ARG(od);

  /* fake async quesion/answer */
  snmp_msg_event(rid);
}

static void
sensorentry_get_value_a(u8_t rid, struct obj_def *od, u16_t len, void *value)
{
  s32_t i;
  s32_t *temperature = (s32_t *)value;
#if SENSORS_USE_FILES
  FILE* sensf;
  char senspath[sizeof(SENSORS_DIR)+1+SENSOR_NAME_LEN+1] = SENSORS_DIR"/";
#endif /* SENSORS_USE_FILES */

  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(len);

  i = od->id_inst_ptr[1];
#if SENSORS_USE_FILES
  strncpy(&senspath[sizeof(SENSORS_DIR)],
          sensor_addr_inf.sensor_files[i],
          SENSOR_NAME_LEN);
  sensf = fopen(senspath,"r");
  if (sensf != NULL)
  {
    fscanf(sensf,"%"S32_F,temperature);
    fclose(sensf);
  }
#else /* SENSORS_USE_FILES */
  if (i <= SENSOR_COUNT) {
    *temperature = sensor_values[i];
  }
#endif /* SENSORS_USE_FILES */
}

static void
sensorentry_get_value_pc(u8_t rid, struct obj_def *od)
{
  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(od);
  /* nop */
}

static void
sensorentry_set_test_q(u8_t rid, struct obj_def *od)
{
  LWIP_UNUSED_ARG(od);
  /* fake async quesion/answer */
  snmp_msg_event(rid);
}

static u8_t
sensorentry_set_test_a(u8_t rid, struct obj_def *od, u16_t len, void *value)
{
  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(od);
  LWIP_UNUSED_ARG(len);
  LWIP_UNUSED_ARG(value);
  /* sensors are read-only */
  return 1; /* 0 -> read only, != 0 -> read/write */
}

static void
sensorentry_set_test_pc(u8_t rid, struct obj_def *od)
{
  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(od);
  /* nop */
}

static void
sensorentry_set_value_q(u8_t rid, struct obj_def *od, u16_t len, void *value)
{
  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(od);
  LWIP_UNUSED_ARG(len);
  LWIP_UNUSED_ARG(value);
  /* fake async quesion/answer */
  snmp_msg_event(rid);
}

static void
sensorentry_set_value_a(u8_t rid, struct obj_def *od, u16_t len, void *value)
{
  s32_t i;
  s32_t *temperature = (s32_t *)value;
#if SENSORS_USE_FILES
  FILE* sensf;
  char senspath[sizeof(SENSORS_DIR)+1+SENSOR_NAME_LEN+1] = SENSORS_DIR"/";
#endif /* SENSORS_USE_FILES */

  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(len);

  i = od->id_inst_ptr[1];
#if SENSORS_USE_FILES
  strncpy(&senspath[sizeof(SENSORS_DIR)],
          sensor_addr_inf.sensor_files[i],
          SENSOR_NAME_LEN);
  sensf = fopen(senspath, "w");
  if (sensf != NULL)
  {
    fprintf(sensf, "%"S32_F, temperature);
    fclose(sensf);
  }
#else /* SENSORS_USE_FILES */
  if (i <= SENSOR_COUNT) {
    sensor_values[i] = *temperature;
  }
#endif /* SENSORS_USE_FILES */
}

static void
sensorentry_set_value_pc(u8_t rid, struct obj_def *od)
{
  LWIP_UNUSED_ARG(rid);
  LWIP_UNUSED_ARG(od);
  /* nop */
}


//�ַ�����ת���ַ���
static char* char_arrays_to_strings(u8_t *buff,u8 len)
{
	int i=0;
	char* b;
	for(i=0;i<len;i++)
	{
		b[i]=buff[i];
	}
	return b;
} 


//#endif /* LWIP_SNMP */
