#include "main.h"
#include "rcc.h"
#include "lcd.h"
#include "adc.h"
#include "seg.h"
#include "tim.h"
#include "uart.h"
#include "i2c_hal.h"
#include "key_led.h"
#include "ds18b20_hal.h"

#define K	3.3/4096   //adc xi_shu

void key_proc(void);
void led_proc(void);
void pwm_proc(void);
void adc_proc(void);
void lcd_proc(void);
void seg_proc(void);
void uart_proc(void);
void ds18b20_proc(void);

//for delay
__IO uint32_t uwTick_key, uwTick_led, uwTick_lcd, uwTick_adc, uwTick_dsb, uwTick_seg, uwTick_uart;

//for key
uint8_t key_val, key_old, key_down, key_up;
uint32_t count_jia, count_jian;

//for led
uint8_t led;

//for seg
uint8_t seg_flag;

//for lcd
uint8_t string_lcd[21];
uint8_t which_index;
uint8_t which_para;
uint8_t clr_flag;

//for adc
float AO1_V;
float AO2_V;

//for pwm 
uint32_t pwm_t;
uint32_t pwm_d;
float pwm_duty;

//for eeprom
uint8_t eeprom_W[2] = {'g', 0};
uint8_t eeprom_R[2];

//for uart
uint8_t read_uart[6];
char send_uart[200];
uint8_t ST_flag;
uint8_t PARA_flag;
uint8_t set_flag;
uint8_t cpl_flag;

//for ds18b20
float temp;

//for task
uint8_t temp_max = 30;
uint8_t temp_max_show = 30;
uint8_t which_AO;
uint8_t which_AO_temp;
uint16_t setting_times;

int main(void)
{

  HAL_Init();
	//HAL_Delay(1);
  SystemClock_Config();

	key_led_init();
	
	MX_ADC2_Init();
	
	MX_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1, read_uart, 1);
	
	I2CInit();
	eeprom_read(eeprom_R, 0, 2);
	if(eeprom_R[0] != 'g')
		eeprom_write(eeprom_W, 0, 2);
	else
	{
		setting_times = eeprom_R[1];
	}
	
	SEG_Init();
	SEG_disp(1,2,3);
	
	MX_TIM3_Init();
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_Base_Start(&htim3);

	LCD_Init();
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	LCD_Clear(Black);
	
  while (1)
  {
		key_proc();
		led_proc();
		lcd_proc();
		adc_proc();
		seg_proc();
		uart_proc();
		ds18b20_proc();
  }

}

void key_proc(void)
{
	if(uwTick - uwTick_key < 50)	return;
	uwTick_key = uwTick;
	
	key_val = read_key();
	key_down = key_val & (key_val ^ key_old);
	key_up  = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 2)
	{
		if(which_index == 1)
		{
			which_para ^= 1;
		}
	}
	
	else if(key_down == 3)
	{
		if(which_index == 1)
		{
			if(which_para == 0)
			{
				if(temp_max_show<40)
					temp_max_show++;
			}			
			else if(which_para == 1)
			{
				which_AO_temp ^= 1;
			}
		}
	}
	
	else if(key_down == 4)
	{
		if(which_index == 1)
		{
			if(which_para == 0)
			{
				if(temp_max_show>20)
					temp_max_show--;
			}
			else if(which_para == 1)
			{
				which_AO_temp ^= 1;
			}
		}	
	}

	//this if must put outside,need to think about it
	if((which_index == 1) && (which_para == 0))
		{	
			switch(read_key())
			{
					case 3:
						count_jia++;
						if(count_jia >= 17)
							if(temp_max_show<40)
								temp_max_show++;
						break;
						
					case 4:
						count_jian++;
						if(count_jian >= 17)
							if(temp_max_show>20)
								temp_max_show--;
						break;		

					case 0:
						count_jia=0;
						count_jian=0;
						break;
			}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	HAL_Delay(50);
	
	which_index ^= 1;
	which_para = 0;
	clr_flag = 1;
	//LCD_Clear(Black);
	//LCD_SetBackColor(Black);
	
	if(which_index == 0)
	{
		if((temp_max == temp_max_show) && (which_AO == which_AO_temp))
		{
		}
			
		else
		{
			setting_times++;
			temp_max = temp_max_show;
			which_AO = which_AO_temp;
		}	
		
		eeprom_W[0] = setting_times;
		eeprom_write(eeprom_W, 1, 1);
	}
}


void ds18b20_proc(void)
{
	if(uwTick - uwTick_dsb < 800)	return;
	uwTick_dsb = uwTick;
	
	if(which_index == 0)
		temp = ds18b20_read()/16.0;

}


void led_proc(void)
{
	if(uwTick - uwTick_led < 200)	return;
	uwTick_led = uwTick;

	if(temp > temp_max)
		led ^= 0x80;
	else 
		led &= ~0x80;

	led_disp(led);
}

void seg_proc(void)
{
	if(uwTick - uwTick_seg < 2000)	return;
	uwTick_seg = uwTick;
	
	if(seg_flag == 0)
	{
		SEG_disp(12, temp_max/10, temp_max%10);
	}
	
	else
	{
		if(which_AO == 0)
			SEG_disp(10, 0, 1);
		else 
			SEG_disp(10, 0, 2);
	}
	
	seg_flag ^= 1;

}

void adc_proc(void)
{
	if(uwTick - uwTick_adc < 50)	return;
	uwTick_adc = uwTick;

	HAL_Delay(1);
	get_AO1();
	HAL_Delay(1);
	AO1_V = (float)get_AO1()*K;
	
	HAL_Delay(1);
	get_AO2();
	HAL_Delay(1);
	AO2_V = (float)get_AO2()*K;
}

void lcd_proc(void)
{
	if(uwTick - uwTick_lcd < 50)	return;
	uwTick_lcd = uwTick;
	
	if(clr_flag == 1)
	{
		LCD_Clear(Black);
		clr_flag = 0;
	}
		
	if(which_index == 0)
	{
		LCD_SetBackColor(Black);
		
		sprintf((char *)string_lcd, "         Main");
		LCD_DisplayStringLine(Line1, string_lcd);	
		
		sprintf((char *)string_lcd, "AO1: %.2f", AO1_V);
		LCD_DisplayStringLine(Line3, string_lcd);	

		sprintf((char *)string_lcd, "AO1: %.2f", AO2_V);
		LCD_DisplayStringLine(Line4, string_lcd);	

		sprintf((char *)string_lcd, "PWM2: %.0f %% ", pwm_duty*100);
		LCD_DisplayStringLine(Line5, string_lcd);	

		sprintf((char *)string_lcd, "Temp: %.2f   ", temp);
		LCD_DisplayStringLine(Line6, string_lcd);	

		sprintf((char *)string_lcd, "N: %d  ", setting_times);
		LCD_DisplayStringLine(Line7, string_lcd);			
	}

	else 
	{
		LCD_SetBackColor(Black);
		sprintf((char *)string_lcd, "         Para");
		LCD_DisplayStringLine(Line1, string_lcd);		
	
		if(which_para == 0)
			LCD_SetBackColor(Yellow);
		else 
			LCD_SetBackColor(Black);
		sprintf((char *)string_lcd, "T: %d ", temp_max_show);
		LCD_DisplayStringLine(Line4, string_lcd);		
		
		if(which_para == 1)
			LCD_SetBackColor(Yellow);
		else 
			LCD_SetBackColor(Black);
		
		if(which_AO_temp == 0)
		{
			sprintf((char *)string_lcd, "X: AO1");
			LCD_DisplayStringLine(Line5, string_lcd);			
		}
		else 
		{
			sprintf((char *)string_lcd, "X: AO2");
			LCD_DisplayStringLine(Line5, string_lcd);			
		}
			
	}	
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			pwm_t = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2);
			pwm_duty = (float)pwm_d / (float)pwm_t;
		}
			
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			pwm_d = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
		}
	}
}

void uart_proc(void)
{
	if(uwTick - uwTick_uart < 1000)	return;
	uwTick_uart = uwTick;

	if(which_AO == 0)
	{
		if(AO1_V > pwm_duty*3.3)
		{
			sprintf(send_uart, "$%2.2f\r\n", temp);
			HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);
			led |= 0x01;
		}
		else 
			led &= ~0x01;
	}

	else if(which_AO == 1)
	{
		if(AO2_V > pwm_duty*3.3)
		{
			sprintf(send_uart, "$%2.2f\r\n", temp);
			HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);
			led |= 0x01;
		}
		else 
			led &= ~0x01;
	}
}


//bu ding chang zm ban?
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(read_uart[0] == 'S')
		ST_flag = 1;
	
	if(ST_flag == 1 && set_flag == 0) 
	{
		HAL_UART_Receive_IT(&huart1, read_uart, 3);
		set_flag = 1;
	}

	else if(ST_flag == 1 && set_flag == 1)
	{
		if(read_uart[0] == 'T' && read_uart[1] == 0x0D && read_uart[2] == 0x0A)
		{
			sprintf(send_uart, "$%2.2f\r\n", temp);
			HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);
			cpl_flag = 1;
		}
	
		if(cpl_flag == 1)
		{
			HAL_UART_Receive_IT(&huart1, read_uart, 1);
			cpl_flag = 0;
			set_flag = 0;
			ST_flag = 0;
		}
	}
	
	
	if(read_uart[0] == 'P')
		PARA_flag = 1;
	
	if(PARA_flag == 1 && set_flag == 0) 
	{
		HAL_UART_Receive_IT(&huart1, read_uart, 5);
		set_flag = 1;
	}

	else if(PARA_flag == 1 && set_flag == 1)
	{
		if(read_uart[0] == 'A' && read_uart[1] == 'R' && read_uart[2] == 'A' && read_uart[3] == 0x0D && read_uart[4] == 0x0A)
		{
			if(which_AO == 0)
			{
				sprintf(send_uart, "#%2d,AO1\r\n", temp_max);
				HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);
			}
			else if(which_AO == 1)
			{
				sprintf(send_uart, "#%2d,AO2\r\n", temp_max);
				HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);	
			}
			cpl_flag = 1;
		}
	
		if(cpl_flag == 1)
		{
			HAL_UART_Receive_IT(&huart1, read_uart, 1);
			cpl_flag = 0;
			set_flag = 0;
			PARA_flag = 0;
		}
	}
//	if(read_uart[0] == 'S' && read_uart[1] == 'T' && read_uart[2] == '\\' && read_uart[3] == 'r' && read_uart[4] == '\\' && read_uart[5] == 'n')
//	{
//		sprintf(send_uart, "$%2.2f\r\n", temp);
//		HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);
//	}
//	
//	else if(read_uart[0] == 'P' && read_uart[1] == 'A' && read_uart[2] == 'R' && read_uart[3] == 'A' && read_uart[4] == '\\' && read_uart[5] == 'r')
//	{
//		if(which_AO == 0)
//		{
//			sprintf(send_uart, "#%2d,AO1\r\n", temp_max);
//			HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);
//		}
//		else if(which_AO == 1)
//		{
//			sprintf(send_uart, "#%2d,AO2\r\n", temp_max);
//			HAL_UART_Transmit(&huart1, (uint8_t *)send_uart, strlen(send_uart), 50);	
//		}
//	}
//	
//	HAL_UART_Receive_IT(&huart1, read_uart, 6);
}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}


