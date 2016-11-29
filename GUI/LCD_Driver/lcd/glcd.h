
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

/* LCD 颜色代码，CL_是Color的简写 */
enum
{
	CL_WHITE        = 0xFFFF,	/* 白色 */
	CL_BLACK        = 0x0000,	/* 黑色 */
	CL_GREY         = 0xF7DE,	/* 灰色 */
	CL_BLUE         = 0x001F,	/* 蓝色 */
	CL_BLUE2        = 0x051F,	/* 浅蓝色 */
	CL_RED          = 0xF800,	/* 红色 */
	CL_MAGENTA      = 0xF81F,	/* 红紫色，洋红色 */
	CL_GREEN        = 0x07E0,	/* 绿色 */
	CL_CYAN         = 0x7FFF,	/* 蓝绿色，青色 */
	CL_YELLOW       = 0xFFE0,	/* 黄色 */
	CL_MASK			= 0x9999	/* 颜色掩码，用于文字背景透明 */
};

#define LCD_PIXEL_WIDTH          480
#define LCD_PIXEL_HEIGHT         272

/* 选择BANK1-NORSRAM1 连接 TFT，地址范围为0X60000000~0X63FFFFFF
 * FSMC_A16 接LCD的DC(寄存器/数据选择)脚
 * 寄存器基地址 = 0X60000000
 * RAM基地址 = 0X60020000 = 0X60000000+2^16*2 = 0X60000000 + 0X20000 = 0X60020000
 * 这里主要用于控制RS进行写寄存器写数据操作，因为板子的液晶RS接在A16管脚，所以要使地址线A16为高电平，则写地址到2的16次方处，并且因为液晶是16bit的
 *所以再乘上2，得出的地址加上BANK所在的地址即可。如RS接A0处，则RAM基地址为bank所在地址加上2的0次方的2倍，如是8bit屏则不需要乘以2
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
