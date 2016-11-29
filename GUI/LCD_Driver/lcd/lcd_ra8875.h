

#ifndef _LCD_RA8875_H
#define _LCD_RA8875_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "fonts.h"
#include "stm32f4xx_fmc.h"

//#include "bsp_timer.h"

/* ѡ��RA8875�Ľӿ�ģʽ, �����Ӳ��ƥ�� */
#define IF_8080_EN		/* 8080 ���߽ӿ� */
//#define IF_SPI_EN			/* SPI�ӿ� */

#ifdef IF_SPI_EN	/* 4-Wire SPI ���� (��Ҫ�ı�ģ���ϵ�2������ñ) */
	#define SPI_WRITE_DATA	0x00
	#define SPI_READ_DATA	  0x40
	#define SPI_WRITE_CMD	  0x80
	#define SPI_READ_STATUS	0xC0
	
#else		/* 8080���� ��ȱʡģʽ�� */
/* ����LCD�������ķ��ʵ�ַ */
	#define RA8875_REG		*(__IO uint16_t *)0x60020000
	#define RA8875_RAM		*(__IO uint16_t *)0x60000000
#endif

enum
{
	RA_FONT_16 = 0,		/* RA8875 ���� 16���� */
	RA_FONT_24 = 1,		/* RA8875 ���� 24���� */
	RA_FONT_32 = 2		/* RA8875 ���� 32���� */
};

/* ���ַŴ���� */
enum
{
	RA_SIZE_X1 = 0,		/* ԭʼ��С */
	RA_SIZE_X2 = 1,		/* �Ŵ�2�� */
	RA_SIZE_X3 = 2,		/* �Ŵ�3�� */
	RA_SIZE_X4 = 3		/* �Ŵ�4�� */
};

/* ����LCD��ʾ����ķֱ��� */
#define LCD_43_HEIGHT	272		/* 4.3����� �߶ȣ���λ������ */
#define LCD_43_WIDTH	480		/* 4.3����� ��ȣ���λ������ */

#define LCD_70_HEIGHT	480		/* 7.0����� �߶ȣ���λ������ */
#define LCD_70_WIDTH	800		/* 7.0����� ��ȣ���λ������ */


/* LCD �Ĵ�������, LR_ǰ׺��LCD Register�ļ�д */
#define LR_CTRL1		0x007	/* ��д�Դ�ļĴ�����ַ */
#define LR_GRAM			0x202	/* ��д�Դ�ļĴ�����ַ */
#define LR_GRAM_X		0x200	/* �Դ�ˮƽ��ַ������X���꣩*/
#define LR_GRAM_Y		0x201	/* �Դ洹ֱ��ַ������Y���꣩*/

#define CHIP_STR_8875	"RA8875"

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
	CL_MASK			    = 0x9999	/* ��ɫ���룬�������ֱ���͸�� */
};

/* ������� */
enum
{
	FC_ST_16 = 0,		/* ����15x16���� ����x�ߣ� */
	FC_ST_12 = 1		/* ����12x12���� ����x�ߣ� */
};

/* ������� */
enum
{
	FC_ST_16X16 = 0,		/* ����15x16���� ����x�ߣ� */
	FC_ST_24X24 = 1			/* ����24x24���� ����x�ߣ� */
};

/* �������Խṹ, ����LCD_DispStr() */
typedef struct
{
	uint16_t usFontCode;	/* ������� 0 ��ʾ16���� */
	uint16_t usTextColor;	/* ������ɫ */
	uint16_t usBackColor;	/* ���ֱ�����ɫ��͸�� */
	uint16_t usSpace;		/* ���ּ�࣬��λ = ���� */
}FONT_T;

/* ��������� */
#define BRIGHT_MAX		255
#define BRIGHT_MIN		0
#define BRIGHT_DEFAULT	200
#define BRIGHT_STEP		5

/* �ɹ��ⲿģ����õĺ��� */
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


