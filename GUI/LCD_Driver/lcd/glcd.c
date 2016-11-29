
#include "glcd.h"


static void RA8875_Delaly1us(void)
{
	uint8_t i;

	for (i = 0; i < 50; i++);	/* 延迟, 不准 */
}

static void RA8875_Delaly1ms(void)
{
	uint16_t i;

	for (i = 0; i < 5000; i++);	/* 延迟1ms, 不准 */
}

static void Delay(uint16_t nCount)
{
	uint32_t TimingDelay;
	
	while(nCount--)
	{
		for(TimingDelay=0;TimingDelay<100000;TimingDelay++);
	}
}

/**************************************************************************************************************
 * 函数名：GLCD_WriteReg()
 * 输入  ：uint8_t LCD_Reg 寄存器地址, uint16_t LCD_RegValue 寄存器的值
 * 输出  ：void
 * 描述  ：写LCD寄存器函数
 * 调用  ：外部调用        
 *****************************************************************************************************************/
void RA8875_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  LCD_REG = LCD_Reg;         /*写寄存器地址*/
  LCD_RAM = LCD_RegValue;    /*写入寄存器值*/
}

/**************************************************************************************************************
 * 函数名：LCD_Init()
 * 输入  ：void
 * 输出  ：void
 * 描述  ：LCD初始化函数
 * 调用  ：外部调用        
 *****************************************************************************************************************/
void GLCD_Init(void)
{ 
	/* 配置LCD控制口线GPIO */
	LCD_GPIO_Config();

	/* 配置FSMC接口，数据总线 */
	LCD_FSMC_Config();

	/* FSMC重置后必须加延迟才能访问总线设备  */
	Delay(200);

	
	/* 初始化PLL.  晶振频率为25M */
	LCD_WriteCmd(0x88);
	RA8875_Delaly1us();		/* 延迟1us */
	LCD_WriteData(10);	/* PLLDIVM [7] = 0 ;  PLLDIVN [4:0] = 10 */

	RA8875_Delaly1ms();

	LCD_WriteCmd(0x89);
	RA8875_Delaly1us();		/* 延迟1us */
	LCD_WriteData(2);	/* PLLDIVK[2:0] = 2, 除以4 */

	/* RA8875 的内部系统频率 (SYS_CLK) 是结合振荡电路及PLL 电路所产生，频率计算公式如下 :
		SYS_CLK = FIN * ( PLLDIVN [4:0] +1 ) / ((PLLDIVM+1 ) * ( 2^PLLDIVK [2:0] ))
						= 25M * (10 + 1) / ((0 + 1) * (2 ^ 2))
				= 68.75MHz
	*/

	/* REG[88h]或REG[89h]被设定后，为保证PLL 输出稳定，须等待一段「锁频时间」(< 100us)。*/
		RA8875_Delaly1ms();

	/*
		配置系统控制寄存器10h。 中文pdf 第18页:

		bit3:2 色彩深度设定 (Color Depth Setting)
			00b : 8-bpp 的通用TFT 接口， i.e. 256 色。
			1xb : 16-bpp 的通用TFT 接口， i.e. 65K 色。	 【选这个】

		bit1:0 MCUIF 选择
			00b : 8-位MCU 接口。
			1xb : 16-位MCU 接口。 【选这个】
	*/
	RA8875_WriteReg(0x10, 0x0A);	/* 配置16位MCU并口，65K色 */

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
	RA8875_Delaly1ms();
	
	RA8875_WriteReg(0x14,0x3B);           //水平设置:水平显示宽度(像素) = (HDWR + 1) * 8 = 480
	RA8875_WriteReg(0x15,0x00);           //水平非显示周期微调宽度设置 (HNDFTR)
	RA8875_WriteReg(0x16,0x01);           //水平非显示宽度(像素) = (HNDR + 1) * 8
	RA8875_WriteReg(0x17,0x00);           //水平同步起始地址宽度 = (HSTR + 1) * 8
	RA8875_WriteReg(0x18, 0x05);          //水平同步信号脉波宽度(像素) = (HPWR + 1) * 8

	RA8875_WriteReg(0x19, 0x0F);          //垂直显示高度低8位 [0-7] （VDHR0） 
	RA8875_WriteReg(0x1A, 0x01);          //垂直显示高度(像素) = VDHR + 1 = 0x010F + 1 = 272
	RA8875_WriteReg(0x1B, 0x02);          //垂直非显示周期位[7:0]（VNDR0） 
	RA8875_WriteReg(0x1C, 0x00);          //垂直非显示周期位[8] （VNDR1）
	RA8875_WriteReg(0x1D, 0x07);          //垂直同步信号起始地址高度位[7:0]  VSTR0 
	RA8875_WriteReg(0x1E, 0x00);          //垂直同步信号起始地址高度位[8]  VSTR1  
	RA8875_WriteReg(0x1F, 0x09);          //垂直同步信号脉波宽度 VPWR
	
	RA8875_WriteReg(0x20, 0x08);          //显示配置寄存器（DPCR）
	RA8875_Delaly1ms();		
	
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

	LCD_SetDispWin(0, 0, 272, 480);
	
    /* 清除显存 */
	LCD_ClrScr(CL_BLACK);	  /* 黑色 */
	
	LCD_SetBackLight(255);	
}

/*
*********************************************************************************************************
*	函 数 名: LCD_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参：_usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
  uint32_t i;
	
	#if 0	/* 这个分支的代码，启动了RA8875的硬件单色填充功能，速度较快 */
	
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
		BTE_SetTarBlock(0, 0, s_LcdHeight, s_LcdWidth, 0);	/* 设置BTE位置和宽度高度 */
		BTE_SetOperateCode(0x0C);	/* 设定BTE 操作码和光栅运算码  REG[51h] Bit[3:0] = 0Ch */
		BTE_SetFrontColor(_usColor);	/* 设置BTE前景色 */
		BTE_Start();				/* 开启BTE 功能 */
		BTE_Wait();					/* 等待操作结束 */
	
	#else 	/* 下面的代码是按普通描点方式清屏，速度较慢 */
	
		LCD_SetCursor(0, 0);	/* 设置光标位置 */
	
		LCD_REG = 0x02; 	/* 用于设定RA8875 进入内存(DDRAM或CGRAM)读取/写入模式 */
	
		for (i = 0; i < 272 * 480; i++)
		{
			LCD_RAM = _usColor;
			while ((LCD_REG & 0x80 ) == 0x80);	//测试无需判忙，未发现问题。
		}
	
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。TFT驱动芯片支持窗口显示模式，这是和一般的12864点阵LCD的最大区别。
*	形    参：
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
	uint16_t usTemp;

	//setting active window X
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

	LCD_SetCursor(_usX, _usY);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetCursor
*	功能说明: 设置光标位置
*	形    参：_usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetCursor(uint16_t _usX, uint16_t _usY)
{
	/* 设置内存写光标的坐标 【0x80-83 是光标图形的坐标】 */
	RA8875_WriteReg(0x46, _usX);
	RA8875_WriteReg(0x47, _usX >> 8);
	RA8875_WriteReg(0x48, _usY);
	RA8875_WriteReg(0x49, _usY >> 8);
}

void GLCD_WaitStatus(void)
{
  while((LCD_REG & 0x80) == 0x80);
}

/**************************************************************************************************************
 * 函数名：LCD_SetTextColor()
 * 输入  ：uint16_t Color 前景色
 * 输出  ：void
 * 描述  ：设置LCD的前景色
 * 调用  ：外部调用        
 *****************************************************************************************************************/
void GLCD_SetTextColor(__IO uint16_t Color)
{
	LCD_WriteCmd(0x63);//BGCR0
	LCD_WriteData((Color&0x001f));
	
	LCD_WriteCmd(0x64);//BGCR0
	LCD_WriteData((Color&0x07e0)>>5);
	
	LCD_WriteCmd(0x65);//BGCR0
	LCD_WriteData((Color&0xf800)>>11);
}

/**************************************************************************************************************
 * 函数名：LCD_SetBackColor()
 * 输入  ：uint16_t Color 背景色
 * 输出  ：void
 * 描述  ：设置LCD的背景色
 * 调用  ：外部调用        
 *****************************************************************************************************************/
void GLCD_SetBackColor(__IO uint16_t Color)
{
  LCD_WriteCmd(0x60);//BGCR0
	LCD_WriteData((Color&0x001f));

	LCD_WriteCmd(0x61);//BGCR0
	LCD_WriteData((Color&0x07e0)>>5);
	
	LCD_WriteCmd(0x62);//BGCR0
	LCD_WriteData((Color&0xf800)>>11);	
	GLCD_WaitStatus();
}

/**************************************************************************************************************
 * 函数名：LCD_GPIO_Config()
 * 输入  ：void
 * 输出  ：void
 * 描述  ：配置LCD的GPIO管脚与FSMC连接
 * 调用  ：内部调用        
 *****************************************************************************************************************/
void LCD_GPIO_Config(void)
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
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);	    //配置GPIOD的Pin0复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);			//配置GPIOD的Pin1复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);			//配置GPIOD的Pin4复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);			//配置GPIOD的Pin5复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FSMC);			//配置GPIOD的Pin7复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);			//配置GPIOD的Pin8复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);			//配置GPIOD的Pin9复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);		//配置GPIOD的Pin10复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);		//配置GPIOD的Pin11复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);		//配置GPIOD的Pin14复用连接到FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);		//配置GPIOD的Pin15复用连接到FSMC
	
   /*配置GPIOE相应的管脚复用连接到FSMC*/
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);			//配置GPIOE的Pin7复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);			//配置GPIOE的Pin8复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);			//配置GPIOE的Pin9复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);		//配置GPIOE的Pin10复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);		//配置GPIOE的Pin11复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);		//配置GPIOE的Pin12复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);	  //配置GPIOE的Pin13复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);		//配置GPIOE的Pin14复用连接到FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);		//配置GPIOE的Pin15复用连接到FSMC	
}

/**************************************************************************************************************
 * 函数名：LCD_FSMC_Config()
 * 输入  ：void
 * 输出  ：void
 * 描述  ：配置LCD的FSMC
 * 调用  ：内部调用        
 *****************************************************************************************************************/
void LCD_FSMC_Config(void)
{    
	FSMC_NORSRAMInitTypeDef  init;
	FSMC_NORSRAMTimingInitTypeDef  timing;	

	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);	
	
	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 1 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	timing.FSMC_AddressSetupTime = 1;
	timing.FSMC_AddressHoldTime = 0;
	timing.FSMC_DataSetupTime = 2;
	timing.FSMC_BusTurnAroundDuration = 0;
	timing.FSMC_CLKDivision = 0;
	timing.FSMC_DataLatency = 0;
	timing.FSMC_AccessMode = FSMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* 注意旧库无这个成员 */
	init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

	init.FSMC_ReadWriteTimingStruct = &timing;
	init.FSMC_WriteTimingStruct = &timing;

	FSMC_NORSRAMInit(&init);

	/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);	
}

/*
*********************************************************************************************************
*	函 数 名: SetBackLight_by8875
*	功能说明: 配置RA8875芯片的PWM1相关寄存器，控制LCD背光
*	形    参：_bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
static void SetBackLight_by8875(uint8_t _bright)
{
	if (_bright == 0)
	{
		/* 关闭PWM, PWM1引脚缺省输出LOW
		备注：如果直接将占空比寄存器P1DCR设置为0，实际输出的PWM波形还是存在很窄的脉冲，因此亮度0值单独
		处理，这样的话，关闭PWM也可以起到省电的目的。
		 */
		RA8875_WriteReg(0x8A, 0);
	}
	else if (_bright == 255)	/* 最大亮度 */
	{
		/* 关闭PWM, PWM1引脚缺省输出HIGH */
		/* 实际测量，不对255最大亮度做特殊处理，PWM波形也能够输出恒定高电平 */
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

		RA8875_WriteReg(0x8A, (1 << 7) | 4);      //修改此句，可以修改背光PWM频率

		/* REG[8Bh] PWM1 Duty Cycle Register (P1DCR) */

		RA8875_WriteReg(0x8B, _bright);
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
  SetBackLight_by8875(_bright);
}

