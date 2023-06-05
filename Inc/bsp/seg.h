#include "main.h"

#define SER_L HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)
#define SER_H HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)

#define RCK_L HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define RCK_H HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)

#define SCK_L HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define SCK_H HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)



void SEG_Init(void);
void SEG_disp(uint8_t bit1, uint8_t bit2, uint8_t bit3);