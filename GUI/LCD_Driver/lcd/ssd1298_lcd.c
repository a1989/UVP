
#include "ssd1298_lcd.h"
#include "font.h"
#include "scarausart.h"
//#define LCD_RAM    *(__IO uint16_t *) (0x6C000002)    	//地址寄存器
//#define LCD_REG    *(__IO uint16_t *) (0x6C000000)	 	//指令寄存器

//#define rw_data_prepare()               write_cmd(34)
#define rw_data_prepare()               write_cmd(0x22)

void write_cmd(unsigned short cmd);
unsigned short read_data(void);
unsigned short DeviceCode;

static void delay(int cnt)
{
	volatile unsigned int dl;
	while(cnt--)
	{
		for(dl=0; dl<500; dl++);
	}
}

/*
 *	
 */
void write_cmd(unsigned short cmd)
{	
	LCD_REG = cmd;	
}

unsigned short read_data(void)
{
	unsigned short temp;	
	temp = LCD_RAM;
	temp = LCD_RAM;
	return temp;
}

void write_data(unsigned short data_code )
{	
	LCD_RAM = data_code;
}

void write_reg(unsigned char reg_addr,unsigned short reg_val)
{
	write_cmd(reg_addr);
	write_data(reg_val);
}

unsigned short read_reg(unsigned char reg_addr)
{
	unsigned short val=0;
	write_cmd(reg_addr);
	val = read_data();
	return (val);
}


void lcd_SetCursor(unsigned int x,unsigned int y)
{
	write_reg(0x004e,x);    /* 0-239 */
	write_reg(0x004f,y);    /* 0-319 */
}
/* 读取指定地址的GRAM */
unsigned short lcd_read_gram(unsigned int x,unsigned int y)
{
	unsigned short temp;
	lcd_SetCursor(x,y);
	rw_data_prepare();
	/* dummy read */
	temp = read_data();
	temp = read_data();
	return temp;
}
static void lcd_data_bus_test(void)
{
	unsigned short temp1;
	unsigned short temp2;
	/* wirte */
	lcd_SetCursor(0,0);
	rw_data_prepare();
	write_data(0x5555);

	lcd_SetCursor(1,0);
	rw_data_prepare();
	write_data(0xAAAA);

	/* read */
	lcd_SetCursor(0,0);
	temp1 = lcd_read_gram(0,0);
	temp2 = lcd_read_gram(1,0);

	if( (temp1 == 0x5555) && (temp2 == 0xAAAA) )
	{
		//printf(" data bus test pass!\r\n");
	}
	else
	{
		//printf(" data bus test error: %04X %04X\r\n",temp1,temp2);
	}
}

void lcd_clear(unsigned short Color)
{
	unsigned int index=0;
	lcd_SetCursor(0,0);
	rw_data_prepare();                      /* Prepare to write GRAM */
	for (index=0; index<(LCD_WIDTH*LCD_HEIGHT); index++)
	{
		write_data(Color);
	}
}

void port_fsmc_init(void)
{	
	FMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	//FSMC_NORSRAMTimingInitTypeDef  p;
	GPIO_InitTypeDef GPIO_InitStructure;

	
	FMC_NORSRAMTimingInitTypeDef  Timing_read,Timing_write;

	FSMC_NORSRAMInitStructure.FMC_ReadWriteTimingStruct = &Timing_read;
	FSMC_NORSRAMInitStructure.FMC_WriteTimingStruct = &Timing_write;

	FMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);

	/*--------------------- read timings configuration ---------------------*/
	Timing_read.FMC_AddressSetupTime = 0;  /* [3:0] F2/F4 1~15 HCLK */
	Timing_read.FMC_AddressHoldTime = 0;   /* [7:4] keep 0x00 in SRAM mode */
	Timing_read.FMC_DataSetupTime = 3;     /* [15:8] F2/F4 0~255 HCLK */
	/* [19:16] Time between NEx high to NEx low (BUSTURN HCLK) */
	Timing_read.FMC_BusTurnAroundDuration = 0;
	Timing_read.FMC_CLKDivision = 0; /* [24:20] keep 0x00 in SRAM mode  */
	Timing_read.FMC_DataLatency = 0; /* [27:25] keep 0x00 in SRAM mode  */
	Timing_read.FMC_AccessMode = FSMC_AccessMode_A;

	/*--------------------- write timings configuration ---------------------*/
	Timing_write.FMC_AddressSetupTime = 0;  /* [3:0] F2/F4 1~15 HCLK */
	Timing_write.FMC_AddressHoldTime = 0;   /* [7:4] keep 0x00 in SRAM mode */
	Timing_write.FMC_DataSetupTime = 0;     /* [15:8] F2/F4 0~255 HCLK */
	/* [19:16] Time between NEx high to NEx low (BUSTURN HCLK) */
	Timing_write.FMC_BusTurnAroundDuration = 0;
	Timing_write.FMC_CLKDivision = 0; /* [24:20] keep 0x00 in SRAM mode  */
	Timing_write.FMC_DataLatency = 0; /* [27:25] keep 0x00 in SRAM mode  */
	Timing_write.FMC_AccessMode = FSMC_AccessMode_A;

	/* Enable GPIOs clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);
	/* Enable FSMC clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);	
	/* Configure the BL pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);	
	

	GPIO_ResetBits(GPIOF,GPIO_Pin_9);

	/*
	+-------------------+--------------------+------------------+------------------+
	+                       SRAM pins assignment                                  +
	+-------------------+--------------------+------------------+------------------+
	| PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0  <-> FSMC_A0 | PG0 <-> FSMC_A10 |
	| PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1  <-> FSMC_A1 | PG1 <-> FSMC_A11 |
	| PD4  <-> FSMC_NOE | PE7  <-> FSMC_D4   | PF2  <-> FSMC_A2 | PG2 <-> FSMC_A12 |
	| PD5  <-> FSMC_NWE | PE8  <-> FSMC_D5   | PF3  <-> FSMC_A3 | PG3 <-> FSMC_A13 |
	| PD8  <-> FSMC_D13 | PE9  <-> FSMC_D6   | PF4  <-> FSMC_A4 | PG4 <-> FSMC_A14 |
	| PD9  <-> FSMC_D14 | PE10 <-> FSMC_D7   | PF5  <-> FSMC_A5 | PG5 <-> FSMC_A15 |
	| PD10 <-> FSMC_D15 | PE11 <-> FSMC_D8   | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
	| PD11 <-> FSMC_A16 | PE12 <-> FSMC_D9   | PF13 <-> FSMC_A7 |------------------+
	| PD12 <-> FSMC_A17 | PE13 <-> FSMC_D10  | PF14 <-> FSMC_A8 | 
	| PD14 <-> FSMC_D0  | PE14 <-> FSMC_D11  | PF15 <-> FSMC_A9 | 
	| PD15 <-> FSMC_D1  | PE15 <-> FSMC_D12  |------------------+
	+-------------------+--------------------+
	*/
	/*
	+-------------------+--------------------+------------------+-------------------+
	+                       STM32 FSMC pins assignment                              +
	+-------------------+--------------------+------------------+-------------------+
	| PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0  <-> FSMC_A0 | PG0  <-> FSMC_A10 |
	| PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1  <-> FSMC_A1 | PG1  <-> FSMC_A11 |
	| PD4  <-> FSMC_NOE | PE3  <-> FSMC_A19  | PF2  <-> FSMC_A2 | PG2  <-> FSMC_A12 |
	| PD5  <-> FSMC_NWE | PE4  <-> FSMC_A20  | PF3  <-> FSMC_A3 | PG3  <-> FSMC_A13 |
	| PD8  <-> FSMC_D13 | PE7  <-> FSMC_D4   | PF4  <-> FSMC_A4 | PG4  <-> FSMC_A14 |
	| PD9  <-> FSMC_D14 | PE8  <-> FSMC_D5   | PF5  <-> FSMC_A5 | PG5  <-> FSMC_A15 |
	| PD10 <-> FSMC_D15 | PE9  <-> FSMC_D6   | PF12 <-> FSMC_A6 | PG9  <-> FSMC_NE2 |
	| PD11 <-> FSMC_A16 | PE10 <-> FSMC_D7   | PF13 <-> FSMC_A7 | PG12 <-> FSMC_NE4 |
	| PD12 <-> FSMC_A17 | PE11 <-> FSMC_D8   | PF14 <-> FSMC_A8 |-------------------+
	| PD13 <-> FSMC_A18 | PE12 <-> FSMC_D9   | PF15 <-> FSMC_A9 |
	| PD14 <-> FSMC_D0  | PE13 <-> FSMC_D10  |------------------+
	| PD15 <-> FSMC_D1  | PE14 <-> FSMC_D11  |
	| PD7  <-> FSMC_NE1 | PE15 <-> FSMC_D12  |
	+-------------------+--------------------+
	*/
	
	/* GPIOD configuration */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

	/* GPIOE configuration */
	//GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOE, GPIO_PinSource3 , GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOE, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

	/* GPIOF configuration */
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
	/*GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FSMC);*/

	/* GPIOG configuration */
	/*GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource3 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource9 , GPIO_AF_FSMC);*/
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource12 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4
		| GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
		| GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
		/*| GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13*/
		| GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = /*GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_3 |*/
		GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 |
		GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
		GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  /*| GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
		GPIO_Pin_4  | GPIO_Pin_5  |
		GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15*/;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =/* GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
		GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_9 |*/ GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	

	FSMC_NORSRAMInitStructure.FMC_Bank = FSMC_Bank1_NORSRAM4;
	FSMC_NORSRAMInitStructure.FMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
	FSMC_NORSRAMInitStructure.FMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FMC_ReadWriteTimingStruct = &Timing_read;
	FSMC_NORSRAMInitStructure.FMC_WriteTimingStruct = &Timing_write;

	FMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

	/*!< Enable FSMC Bank1_SRAM1 Bank */
	FMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);	
}

void SCARA_lcd_init(void)
{
	port_fsmc_init();
	//GPIO_ResetBits(GPIOD,GPIO_Pin_3);	
	//delay(500);
	//GPIO_SetBits(GPIOD,GPIO_Pin_3);	
	//delay(50);
	GPIO_SetBits(GPIOF,GPIO_Pin_9);
	delay(100);
	DeviceCode = read_reg(0x0000);
    DeviceCode = 0x8999;
	if (DeviceCode == 0x8999)
	{
		// power supply setting
		// set R07h at 0021h (GON=1,DTE=0,D[1:0]=01)
		write_reg(0x07,0x0021);
		// set R00h at 0001h (OSCEN=1)
		write_reg(0x00,0x0001);
		// set R07h at 0023h (GON=1,DTE=0,D[1:0]=11)
		write_reg(0x07,0x0023);
		// set R10h at 0000h (Exit sleep mode)
		write_reg(0x10,0x0000);
		// Wait 30ms
		delay(3000);
		// set R07h at 0033h (GON=1,DTE=1,D[1:0]=11)
		write_reg(0x07,0x0033);
		// Entry mode setting (R11h)
		// R11H Entry mode
		// vsmode DFM1 DFM0 TRANS OEDef WMode DMode1 DMode0 TY1 TY0 ID1 ID0 AM LG2 LG2 LG0
		//   0     1    1     0     0     0     0      0     0   1   1   1  *   0   0   0
		write_reg(0x11,0x6070);
		// LCD driver AC setting (R02h)
		write_reg(0x02,0x0600);
		// power control 1
		// DCT3 DCT2 DCT1 DCT0 BT2 BT1 BT0 0 DC3 DC2 DC1 DC0 AP2 AP1 AP0 0
		// 1     0    1    0    1   0   0  0  1   0   1   0   0   1   0  0
		// DCT[3:0] fosc/4 BT[2:0]  DC{3:0] fosc/4
		write_reg(0x03,0x0804);//0xA8A4
		write_reg(0x0C,0x0000);//
		write_reg(0x0D,0x080C);//
		// power control 4
		// 0 0 VCOMG VDV4 VDV3 VDV2 VDV1 VDV0 0 0 0 0 0 0 0 0
		// 0 0   1    0    1    0    1    1   0 0 0 0 0 0 0 0
		write_reg(0x0E,0x2900);
		write_reg(0x1E,0x00B8);
		write_reg(0x01,0x3B3F);//驱动输出控制320*240  0x6B3F
		write_reg(0x10,0x0000);
		write_reg(0x05,0x0000);
		write_reg(0x06,0x0000);
		write_reg(0x16,0xEF1C);
		write_reg(0x17,0x0003);
		write_reg(0x07,0x0233);//0x0233
		write_reg(0x0B,0x0000|(3<<6));
		write_reg(0x0F,0x0000);//扫描开始地址
		write_reg(0x41,0x0000);
		write_reg(0x42,0x0000);
		write_reg(0x48,0x0000);
		write_reg(0x49,0x013F);
		write_reg(0x4A,0x0000);
		write_reg(0x4B,0x0000);
		write_reg(0x44,0xEF00);
		write_reg(0x45,0x0000);
		write_reg(0x46,0x013F);
		write_reg(0x30,0x0707);
		write_reg(0x31,0x0204);
		write_reg(0x32,0x0204);
		write_reg(0x33,0x0502);
		write_reg(0x34,0x0507);
		write_reg(0x35,0x0204);
		write_reg(0x36,0x0204);
		write_reg(0x37,0x0502);
		write_reg(0x3A,0x0302);
		write_reg(0x3B,0x0302);
		write_reg(0x23,0x0000);
		write_reg(0x24,0x0000);
		write_reg(0x25,0x8000);   // 65hz
		write_reg(0x4f,0);        // 行首址0
		write_reg(0x4e,0);        // 列首址0	

	}	
	else
	{
		//printf("LCD model is not recognized,DeviceCode = 0x%x!\r\n",DeviceCode);
		//return;
	}
	//数据总线测试,用于测试硬件连接是否正常.
	lcd_data_bus_test();
	lcd_clear(Red);	
}

void ILI9325_Init(void)
{
		port_fsmc_init();
		//GPIO_ResetBits(GPIOD,GPIO_Pin_3);	
		//delay(500);
		//GPIO_SetBits(GPIOD,GPIO_Pin_3);	
		//delay(50);
		GPIO_SetBits(GPIOF,GPIO_Pin_9);
		delay(100);
		DeviceCode = read_reg(0x0000);
//		DeviceCode = 0x9325;
		USART_Put_I2A(DeviceCode);
	if(DeviceCode==0x9325)			  	//对应屏幕为2.8寸初始化
  {
	  delay(5); /* delay 50 ms */
	  delay(5); /* delay 50 ms */
	  // Start Initial Sequence
	  write_reg(0x00FF,0x0001);
	  write_reg(0x00F3,0x0008);
	  write_reg(0x0001,0x0100);
	  write_reg(0x0002,0x0700);
	  write_reg(0x0003,0x1030);  //0x1030
	  write_reg(0x0008,0x0302);
	  write_reg(0x0008,0x0207);
	  write_reg(0x0009,0x0000);
	  write_reg(0x000A,0x0000);
	  write_reg(0x0010,0x0000);  //0x0790
	  write_reg(0x0011,0x0005);
	  write_reg(0x0012,0x0000);
	  write_reg(0x0013,0x0000);
	  delay(5);
	  write_reg(0x0010,0x12B0);
	  delay(5);
	  write_reg(0x0011,0x0007);
	  delay(5);
	  write_reg(0x0012,0x008B);
	  delay(5);
	  write_reg(0x0013,0x1700);
	  delay(5);
	  write_reg(0x0029,0x0022);
			
	  //################# void Gamma_Set(void) ####################//
	  write_reg(0x0030,0x0000);
	  write_reg(0x0031,0x0707);
	  write_reg(0x0032,0x0505);
	  write_reg(0x0035,0x0107);
	  write_reg(0x0036,0x0008);
	  write_reg(0x0037,0x0000);
	  write_reg(0x0038,0x0202);
	  write_reg(0x0039,0x0106);
	  write_reg(0x003C,0x0202);
	  write_reg(0x003D,0x0408);
	  delay(5);
			
	  write_reg(0x0050,0x0000);		
	  write_reg(0x0051,0x00EF);		
	  write_reg(0x0052,0x0000);		
	  write_reg(0x0053,0x013F);		
	  write_reg(0x0060,0x2700);		
	  write_reg(0x0061,0x0001);
	  write_reg(0x0090,0x0033);				
	  write_reg(0x002B,0x000B);		
	  write_reg(0x0007,0x0133);
	  delay(5); 
	}
}

void LCD_WR_REG(uint16_t LCD_Reg)
{
	/* Write 16-bit Index, then Write Reg */
	LCD_REG = LCD_Reg;	  
}
/**
	*名称：
	*参数：
	*返回：
	*功能：
	*备注：开始写GRAM
**/
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0x22);
}
void LCD_WR_DATA(uint16_t LCD_Data)	
{
	/* Write 16-bit Reg */
	LCD_RAM = LCD_Data;
} 
void LCD_WriteReg(uint16_t LCD_Reg ,uint16_t LCD_RegValue)
{
	/* Write 16-bit Index, then Write Reg */
	LCD_REG = LCD_Reg;
	/* Write 16-bit Reg */
	LCD_RAM = LCD_RegValue;	
}

/**
	*名称：void LCD_SetCursor(u16 Xpos, u16 Ypos)
	*参数：Xpos 横坐标
		   Ypos 纵坐标
	*返回：无
	*功能：设置光标位置
**/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	write_reg(0x004E, Xpos);
	write_reg(0X004F, Ypos);
//		write_reg(0x0020, Xpos);
//		write_reg(0x0021, Ypos);
}
/**					
	*名称：void LCD_WindowMax()
	*参数：
	*返回：无
	*功能：设置窗口
	*备注：
**/
void LCD_WindowMax (unsigned int x,unsigned int y,unsigned int x_end,unsigned int y_end) 
{
	write_reg(0x44,x|((x_end-1)<<8));
	write_reg(0x45,y);
	write_reg(0x46,y_end-1);
}
/**
	*名称：void LCD_DrawPoint(void)
	*参数：xsta X起始坐标 0~239
		   ysta	Y起始坐标 0~319
	*返回：无
	*功能：POINT_COLOR指定点的颜色

**/
void LCD_DrawPoint(uint16_t xsta, uint16_t ysta, uint16_t PointColor)
{
	LCD_SetCursor(xsta,ysta);  /*设置光标位置  */
	LCD_WR_REG(0x22);           /*开始写入GRAM */
	LCD_WR_DATA(PointColor); 
}
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t Color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 											/*计算坐标增量		*/ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 									/*设置单步方向		*/ 
	else if(delta_x==0)incx=0;								/*垂直线 	  		*/
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;								/*水平线 	  		*/
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; 					/*选取基本增量坐标轴*/ 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )								/*画线输出 			*/
	{  
		//LCD_DrawPoint(uRow,uCol);							/*画点 				*/
        LCD_DrawPoint(uRow,uCol, Color);
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}
/**
	*名称: u16 findHzIndex(u8 *hz)
	*参数：hz
	*功能：索引汉字存储的内存地址
	*备注：
**/
u16 findHzIndex(u8 *hz)                            /* 在自定义汉字库在查找所要显示 */
                                                      /* 的汉字的位置 */
{
	u16 i=0;
	FNT_GB16 *ptGb16 = (FNT_GB16 *)GBHZ_16;		  /*ptGb16指向GBHZ_16*/
	while(ptGb16[i].Index[0] > 0x80)
	{
	    if ((*hz == ptGb16[i].Index[0]) && (*(hz+1) == ptGb16[i].Index[1])) /*汉字用两位来表示地址码*/
		{
	        return i;
	    }
	    i++;
	    if(i > (sizeof((FNT_GB16 *)GBHZ_16) / sizeof(FNT_GB16) - 1))  /* 搜索下标约束 */
	    {
		    break;
	    }
	}
	return 0;
}
/**
	*名称：
	*参数：
	*返回：
	*功能：LCD写GRAM
	*备注：
**/
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code);    /*写十六位GRAM*/
}
/**
	*名称：void WriteOneHz(uint16_t x0, uint16_t y0, uint8_t *pucMsk, uint16_t PenColor, uint16_t BackColor)
	*参数：x0,y0     起始坐标
		   *pucMsk   指向
		   PenColor	 字符颜色
		   BackColor 背景颜色
	*功能：
	*备注：此函数不能单独作为汉字字符显示											  
**/					
void WriteOneHz(u16 x0, u16 y0, u8 *pucMsk, u16 PenColor, u16 BackColor)
{
    u16 i,j;
    u16 mod[16];                                      /* 当前字模 16*16 */
    u16 *pusMsk;                                      /* 当前字库地址  */
    u16 y;

	u16 size = 16;       /*汉字默认大小16*16*/

    pusMsk = (u16 *)pucMsk;


    for(i=0; i<16; i++)                                    /* 保存当前汉字点阵式字模       */
    {
        mod[i] = *pusMsk;                                /* 取得当前字模，半字对齐访问   */
        mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* 字模交换高低字节*/
		pusMsk = pusMsk+1;
    }
    y = y0;
	LCD_WindowMax(x0,y0,x0+size,y0+size);	 	/*设置窗口*/
	LCD_SetCursor(x0,y0);                       /*设置光标位置 */ 
	LCD_WriteRAM_Prepare();                     /*开始写入GRAM*/  
    for(i=0; i<16; i++)                                    /* 16行   */
    {                                              
        for(j=0; j<16; j++)                                /* 16列   */
        {
		    if((mod[i] << j) & 0x8000)       /* 显示第i行 共16个点 */
            { 
			    LCD_WriteRAM(PenColor);
            } 
			else 
			{
                LCD_WriteRAM(BackColor);      /* 用读方式跳过写空白点的像素*/
			}
        }
        y++;
    }
	LCD_WindowMax(0x0000,0x0000,240,320);  	/*恢复窗体大小*/
}

/**
	*名称：void LCD_ShowChar(u8 x, u16 y, u8 num, u8 size, u16 PenColor, u16 BackColor)
	*参数：x，y      起始坐标（x:0~234 y:0~308）
		   num       字符ASCII码值
		   size      字符大小，使用默认8*16
		   PenColor  字体颜色
		   BackColor 字体背景颜色
	*功能：
	*备注：注意屏幕大小
**/
void LCD_ShowChar(u8 x, u16 y, u8 num, u8 size, u16 PenColor, u16 BackColor)
{       
#define MAX_CHAR_POSX 232
#define MAX_CHAR_POSY 304 
    u8 temp;
    u8 pos,t;
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY)return;		    
	LCD_WindowMax(x,y,x+size/2,y+size);	   /*设置窗口	*/										
	LCD_SetCursor(x, y);                  /*设置光标位置 */
  
	LCD_WriteRAM_Prepare();               /*开始写入GRAM  */   
	num=num-' ';                         /*得到偏移后的值 */
	for(pos=0;pos<size;pos++)
	{
		if(size==12)
			temp=asc2_1206[num][pos];/*调用1206字体*/
		else 
			temp=asc2_1608[num][pos];		 /*调用1608字体	*/
		for(t=0;t<size/2;t++)
	    {                 
	        if(temp&0x01)			   /*从低位开始*/
			{
				LCD_WR_DATA(PenColor);  /*画字体颜色 一个点*/
			}
			else 
				LCD_WR_DATA(BackColor);	   /*画背景颜色 一个点*/     
	        temp>>=1; 
	    }
	}			
	LCD_WindowMax(0x0000,0x0000,240,320);	/*恢复窗体大小*/	 
} 
/**
	*名称：void LCD_ShowHzString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor)
	*参数：x0，y0    起始坐标
		   pcStr     指向
		   PenColor  字体颜色
		   BackColor 字体背景
	*功能：显示汉字字符串
	*备注：这个函数不能单独调用	       
**/
void LCD_ShowHzString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor)
{
#define MAX_HZ_POSX 224
#define MAX_HZ_POSY 304 
	u16 usIndex;
	u8 size = 16; 
	FNT_GB16 *ptGb16 = 0;    
    ptGb16 = (FNT_GB16 *)GBHZ_16; 

	if(x0>MAX_HZ_POSX){x0=0;y0+=size;}			         /*超出X轴字体最小单位，换行*/
    if(y0>MAX_HZ_POSY){y0=x0=0;lcd_clear(White);}	   /*超出Y轴字体最小单位，回到原点，并且清屏*/

	usIndex = findHzIndex(pcStr);
	WriteOneHz(x0, y0, (u8 *)&(ptGb16[usIndex].Msk[0]),  PenColor, BackColor); /* 显示字符 */
} 
/**
	名称：void LCD_ShowCharString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t PenColor, uint16_t BackColor)
	参数：x，y      起始坐标
	      p         指向字符串起始地址
		  PenColor  字符颜色
		  BackColor 背景颜色
	功能：
	备注：用16字体，可以调节 此函数不能单独调用
**/
void LCD_ShowCharString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t PenColor, uint16_t BackColor)
{   
	uint8_t size = 16;     /*---字符大小默认16*8---*/
	 
    if(x>MAX_CHAR_POSX){x=0;y+=size;}			         /*超出X轴字体最小单位，换行*/
    if(y>MAX_CHAR_POSY){y=x=0;lcd_clear(White);}	 /*超出Y轴字体最小单位，回到原点，并且清屏*/
    LCD_ShowChar(x, y, *p, size, PenColor, BackColor);			   /*0表示非叠加方式*/
}
/**
	*名称：void LCD_ShowString(u16 x0, u16 y0, u8 *pcstr, u16 PenColor, u16 BackColor)
	*参数：x0 y0     起始坐标
		   pcstr     字符串指针
		   PenColor  字体颜色
		   BackColor 字体背景色
	*功能：调用字符和汉字显示函数，实现字符串显示
	*备注：	
**/
void LCD_ShowString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor)
{
	while(*pcStr!='\0')
	{
	 	if(*pcStr>0x80) /*显示汉字*/
		{
			LCD_ShowHzString(x0, y0, pcStr, PenColor, BackColor);
			pcStr += 2;
			x0 += 16;	
		}
		else           /*显示字符*/
		{
			LCD_ShowCharString(x0, y0, pcStr, PenColor, BackColor);	
			pcStr +=1;
			x0+= 8;
		}
	
	}	

} 
/**
	*名称：void LCD_Fill(uint8_t xsta, uint16_t ysta, uint8_t xend, uint16_t yend, uint16_t colour)
	*参数：xsta 起始X坐标
		   ysta 起始Y坐标
		   xend 结束X坐标
		   yend 结束Y坐标
		   color 待填充颜色
	*返回：无
	*功能：在指定矩形区域填充指定颜色，区域大小(xend-xsta)*(yend-ysta)
	*备注：部分区域少一行像素点	
**/
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t colour)
{                    
    u32 n;

	/*设置窗口	*/	
	LCD_WindowMax (xsta, ysta, xend, yend); 
	LCD_SetCursor(xsta,ysta);        /*设置光标位置 */ 
	LCD_WriteRAM_Prepare();         /*开始写入GRAM*/	 	   	   
	n=(u32)(yend-ysta+1)*(xend-xsta+1);    
	while(n--){LCD_WR_DATA(colour);} /*显示所填充的颜色*/
	 
	/*恢复窗口*/
	LCD_WindowMax (0, 0, 240, 320); 
} 
/**
	*名称：void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r)
	*参数：x0 中心点横坐标
	       y0 中心点纵坐标
		   r  半径
	*返回：无
	*功能：在指定位置画一个指定大小的圆
	*备注：加入颜色参数，是否填充等
**/
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t PointColor)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             /*判断下个点位置的标志*/
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,PointColor);             //3           
		LCD_DrawPoint(x0+b,y0-a,PointColor);             //0           
		LCD_DrawPoint(x0-a,y0+b,PointColor);             //1       
		LCD_DrawPoint(x0-b,y0-a,PointColor);             //7           
		LCD_DrawPoint(x0-a,y0-b,PointColor);             //2             
		LCD_DrawPoint(x0+b,y0+a,PointColor);             //4               
		LCD_DrawPoint(x0+a,y0-b,PointColor);             //5
		LCD_DrawPoint(x0+a,y0+b,PointColor);             //6 
		LCD_DrawPoint(x0-b,y0+a,PointColor);             
		a++;

		/*使用Bresenham算法画圆*/     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b,PointColor);
	}
} 
/**
	*名称：void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend)
	*参数：xsta X起始坐标
	       ysta Y起始坐标
		   xend X结束坐标
		   yend Y结束坐标
	*返回：无
	*功能：在指定区域画矩形
	*备注：

**/
void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t Color)
{
	LCD_DrawLine(xsta,ysta,xend,ysta, Color);
	LCD_DrawLine(xsta,ysta,xsta,yend, Color);
	LCD_DrawLine(xsta,yend,xend,yend, Color);
	LCD_DrawLine(xend,ysta,xend,yend, Color);
} 
void LCD_DrawPicture(u16 StartX,u16 StartY,u16 Xend,u16 Yend,u8 *pic)
{
	static	u16 i=0,j=0;
	u16 *bitmap = (u16 *)pic;
	/*设置图片显示窗口大小*/
	LCD_WindowMax(StartX, StartY, Xend, Yend);	
	LCD_SetCursor(StartX,StartY);
	LCD_WriteRAM_Prepare();
	for(j=0; j<Yend-StartY; j++)
	{
		for(i=0; i<Xend-StartX; i++) LCD_WriteRAM(*bitmap++); 	
	}
	/*恢复窗口*/
	LCD_WindowMax(0, 0, 240, 320);
}
