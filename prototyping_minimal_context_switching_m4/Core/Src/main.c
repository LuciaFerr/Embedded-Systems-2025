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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);

/* USER CODE BEGIN PFP */
static void delay(volatile uint32_t delay_ms);
static void task1_handler(void);
static void task2_handler(void);
static void task3_handler(void);
static void task4_handler(void);


/* put near top of the .c file */
volatile uint32_t task1_count = 0;
volatile uint32_t task2_count = 0;
volatile uint32_t task3_count = 0;
volatile uint32_t task4_count = 0;
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration --------------------------------------------------------*/

  /* Reset all peripherals, initialize Flash interface and Systick */
  HAL_Init();
  HAL_NVIC_DisableIRQ(WWDG_IRQn);

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize peripherals */
  MX_GPIO_Init();
  //MX_USART3_UART_Init();
  //MX_USB_OTG_FS_PCD_Init();

  /* ensure trace/ITM enabled in the core (CMSIS registers) */
//  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   // enable trace system
//  ITM->LAR = 0xC5ACCE55;                           // unlock ITM
//  ITM->TCR = ITM_TCR_ITMENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_TSENA_Msk;
//  ITM->TER |= (1UL << 0);                          // enable stimulus port 0

  /* Initialize task stacks */
  static os_stack_t stack1[128];
  static os_stack_t stack2[128];
  static os_stack_t stack3[128];
  static os_stack_t stack4[128];

  os_init();
  os_task_init(&task1_handler, stack1, 128);
  os_task_init(&task2_handler, stack2, 128);
  os_task_init(&task3_handler, stack3, 128);
  os_task_init(&task4_handler, stack4, 128);

  /* Start scheduler */
  os_start(SystemCoreClock);

  while (1)
  {
      /* Idle loop */
  }
}

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
      Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
      Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
      Error_Handler();
  }
}

/**
  * @brief USB_OTG_FS Initialization Function
  */
/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;  /* <-- fixed: use .Init, not _Init */
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
      Error_Handler();
  }
}




/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /* USER Button */
  GPIO_InitStruct.Pin  = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /* LED Pins */
  GPIO_InitStruct.Pin   = LD1_Pin | LD3_Pin | LD2_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USB Power Switch */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /* USB Overcurrent */
  GPIO_InitStruct.Pin  = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

/* HardFault handler */
void HardFault_Handler(void)
{
  while (1);
}

/* Simple blocking delay */
static void delay(volatile uint32_t time)
{
    while (time--);
}

/* Task handlers */
static void task1_handler(void)
{
  while (1)
  {
	  //ITM_SendChar('1');
	  task1_count++;
	  //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
      //delay(400000);
  }
}

static void task2_handler(void)
{
  while (1)
  {
	  //ITM_SendChar('1');
	  task2_count++;
	  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
      delay(400000);
  }
}

static void task3_handler(void)
{
  while (1)
  {
	  //ITM_SendChar('2');
	  task3_count++;
	  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
      delay(200000);
  }
}

static void task4_handler(void)
{
  while (1)
  {
	  //ITM_SendChar('3');
	  task4_count++;
	  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
      delay(600000);
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User may add custom handling here */
}
#endif
