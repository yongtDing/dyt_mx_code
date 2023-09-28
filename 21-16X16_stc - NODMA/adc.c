#include  "adc.h"
#include	"config.h"
#include	"STC32G_GPIO.h"
#include	"STC32G_ADC.h"
#include	"STC32G_UART.h"
#include	"STC32G_DMA.h"
#include	"STC32G_NVIC.h"
#include	"STC32G_Delay.h"
#include	"STC32G_Switch.h"
#include 	"stdio.h"
#include 	<string.h>


/*************	���ر�������	**************/

u16 xdata DmaAdBuffer[ADC_CH][ADC_DATA] = {0};
u16 adc_value[ADC_CH] = {0};
Adc_value processe_adc_value;


#define ADC_DELAY_TIME (55 * 42) //9 ns * 120

void Hc4051Delay(u32 num)
{
    while(num --);
}

/******************** IO������ ********************/
void	ADC_GPIO_config(void)
{
  P1_MODE_IN_HIZ(GPIO_Pin_0);		//P1.0~P1.7 ����Ϊ�������� adc
	
	P3_MODE_OUT_PP(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7);
	P2_MODE_OUT_PP(GPIO_Pin_All);
	P0_MODE_OUT_PP(GPIO_Pin_0 | GPIO_Pin_1);
	P1_MODE_OUT_PP(GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3 | GPIO_Pin_2); //16_selector
	
	P3_PULL_UP_DISABLE(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7);
	P2_PULL_UP_DISABLE(GPIO_Pin_All);
	P0_PULL_UP_DISABLE(GPIO_Pin_0 | GPIO_Pin_1);
	P1_PULL_UP_DISABLE(GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3 | GPIO_Pin_2);
	
	P3_DRIVE_HIGH(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7);
	P2_DRIVE_HIGH(GPIO_Pin_All);
	P0_DRIVE_HIGH(GPIO_Pin_0 | GPIO_Pin_1);
	
	P3_SPEED_HIGH(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7);
	P2_SPEED_HIGH(GPIO_Pin_All);
  P0_SPEED_HIGH(GPIO_Pin_0 | GPIO_Pin_1);
	P1_SPEED_HIGH(GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3 | GPIO_Pin_2);
	
}


/******************** ADC ���� ********************/
void ADC_config(void)
{
	ADC_InitTypeDef		ADC_InitStructure;		//�ṹ����

	ADC_InitStructure.ADC_SMPduty   = 31;		//ADC ģ���źŲ���ʱ�����, 0~31��ע�⣺ SMPDUTY һ����������С�� 10��
	ADC_InitStructure.ADC_CsSetup   = 0;		//ADC ͨ��ѡ��ʱ����� 0(Ĭ��),1
	ADC_InitStructure.ADC_CsHold    = 1;		//ADC ͨ��ѡ�񱣳�ʱ����� 0,1(Ĭ��),2,3
	ADC_InitStructure.ADC_Speed     = ADC_SPEED_2X16T;		//���� ADC ����ʱ��Ƶ��	ADC_SPEED_2X1T~ADC_SPEED_2X16T
	ADC_InitStructure.ADC_AdjResult = ADC_RIGHT_JUSTIFIED;	//ADC�������,	ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
	ADC_Inilize(&ADC_InitStructure);		//��ʼ��
	ADC_PowerControl(ENABLE);						//ADC��Դ����, ENABLE��DISABLE
	NVIC_ADC_Init(DISABLE,Priority_0);		//�ж�ʹ��, ENABLE/DISABLE; ���ȼ�(�͵���) Priority_0,Priority_1,Priority_2,Priority_3
}

/******************** DMA ���� ********************/
void	ADC_DMA_config(void)
{
	DMA_ADC_InitTypeDef		DMA_ADC_InitStructure;		//�ṹ����

	DMA_ADC_InitStructure.DMA_Enable = ENABLE;			//DMAʹ��  	ENABLE,DISABLE
	DMA_ADC_InitStructure.DMA_Channel = 0x0001;			//ADCͨ��ʹ�ܼĴ���, 1:ʹ��, bit15~bit0 ��Ӧ ADC15~ADC0
	DMA_ADC_InitStructure.DMA_Buffer = (u16)DmaAdBuffer;	//ADCת�����ݴ洢��ַ
	DMA_ADC_InitStructure.DMA_Times = ADC_4_Times;	//ÿ��ͨ��ת������, ADC_1_Times,ADC_2_Times,ADC_4_Times,ADC_8_Times,ADC_16_Times,ADC_32_Times,ADC_64_Times,ADC_128_Times,ADC_256_Times
	DMA_ADC_Inilize(&DMA_ADC_InitStructure);		//��ʼ��
	NVIC_DMA_ADC_Init(ENABLE,Priority_0,Priority_0);		//�ж�ʹ��, ENABLE/DISABLE; ���ȼ�(�͵���) Priority_0~Priority_3; �������ȼ�(�͵���) Priority_0~Priority_3
	DMA_ADC_TRIG();		//��������ת��
}


void ADC_DMA_Init(void)
{
	ADC_GPIO_config();
	ADC_config();
	ADC_DMA_config();
}


u16 GetAdcValue(u8 index)
{
		if(index < ADC_CH)
		{
			return DmaAdBuffer[0][5]; //[i][j] ->i- channel_num / j - adc_4times [0]-[3]; [4]-channel_num; [5] -4times avergr
		}
		else
		{
			return 0;
		}	
}


void AX_scan(u8 index)	
{
		u16  AX_scan_array[16] =  {  0x0001,0x0002,0x0004,0x0008,
																 0x0010,0x0020,0x0040,0x0080,
																 0x0100,0x0200,0x0400,0x0800,
																 0x1000,0x2000,0x4000,0x8000  };
		
			AX15( AX_scan_array[index] & 0x0001);
			AX14((AX_scan_array[index] & 0x0002) >>  1);
			AX13((AX_scan_array[index] & 0x0004) >>  2);
			AX12((AX_scan_array[index] & 0x0008) >>  3);
			AX11((AX_scan_array[index] & 0x0010) >>  4);
			AX10((AX_scan_array[index] & 0x0020) >>  5);
			AX9 ((AX_scan_array[index] & 0x0040) >>  6);
			AX8 ((AX_scan_array[index] & 0x0080) >>  7);
			AX7 ((AX_scan_array[index] & 0x0100) >>  8);
			AX6 ((AX_scan_array[index] & 0x0200) >>  9);
			AX5 ((AX_scan_array[index] & 0x0400) >> 10);
			AX4 ((AX_scan_array[index] & 0x0800) >> 11);
			AX3 ((AX_scan_array[index] & 0x1000) >> 12);
			AX2 ((AX_scan_array[index] & 0x2000) >> 13);
			AX1 ((AX_scan_array[index] & 0x4000) >> 14);
			AX0 ((AX_scan_array[index] & 0x8000) >> 15);

}

void AY_scan(u8 index)
{			
			CS0(index  & 0x0001);
			CS1((index & 0x0002) >> 1);
			CS2((index & 0x0004) >> 2);
			CS3((index & 0x0008) >> 3);	
}

void Get_Voltage_Value(Adc_value *process_adc_value)
{
	u8 ax, ay;
	u8 adc_rank[16] = {0,1,2,3,4,5,6,7,15,14,13,12,11,10,9,8};

	for(ay = 0; ay < AY_NUM; ay++)
	{
		AY_scan(ay);
		for(ax = 0; ax < AY_NUM; ax++)
		{
			AX_scan(ax);
			Hc4051Delay(ADC_DELAY_TIME);
//			process_adc_value->raw_value[ax][adc_rank[ay]]  = (255 *  (u32)GetAdcValue(0)) / 4096;
			process_adc_value->raw_value[ax][adc_rank[ay]]  = GetAdcValue(0) / 2;
		}
				if(ax >= AX_NUM);
				{
					ax = 0;
				}
	}
			if(ay >= AY_NUM )
			{
				ay = 0;
			}
//		printf("%u\n",process_adc_value->raw_value[0][0]);
}

void ProcessADCvalue(Adc_value *process_adc_value)
{
		u16 Vref = 4096;
		u8 flag_sum = 1;
		u8 display_threshold_value = 20;
		u8 AX, AY;
		u8 i; 
		while(flag_sum)
		{					
				for(AY = 0; AY < AY_NUM; ++AY)
				{
						for(AX = 0; AX < AX_NUM; ++AX)
						{
							process_adc_value->sumAX[AY] += process_adc_value->raw_value[AX][AY];
						}
				}	
				flag_sum = 0;
				AX = 0;
				AY = 0;
		}
		
		while(flag_sum == 0)
		{
				
				for(AY = 0; AY < AY_NUM; ++AY)
				{			
							for(AX = 0; AX < AX_NUM; ++AX)
							{	
								if(process_adc_value->raw_value[AX][AY] > display_threshold_value)
								{									
									process_adc_value->processed_adc_value[AX][AY] 
										=	 (255 * (u32)process_adc_value->raw_value[AX][AY]) 
											/ (Vref - (process_adc_value->sumAX[AY] - process_adc_value->raw_value[AX][AY]));
								}
								else
								{
									process_adc_value->processed_adc_value[AX][AY] = 0;
								}
							}
				} 	
				AX = 0;
				AY = 0;
				memset(process_adc_value->sumAX, 0x0000 , sizeof(process_adc_value->sumAX));
				
				flag_sum = 1;
		}
}

