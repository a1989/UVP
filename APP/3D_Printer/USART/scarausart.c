#include "scarausart.h"
#include "misc.h"
#include "stdlib.h"
#include "stdio.h"
#include "defines.h"
struct ring_buffer rx_buffer = { { 0 }, 0, 0 };

void NVIC_USART_Config(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}



void Usart_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE  );
		 /* Configure USART1 Tx (PA.09) as alternate function push-pull */
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);				 
			
		/* Configure USART1 Rx (PA.10) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Usart_Config(uint32_t bauderate)
{
		USART_InitTypeDef USART_InitStructure;
		USART_ClockInitTypeDef  USART_ClockInitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE  );

		USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;			// ???????
		USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;				// ?????
		USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;				// ?????????????
		USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;		// ?????????????SCLK??
/* Configure the USART1 synchronous paramters */
		USART_ClockInit(USART1, &USART_ClockInitStructure);					// ?????????
																	 
		USART_InitStructure.USART_BaudRate = bauderate;						  // ????:115200
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;			  // 8???
		USART_InitStructure.USART_StopBits = USART_StopBits_1;				  // ??????1????
		USART_InitStructure.USART_Parity = USART_Parity_No ;				  // ????
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// ???????

		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		  // ????+????
/* Configure USART1 basic and asynchronous paramters */
		USART_Init(USART1, &USART_InitStructure);
    
  /* Enable USART1 */
		USART_ClearFlag(USART1, USART_IT_RXNE); 			//???,??????????????
		USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);		//??USART1???
//		USART_ITConfig(USART1,USART_IT_IDLE, ENABLE);
//		USART_ITConfig(USART1,USART_IT_RXNE, DISABLE);
		USART_Cmd(USART1, ENABLE);				
}

void Usart_Init(void)
{
		NVIC_USART_Config();
		Usart_GPIO_Config();
		Usart_Config(BAUDRATE);
}

char usart_getch(void)
{
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
		{
				
		}
		
		return USART_ReceiveData(USART1);
}

char usart_getchar(void)
{
//		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
//		{
//				
//		}
//		
		return USART_ReceiveData(USART1);
}

void usart_putstr(char *p)
{
		while(*p)
		{
				USART_SendData(USART1, *p);
				while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
				{
				}
				p++;
		}
}

void usart_putstrln(char *p)
{
		while(*p)
		{
				USART_SendData(USART1, *p);
				while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
				{
				}
				p++;
		}
		
		USART_SendData(USART1, '\n');
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		{
			
		}
}

void USART_Put_I2A(uint32_t n)
{	
		int i;
		char num[20];
		for(i = 0; i < 20; i++)
		{
				num[i] = 0;
		}	
		sprintf(num, "%d", n);
		usart_putstrln(num);
}

void USART_Put_F2A(double n)
{
		int i;
		char num[20];
		for(i = 0; i < 20; i++)
		{
				num[i] = 0;
		}
		sprintf(num, "%f", n);
		usart_putstr(num);
}

uint32_t Serial_Available(void)
{
		return (RX_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % RX_BUFFER_SIZE;
}

char read_cmd(void)
{
		char c;
		if(rx_buffer.head == rx_buffer.tail)
		{
//				rx_buffer.tail = 0;
				return -1;
		}
		else
		{
				c = rx_buffer.buffer[rx_buffer.tail];
				rx_buffer.tail = (rx_buffer.tail + 1) % RX_BUFFER_SIZE;
				return c;
		}
}

void buffer_flush(void)
{
		rx_buffer.head = rx_buffer.tail;
}