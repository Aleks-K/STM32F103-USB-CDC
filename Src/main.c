/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 14/03/2015 08:37:20
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "RingBuffer.h"
#include "main.h"
#include "MasterPacketProc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define VDD_APPLI                      ((uint32_t) 3300)   /* Value of analog voltage supply Vdda (unit: mV) */
#define RANGE_12BITS                   ((uint32_t) 4095)   /* Max value with a full range of 12 bits */
#define USERBUTTON_CLICK_COUNT_MAX     ((uint32_t)    4)   /* Maximum value of variable "UserButtonClickCount" */

#define ADCCONVERTEDVALUES_BUFFER_SIZE ((uint32_t)    3)   /* Size of array containing ADC converted values: set to ADC sequencer number of ranks converted, to have a rank in each address */

/* Internal temperature sensor: constants data used for indicative values in  */
/* this example. Refer to device datasheet for min/typ/max values.            */
/* For more accurate values, device should be calibrated on offset and slope  */
/* for application temperature range.                                         */
#define INTERNAL_TEMPSENSOR_V25        ((int32_t)1430)         /* Internal temperature sensor, parameter V25 (unit: mV). Refer to device datasheet for min/typ/max values. */
#define INTERNAL_TEMPSENSOR_AVGSLOPE   ((int32_t)4300)         /* Internal temperature sensor, parameter Avg_Slope (unit: uV/DegCelsius). Refer to device datasheet for min/typ/max values. */                                                               /* This calibration parameter is intended to calculate the actual VDDA from Vrefint ADC measurement. */

/* Private macro -------------------------------------------------------------*/

/**
  * @brief  Computation of temperature (unit: degree Celsius) from the internal
  *         temperature sensor measurement by ADC.
  *         Computation is using temperature sensor standard parameters (refer
  *         to device datasheet).
  *         Computation formula:
  *         Temperature = (VTS - V25)/Avg_Slope + 25
  *         with VTS = temperature sensor voltage
  *              Avg_Slope = temperature sensor slope (unit: uV/DegCelsius)
  *              V25 = temperature sensor @25degC and Vdda 3.3V (unit: mV)
  *         Calculation validity conditioned to settings: 
  *          - ADC resolution 12 bits (need to scale value if using a different 
  *            resolution).
  *          - Power supply of analog voltage Vdda 3.3V (need to scale value 
  *            if using a different analog voltage supply value).
  * @param TS_ADC_DATA: Temperature sensor digital value measured by ADC
  * @retval None
  */
#define COMPUTATION_TEMPERATURE_STD_PARAMS(TS_ADC_DATA)                        \
  ((((int32_t)(INTERNAL_TEMPSENSOR_V25 - (((TS_ADC_DATA) * VDD_APPLI) / RANGE_12BITS)   \
     ) * 1000                                                                  \
    ) / INTERNAL_TEMPSENSOR_AVGSLOPE                                           \
   ) + 25                                                                      \
  )

/**
  * @brief  Computation of voltage (unit: mV) from ADC measurement digital
  *         value on range 12 bits.
  *         Calculation validity conditioned to settings: 
  *          - ADC resolution 12 bits (need to scale value if using a different 
  *            resolution).
  *          - Power supply of analog voltage Vdda 3.3V (need to scale value 
  *            if using a different analog voltage supply value).
  * @param ADC_DATA: Digital value measured by ADC
  * @retval None
  */
#define COMPUTATION_DIGITAL_12BITS_TO_VOLTAGE(ADC_DATA)                        \
  ( (ADC_DATA) * VDD_APPLI / RANGE_12BITS)

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* USER CODE BEGIN PV */

uint32_t period = 250;

/* Variable containing ADC conversions results */
__IO uint16_t   aADCxConvertedValues[ADCCONVERTEDVALUES_BUFFER_SIZE];

/* Variables for ADC conversions results computation to physical values */
uint16_t   uhADCChannelToDAC_mVolt = 0;
uint16_t   uhVrefInt_mVolt = 0;
int32_t    wTemperature_DegreeCelsius = 0;

/* Variable to report ADC sequencer status */
uint8_t         ubSequenceCompleted = RESET;     /* Set when all ranks of the sequence have been converted */

RingBuffer_t usbRingBuffer;
uint8_t usbRx[UsbRxBufSize];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  RB_InitStatic(&usbRingBuffer, usbRx, UsbRxBufSize);
  
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */

  /* Run the ADC calibration */  
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    /* Calibration Error */
//    User_Adc_Error_Handler();
  }

  /*## Start ADC conversions #################################################*/
  
  /* Start ADC conversion on regular group with transfer by DMA */
  if (HAL_ADC_Start_DMA(&hadc1,
                        (uint32_t *)aADCxConvertedValues,
                        ADCCONVERTEDVALUES_BUFFER_SIZE
                       ) != HAL_OK)
  {
    /* Start Error */
//    User_Adc_Error_Handler();
  }

  /* USER CODE END 2 */

  /* USER CODE BEGIN 3 */
  /* Infinite loop */
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
    HAL_Delay(period);
    
    /* Start ADC conversion */
    /* Since sequencer is enabled in discontinuous mode, this will perform    */
    /* the conversion of the next rank in sequencer.                          */
    /* Note: For this example, conversion is triggered by software start,     */
    /*       therefore "HAL_ADC_Start()" must be called for each conversion.  */
    /*       Since DMA transfer has been initiated previously by function     */
    /*       "HAL_ADC_Start_DMA()", this function will keep DMA transfer      */
    /*       active.                                                          */
    HAL_ADC_Start(&hadc1);
      
    /* Wait for conversion completion before conditional check hereafter */
    HAL_ADC_PollForConversion(&hadc1, 1);
    
    /* Turn-on/off LED1 in function of ADC sequencer status */
    /* - Turn-off if sequencer has not yet converted all ranks */    
    /* - Turn-on if sequencer has converted all ranks */
    if (ubSequenceCompleted != RESET)
    {
      /* Computation of ADC conversions raw data to physical values */
      /* Note: ADC results are transferred into array "aADCxConvertedValues"  */
      /*       in the order of their rank in ADC sequencer.                   */
      uhADCChannelToDAC_mVolt    = COMPUTATION_DIGITAL_12BITS_TO_VOLTAGE(aADCxConvertedValues[0]);
      uhVrefInt_mVolt            = COMPUTATION_DIGITAL_12BITS_TO_VOLTAGE(aADCxConvertedValues[2]);
      wTemperature_DegreeCelsius = COMPUTATION_TEMPERATURE_STD_PARAMS(aADCxConvertedValues[1]);

      /* Reset variable for next loop iteration */
      ubSequenceCompleted = RESET;
    }
    
    UsbRxProc(&usbRingBuffer);
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBPLLCLK_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_AFIO_CLK_ENABLE();

}

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  HAL_ADC_Init(&hadc1);

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 2;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = 3;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}

/** 
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOD_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PB1 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_8)
  {
    period *= 2;
  }
}

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
  /* Report to main program that ADC sequencer has reached its end */
  ubSequenceCompleted = SET;
}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode 
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{

}

/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  /* In case of ADC error, call main error handler */
//  User_Adc_Error_Handler();
}

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
