

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H
#include "stm32f4xx.h"
/* Ŀǰ�ǿղ������û����Զ�����CPU����IDLE״̬�ĺ�����ι������ */
#define CPU_IDLE()

/* ��ʱ���ṹ�壬��Ա���������� volatile, ����C�������Ż�ʱ���������� */
typedef struct
{
	volatile uint32_t count;	/* ������ */
	volatile uint8_t flag;		/* ��ʱ�����־  */
}SOFT_TMR;

/* ���ⲿ���õĺ������� */
void bsp_InitTimer(void);
void bsp_DelayMS(uint32_t n);
void myLCD_Delay(unsigned int Count);
void LCD_Delay1ms(void);
#endif
