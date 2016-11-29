

#ifndef __LCD_H__
#define __LCD_H__	1
//#include "main.h"
#include "stm32f4xx.h"
#include "stm32f4xx_fmc.h"

#define LCD_RAM    *(__IO uint16_t *) (0x6C000002)    	//µØÖ·¼Ä´æÆ÷
#define LCD_REG    *(__IO uint16_t *) (0x6C000000)	 	//Ö¸Áî¼Ä´æÆ÷

//#define LCD_RAM    *(__IO uint16_t *) (0x6C000800)
//#define LCD_REG    *(__IO uint16_t *) (0x6C000000)
/*lcd reset pin def*/
//#define LCD_RST_PORT_RCC	RCC_AHB1Periph_GPIOD
//#define LCD_RST_PIN			GPIO_Pin_3
//#define LCD_RST_PORT		GPIOD






void SCARA_lcd_init(void);
void ILI9325_Init(void);
void lcd_clear(unsigned short color);




#define LCD_WIDTH   240
#define LCD_HEIGHT  320

/* LCD color */
#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0


//void lcd_init(void);
void lcd_clear(unsigned short Color);
void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t Color);
void LCD_ShowString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor);
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t colour);
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t PointColor);
void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t Color);
void LCD_DrawPicture(u16 StartX,u16 StartY,u16 Xend,u16 Yend,u8 *pic);
void LCD_DrawPoint(uint16_t xsta, uint16_t ysta, uint16_t PointColor);
unsigned short lcd_read_gram(unsigned int x,unsigned int y);
void write_data(unsigned short data_code );
void write_reg(unsigned char reg_addr,unsigned short reg_val);

#endif
