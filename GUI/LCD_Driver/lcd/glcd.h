
#ifndef __GLCD_H
#define __GLCD_H

#include "stm32f4xx.h"
#include "delay.h"

#define LCD_XMAX 480
#define LCD_YMAX 272

#define RED	  0XF800
#define GREEN 0X07E0
#define BLUE  0X001F  
#define BRED  0XF81F
#define GRED  0XFFE0
#define GBLUE 0X07FF
#define BLACK 0X0000
#define WHITE 0XFFFF

/* LCD ��ɫ���룬CL_��Color�ļ�д */
enum
{
	CL_WHITE        = 0xFFFF,	/* ��ɫ */
	CL_BLACK        = 0x0000,	/* ��ɫ */
	CL_GREY         = 0xF7DE,	/* ��ɫ */
	CL_BLUE         = 0x001F,	/* ��ɫ */
	CL_BLUE2        = 0x051F,	/* ǳ��ɫ */
	CL_RED          = 0xF800,	/* ��ɫ */
	CL_MAGENTA      = 0xF81F,	/* ����ɫ�����ɫ */
	CL_GREEN        = 0x07E0,	/* ��ɫ */
	CL_CYAN         = 0x7FFF,	/* ����ɫ����ɫ */
	CL_YELLOW       = 0xFFE0,	/* ��ɫ */
	CL_MASK			= 0x9999	/* ��ɫ���룬�������ֱ���͸�� */
};

#define LCD_PIXEL_WIDTH          480
#define LCD_PIXEL_HEIGHT         272

/* ѡ��BANK1-NORSRAM1 ���� TFT����ַ��ΧΪ0X60000000~0X63FFFFFF
 * FSMC_A16 ��LCD��DC(�Ĵ���/����ѡ��)��
 * �Ĵ�������ַ = 0X60000000
 * RAM����ַ = 0X60020000 = 0X60000000+2^16*2 = 0X60000000 + 0X20000 = 0X60020000
 * ������Ҫ���ڿ���RS����д�Ĵ���д���ݲ�������Ϊ���ӵ�Һ��RS����A16�ܽţ�����Ҫʹ��ַ��A16Ϊ�ߵ�ƽ����д��ַ��2��16�η�����������ΪҺ����16bit��
 *�����ٳ���2���ó��ĵ�ַ����BANK���ڵĵ�ַ���ɡ���RS��A0������RAM����ַΪbank���ڵ�ַ����2��0�η���2��������8bit������Ҫ����2
 */
#define LCD_RAM   *(vu16*)(u32)0x60000000  //disp Data ADDR
#define LCD_REG   *(vu16*)(u32)0x60020000	 //disp Reg  ADDR

#define LCD_WriteCmd(x)   LCD_REG = x
#define LCD_WriteData(x)  LCD_RAM = x

#define ABS(X)  ((X) > 0 ? (X) : -(X))


void RA8875_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
void GLCD_Init(void);
static void GDrawPixel(uint16_t x, uint16_t y);
void GLCD_SetTextColor(__IO uint16_t Color);
void GLCD_SetBackColor(__IO uint16_t Color);
void GLCD_WaitStatus(void);
void LCD_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
void LCD_SetCursor(uint16_t _usX, uint16_t _usY);
void LCD_GPIO_Config(void);
void LCD_FSMC_Config(void);
void LCD_ClrScr(uint16_t _usColor);
void LCD_SetBackLight(uint8_t _bright);

#endif
