#include "setpins.h"
#include "stm32f4xx_gpio.h"
#include "defines.h"
#include "stm32f4xx_dma.h"

GPIO_InitTypeDef GPIO_InitStructure;
#define ADC1_DR_ADDRESS     ((uint32_t)0x4001204C)
#define ADC2_DR_ADDRESS     ((uint32_t)0x4001214C)

__IO uint16_t uhADC1ConvertedValue;
__IO uint16_t uhADC2ConvertedValue;

uint16_t PrescalerValue = 0;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;

uint16_t CCR1_Val = 350;
uint16_t CCR2_Val = 350;

void Config_Pins_As_Output(GPIO_TypeDef* Port, uint16_t Pin)
{
//		RCC_AHB1PeriphClockCmd(Port, ENABLE);
		GPIO_InitStructure.GPIO_Pin = Pin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
		GPIO_Init(Port, &GPIO_InitStructure);
}

void Config_Pins_As_Input(GPIO_TypeDef* Port, uint16_t Pin)
{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = Pin;
		GPIO_Init(Port, &GPIO_InitStructure);
}

void WRITE_Pin(GPIO_TypeDef* Port, uint16_t Pin, BitAction BitVal)
{
		GPIO_WriteBit(Port, Pin, BitVal);
}

uint8_t READ_Pin(GPIO_TypeDef* Port, uint16_t Pin)
{
		return GPIO_ReadInputDataBit(Port, Pin);
}

void set_motor_pins(void)
{
		Config_Pins_As_Output(X_DIR_PIN_PORT, X_DIR_PIN);
		Config_Pins_As_Output(X_STEP_PIN_PORT, X_STEP_PIN);
		Config_Pins_As_Output(X_ENABLE_PIN_PORT, X_ENABLE_PIN);
	
		Config_Pins_As_Output(Y_DIR_PIN_PORT, Y_DIR_PIN);
		Config_Pins_As_Output(Y_STEP_PIN_PORT, Y_STEP_PIN);
		Config_Pins_As_Output(Y_ENABLE_PIN_PORT, Y_ENABLE_PIN);
	
		Config_Pins_As_Output(Z_DIR_PIN_PORT, Z_DIR_PIN);
		Config_Pins_As_Output(Z_STEP_PIN_PORT, Z_STEP_PIN);
		Config_Pins_As_Output(Z_ENABLE_PIN_PORT, Z_ENABLE_PIN);
	
		Config_Pins_As_Output(E0_DIR_PIN_PORT, E0_DIR_PIN);
		Config_Pins_As_Output(E0_STEP_PIN_PORT, E0_STEP_PIN);
		Config_Pins_As_Output(E0_ENABLE_PIN_PORT, E0_ENABLE_PIN);
}

void set_endstop_pins(void)
{
		Config_Pins_As_Input(X_MAX_PIN_PORT, X_MAX_PIN);
//		WRITE_Pin(X_MIN_PIN_PORT, X_MIN_PIN, HIGH);
		Config_Pins_As_Input(Y_MAX_PIN_PORT, Y_MAX_PIN);
//		WRITE_Pin(Y_MIN_PIN_PORT, Y_MIN_PIN, HIGH);
		Config_Pins_As_Input(Z_MIN_PIN_PORT, Z_MIN_PIN);
//		WRITE_Pin(Z_MIN_PIN_PORT, Z_MIN_PIN, HIGH);
}

void set_limit_switch_pins(void)
{

}

void set_heater_pins(void)
{
//		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = HEATER_END_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
		GPIO_Init(HEATER_END_PIN_PORT, &GPIO_InitStructure); 
	
		GPIO_InitStructure.GPIO_Pin = HEATER_BED_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
		GPIO_Init(HEATER_BED_PIN_PORT, &GPIO_InitStructure); 
	
//	  GPIO_PinAFConfig(HEATER_END_PIN_PORT, GPIO_PinSource5, GPIO_AF_TIM2);
//		GPIO_PinAFConfig(HEATER_BED_PIN_PORT, GPIO_PinSource2, GPIO_AF_TIM2); 
}

void TIM_Heater_PWM_Config(void)
{
		RCC_TIMCLKPresConfig(RCC_TIMPrescActivated);
	
		PrescalerValue = (uint16_t) (SystemCoreClock / 21000000) - 1;

		/* Time base configuration */
		TIM_TimeBaseStructure.TIM_Period = 699;
		TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

		/* PWM1 Mode configuration: Channel1 */
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

		TIM_OC1Init(TIM2, &TIM_OCInitStructure);

		TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

		/* PWM1 Mode configuration: Channel2 */
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse = CCR2_Val;

		TIM_OC2Init(TIM2, &TIM_OCInitStructure);

		TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

		TIM_ARRPreloadConfig(TIM2, ENABLE);

		/* TIM4 enable counter */
		TIM_Cmd(TIM2, ENABLE);
}

void IO_Init(void)
{
		
}

void enable_x(void)
{
		WRITE_Pin(X_ENABLE_PIN_PORT, X_ENABLE_PIN, X_ENABLE_ON);
}

void enable_y(void)
{
		WRITE_Pin(Y_ENABLE_PIN_PORT, Y_ENABLE_PIN, Y_ENABLE_ON);
}

void enable_z(void)
{
		WRITE_Pin(Z_ENABLE_PIN_PORT, Z_ENABLE_PIN, Z_ENABLE_ON);
}

void enable_e0(void)
{
		WRITE_Pin(E0_ENABLE_PIN_PORT, E0_ENABLE_PIN, E_ENABLE_ON);
}

void enable_e1(void)
{
		
}

void enable_e2(void)
{
		
}

void disable_x(void)
{
		WRITE_Pin(X_ENABLE_PIN_PORT, X_ENABLE_PIN,!X_ENABLE_ON);
}	

void disable_y(void)
{
		WRITE_Pin(Y_ENABLE_PIN_PORT, Y_ENABLE_PIN,!Y_ENABLE_ON);
}	

void disable_z(void)
{
		WRITE_Pin(Z_ENABLE_PIN_PORT, Z_ENABLE_PIN,!Z_ENABLE_ON);
}	

void disable_e0(void)
{
		WRITE_Pin(E0_ENABLE_PIN_PORT, E0_ENABLE_PIN,!E_ENABLE_ON);
}	

void ADC_DMA_Config(void)
{
		ADC_InitTypeDef       ADC_InitStructure;
		ADC_CommonInitTypeDef ADC_CommonInitStructure;
		DMA_InitTypeDef       DMA_InitStructure;
		GPIO_InitTypeDef      GPIO_InitStructure;
		
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);

		DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uhADC1ConvertedValue;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
//		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		
		DMA_Init(DMA2_Stream0, &DMA_InitStructure);
		DMA_Cmd(DMA2_Stream0, ENABLE);
		
		DMA_InitStructure.DMA_Channel = DMA_Channel_1;  
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC2_DR_ADDRESS;
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uhADC2ConvertedValue;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
//		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		
		DMA_Init(DMA2_Stream2, &DMA_InitStructure);
		DMA_Cmd(DMA2_Stream2, ENABLE);

		/* Configure ADC3 Channel13 pin as analog input ******************************/
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
//		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		/* ADC Common Init **********************************************************/
		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
		ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
		ADC_CommonInit(&ADC_CommonInitStructure);

		/* ADC3 Init ****************************************************************/
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion = 1;
		ADC_Init(ADC1, &ADC_InitStructure);
		
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion = 1;
		ADC_Init(ADC2, &ADC_InitStructure);

		/* ADC3 regular channel13 configuration *************************************/
		ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);
		ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);

	 /* Enable DMA request after last transfer (Single-ADC mode) */
		ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
		ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
		/* Enable ADC3 DMA */
		ADC_DMACmd(ADC1, ENABLE);
		ADC_DMACmd(ADC2, ENABLE);

		/* Enable ADC3 */
		ADC_Cmd(ADC1, ENABLE);
		ADC_Cmd(ADC2, ENABLE);
		
//		ADC_SoftwareStartConv(ADC1);
}

void TIM_Process_Config(void)
{
		NVIC_InitTypeDef   NVIC_InitStructure;
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	
//		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM_TimeBaseStructure.TIM_Period = 1599;
		TIM_TimeBaseStructure.TIM_Prescaler = 899;
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM3,TIM_FLAG_Update);
//		GUI_DispStringAt("TIM init end!", 0, 0);
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
//		GUI_DispStringAt("TIM init end!", 0, 0);
		TIM_Cmd(TIM3, ENABLE);
//		TIM_Cmd(TIM4, DISABLE);
}

void tp_init(void)
{
		ADC_DMA_Config();
		set_heater_pins();
//		TIM_Heater_PWM_Config();
		TIM_Process_Config();
		ADC_SoftwareStartConv(ADC1);
		ADC_SoftwareStartConv(ADC2);
}

void st_init(void)
{
		set_motor_pins();
		set_endstop_pins();
		
		WRITE_Pin(X_MIN_PIN_PORT, X_MIN_PIN, HIGH);
		WRITE_Pin(Y_MIN_PIN_PORT, Y_MIN_PIN, HIGH);
		WRITE_Pin(Z_MIN_PIN_PORT, Z_MIN_PIN, HIGH);
	
		WRITE_Pin(X_STEP_PIN_PORT, X_STEP_PIN, INVERT_X_STEP_PIN);
		disable_x();
	
		WRITE_Pin(Y_STEP_PIN_PORT, Y_STEP_PIN, INVERT_Y_STEP_PIN);
		disable_y();
	
		WRITE_Pin(Z_STEP_PIN_PORT, Z_STEP_PIN, INVERT_Z_STEP_PIN);
		disable_z();

		WRITE_Pin(E0_STEP_PIN_PORT, E0_STEP_PIN, INVERT_E_STEP_PIN);
		disable_e0();
}
