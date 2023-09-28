/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* ���Ҫ�ڳ�����ʹ�ô˴���,���ڳ�����ע��ʹ����STC�����ϼ�����            */
/*---------------------------------------------------------------------*/

#include	"config.h"
#include	"STC32G_GPIO.h"
#include	"STC32G_ADC.h"
#include	"STC32G_UART.h"
#include	"STC32G_DMA.h"
#include	"STC32G_NVIC.h"
#include	"STC32G_Delay.h"
#include	"STC32G_Switch.h"
#include 	"stdio.h"
#include  "adc.h"
#include  "led.h"
#include "UART_DMA.h"

/*************   ����˵��   ***************

��������ʾ��·ADC DMA����.

��ʼ��ʱ�Ȱ�ҪADCת������������Ϊ��������.

����������������(DMA)���ܣ�����ͨ��һ��ѭ���ɼ��������Զ���ŵ�DMA�����xdata�ռ�.

ͨ������2(P4.6 P4.7)��DMA�����xdata�ռ����յ������ݷ��͸���λ����������115200,N,8,1

�ö�ʱ���������ʷ�����������ʹ��1Tģʽ(���ǵͲ�������12T)����ѡ��ɱ�������������ʱ��Ƶ�ʣ�����߾���.

����ʱ, ѡ��ʱ�� 22.1184MHz (�����������ļ�"config.h"���޸�).

******************************************/

/*************	���س�������	**************/
#define ADC_value_num (AX_NUM * AY_NUM)
#define PACK_LEN (8 + ADC_value_num)
#define sof 0x5aa5

/*************	���ر�������	**************/

/*************	���غ�������	**************/

void Uart1SendData(u8 *puts, u32 len);
u16 CalChecksum(u8 * put_data, u16 len);
/*************  �ⲿ�����ͱ������� *****************/
Adc_value adc_read;
/******************** UART���� ********************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;		//�ṹ����

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//ģʽ,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;			//ѡ�����ʷ�����, BRT_Timer2 (ע��: ����2�̶�ʹ��BRT_Timer2, ���Բ���ѡ��)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//������,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//��������,   ENABLE �� DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//��ʼ������2 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//�ж�ʹ��, ENABLE/DISABLE; ���ȼ�(�͵���) Priority_0,Priority_1,Priority_2,Priority_3

	UART1_SW(UART1_SW_P30_P31);		//UART2_SW_P10_P11,UART2_SW_P46_P47
}


/******************** task A **************************/
void main(void)
{

	u8 serial_frame[PACK_LEN] = { 0 };
	u8 adc_raw_value[ADC_value_num] = { 0 };
	u16 checksum = 0; 
	
	u32 time_1ms = 0;

	WTST = 0;		//���ó���ָ����ʱ��������ֵΪ0�ɽ�CPUִ��ָ����ٶ�����Ϊ���
	EAXSFR();		//��չSFR(XFR)����ʹ�� 
	CKCON = 0;      //��߷���XRAM�ٶ�
	
  led_GPIO_config();

	UART_config();
	ADC_DMA_Init();

	
	EA = 1;
	
	
	while(1)
	{
		time_1ms ++;

		if(time_1ms % 100 == 0)
		{
			LED_GREEN = ~ LED_GREEN;		
		}
	
		
		if(time_1ms % 4 == 0)
		{
			Get_Voltage_Value(&adc_read);
		}

		if(time_1ms % 4 == 0)
		{
			u8 ax = 0, ay = 0;
			u16 data_index = 0;
			
			serial_frame[0] = sof;
			serial_frame[1] = sof >> 8;
			
			serial_frame[2] = 0x01;
			
			serial_frame[3] = (PACK_LEN - 2) & 0x00ff;
			serial_frame[4] = (PACK_LEN - 2) >> 8;
			
			serial_frame[5] = 0x01;
			ProcessADCvalue(&adc_read);
			
			for(ax = 0; ax < 16; ++ax)
			{
				for(ay = 0; ay < 16; ++ay)
				{
					serial_frame[6 + data_index] = adc_read.processed_adc_value[ax][ay];
					data_index ++;
				}
			}
			data_index = 0;
			
			checksum = CalChecksum(&serial_frame[0], PACK_LEN - 2);
			serial_frame[PACK_LEN - 2] = checksum & 0x00ff;
			serial_frame[PACK_LEN - 1] = checksum >> 8;
		
			Uart1SendData(serial_frame, PACK_LEN);
		}
		
		delay_ms(1);
	}
	
}


u16 CalChecksum(u8 * put_data, u16 len)
{
    u16 sum = 0;
	  u32 i;
    for(i = 0; i < len; ++i)
    {
        sum += put_data[i];
    }
    return sum;
}

void Uart1SendData(u8 *puts, u32 len)
{
		u32 i = 0;
		for(i = 0; i < len; ++i)
    {
	  	TX1_write2buff(puts[i]);
		}
}

