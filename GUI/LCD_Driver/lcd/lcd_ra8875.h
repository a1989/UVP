

#ifndef _LCD_RA8875_H
#define _LCD_RA8875_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "fonts.h"
#include "stm32f4xx_fmc.h"

//#include "bsp_timer.h"

/* 选择RA8875的接口模式, 必须和硬件匹配 */
#define IF_8080_EN		/* 8080 总线接口 */
//#define IF_SPI_EN			/* SPI接口 */

#ifdef IF_SPI_EN	/* 4-Wire SPI 界面 (需要改变模块上的2个跳线帽) */
	#define SPI_WRITE_DATA	0x00
	#define SPI_READ_DATA	  0x40
	#define SPI_WRITE_CMD	  0x80
	#define SPI_READ_STATUS	0xC0
	
#else		/* 8080总线 （缺省模式） */
/* 定义LCD驱动器的访问地址 */
	#define RA8875_REG		*(__IO uint16_t *)0x60020000
	#define RA8875_RAM		*(__IO uint16_t *)0x60000000
#endif

enum
{
	RA_FONT_16 = 0,		/* RA8875 字体 16点阵 */
	RA_FONT_24 = 1,		/* RA8875 字体 24点阵 */
	RA_FONT_32 = 2		/* RA8875 字体 32点阵 */
};

/* 文字放大参数 */
enum
{
	RA_SIZE_X1 = 0,		/* 原始大小 */
	RA_SIZE_X2 = 1,		/* 放大2倍 */
	RA_SIZE_X3 = 2,		/* 放大3倍 */
	RA_SIZE_X4 = 3		/* 放大4倍 */
};

/* 定义LCD显示区域的分辨率 */
#define LCD_43_HEIGHT	272		/* 4.3寸宽屏 高度，单位：像素 */
#define LCD_43_WIDTH	480		/* 4.3寸宽屏 宽度，单位：像素 */

#define LCD_70_HEIGHT	480		/* 7.0寸宽屏 高度，单位：像素 */
#define LCD_70_WIDTH	800		/* 7.0寸宽屏 宽度，单位：像素 */


/* LCD 寄存器定义, LR_前缀是LCD Register的简写 */
#define LR_CTRL1		0x007	/* 读写显存的寄存器地址 */
#define LR_GRAM			0x202	/* 读写显存的寄存器地址 */
#define LR_GRAM_X		0x200	/* 显存水平地址（物理X坐标）*/
#define LR_GRAM_Y		0x201	/* 显存垂直地址（物理Y坐标）*/

#define CHIP_STR_8875	"RA8875"

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
	CL_MASK			    = 0x9999	/* 颜色掩码，用于文字背景透明 */
};

/* 字体代码 */
enum
{
	FC_ST_16 = 0,		/* 宋体15x16点阵 （宽x高） */
	FC_ST_12 = 1		/* 宋体12x12点阵 （宽x高） */
};

/* 字体代码 */
enum
{
	FC_ST_16X16 = 0,		/* 宋体15x16点阵 （宽x高） */
	FC_ST_24X24 = 1			/* 宋体24x24点阵 （宽x高） */
};

/* 字体属性结构, 用于LCD_DispStr() */
typedef struct
{
	uint16_t usFontCode;	/* 字体代码 0 表示16点阵 */
	uint16_t usTextColor;	/* 字体颜色 */
	uint16_t usBackColor;	/* 文字背景颜色，透明 */
	uint16_t usSpace;		/* 文字间距，单位 = 像素 */
}FONT_T;

/* 背景光控制 */
#define BRIGHT_MAX		255
#define BRIGHT_MIN		0
#define BRIGHT_DEFAULT	200
#define BRIGHT_STEP		5

/* 可供外部模块调用的函数 */
void RA8875_InitHard(void);
void LCD_InitHard(void);
uint16_t RA8875_ReadID(void);
uint8_t RA8875_IsBusy(void);
void RA8875_Layer1Visable(void);
void RA8875_Layer2Visable(void);
void RA8875_DispOn(void);
void RA8875_DispOff(void);
void RA8875_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor);
uint16_t RA8875_GetPixel(uint16_t _usX, uint16_t _usY);
void RA8875_SetFrontColor(uint16_t _usColor);
void RA8875_SetBackColor(uint16_t _usColor);
void RA8875_SetFont(uint8_t _ucFontType, uint8_t _ucLineSpace, uint8_t _ucCharSpace);
void RA8875_SetTextZoom(uint8_t _ucHSize, uint8_t _ucVSize);
void RA8875_DispAscii(uint16_t _usX, uint16_t _usY, char *_ptr);
void RA8875_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr);
void RA8875_ClrScr(uint16_t _usColor);
void RA8875_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void RA8875_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor);
void RA8875_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void RA8875_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void RA8875_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void RA8875_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void RA8875_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
void RA8875_QuitWinMode(void);
void RA8875_SetBackLight(uint8_t _bright);
void RA8875_TouchInit(void);
uint16_t RA8875_TouchReadX(void);
uint16_t RA8875_TouchReadY(void);
void LCD_SetBackLight(uint8_t bright);
void LCD_GetChipDescribe(char *_str);
uint16_t LCD_GetHeight(void);
uint16_t LCD_GetWidth(void);
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont);
uint8_t LCD_GetBackLight(void);
void LCD_SetBackLight(uint8_t _bright);
void LCD_DrawIcon(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void LCD_DrawIconActive(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void RA8875_InitSPI(void);

#endif


