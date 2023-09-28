#ifndef __74HC4051B_h
#define __74HC4051B_h
#include "gd32e10x.h"
#include "string.h"
#include "systick.h"
#include "stdbool.h"
#include "flash.h"

#define GREEN_LED(a)	if (a)	\
    gpio_bit_set(GPIOA, GPIO_PIN_15);\
    else		\
    gpio_bit_reset(GPIOA, GPIO_PIN_15)			

#define UART_TX(a)	if (a)	\
    gpio_bit_set(GPIOA, GPIO_PIN_9);\
    else		\
    gpio_bit_reset(GPIOA, GPIO_PIN_9)							

#define XC_A(a)	if (a)	\
    gpio_bit_set(GPIOA, GPIO_PIN_8);\
    else		\
    gpio_bit_reset(GPIOA, GPIO_PIN_8)				

#define XC_B(a)	if (a)	\
    gpio_bit_set(GPIOA, GPIO_PIN_11);\
    else		\
    gpio_bit_reset(GPIOA, GPIO_PIN_11)				

#define XC_C(a)	if (a)	\
    gpio_bit_set(GPIOA, GPIO_PIN_12);\
    else		\
    gpio_bit_reset(GPIOA, GPIO_PIN_12)				


#define SENSOR_POS_X 32
#define SENSOR_POS_Y 64
#define VOLT_MULT 75

typedef enum {
    SINGLE_VCC_READ,
    TOTAL_VCC_READ
} ADC_MODE_TYPE_T;


typedef struct {
    uint8_t printf_buffer[128];
    uint16_t adc_raw_value[SENSOR_POS_X][SENSOR_POS_Y];
    uint8_t adc_raw_value_last[SENSOR_POS_X][SENSOR_POS_Y];
		uint8_t adc_cali_value_display[4][43];
    uint32_t adc_total_vcc_value[SENSOR_POS_Y];
    uint8_t adc_cali_value[SENSOR_POS_X][SENSOR_POS_Y];
    uint8_t x_max;
    uint8_t y_max;
} process_handle_t;

void adc_value_read(process_handle_t *process_handle, uint8_t count_y, ADC_MODE_TYPE_T mode_type);
void Hc4051IoInit(void);
void select_y_control_volt(process_handle_t *process_handle, uint8_t y_value, ADC_MODE_TYPE_T adc_mode);
void adc_calculation_calibration(process_handle_t *process_handle);
void select_x_control(uint8_t x_value);
void adc_calculation_calibration_once(process_handle_t *process_handle);
void visual_remapping(process_handle_t *process_handle);

#endif
