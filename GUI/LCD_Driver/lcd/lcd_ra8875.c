

#include "stm32f4xx.h"
#include <stdio.h>
#include "lcd_ra8875.h"

	
/* 片选口线置低选中  */
#define RA8875_CS_0()      GPIO_ResetBits(GPIOA, GPIO_Pin_4)

/* 片选口线置高不选中 */
#define RA8875_CS_1()      GPIO_SetBits(GPIOA, GPIO_Pin_4)	
	
//#ifdef IF_SPI_EN	/* 4-Wire SPI 界面 (需要改变模块上的2个跳线帽) */
//	#define SPI_WRITE_DATA	0x00
//	#define SPI_READ_DATA	  0x40
//	#define SPI_WRITE_CMD	  0x80
//	#define SPI_READ_STATUS	0xC0
//	
//#else		/* 8080总线 （缺省模式） */
///* 定义LCD驱动器的访问地址 */
//	#define RA8875_REG		*(__IO uint16_t *)0x60020000
//	#define RA8875_RAM		*(__IO uint16_t *)0x60000000
//#endif

uint16_t g_LcdHeight = 272;			/* 显示屏分辨率-高度 */
uint16_t g_LcdWidth = 480;			/* 显示屏分辨率-宽度 */	

static __IO uint16_t s_usTouchX, s_usTouchY;	/* 触摸屏ADC值 */
static __IO uint8_t s_ucRA8875Busy = 0;

/* 保存当前显示窗口的位置和大小，这几个变量由 RA8875_SetDispWin() 进行设置 */
static __IO uint16_t s_WinX = 0;
static __IO uint16_t s_WinY = 0;
static __IO uint16_t s_WinHeight = 272;
static __IO uint16_t s_WinWidth = 480;

static void RA8875_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue);
static uint8_t RA8875_ReadReg(uint8_t _ucRegAddr);
static uint8_t RA8875_ReadStatus(void);

static void RA8875_SetCursor(uint16_t _usX, uint16_t _usY);
static void RA8875_SetReadCursor(uint16_t _usX, uint16_t _usY);
static void RA8875_SetTextCursor(uint16_t _usX, uint16_t _usY);

static void BTE_SetTarBlock(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t _ucLayer);
static void BTE_SetOperateCode(uint8_t _ucOperate);
static void BTE_Start(void);

#ifdef IF_SPI_EN
static uint8_t SPI_ShiftByte(uint8_t _ucByte);
void RA8875_HighSpeedSPI(void);
#endif

uint8_t s_ucBright = 200;					    /* 背光亮度参数，默认为200 */

////////////////////////////////////////////////////////////////////

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteCmd
*	功能说明: 写RA8875指令寄存器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void myLCD_Delay(unsigned int Count)
{
		volatile unsigned int TimingDelay;
		while(Count--)
		{
				for(TimingDelay = 0; TimingDelay < 100000; TimingDelay++)
				{
				}
		}
}

void LCD_Delay1ms(void)
{
			uint16_t i;

	for (i = 0; i < 5000; i++);	/* 延迟1ms, 不准 */
}

static void RA8875_WriteCmd(uint8_t _ucRegAddr)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_CMD);
	SPI_ShiftByte(_ucRegAddr);
	RA8875_CS_1();

#else	/* 8080 总线 */
	RA8875_REG = _ucRegAddr;	/* 设置寄存器地址 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteData
*	功能说明: 写RA8875指令寄存器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_WriteData(uint8_t _ucRegValue)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_ucRegValue);
	RA8875_CS_1();

#else	/* 8080 总线 */
	RA8875_RAM = _ucRegValue;	/* 设置寄存器地址 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadData
*	功能说明: 读RA8875寄存器值
*	形    参: 无
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint8_t RA8875_ReadData(uint8_t _ucRegAddr)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	uint16_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#else	/* 8080 总线 */
	uint8_t value;

	value = RA8875_RAM;		/* 读取寄存器值 */

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteData16
*	功能说明: 写RA8875数据总线，16bit，用于RGB显存写入
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_WriteData16(uint16_t _usRGB)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_usRGB >> 8);
	RA8875_CS_1();

	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_usRGB);
	RA8875_CS_1();

#else	/* 8080 总线 */
	RA8875_RAM = _usRGB;	/* 设置寄存器地址 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadData16
*	功能说明: 读RA8875显存，16bit RGB
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint16_t RA8875_ReadData16(void)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	uint16_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#else	/* 8080 总线 */
	uint16_t value;

	value = RA8875_RAM;		/* 读取寄存器值 */

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteReg
*	功能说明: 写RA8875寄存器. RA8875的寄存器地址和数据都是8bit的
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	s_ucRA8875Busy = 1;
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_CMD);
	SPI_ShiftByte(_ucRegAddr);
	RA8875_CS_1();

	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_ucRegValue);
	RA8875_CS_1();
	s_ucRA8875Busy = 0;
#else	/* 8080 总线 */
	s_ucRA8875Busy = 1;
	RA8875_REG = _ucRegAddr;	/* 设置寄存器地址 */
	RA8875_RAM = _ucRegValue;	/* 写入寄存器值 */
	s_ucRA8875Busy = 0;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteReg
*	功能说明: 写RA8875寄存器. RA8875的寄存器地址和数据都是8bit的
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t RA8875_ReadReg(uint8_t _ucRegAddr)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	uint8_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_CMD);
	SPI_ShiftByte(_ucRegAddr);
	RA8875_CS_1();

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#else	/* 8080 总线 */
	uint8_t value;

	RA8875_REG = _ucRegAddr;/* 设置寄存器地址 */
	value = RA8875_RAM;		/* 读取寄存器值 */

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadStatus
*	功能说明: 读RA8875状态寄存器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t RA8875_ReadStatus(void)
{
#ifdef IF_SPI_EN	/* 四线SPI */
	uint8_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_STATUS);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#else	/* 8080 总线 */
	uint8_t value;

	value = RA8875_REG;

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_InitHard
*	功能说明: 初始化RA8875驱动芯片
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_InitHard(void)
{

	  g_LcdHeight = LCD_43_HEIGHT;
		g_LcdWidth = LCD_43_WIDTH;

		/* 初始化PLL.  晶振频率为25M */
		RA8875_WriteCmd(0x88);
//		bsp_DelayMS(1);
		LCD_Delay1ms();
		RA8875_WriteData(10);	/* PLLDIVM [7] = 0 ;  PLLDIVN [4:0] = 10 */

//	  bsp_DelayMS(1);
		LCD_Delay1ms();
		RA8875_WriteCmd(0x89);
//		bsp_DelayMS(1);
		LCD_Delay1ms();
		RA8875_WriteData(2);	/* PLLDIVK[2:0] = 2, 除以4 */

		/* RA8875 的内部系统频率 (SYS_CLK) 是结合振荡电路及PLL 电路所产生，频率计算公式如下 :
		  SYS_CLK = FIN * ( PLLDIVN [4:0] +1 ) / ((PLLDIVM+1 ) * ( 2^PLLDIVK [2:0] ))
		          = 25M * (10 + 1) / ((0 + 1) * (2 ^ 2))
				  = 68.75MHz
		*/

		/* REG[88h]或REG[89h]被设定后，为保证PLL 输出稳定，须等待一段「锁频时间」(< 100us)。*/
//	    bsp_DelayMS(1);
		LCD_Delay1ms();
		/*
			配置系统控制寄存器。 中文pdf 第18页:

			bit3:2 色彩深度设定 (Color Depth Setting)
				00b : 8-bpp 的通用TFT 接口， i.e. 256 色。
				1xb : 16-bpp 的通用TFT 接口， i.e. 65K 色。	 【选这个】

			bit1:0 MCUIF 选择
				00b : 8-位MCU 接口。
				1xb : 16-位MCU 接口。 【选这个】
		*/
		#ifdef IF_SPI_EN
			RA8875_WriteReg(0x10, (1 <<3 ) | (0 << 1));	/* SPI接口时，配置8位，65K色 */
		#else
			RA8875_WriteReg(0x10, (1 <<3 ) | (1 << 1));	/* 配置16位MCU并口，65K色 */
		#endif

		/* REG[04h] Pixel Clock Setting Register   PCSR
			bit7  PCLK Inversion
				0 : PDAT 是在PCLK 正缘上升 (Rising Edge) 时被取样。
				1 : PDAT 是在PCLK 负缘下降 (Falling Edge) 时被取样。
			bit1:0 PCLK 频率周期设定
				Pixel Clock  PCLK 频率周期设定。
				00b: PCLK 频率周期= 系统频率周期。
				01b: PCLK 频率周期= 2 倍的系统频率周期。
				10b: PCLK 频率周期= 4 倍的系统频率周期。
				11b: PCLK 频率周期= 8 倍的系统频率周期。
		*/
	  RA8875_WriteReg(0x04,0x82);           //通过04H设置采样边沿和PCLK像素时钟(Pixel Clock)
//	  bsp_DelayMS(1);
		LCD_Delay1ms();
		
	  RA8875_WriteReg(0x14,0x3B);           //水平设置:水平显示宽度(像素) = (HDWR + 1) * 8 = 480
	  RA8875_WriteReg(0x15,0x00);           //水平非显示周期微调宽度设置 (HNDFTR)
		RA8875_WriteReg(0x16,0x01);           //水平非显示宽度(像素) = (HNDR + 1) * 8
		RA8875_WriteReg(0x17,0x00);           //水平同步起始地址宽度 = (HSTR + 1) * 8
		RA8875_WriteReg(0x18, 0x05);          //水平同步信号脉波宽度(像素) = (HPWR + 1) * 8

		RA8875_WriteReg(0x19, 0x0F);          //垂直显示高度低8位 [0-7] （VDHR0） 
		RA8875_WriteReg(0x1A, 0x01);          //垂直显示高度(像素) = VDHR + 1 = 0X10F + 1 = 272
		RA8875_WriteReg(0x1B, 0x02);          //垂直非显示周期位[7:0]（VNDR0） 
		RA8875_WriteReg(0x1C, 0x00);          //垂直非显示周期位[8] （VNDR1）
		RA8875_WriteReg(0x1D, 0x07);          //垂直同步信号起始地址高度位[7:0]  VSTR0 
		RA8875_WriteReg(0x1E, 0x00);          //垂直同步信号起始地址高度位[8]  VSTR1  
		RA8875_WriteReg(0x1F, 0x09);          //垂直同步信号脉波宽度 VPWR


	/* 设置TFT面板的 DISP  引脚为高，使能面板. 模块的DISP引脚连接到RA8875芯片的GP0X脚 */
	RA8875_WriteReg(0xC7, 0x01);	/* DISP = 1 */

	/* LCD 显示/关闭讯号 (LCD Display on) */
	RA8875_WriteReg(0x01, 0x80);

	/* 	REG[40h] Memory Write Control Register 0 (MWCR0)

		Bit 7	显示模式设定
			0 : 绘图模式。
			1 : 文字模式。

		Bit 6	文字写入光标/内存写入光标设定
			0 : 设定文字/内存写入光标为不显示。
			1 : 设定文字/内存写入光标为显示。

		Bit 5	文字写入光标/内存写入光标闪烁设定
			0 : 游标不闪烁。
			1 : 游标闪烁。

		Bit 4   NA

		Bit 3-2  绘图模式时的内存写入方向
			00b : 左 -> 右，然后上 -> 下。
			01b : 右 -> 左，然后上 -> 下。
			10b : 上 -> 下，然后左 -> 右。
			11b : 下 -> 上，然后左 -> 右。

		Bit 1 	内存写入光标自动增加功能设定
			0 : 当内存写入时光标位置自动加一。
			1 : 当内存写入时光标位置不会自动加一。

		Bit 0 内存读取光标自动增加功能设定
			0 : 当内存读取时光标位置自动加一。
			1 : 当内存读取时光标位置不会自动加一。
	*/
	RA8875_WriteReg(0x40, 0x00);	/* 选择绘图模式 */


	/* 	REG[41h] Memory Write Control Register1 (MWCR1)
		写入目的位置，选择图层1
	*/
	RA8875_WriteReg(0x41, 0x00);	/* 选择绘图模式, 目的为CGRAM */

	RA8875_SetDispWin(0, 0, g_LcdHeight, g_LcdWidth);

#ifdef IF_SPI_EN
	RA8875_HighSpeedSPI();		/* 配置完毕后，切换SPI到高速模式 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: BTE_SetTarBlock
*	功能说明: 设置RA8875 BTE目标区块以及目标图层
*	形    参:
*			uint16_t _usX : 水平起点坐标
*			uint16_t _usY : 垂直起点坐标
*			uint16_t _usHeight : 区块高度
*			uint16_t _usWidth : 区块宽度
*			uint8_t _ucLayer ： 0 图层1； 1 图层2
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_SetTarBlock(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t _ucLayer)
{
	/* 设置起点坐标 */
	RA8875_WriteReg(0x58, _usX);
	RA8875_WriteReg(0x59, _usX >> 8);

	RA8875_WriteReg(0x5A, _usY);
	if (_ucLayer == 0)	/* 图层2 */
	{
		RA8875_WriteReg(0x5B, _usY >> 8);
	}
	else
	{
		RA8875_WriteReg(0x5B, (1 << 7) | (_usY >> 8));	/* Bit7 表示图层， 0 图层1； 1 图层2*/
	}

	/* 设置区块宽度 */
	RA8875_WriteReg(0x5C, _usWidth);
	RA8875_WriteReg(0x5D, _usWidth >> 8);

	/* 设置区块高度 */
	RA8875_WriteReg(0x5E, _usHeight);
	RA8875_WriteReg(0x5F, _usHeight >> 8);
}

/*
*********************************************************************************************************
*	函 数 名: BTE_SetOperateCode
*	功能说明: 设定BTE 操作码和光栅运算码
*	形    参: _ucOperate : 操作码
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_SetOperateCode(uint8_t _ucOperate)
{
	/*  设定BTE 操作码和光栅运算码  */
	RA8875_WriteReg(0x51, _ucOperate);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetFrontColor
*	功能说明: 设定前景色
*	形    参: 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetFrontColor(uint16_t _usColor)
{
	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x63, (_usColor & 0xF800) >> 11);	/* R5  */
	RA8875_WriteReg(0x64, (_usColor & 0x07E0) >> 5);	/* G6 */
	RA8875_WriteReg(0x65, (_usColor & 0x001F));			/* B5 */
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetBackColor
*	功能说明: 设定背景色
*	形    参: 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetBackColor(uint16_t _usColor)
{
	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x60, (_usColor & 0xF800) >> 11);	/* R5  */
	RA8875_WriteReg(0x61, (_usColor & 0x07E0) >> 5);	/* G6 */
	RA8875_WriteReg(0x62, (_usColor & 0x001F));			/* B5 */
	s_ucRA8875Busy = 0;
}


/*
*********************************************************************************************************
*	函 数 名: BTE_Start
*	功能说明: 启动BTE操作
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_Start(void)
{
	s_ucRA8875Busy = 1;
	/* RA8875_WriteReg(0x50, 0x80);  不能使用这个函数，因为内部已经操作了 s_ucRA8875Busy 标志 */
	RA8875_WriteCmd(0x50);	/* 设置寄存器地址 */
	RA8875_WriteData(0x80);	/* 写入寄存器值 */
}

/*
*********************************************************************************************************
*	函 数 名: BTE_Wait
*	功能说明: 等待BTE操作结束
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_Wait(void)
{
	while ((RA8875_ReadStatus() & 0x40) == 0x40);
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_IsBusy
*	功能说明: RA8875是否忙
*	形    参: 无
*	返 回 值: 1 表示忙； 0 表示空闲
*********************************************************************************************************
*/
uint8_t RA8875_IsBusy(void)
{
	if (s_ucRA8875Busy == 0)
	{
		return 0;
	}
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WaitBusy
*	功能说明: 等待RA8875空闲
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_WaitBusy(void)
{
	while ((RA8875_ReadStatus() & 0x80) == 0x80);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_Layer1Visable
*	功能说明: RA8875 图层1显示打开
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_Layer1Visable(void)
{
	/* 0x52 寄存器的 Bit2:0
		图层显示模式
		000b : 只有图层1 显示。
		001b : 只有图层2 显示。
		010b : 显示图层1 与图层2 的渐进/渐出模式。
		011b : 显示图层1 与图层2 的通透模式。
		100b : Boolean OR。
		101b : Boolean AND。
		110b : 浮动窗口模式 (Floating window mode)。
		111b :保留。
	*/
	RA8875_WriteReg(0x52, RA8875_ReadReg(0x52) & 0xF8);	/* 只有图层1 显示 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_Layer2Visable
*	功能说明: 只显示图层2
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_Layer2Visable(void)
{
	/* 0x52 寄存器的 Bit2:0
		图层显示模式
		000b : 只有图层1 显示。
		001b : 只有图层2 显示。
		010b : 显示图层1 与图层2 的渐进/渐出模式。
		011b : 显示图层1 与图层2 的通透模式。
		100b : Boolean OR。
		101b : Boolean AND。
		110b : 浮动窗口模式 (Floating window mode)。
		111b :保留。
	*/
	RA8875_WriteReg(0x52, (RA8875_ReadReg(0x52) & 0xF8) | 0x01);	/* 只有图层2 显示 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_TouchInit
*	功能说明: 初始化触摸板
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_TouchInit(void)
{
	RA8875_WriteReg(0xF0, (1 << 2));	/* 开启触控面板中断位 */

	/*
		REG[71h] Touch Panel Control Register 1 (TPCR1)

		Bit7 N/A
		Bit6 触控面板模式设定
			0 : 自动模式。
			1 : 手动模式。
		Bit5 触控面板控制器ADC 参考电压(Vref)来源设定
			0 : 内部产生参考电压。
			1 : 外部输入参考电压，ADC 参考电压准位= 1/2 VDD。
		Bit4-3 N/A
		Bit2 触控中断讯号的消除弹跳电路选择
			0: 关闭消除弹跳电路。
			1: 开启消除弹跳电路。
		Bit1-0 触控面板手动模式之选择位
			00b : 闲置模式。触控控制单元进入闲置模式。
			01b : 侦测触摸事件发生。在此模式控制器会侦测触摸事件的发
				生，事件发生可以引发中断或是由缓存器得知(REG[F1h]
				Bit2)。
			10b : X 轴数据撷取模式。在此模式触摸位置的X 轴数据会被储
				存至 REG[72h] 和REG[74h]。
			11b : Y 轴数据撷取模式。在此模式触摸位置的Y 轴数据会被储
				存至REG[73h] and REG[74h]。
	*/
	RA8875_WriteReg(0x71, 0x04);	/* 选择自动模式、内部产生参考电压 */

	/*
		REG[70h] Touch Panel Control Register 0 (TPCR0)

		Bit7 触控面板功能设定
			0 : 关闭。
			1 : 开启。
		Bit6-4 触控面板控制器取样时间设定
			000b : ADC 取样时间为512 个系统频率周期。
			001b : ADC 取样时间为 1024 个系统频率周期。
			010b : ADC 取样时间为 2048 个系统频率周期。
			011b : ADC 取样时间为 4096 个系统频率周期。
			100b : ADC 取样时间为 8192 个系统频率周期。
			101b : ADC 取样时间为 16384 个系统频率周期。
			110b : ADC 取样时间为 32768 个系统频率周期。
			111b : ADC 取样时间为65536 个系统频率周期。
		Bit3 触控面板唤醒模式
			0 : 关闭触控事件唤醒模式。
			1 : 触控事件可唤醒睡眠模式。
		Bit2-0 触控面板控制器ADC 频率设定
			000b : 系统频率。
			001b : 系统频率 / 2。
			010b : 系统频率 / 4。
			011b : 系统频率 / 8。
			100b : 系统频率 / 16。
			101b : 系统频率 / 32。
			110b : 系统频率 / 64。
			111b : 系统频率 / 128。

		注 : ADC 的输入频率设定不能超过10MHz。
	*/
	RA8875_WriteReg(0x70,0xC3);	/* 开启触摸功能； */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_TouchReadX
*	功能说明: 读取X位置ADC   （这个函数被定时中断服务程序调用，需要避免寄存器访问冲突 )
*	形    参: 无
*	返 回 值: ADC值，10Bit
*********************************************************************************************************
*/
uint16_t RA8875_TouchReadX(void)
{
	uint16_t usAdc;
	uint8_t ucRegValue;
	uint8_t ucReg74;

	/* 软件读取中断事件标志 */
	ucRegValue = RA8875_ReadReg(0xF1);
	if (ucRegValue & (1 << 2))
	{
		ucReg74 = RA8875_ReadReg(0x74);
		usAdc = RA8875_ReadReg(0x72);	/* Bit9-2 */
		usAdc <<= 2;
		usAdc += (ucReg74 & 0x03);

		s_usTouchX = usAdc;

		usAdc = RA8875_ReadReg(0x73);	/* Bit9-2 */
		usAdc <<= 2;
		usAdc += ((ucReg74 & 0x0C) >> 2);

		s_usTouchY = usAdc;
	}
	else
	{
		s_usTouchX = 0;
		s_usTouchY = 0;
	}

	/*
		bit2 写入功能?? 触控面板中断清除位
		0 : 未操作。
		1 : 清除触控面板中断。
		读取功能?? 触控面板中断状态
		0 : 未发生触控面板中断。
		1 : 发生触控面板中断。
	*/

	/*
		不要调用这个函数写寄存器，因为该函数改写了busy标志
		RA8875_WriteReg(0xF1, (1 << 2));	 必须清除， 才会下次采样.
	*/
	RA8875_WriteCmd(0xF1);
	RA8875_WriteData(1 << 2);

	return s_usTouchX;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_TouchReadY
*	功能说明: 读取Y位置ADC ； 必须先调用 RA8875_TouchReadX()
*	形    参: 无
*	返 回 值: ADC值，10Bit
*********************************************************************************************************
*/
uint16_t RA8875_TouchReadY(void)
{
	return s_usTouchY;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DispOn
*	功能说明: 打开显示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DispOn(void)
{
	RA8875_WriteReg(0x01, 0x80);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DispOff
*	功能说明: 关闭显示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DispOff(void)
{
	RA8875_WriteReg(0x01, 0x00);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_PutPixel
*	功能说明: 画1个像素
*	形    参：
*			_usX,_usY : 像素坐标
*			_usColor  ：像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	RA8875_SetCursor(_usX, _usY);	/* 设置光标位置 */

	s_ucRA8875Busy = 1;

	RA8875_WriteCmd(0x02); 		/* 用于设定RA8875 进入内存(DDRAM或CGRAM)读取/写入模式 */
	RA8875_WriteData16(_usColor);
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_GetPixel
*	功能说明: 读取1个像素
*	形    参：
*			_usX,_usY : 像素坐标
*			_usColor  ：像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t RA8875_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	RA8875_SetReadCursor(_usX, _usY);	/* 设置读取光标位置 */

	s_ucRA8875Busy = 1;

	RA8875_WriteCmd(0x02);
	usRGB = RA8875_ReadData16();

	s_ucRA8875Busy = 0;

	return usRGB;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ClrScr
*	功能说明: 根据输入的颜色值清屏.RA8875支持硬件单色填充。该函数仅对当前激活的显示窗口进行清屏. 显示
*			 窗口的位置和大小由 RA8875_SetDispWin() 函数进行设置
*	形    参：_usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_ClrScr(uint16_t _usColor)
{
	/* 也可以通过0x30-0x37寄存器获得获得当前激活的显示窗口 */

	/* 单色填满功能, 中文pdf 第162页
	此功能使用于将选定特定区域画面清除或是塡入给定某种前景色，R8875 填入的单色设定为BTE 前景色。

	操作步骤：
		1. 设定目的图层和位置 REG[58h], [59h], [5Ah], [5Bh]
		2. 设定BTE 宽度和高度 REG[5Ch], [5Dh], [5Eh], [5Fh]
		3. 设定BTE 操作码和光栅运算码  REG[51h] Bit[3:0] = 0Ch
		4. 设定前景色  REG[63h], [64h], [65h]
		5. 开启BTE 功能  REG[50h] Bit7 = 1
		6. 检查状态缓存器 STSR Bit6，确认BTE 是否完成
	*/
	BTE_SetTarBlock(s_WinX, s_WinY, s_WinHeight, s_WinWidth, 0);	/* 设置BTE位置和宽度高度以及目标图层（0或1） */
	BTE_SetOperateCode(0x0C);		/* 设定BTE 操作码和光栅运算码  REG[51h] Bit[3:0] = 0Ch */
	RA8875_SetFrontColor(_usColor);	/* 设置BTE前景色 */
	BTE_Start();					/* 开启BTE 功能 */
	BTE_Wait();						/* 等待操作结束 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参：
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint32_t index = 0;
	const uint16_t *p;

	/* 设置图片的位置和大小， 即设置显示窗口 */
	RA8875_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	s_ucRA8875Busy = 1;

	RA8875_WriteCmd(0x02); 		/* 准备读写显存 */

	p = _ptr;
	for (index = 0; index < _usHeight * _usWidth; index++)
	{
		/*
      进行优化, 函数就地展开
			RA8875_WriteRAM(_ptr[index]);

			此处可考虑用DMA操作
		*/
		RA8875_WriteData16(*p++);
	}
	s_ucRA8875Busy = 0;

	/* 退出窗口绘图模式 */
	RA8875_QuitWinMode();
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DrawLine
*	功能说明: 采用RA8875的硬件功能，在2点间画一条直线。
*	形    参：
*			_usX1, _usY1 ：起始点坐标
*			_usX2, _usY2 ：终止点Y坐标
*			_usColor     ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	/* pdf 第131页
		RA8875 支持直线绘图功能，让使用者以简易或低速的MCU 就可以在TFT 模块上画直线。先设
		定直线的起始点REG[91h~94h] 与结束点REG[95h~98h]，直线的颜色REG[63h~65h]，然后启
		动绘图设定REG[90h] Bit4 = 0, Bit0=0 且REG[90h] Bit7 = 1，RA8875 就会将直线的图形写入
		DDRAM，相对的在TFT 模块上就可以显示所画的直线。
	*/

	/* 设置起点坐标 */
	RA8875_WriteReg(0x91, _usX1);
	RA8875_WriteReg(0x92, _usX1 >> 8);
	RA8875_WriteReg(0x93, _usY1);
	RA8875_WriteReg(0x94, _usY1 >> 8);

	/* 设置终点坐标 */
	RA8875_WriteReg(0x95, _usX2);
	RA8875_WriteReg(0x96, _usX2 >> 8);
	RA8875_WriteReg(0x97, _usY2);
	RA8875_WriteReg(0x98, _usY2 >> 8);

	RA8875_SetFrontColor(_usColor);	/* 设置直线的颜色 */

	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x90, (1 << 7) | (0 << 4) | (0 << 0));	/* 开始画直线 */
//	while (RA8875_ReadReg(0x90) & (1 << 7));				/* 等待结束 */
	RA8875_WaitBusy();
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DrawRect
*	功能说明: 采用RA8875硬件功能绘制矩形
*	形    参：
*			_usX,_usY：矩形左上角的坐标
*			_usHeight ：矩形的高度
*			_usWidth  ：矩形的宽度
*			_usColor  ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
		RA8875 支持方形绘图功能，让使用者以简易或低速的MCU 就可以在TFT 模块上画方形。先设
	定方形的起始点REG[91h~94h]与结束点REG[95h~98h]，方形的颜色REG[63h~65h]，然后启
	动绘图设定REG[90h] Bit4=1, Bit0=0 且REG[90h] Bit7 = 1，RA8875 就会将方形的图形写入
	DDRAM，相对的在TFT 模块上就可以显示所画的方形。若设定REG[90h] Bit5 = 1，则可画出一
	实心方形 (Fill)

	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	/* 设置起点坐标 */
	RA8875_WriteReg(0x91, _usX);
	RA8875_WriteReg(0x92, _usX >> 8);
	RA8875_WriteReg(0x93, _usY);
	RA8875_WriteReg(0x94, _usY >> 8);

	/* 设置终点坐标 */
	RA8875_WriteReg(0x95, _usX + _usWidth - 1);
	RA8875_WriteReg(0x96, (_usX + _usWidth - 1) >> 8);
	RA8875_WriteReg(0x97, _usY + _usHeight - 1);
	RA8875_WriteReg(0x98, (_usY + _usHeight - 1) >> 8);

	RA8875_SetFrontColor(_usColor);	/* 设置颜色 */

	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x90, (1 << 7) | (0 << 5) | (1 << 4) | (0 << 0));	/* 开始画矩形  */
//	while (RA8875_ReadReg(0x90) & (1 << 7));							/* 等待结束 */
	RA8875_WaitBusy();
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_FillRect
*	功能说明: 采用RA8875硬件功能填充一个矩形为单色
*	形    参：
*			_usX,_usY：矩形左上角的坐标
*			_usHeight ：矩形的高度
*			_usWidth  ：矩形的宽度
*			_usColor  ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
		RA8875 支持方形绘图功能，让使用者以简易或低速的MCU 就可以在TFT 模块上画方形。先设
	定方形的起始点REG[91h~94h]与结束点REG[95h~98h]，方形的颜色REG[63h~65h]，然后启
	动绘图设定REG[90h] Bit4=1, Bit0=0 且REG[90h] Bit7 = 1，RA8875 就会将方形的图形写入
	DDRAM，相对的在TFT 模块上就可以显示所画的方形。若设定REG[90h] Bit5 = 1，则可画出一
	实心方形 (Fill)

	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	/* 设置起点坐标 */
	RA8875_WriteReg(0x91, _usX);
	RA8875_WriteReg(0x92, _usX >> 8);
	RA8875_WriteReg(0x93, _usY);
	RA8875_WriteReg(0x94, _usY >> 8);

	/* 设置终点坐标 */
	RA8875_WriteReg(0x95, _usX + _usWidth - 1);
	RA8875_WriteReg(0x96, (_usX + _usWidth - 1) >> 8);
	RA8875_WriteReg(0x97, _usY + _usHeight - 1);
	RA8875_WriteReg(0x98, (_usY + _usHeight - 1) >> 8);

	RA8875_SetFrontColor(_usColor);	/* 设置颜色 */

	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x90, (1 << 7) | (1 << 5) | (1 << 4) | (0 << 0));	/* 开始填充矩形  */
//	while (RA8875_ReadReg(0x90) & (1 << 7));							/* 等待结束 */
	RA8875_WaitBusy();
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参：
*			_usX,_usY  ：圆心的坐标
*			_usRadius  ：圆的半径
*			_usColor  ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	/*
		RA8875 支持圆形绘图功能，让使用者以简易或低速的MCU 就可以在TFT 模块上画圆。先设定
		圆的中心点REG[99h~9Ch]，圆的半径REG[9Dh]，圆的颜色REG[63h~65h]，然后启动绘图
		REG[90h] Bit6 = 1，RA8875 就会将圆的图形写入DDRAM，相对的在TFT 模块上就可以显示所
		画的圆。若设定REG[90h] Bit5 = 1，则可画出一实心圆 (Fill)；若设定REG[90h] Bit5 = 0，则可
		画出空心圆 (Not Fill
	*/
	/* 设置圆的半径 */
	if (_usRadius > 255)
	{
		return;
	}

	/* 设置圆心坐标 */
	RA8875_WriteReg(0x99, _usX);                      //画圆形中心点的水平位置[7:0]  
	RA8875_WriteReg(0x9A, _usX >> 8);                 //画圆形中心点的水平位置[9:8]
	RA8875_WriteReg(0x9B, _usY);                      //画圆形中心点的垂直位置[7:0]  
	RA8875_WriteReg(0x9C, _usY >> 8);                 //画圆形中心点的垂直位置[8]  

	RA8875_WriteReg(0x9D, _usRadius);	                //画圆形的半径 [7:0]

	RA8875_SetFrontColor(_usColor);	                  //设置颜色

	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x90, (1 << 6) | (0 << 5));				//开始画圆, 不填充
  while (RA8875_ReadReg(0x90) & (1 << 6));				  //等待画圆结束
	//RA8875_WaitBusy();
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_FillCircle
*	功能说明: 填充一个圆
*	形    参：
*			_usX,_usY  ：圆心的坐标
*			_usRadius  ：圆的半径
*			_usColor   ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	/*
		RA8875 支持圆形绘图功能，让使用者以简易或低速的MCU 就可以在TFT 模块上画圆。先设定
		圆的中心点REG[99h~9Ch]，圆的半径REG[9Dh]，圆的颜色REG[63h~65h]，然后启动绘图
		REG[90h] Bit6 = 1，RA8875 就会将圆的图形写入DDRAM，相对的在TFT 模块上就可以显示所
		画的圆。若设定REG[90h] Bit5 = 1，则可画出一实心圆 (Fill)；若设定REG[90h] Bit5 = 0，则可
		画出空心圆 (Not Fill
	*/
	/* 设置圆的半径 */
	if (_usRadius > 255)
	{
		return;
	}

	/* 设置圆心坐标 */
	RA8875_WriteReg(0x99, _usX);
	RA8875_WriteReg(0x9A, _usX >> 8);
	RA8875_WriteReg(0x9B, _usY);
	RA8875_WriteReg(0x9C, _usY >> 8);

	RA8875_WriteReg(0x9D, _usRadius);	                /* 设置圆的半径 */

	RA8875_SetFrontColor(_usColor);	                  /* 设置颜色 */

	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x90, (1 << 6) | (1 << 5));				/* 开始画圆, 填充  */
  while (RA8875_ReadReg(0x90) & (1 << 6));				  /* 等待结束 */
	//RA8875_WaitBusy();
	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetFont
*	功能说明: 文本模式，设置文字字体、行距和字距
*	形    参：
*			_ucFontType : 字体类型: RA_FONT_16, RA_FONT_24, RA_FONT_32
*			_ucLineSpace : 行距，像素单位 (0-31)
*			_ucCharSpace : 字距，像素单位 (0-63)
*
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetFont(uint8_t _ucFontType, uint8_t _ucLineSpace, uint8_t _ucCharSpace)
{
	/*
		[29H]在文字模式下，用来设定文字间的行距 (单位: 像素) 。
		只有低5个bit有效，0-31
	*/
	if (_ucLineSpace >31)
	{
		_ucLineSpace = 31;
	}
	RA8875_WriteReg(0x29, _ucLineSpace);

	/*
		[2EH] 设置字符间距（像素单位，0-63），和字体（16*16，24*24，32*32）
	*/
	if (_ucCharSpace > 63)
	{
		_ucCharSpace = 63;
	}
	if (_ucFontType > RA_FONT_32)
	{
		_ucFontType = RA_FONT_16;
	}
	RA8875_WriteReg(0x2E, (_ucCharSpace & 0x3F) | (_ucFontType << 6));
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetFontZoom
*	功能说明: 文本模式，设置文字的放大模式，1X,2X,3X, 4X
*	形    参：
*			_ucHSize : 文字水平放大倍数，0-3 分别对应 X1、X2、X3、X4
*			_ucVSize : 文字处置放大倍数，0-3 分别对应 X1、X2、X3、X4
*		建议输入参数用枚举：RA_SIZE_X1、RA_SIZE_X2、RA_SIZE_X3、RA_SIZE_X4
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetTextZoom(uint8_t _ucHSize, uint8_t _ucVSize)
{
	/*
		pdf 第22页		[22H]
		bit7 用于对齐，一般不用，缺省设0
		bit6 用于通透，一般不用，缺省设0
		bit4 用于旋转90读，一般不用，缺省设0
		bit3-2 水平放大倍数
		bit1-0 垂直放大倍数
	*/
	RA8875_WriteReg(0x22, ((_ucHSize & 0x03) << 2) | ( _ucVSize & 0x03));
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DispAscii
*	功能说明: 显示ASCII字符串，字符点阵来自于RA8875内部ROM中的ASCII字符点阵（8*16），不支持汉字.
*			文字颜色、背景颜色、是否通透由其他的函数进行设置
*			void RA8875_SetFrontColor(uint16_t _usColor);
*			void RA8875_SetBackColor(uint16_t _usColor);
*	形    参：_usX, _usY : 显示位置（坐标）
*			 _ptr : AascII字符串，已0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DispAscii(uint16_t _usX, uint16_t _usY, char *_ptr)
{
	/*
		RA8875 内建8x16 点的ASCII 字型ROM，提供使用者更方便的方式用特定编码 (Code) 输入文
		字。内建的字集支持ISO/IEC 8859-1~4 编码标准，此外，使用者可以透过REG[60h~62h] 选择
		文字前景颜色，以及透过REG[63h~65h] 选择背景颜色.

		ISO/IEC 8859-1，又称Latin-1或“西欧语言”，是国际标准化组织内ISO/IEC 8859的第一个8位字符集。
		它以ASCII为基础，在空置的0xA0-0xFF的范围内，加入192个字母及符号，藉以供使用变音符号的拉丁字母语言使用。

		ISO/IEC 8859-2 Latin-2或“中欧语言”，是国际标准化组织内ISO/IEC 8859的其中一个8位字符集 .
		ISO/IEC 8859-3 南欧语言字符集
		ISO/IEC 8859-4 北欧语言字符集
	*/

	/*
	(1) Text mode  REG[40h] bit7=1
	(2) Internal Font ROM Select   REG[21h] bit7=0, bit5=0
	(3) Font foreground and background color Select  REG[63h~65h], REG[60h~62h]
	(4) Write the font Code  CMD_WR[02h]    DATA_WR[font_code]
	*/

	RA8875_SetTextCursor(_usX, _usY);

	RA8875_WriteReg(0x40, (1 << 7));	/* 设置为文本模式 */

	/* 选择CGROM font; 选择内部CGROM; 内部CGROM 编码选择ISO/IEC 8859-1. */
	RA8875_WriteReg(0x2F, 0x00);
	RA8875_WriteReg(0x21, (0 << 7) | (0 << 5) | (0 << 1) | (0 << 0));

	s_ucRA8875Busy = 1;


	RA8875_WriteCmd(0x02); 			/* 用于设定RA8875 进入内存(DDRAM或CGRAM)读取/写入模式 */

	/* 开始循环处理字符 */
	while (*_ptr != 0)
	{
		RA8875_WriteData(*_ptr);
		while ((RA8875_ReadStatus() & 0x80) == 0x80);	/* 必须等待内部写屏操作完成 */
		_ptr++;
	}
	s_ucRA8875Busy = 0;

	RA8875_WriteReg(0x40, (0 << 7));	/* 还原为图形模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DispStr
*	功能说明: 显示字符串，字符点阵来自于RA8875外接的字库芯片，支持汉字.
*			文字颜色、背景颜色、是否通透由其他的函数进行设置
*			void RA8875_SetFrontColor(uint16_t _usColor);
*			void RA8875_SetBackColor(uint16_t _usColor);
*	形    参：_usX, _usY : 显示位置（坐标）
*			 _ptr : AascII字符串，已0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr)
{
	/*
		RA8875 透过使用 — 上海集通公司 (Genitop Inc) 外部串行式字体内存 (Font ROM)，可支持各样
		的文字写入到DDRAM 中。RA8875 与上海集通公司 (Genitop Inc) 兼容的产品包含 :
		GT21L16TW/GT21H16T1W 、GT23L16U2W 、GT23L24T3Y/GT23H24T3Y 、GT23L24M1Z 、
		及 GT23L32S4W/GT23H32S4W。这些字体包含16x16, 24x24, 32x32 点 (Dot) 与不同的字

		模块集成的字库芯片为 集通字库芯片_GT23l32S4W

		GT23L32S4W是一款内含11X12点阵、15X16点、24X24点阵、32X32点阵的汉字库芯片，支持GB2312
		国标汉字（含有国家信标委合法授权）及SCII字符。排列格式为横置横排。用户通过字符内码，利用本手
		册提供的方法计算出该字符点阵在芯片中的地址，可从该地址连续读出字符点阵信息。
	*/

	/* 设置文本显示位置，注意文本模式的写入光标和图形模式的写入光标是不同的寄存器 */
	RA8875_SetTextCursor(_usX, _usY);

	RA8875_WriteReg(0x40, (1 << 7));	/* 设置为文本模式 */

	/*
		Serial Flash/ROM 频率频率设定
			0xb: SFCL 频率 = 系统频率频率(当DMA 为致能状态，并且色彩深度为256 色，则SFCL 频率
				固定为=系统频率频率/ 2)
			10b: SFCL 频率 =系统频率频率/ 2
			11b: SFCL 频率 =系统频率频率/ 4

		模块系统频率为 68MHz

		GT23L32S4W的访问速度：SPI 时钟频率：20MHz(max.)

		因此需要设置 4 分频, 17MHz
	*/
	RA8875_WriteReg(0x06, (3 << 0));	/* 设置为文本模式 */

	/* 选择外部字体ROM */
	RA8875_WriteReg(0x21, (0 << 7) | (1 << 5));

	/* 05H  REG[05h] Serial Flash/ROM Configuration Register (SROC)
		7	Serial Flash/ROM I/F # 选择
				0:选择Serial Flash/ROM 0 接口。[模块的字库芯片接在 0 接口]
				1:选择Serial Flash/ROM 1 接口。
		6	Serial Flash/ROM 寻址模式
				0: 24 位寻址模式。
				此位必须设为0。
		5	Serial Flash/ROM 波形模式
				0: 波形模式 0。
				1: 波形模式 3。
		4-3	Serial Flash /ROM 读取周期 (Read Cycle)
			00b: 4 bus ?? 无空周期 (No Dummy Cycle)。
			01b: 5 bus ??1 byte 空周期。
			1Xb: 6 bus ??2 byte 空周期。
		2	Serial Flash /ROM 存取模式 (Access Mode)
			0: 字型模式 。
			1: DMA 模式。
		1-0	Serial Flash /ROM I/F Data Latch 选择模式
			0Xb: 单一模式。
			10b: 双倍模式0。
			11b: 双倍模式1。
	*/
	RA8875_WriteReg(0x05, (0 << 7) | (0 << 6) | (1 << 5) | (1 << 3) | (0 << 2) | (0 << 1));

	/*
		设置外部字体芯片型号为 GT23L32S4W, 编码为GB2312,

		Bit1:0 决定ASCII字符的格式：
			0 = NORMAL		 [笔画细, 和汉字顶部对齐]
			1 = Arial		 [笔画粗，和汉字底部对齐]
			2 = Roman		 [笔画细, 和汉字底部对齐]
			3 = Bold		 [乱码,不可用]
	 */
	RA8875_WriteReg(0x2F, (4 << 5) | (0 << 2) | (1 << 0));

	s_ucRA8875Busy = 1;
	RA8875_WriteCmd(0x02); 			/* 用于设定RA8875 进入内存(DDRAM或CGRAM)读取/写入模式 */

	/* 开始循环处理字符 */
	while (*_ptr != 0)
	{
		RA8875_WriteData(*_ptr);
		while ((RA8875_ReadStatus() & 0x80) == 0x80);
		_ptr++;
	}
	s_ucRA8875Busy = 0;

	RA8875_WriteReg(0x40, (0 << 7));	/* 还原为图形模式 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispStr
*	功能说明: 在LCD指定坐标（左上角）显示一个字符串
*	形    参：
*		_usX : X坐标，对于4.3寸宽屏，范围为【0 - 479】
*		_usY : Y坐标，对于4.3寸宽屏，范围为 【0 - 271】
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address;
	uint8_t buf[24 * 24 / 8];	/* 最大支持24点阵汉字 */
	uint8_t m, width;
	uint8_t font_width,font_height, font_bytes;
	uint16_t x, y;
	const uint8_t *pAscDot;
	const uint8_t *pHzDot;

	/* 如果字体结构为空指针，则缺省按16点阵 */
	if (_tFont->usFontCode == FC_ST_12)
	{
		font_height = 12;
		font_width = 12;
		font_bytes = 24;
		pAscDot = g_Ascii12;
		pHzDot = g_Hz12;
	}
	else
	{
		/* 缺省是16点阵 */
		font_height = 16;
		font_width = 16;
		font_bytes = 32;
		pAscDot = g_Ascii16;
		pHzDot = g_Hz16;
	}

	/* 开始循环处理字符 */
	while (*_ptr != 0)
	{
		code1 = *_ptr;	/* 读取字符串数据， 该数据可能是ascii代码，也可能汉字代码的高字节 */
		if (code1 < 0x80)
		{
			/* 将ascii字符点阵复制到buf */
			memcpy(buf, &pAscDot[code1 * (font_bytes / 2)], (font_bytes / 2));
			width = font_width / 2;
		}
		else
		{
			code2 = *++_ptr;
			if (code2 == 0)
			{
				break;
			}

			/* 计算16点阵汉字点阵地址
				ADDRESS = [(code1-0xa1) * 94 + (code2-0xa1)] * 32
				;
			*/
			#ifdef USE_SMALL_FONT
				m = 0;
				while(1)
				{
					address = m * (font_bytes + 2);
					m++;
					if ((code1 == pHzDot[address + 0]) && (code2 == pHzDot[address + 1]))
					{
						address += 2;
						memcpy(buf, &pHzDot[address], font_bytes);
						break;
					}
					else if ((pHzDot[address + 0] == 0xFF) && (pHzDot[address + 1] == 0xFF))
					{
						/* 字库搜索完毕，未找到，则填充全FF */
						memset(buf, 0xFF, font_bytes);
						break;
					}
				}
			#else	/* 用全字库 */
				/* 此处需要根据字库文件存放位置进行修改 */
				address = ((code1-0xa1) * 94 + (code2-0xa1)) * font_bytes + HZK16_ADDR;
				memcpy(buf, (const uint8_t *)address, font_bytes);
			#endif

				width = font_width;
		}

		y = _usY;
		/* 开始刷LCD */
		for (m = 0; m < font_height; m++)	/* 字符高度 */
		{
			x = _usX;
			for (i = 0; i < width; i++)	/* 字符宽度 */
			{
				if ((buf[m * ((2 * width) / font_width) + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
				{
					RA8875_PutPixel(x, y, _tFont->usTextColor);	/* 设置像素颜色为文字色 */
				}
				else
				{
					RA8875_PutPixel(x, y, _tFont->usBackColor);	/* 设置像素颜色为文字背景色 */
				}

				x++;
			}
			y++;
		}

		if (_tFont->usSpace > 0)
		{
			/* 如果文字底色按_tFont->usBackColor，并且字间距大于点阵的宽度，那么需要在文字之间填充(暂时未实现) */
		}
		_usX += width + _tFont->usSpace;	/* 列地址递增 */
		_ptr++;			/* 指向下一个字符 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。在窗口显示模式，连续写显存，光标会自动在设定窗口内进行递增
*	形    参：
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{

	uint16_t usTemp;

	/* 坐标系统示意图： （横屏）
			 -----------------------------
			|(0,0)                        |
			|     --------->              |
			|         |                   |
			|         |                   |
			|         |                   |
			|         V                   |
			|     --------->              |
			|                    (479,271)|
			 -----------------------------

		左上角是坐标零点, 扫描方向，先从左到右，再从上到下。

		如果需要做竖屏方式，你需要进行物理坐标和逻辑坐标的转换
	*/

	RA8875_WriteReg(0x30, _usX);
    RA8875_WriteReg(0x31, _usX >> 8);

	RA8875_WriteReg(0x32, _usY);
    RA8875_WriteReg(0x33, _usY >> 8);

	usTemp = _usWidth + _usX - 1;
	RA8875_WriteReg(0x34, usTemp);
    RA8875_WriteReg(0x35, usTemp >> 8);

	usTemp = _usHeight + _usY - 1;
	RA8875_WriteReg(0x36, usTemp);
    RA8875_WriteReg(0x37, usTemp >> 8);

	RA8875_SetCursor(_usX, _usY);

	/* 保存当前窗口信息，提高以后单色填充操作的效率.
	另外一种做法是通过读取0x30-0x37寄存器获得当前窗口，不过效率较低 */
	s_WinX = _usX;
	s_WinY = _usY;
	s_WinHeight = _usHeight;
	s_WinWidth = _usWidth;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetCursor
*	功能说明: 设置写显存的光标位置（图形模式）
*	形    参：_usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_SetCursor(uint16_t _usX, uint16_t _usY)
{
	/* 设置内存写光标的坐标 【注意0x80-83 是光标图形的坐标】 */
	RA8875_WriteReg(0x46, _usX);
	RA8875_WriteReg(0x47, _usX >> 8);
	RA8875_WriteReg(0x48, _usY);
	RA8875_WriteReg(0x49, _usY >> 8);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetCursor
*	功能说明: 设置读显存的光标位置。 很多其他的控制器写光标和读光标是相同的寄存器，但是RA8875是不同的。
*	形    参：_usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_SetReadCursor(uint16_t _usX, uint16_t _usY)
{
	/* 设置内存读光标的坐标 */
	RA8875_WriteReg(0x4A, _usX);
	RA8875_WriteReg(0x4B, _usX >> 8);
	RA8875_WriteReg(0x4C, _usY);
	RA8875_WriteReg(0x4D, _usY >> 8);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetTextCursor
*	功能说明: 设置文本写入光标位置
*	形    参：_usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_SetTextCursor(uint16_t _usX, uint16_t _usY)
{
	/* 设置内存读光标的坐标 */
	RA8875_WriteReg(0x2A, _usX);
	RA8875_WriteReg(0x2B, _usX >> 8);
	RA8875_WriteReg(0x2C, _usY);
	RA8875_WriteReg(0x2D, _usY >> 8);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_QuitWinMode
*	功能说明: 退出窗口显示模式，变为全屏显示模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_QuitWinMode(void)
{
	RA8875_SetDispWin(0, 0, g_LcdHeight, g_LcdWidth);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetBackLight
*	功能说明: 配置RA8875芯片的PWM1相关寄存器，控制LCD背光
*	形    参：_bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetBackLight(uint8_t _bright)
{
	if (_bright == 0)
	{
		/* 关闭PWM, PWM1引脚缺省输出LOW  */
		RA8875_WriteReg(0x8A, 0 << 6);
	}
	else if (_bright == BRIGHT_MAX)	/* 最大亮度 */
	{
		/* 关闭PWM, PWM1引脚缺省输出HIGH */
		RA8875_WriteReg(0x8A, 1 << 6);
	}
	else
	{
		/* 使能PWM1, 进行占空比调节 */

		/* 	REG[8Ah] PWM1 Control Register (P1CR)

			Bit7 脉波宽度调变 (PWM1) 设定
				0 : 关闭，此状态下，PWM1 输出准位依照此缓存器Bit6 决定。
				1 : 开启。

			Bit6 PWM1 关闭时的准位
				0 : 当PWM 关闭或于睡眠模式时，PWM1 输出为”Low” 状态。
				1 : 当PWM 关闭或于睡眠模式时，PWM1 输出为”High” 状态。

			Bit5 保留

			Bit4 PWM1 功能选择
				0 : PWM1 功能。
				1 : PWM1 固定输出一频率为外部晶体振荡器Clock (Fin) 频率 1 /16 的Clock

			Bit3-0  PWM1 电路的频率来源选择:
				0000b : SYS_CLK / 1 1000b : SYS_CLK / 256
				0001b : SYS_CLK / 2 1001b : SYS_CLK / 512
				0010b : SYS_CLK / 4 1010b : SYS_CLK / 1024
				0011b : SYS_CLK / 8 1011b : SYS_CLK / 2048
				0100b : SYS_CLK / 16 1100b : SYS_CLK / 4096
				0101b : SYS_CLK / 32 1101b : SYS_CLK / 8192
				0110b : SYS_CLK / 64 1110b : SYS_CLK / 16384
				0111b : SYS_CLK / 128 1111b : SYS_CLK / 32768

		*/

		RA8875_WriteReg(0x8A, (1 << 7) | 3);           //修改此句，可以修改背光PWM频率

		/* REG[8Bh] PWM1 Duty Cycle Register (P1DCR) */
		RA8875_WriteReg(0x8B, _bright);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawIcon
*	功能说明: 在LCD上绘制一个图标，四角自动切为弧脚
*	形    参：
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIcon(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* 用于记录窗口内的相对坐标 */

	p = _ptr;
	for (y = 0; y < _usHeight; y++)
	{
		for (x = 0; x < _usWidth; x++)
		{
			usNewRGB = *p++;	/* 读取图标的颜色值后指针加1 */
			/* 将图标的4个直角切割为弧角，弧角外是背景图标 */
			if ((y == 0 && (x < 6 || x > _usWidth - 7)) ||
				(y == 1 && (x < 4 || x > _usWidth - 5)) ||
				(y == 2 && (x < 3 || x > _usWidth - 4)) ||
				(y == 3 && (x < 2 || x > _usWidth - 3)) ||
				(y == 4 && (x < 1 || x > _usWidth - 2)) ||
				(y == 5 && (x < 1 || x > _usWidth - 2))	||

				(y == _usHeight - 1 && (x < 6 || x > _usWidth - 7)) ||
				(y == _usHeight - 2 && (x < 4 || x > _usWidth - 5)) ||
				(y == _usHeight - 3 && (x < 3 || x > _usWidth - 4)) ||
				(y == _usHeight - 4 && (x < 2 || x > _usWidth - 3)) ||
				(y == _usHeight - 5 && (x < 1 || x > _usWidth - 2)) ||
				(y == _usHeight - 6 && (x < 1 || x > _usWidth - 2))
				)
			{
				;
			}
			else
			{
				RA8875_PutPixel(x + _usX, y + _usY, usNewRGB);
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawIconActive
*	功能说明: 在LCD上绘制一个激活选中的图标
*	形    参：
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIconActive(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* 用于记录窗口内的相对坐标 */

	p = _ptr;
	for (y = 0; y < _usHeight; y++)
	{
		for (x = 0; x < _usWidth; x++)
		{
			usNewRGB = *p++;	/* 读取图标的颜色值后指针加1 */
			if ((y == 0 && (x < 6 || x > _usWidth - 7)) ||
				(y == 1 && (x < 4 || x > _usWidth - 5)) ||
				(y == 2 && (x < 3 || x > _usWidth - 4)) ||
				(y == 3 && (x < 2 || x > _usWidth - 3)) ||
				(y == 4 && (x < 1 || x > _usWidth - 2)) ||
				(y == 5 && (x < 1 || x > _usWidth - 2))	||

				(y == _usHeight - 1 && (x < 6 || x > _usWidth - 7)) ||
				(y == _usHeight - 2 && (x < 4 || x > _usWidth - 5)) ||
				(y == _usHeight - 3 && (x < 3 || x > _usWidth - 4)) ||
				(y == _usHeight - 4 && (x < 2 || x > _usWidth - 3)) ||
				(y == _usHeight - 5 && (x < 1 || x > _usWidth - 2)) ||
				(y == _usHeight - 6 && (x < 1 || x > _usWidth - 2))
				)
			{
				;
			}
			else
			{
				/* 降低原始像素的亮度，实现图标被激活选中的效果 */
				uint16_t R,G,B;
				uint16_t bright = 15;

				/* rrrr rggg gggb bbbb */
				R = (usNewRGB & 0xF800) >> 11;
				G = (usNewRGB & 0x07E0) >> 5;
				B =  usNewRGB & 0x001F;
				if (R > bright)
				{
					R -= bright;
				}
				else
				{
					R = 0;
				}
				if (G > 2 * bright)
				{
					G -= 2 * bright;
				}
				else
				{
					G = 0;
				}
				if (B > bright)
				{
					B -= bright;
				}
				else
				{
					B = 0;
				}

				RA8875_PutPixel(_usX + x, _usY + y, (R << 11) + (G << 5) + B);
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetBackLight
*	功能说明: 初始化控制LCD背景光的GPIO,配置为PWM模式。
*			当关闭背光时，将CPU IO设置为浮动输入模式（推荐设置为推挽输出，并驱动到低电平)；将TIM3关闭 省电
*	形    参：_bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
	
	s_ucBright =  _bright;	/* 保存背光值 */

	RA8875_SetBackLight(_bright);

}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetBackLight
*	功能说明: 获得背光亮度参数
*	形    参：无
*	返 回 值: 背光亮度参数
*********************************************************************************************************
*/
uint8_t LCD_GetBackLight(void)
{
	return s_ucBright;
}


/*
*********************************************************************************************************
*	函 数 名: LCD_CtrlLinesConfig
*	功能说明: 配置LCD控制口线，FSMC管脚设置为复用功能
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_CtrlLinesConfig(void)
{
	
/*******************液晶屏硬件连接******************************

PD14  -----FSMC_D0  ----D0
PD15  -----FSMC_D1  ----D1
PD0   -----FSMC_D2  ----D2
PD1   -----FSMC_D3  ----D3
PE7   -----FSMC_D4  ----D4
PE8   -----FSMC_D5  ----D5
PE9   -----FSMC_D6  ----D6
PE10  -----FSMC_D7  ----D7
PE11  -----FSMC_D8  ----D8
PE12  -----FSMC_D9  ----D9
PE13  -----FSMC_D10 ----D10
PE14  -----FSMC_D11 ----D11
PE15  -----FSMC_D12 ----D12
PD8   -----FSMC_D13 ----D13
PD9   -----FSMC_D14 ----D14
PD10  -----FSMC_D15 ----D15
PD4   -----FSMC_NOE ----RD
PD5   -----FSMC_NWE ----WR
PD7   -----FSMC_NE1 ----CS
PD11  -----FSMC_A16 ----RS
***************************************************************/

  GPIO_InitTypeDef GPIO_InitStructure;		     
	 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

 /* 配置基于FSMC的液晶屏的数据线
 * 液晶屏数据线：FSMC-D0~D15: PD 14 15 0 1,PE 7 8 9 10 11 12 13 14 15,PD 8 9 10
 */	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;										  //设置翻转速度为100Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;											      //设置管脚模式为复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | 
																GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);													  
																																		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
																GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
																GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
	
 /* 配置基于FSMC的液晶屏控制线
 * PD4-FSMC_NOE  :LCD-RD
 * PD5-FSMC_NWE  :LCD-WR
 * PD7-FSMC_NE3  :LCD-CS
 * PD11-FSMC_A16 :LCD-DC
 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
   
  /*配置GPIOD相应的管脚复用连接到FSMC*/
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FMC);	    //配置GPIOD的Pin0复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FMC);			//配置GPIOD的Pin1复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FMC);			//配置GPIOD的Pin4复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FMC);			//配置GPIOD的Pin5复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FMC);			//配置GPIOD的Pin7复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FMC);			//配置GPIOD的Pin8复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FMC);			//配置GPIOD的Pin9复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FMC);		//配置GPIOD的Pin10复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FMC);		//配置GPIOD的Pin11复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FMC);		//配置GPIOD的Pin14复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FMC);		//配置GPIOD的Pin15复用连接到FSMC
	
   /*配置GPIOE相应的管脚复用连接到FSMC*/
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FMC);			//配置GPIOE的Pin7复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FMC);			//配置GPIOE的Pin8复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FMC);			//配置GPIOE的Pin9复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FMC);		//配置GPIOE的Pin10复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FMC);		//配置GPIOE的Pin11复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FMC);		//配置GPIOE的Pin12复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FMC);	  //配置GPIOE的Pin13复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FMC);		//配置GPIOE的Pin14复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FMC);		//配置GPIOE的Pin15复用连接到FSMC	
}

/*
*********************************************************************************************************
*	函 数 名: LCD_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_FSMCConfig(void)
{
	FMC_NORSRAMInitTypeDef  init;
	FMC_NORSRAMTimingInitTypeDef  timing;	

	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);	
	
	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 1 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	timing.FMC_AddressSetupTime = 1;
	timing.FMC_AddressHoldTime = 0;
	timing.FMC_DataSetupTime = 2;
	timing.FMC_BusTurnAroundDuration = 0;
	timing.FMC_CLKDivision = 0;
	timing.FMC_DataLatency = 0;
	timing.FMC_AccessMode = FSMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FMC_Bank = FSMC_Bank1_NORSRAM1;
	init.FMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	init.FMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* 注意旧库无这个成员 */
	init.FMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FMC_WrapMode = FSMC_WrapMode_Disable;
	init.FMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FMC_WriteBurst = FSMC_WriteBurst_Disable;

	init.FMC_ReadWriteTimingStruct = &timing;
	init.FMC_WriteTimingStruct = &timing;

	FMC_NORSRAMInit(&init);

	/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */
	FMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);	
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetChipDescribe
*	功能说明: 读取LCD驱动芯片的描述符号，用于显示
*	形    参：char *_str : 描述符字符串填入此缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_GetChipDescribe(char *_str)
{

	strcpy(_str, CHIP_STR_8875);
	
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetHeight
*	功能说明: 读取LCD分辨率之高度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetHeight(void)
{
	return g_LcdHeight;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetWidth
*	功能说明: 读取LCD分辨率之宽度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetWidth(void)
{
	return g_LcdWidth;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_InitHard
*	功能说明: 初始化LCD
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	/* 配置LCD控制口线GPIO */
	LCD_CtrlLinesConfig();

	/* 配置FSMC接口，数据总线 */
	LCD_FSMCConfig();

	#ifdef IF_SPI_EN
		RA8875_InitSPI();
	#endif

	/* FSMC重置后必须加延迟才能访问总线设备  */
//	bsp_DelayMS(50);
	myLCD_Delay(20);

	RA8875_InitHard();	/* 初始化RA8875芯片 */

	RA8875_ClrScr(CL_BLACK);	/* 清屏，显示全黑 */

	RA8875_SetBackLight(BRIGHT_DEFAULT);	 /* 打开背光，设置为缺省亮度 */
}

#ifdef IF_SPI_EN	/* 四线SPI接口模式 */

/*
*********************************************************************************************************
*	函 数 名: RA8875_InitSPI
*	功能说明: 配置STM32和RA8875的SPI口线，使能硬件SPI1, 片选由软件控制
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_InitSPI(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	SPI_InitTypeDef   SPI_InitStructure;

	/* 打开 GPIOA 外设时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* 打开 SPI1 外设时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); 
	
	SPI_Cmd(SPI1,DISABLE);	

  GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); 			

	/* 配置 PA5、PA6、PA7 为复用推挽输出，用于 SCK, MISO and MOSI */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);	


	/* 配置 PA4 脚为推挽输出，用于 CS  */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;		
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	RA8875_CS_1();	
	

	/* 配置 SPI1工作模式 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   /* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;      /* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  /* 数据位长度 ： 8位 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 		/* 软件控制片选 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	/* 使能 SPI1 */
	SPI_Cmd(SPI1,ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_HighSpeedSPI
*	功能说明: 配置STM32的SPI速度为高速。  RA8875在内部PLL倍频电路未启动前，只能用低速SPI通信。
*			  初始化完成后，可以将SPI切换到高速模式，以提高刷屏效率。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_HighSpeedSPI(void)
{
	SPI_InitTypeDef   SPI_InitStructure;

	SPI_Cmd(SPI1,DISABLE);

	/* 配置 SPI1工作模式 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 		/* 软件控制片选 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	/* 使能 SPI1 */
	SPI_Cmd(SPI1,ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: SPI_ShiftByte
*	功能说明: 向SPI总线发送一个字节，同时返回接收到的字节
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t SPI_ShiftByte(uint8_t _ucByte)
{
	uint8_t ucRxByte;

	/* 等待发送缓冲区空 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* 发送一个字节 */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* 等待数据接收完毕 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取接收到的数据 */
	ucRxByte = SPI_I2S_ReceiveData(SPI1);

	/* 返回读到的数据 */
	return ucRxByte;	
}

#endif

