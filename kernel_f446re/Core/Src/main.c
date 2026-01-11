/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "os.h"
#include "uart.h"


#define IDLE_PRIORITY  10

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

#define STACK_SIZE 128
static uint32_t task1_stack[STACK_SIZE];
static uint32_t task2_stack[STACK_SIZE];
static uint32_t task3_stack[STACK_SIZE];
static uint32_t task4_stack[STACK_SIZE];
static uint32_t task5_stack[STACK_SIZE];
static uint32_t idle_task_stack[STACK_SIZE];

volatile uint32_t trace1 = 0;
volatile uint32_t trace2 = 0;
volatile uint32_t trace3 = 0;
volatile uint32_t trace4 = 0;

volatile uint32_t trace_idle = 0;

static os_task_t *idle_task_handle;


#define LED1_ON()   (GPIOA->BSRR = (1 << 5));
#define LED1_OFF()  (GPIOA->BSRR = (1 << (5 + 16)));   // D13 OFF

#define LED2_ON()   (GPIOA->BSRR = (1 << 6)) //D12
#define LED2_OFF()  (GPIOA->BSRR = (1 << (6 + 16)))

#define LED3_ON()   (GPIOA->BSRR = (1 << 7)) //D11
#define LED3_OFF()  (GPIOA->BSRR = (1 << (7 + 16)))


void idle_task(void)
{
    while (1)
    {
        __WFI();   // Wait For Interrupt
        trace_idle++;
    }
}

/*
/////////////TEST TASKS INDIVUIDUAL STACK//////////////////////////
void task1(void)
{
    while (1)
    {
        uint32_t psp = os_get_psp();
        uart_print_psp("Task 1", psp);
        os_delay(500);
    }
}



void task2(void)
{
    while (1)
    {
        uint32_t psp = os_get_psp();
        uart_print_psp("Task 2", psp);
        os_delay(500);
    }
}


void task3(void)
{
    while (1)
    {
        uint32_t psp = os_get_psp();
        uart_print_psp("Task 3", psp);
        os_delay(500);
    }
}

void task4(void)
{
    while (1)
    {
        uint32_t psp = os_get_psp();
        uart_print_psp("Task 4", psp);
        os_delay(500);
    }
}
void task5(void)
{
    while (1)
    {
        uint32_t psp = os_get_psp();
        uart_print_psp("Task 5", psp);
        os_delay(500);
    }
}
////////////////////////////////////////////

*/


////////////// LEDS TEST OUTPUTS D13, D12 //////////////////////
void task1(void) {
    while (1) {

        trace1++;
        if ((tick_debug % 500) < 250){
            	                  LED1_ON();  //D13
            	        }else{
            	                  LED1_OFF();
            	        }

      os_delay(50);
    }
}

void task2(void) {
    while (1) {
    		trace2++;
    	   if ((tick_debug % 500) < 250){
    	                  LED2_ON();       //D12
    	        }else{
    	                  LED2_OFF();
    	        }
            //os_delay(200);

    }
}



/*

void task3(void) {
    while (1) {
    		trace3++;
    		os_delay(1000);
    }
}
*/

void led_setup(void){
	  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	  // Limpar modo
	  GPIOA->MODER &= ~(
	      (3 << (5 * 2)) |
	      (3 << (6 * 2)) |
	      (3 << (7 * 2))
	  );

	  // Output mode (01)
	  GPIOA->MODER |=
	      (1 << (5 * 2)) |
	      (1 << (6 * 2)) |
	      (1 << (7 * 2));
}
///////////////////////////////////


int main(void)
{

  uart_init();

  uart_print("UART initialized\r\n"); //115200, COM3
  led_setup();


  os_init();


  //LED TEST TASKS

  os_task_init(task1, task1_stack, STACK_SIZE, 0);
  os_task_init(task2, task2_stack, STACK_SIZE, 1);
  //os_task_init(task3, task3_stack, STACK_SIZE, 0);


/*

  // STACK TEST
  os_task_init(task1, task1_stack, STACK_SIZE, 0);
  os_task_init(task2, task2_stack, STACK_SIZE, 1);
  os_task_init(task3, task3_stack, STACK_SIZE, 2);
  os_task_init(task4, task4_stack, STACK_SIZE, 3);
  os_task_init(task5, task5_stack, STACK_SIZE, 3);

*/

  idle_task_handle = os_task_init(idle_task, idle_task_stack, STACK_SIZE, IDLE_PRIORITY);

  idle_task_ptr = idle_task_handle;
  // 1 ms tick
  os_start(SystemCoreClock / 1000);


  while (1)
  {



  }

}


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
