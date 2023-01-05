/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "max30102_for_stm32_hal.h"
#include "oled_i2c.h"
#include "ring_buffer.h"
#include "algorithm_by_RF.h"

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

/* USER CODE BEGIN PV */

max30102_t max30102;
RingBuff_t ringBuffer_ir, ringBuffer_red;
calculated_data_t cal_data = {0, 0, 0, 0, 0, 0};
uint8_t buff_not_full_count = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// Override plot function
void max30102_plot(uint32_t ir_sample, uint32_t red_sample)
{
  RingBuff_Write(&ringBuffer_ir, ir_sample);
  RingBuff_Write(&ringBuffer_red, red_sample);
#if SENSOR_DATA_PRINT
  u1_printf("ir,r:%u,%u\n", ir_sample, red_sample); // Print IR and Red
#endif
}

// Config MAX30102 
void max30102_user_config(void)
{
  // Initiation
  max30102_init(&max30102, &hi2c1);
  max30102_reset(&max30102);
  max30102_clear_fifo(&max30102);
  max30102_set_fifo_config(&max30102, max30102_smp_ave_8, 0, 7);

  // Sensor settings
  max30102_set_led_pulse_width(&max30102, max30102_pw_16_bit);
  max30102_set_adc_resolution(&max30102, max30102_adc_2048);
  max30102_set_sampling_rate(&max30102, max30102_sr_800);
  max30102_set_led_current_1(&max30102, 4.2);
  max30102_set_led_current_2(&max30102, 4.2);

  // Enter SpO2 mode
  max30102_set_mode(&max30102, max30102_spo2);
  max30102_set_a_full(&max30102, 1);

  // Initiate 1 temperature measurement
  max30102_set_die_temp_en(&max30102, 1);
  max30102_set_die_temp_rdy(&max30102, 1);

  uint8_t en_reg[2] = {0};
  max30102_read(&max30102, 0x00, en_reg, 1);
}

void timer_event_handler(void)
{
  if (RingBuff_CheckFull(&ringBuffer_ir) && RingBuff_CheckFull(&ringBuffer_red))
  {
    buff_not_full_count = 0;
  }
  else // 处理缓冲区没填满的情况
  {
    buff_not_full_count++;        // 记录没填满的次数
    if (buff_not_full_count >= 3) // 7 * 3 = 21s 如果填不满时间超过9s, 则作出处理
    {
      RingBuff_Clear(&ringBuffer_ir);
      RingBuff_Clear(&ringBuffer_red);
      buff_not_full_count = 0;
      u1_printf("(DBG:) [WARN] Buffer_not_full TIMEOUT. Set max30102 interrupt flag = 1\n");
      max30102._interrupt_flag = 1; // 重新设置模块 以防模块死机
    }
  }

  if (!(cal_data.spo2_valid && cal_data.heart_rate_valid))
  {
    OLED_Customized_Show(&cal_data, 1); // 数据长时间无效时切换屏幕
  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  u1_printf("(DBG:) System Enabled.\n");

  OLED_Init();
  OLED_CLS();
  OLED_Customized_Show(&cal_data, 1);
  RingBuff_Clear(&ringBuffer_ir);
  RingBuff_Clear(&ringBuffer_red);
  max30102_user_config();
  HAL_TIM_Base_Start_IT(&htim3);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    if (max30102_has_interrupt(&max30102))
    {
      max30102_interrupt_handler(&max30102);
    }

    if (RingBuff_CheckFull(&ringBuffer_ir) && RingBuff_CheckFull(&ringBuffer_red))
    {
      buff_not_full_count = 0;
#if BUFFER_DATA_PRINT
      for (uint16_t k = 0; k < RINGBUFF_LEN; ++k)
      {
        u1_printf("ir,red:%u,%u\n", ringBuffer_ir.Ring_data[k], ringBuffer_red.Ring_data[k]);
      }
#endif
      // Calculate
      rf_heart_rate_and_oxygen_saturation(
          ringBuffer_ir.Ring_data, RINGBUFF_LEN, ringBuffer_red.Ring_data, &(cal_data.spo2), &(cal_data.spo2_valid),
          &(cal_data.heart_rate), &(cal_data.heart_rate_valid), &(cal_data.ratio), &(cal_data.correl));

      // Print
      u1_printf(
          "(DBG:) SpO2 %4.3f, Hr %4d, SpO2_v %d, Hr_v %d, Ratio %2.2f, Correl %2.2f\n",
          cal_data.spo2, cal_data.heart_rate, cal_data.spo2_valid, cal_data.heart_rate_valid,
          cal_data.ratio, cal_data.correl);

      // OLED Show
      if (cal_data.spo2_valid && cal_data.heart_rate_valid)
      {
        OLED_Customized_Show(&cal_data, 2);
      }

      RingBuff_Clear(&ringBuffer_ir);
      RingBuff_Clear(&ringBuffer_red);
    }

    if (HAL_GPIO_ReadPin(MAX30102_INT_GPIO_Port, MAX30102_INT_Pin) == GPIO_PIN_RESET)
    {
      // u1_printf("(DBG:) Detect MAX30102 INT low evel\n");
      max30102._interrupt_flag = 1;
    }

    // u1_printf("(DBG:) loop.\n");
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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

#ifdef  USE_FULL_ASSERT
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
