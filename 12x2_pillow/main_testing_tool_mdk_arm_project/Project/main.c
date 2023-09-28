/*!
  \file  main.c
  \brief ADC0 regular channel with DMA

  \version 2017-12-26, V1.0.0, firmware for GD32E10x
  */

/*
   Copyright (c) 2017, GigaDevice Semiconductor Inc.

   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification, 
   are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this 
   list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.
   3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software without 
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
   IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
   OF SUCH DAMAGE.
   */

#include "gd32e10x.h"
#include "systick.h"
#include <stdio.h>
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "74hc4051bq.h"
#include "flash.h"
#include "main.h"


/*!
  \brief      main function
  \param[in]  none
  \param[out] none
  \retval     none
  */

#pragma pack (1)

#define LOG_PRINTF 0
#define adc_chanel_test 0
#define row_choose_test 1
#define column_choose_test 0

typedef struct {
    uint16_t sof;
    uint8_t tran_type;
    uint16_t len;
    uint8_t type;
    uint8_t adc_value[24];
    uint16_t checksum;
} SerialFrame_t;


#pragma pack ()

uint8_t printf_buffer[512] = {0};


bool timer3_interrupt = true;
uint32_t time_1ms = 0;

Hc4051AdcValue temp_adc_value;

SerialFrame_t serial_frame;

uint16_t CalChecksum(uint8_t * data, uint16_t len);

int main(void)
{
    systick_config();
    adc_init();
    Hc4051IoInit();
    //	nvic_irq_enable( TIMER3_IRQn, 2, 0 );
    //	timer_config(TIMER3, 1); //20ms

    usart0_init();
 
    while( 1 )
    {
        time_1ms ++;
        //LED��˸
        if(time_1ms % 500 == 0)
        {
            static bool led_flag = false;
            led_flag = !led_flag;
            GREEN_LED(led_flag);
        }

        //50Hz���ʲɼ���һ��ѹ������
        if(time_1ms % 50 == 0)
        {
            ReadAdcValue(&temp_adc_value);		
        }


      //��Լ25Hz�ϴ����е�����
        if(time_1ms % 50 == 0)
        {
						/*Serial is the data transmit to the upper PC*/
            serial_frame.sof = 0x5aa5;
            serial_frame.tran_type = 0x01;
            serial_frame.len = sizeof(serial_frame) - 2;
            serial_frame.type = 0x01;
						

             /*Relationship between Serial_frame and temp_adc_value */   
//						uint32_t Column_Choose_Control[24]={ 7,  8,  9, 10, 11, 12, 14, 15,
//																								16, 17, 18, 19,  6,  5,  4,  3,
//																								 2,  1, 26, 25, 24, 23, 22, 21};
							/*C13 & C20 IS CO-PIN*/					
							uint32_t Column_Choose_Control[24]={ 6,  7,  8,  9, 10, 11, 12, 13,
																									14, 15, 16, 17,  5,  4,  3,  2,
																									 1,  0, 23, 22, 21, 20, 19, 18};
						for(int i = 0; i < 24; ++i)
						{
							serial_frame.adc_value[i] = temp_adc_value.value[Column_Choose_Control[i]][0];
						}

						serial_frame.checksum = CalChecksum((uint8_t *)&serial_frame, sizeof(serial_frame) - 2);
					

						if (LOG_PRINTF)
							dma_send_data((uint8_t *)&printf_buffer, sizeof(printf_buffer));
					else
							dma_send_data((uint8_t *)&serial_frame, sizeof(serial_frame));
						
																														 
//																										

        }
        delay_1ms(1);
    }
}

uint16_t CalChecksum(uint8_t * data, uint16_t len)
{
    uint16_t sum = 0;
    for(int i = 0; i < len; ++i)
    {
        sum += data[i];
    }
    return sum;
}



void TIMER3_IRQHandler( void )
{
    if( SET == timer_interrupt_flag_get( TIMER3, TIMER_INT_FLAG_UP ) )
    {
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear( TIMER3, TIMER_INT_FLAG_UP );

    }
}

