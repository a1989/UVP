
#include "glcd.h"


static void RA8875_Delaly1us(void)
{
	uint8_t i;

	for (i = 0; i < 50; i++);	/* �ӳ�, ��׼ */
}

static void RA8875_Delaly1ms(void)
{
	uint16_t i;

	for (i = 0; i < 5000; i++);	/* �ӳ�1ms, ��׼ */
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
 * ��������GLCD_WriteReg()
 * ����  ��uint8_t LCD_Reg �Ĵ�����ַ, uint16_t LCD_RegValue �Ĵ�����ֵ
 * ���  ��void
 * ����  ��дLCD�Ĵ�������
 * ����  ���ⲿ����        
 *****************************************************************************************************************/
void RA8875_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  LCD_REG = LCD_Reg;         /*д�Ĵ�����ַ*/
  LCD_RAM = LCD_RegValue;    /*д��Ĵ���ֵ*/
}

/**************************************************************************************************************
 * ��������LCD_Init()
 * ����  ��void
 * ���  ��void
 * ����  ��LCD��ʼ������
 * ����  ���ⲿ����        
 *****************************************************************************************************************/
void GLCD_Init(void)
{ 
	/* ����LCD���ƿ���GPIO */
	LCD_GPIO_Config();

	/* ����FSMC�ӿڣ��������� */
	LCD_FSMC_Config();

	/* FSMC���ú������ӳٲ��ܷ��������豸  */
	Delay(200);

	
	/* ��ʼ��PLL.  ����Ƶ��Ϊ25M */
	LCD_WriteCmd(0x88);
	RA8875_Delaly1us();		/* �ӳ�1us */
	LCD_WriteData(10);	/* PLLDIVM [7] = 0 ;  PLLDIVN [4:0] = 10 */

	RA8875_Delaly1ms();

	LCD_WriteCmd(0x89);
	RA8875_Delaly1us();		/* �ӳ�1us */
	LCD_WriteData(2);	/* PLLDIVK[2:0] = 2, ����4 */

	/* RA8875 ���ڲ�ϵͳƵ�� (SYS_CLK) �ǽ���񵴵�·��PLL ��·��������Ƶ�ʼ��㹫ʽ���� :
		SYS_CLK = FIN * ( PLLDIVN [4:0] +1 ) / ((PLLDIVM+1 ) * ( 2^PLLDIVK [2:0] ))
						= 25M * (10 + 1) / ((0 + 1) * (2 ^ 2))
				= 68.75MHz
	*/

	/* REG[88h]��REG[89h]���趨��Ϊ��֤PLL ����ȶ�����ȴ�һ�Ρ���Ƶʱ�䡹(< 100us)��*/
		RA8875_Delaly1ms();

	/*
		����ϵͳ���ƼĴ���10h�� ����pdf ��18ҳ:

		bit3:2 ɫ������趨 (Color Depth Setting)
			00b : 8-bpp ��ͨ��TFT �ӿڣ� i.e. 256 ɫ��
			1xb : 16-bpp ��ͨ��TFT �ӿڣ� i.e. 65K ɫ��	 ��ѡ�����

		bit1:0 MCUIF ѡ��
			00b : 8-λMCU �ӿڡ�
			1xb : 16-λMCU �ӿڡ� ��ѡ�����
	*/
	RA8875_WriteReg(0x10, 0x0A);	/* ����16λMCU���ڣ�65Kɫ */

	/* REG[04h] Pixel Clock Setting Register   PCSR
		bit7  PCLK Inversion
			0 : PDAT ����PCLK ��Ե���� (Rising Edge) ʱ��ȡ����
			1 : PDAT ����PCLK ��Ե�½� (Falling Edge) ʱ��ȡ����
		bit1:0 PCLK Ƶ�������趨
			Pixel Clock  PCLK Ƶ�������趨��
			00b: PCLK Ƶ������= ϵͳƵ�����ڡ�
			01b: PCLK Ƶ������= 2 ����ϵͳƵ�����ڡ�
			10b: PCLK Ƶ������= 4 ����ϵͳƵ�����ڡ�
			11b: PCLK Ƶ������= 8 ����ϵͳƵ�����ڡ�
	*/
	RA8875_WriteReg(0x04,0x82);           //ͨ��04H���ò������غ�PCLK����ʱ��(Pixel Clock)
	RA8875_Delaly1ms();
	
	RA8875_WriteReg(0x14,0x3B);           //ˮƽ����:ˮƽ��ʾ���(����) = (HDWR + 1) * 8 = 480
	RA8875_WriteReg(0x15,0x00);           //ˮƽ����ʾ����΢��������� (HNDFTR)
	RA8875_WriteReg(0x16,0x01);           //ˮƽ����ʾ���(����) = (HNDR + 1) * 8
	RA8875_WriteReg(0x17,0x00);           //ˮƽͬ����ʼ��ַ��� = (HSTR + 1) * 8
	RA8875_WriteReg(0x18, 0x05);          //ˮƽͬ���ź��������(����) = (HPWR + 1) * 8

	RA8875_WriteReg(0x19, 0x0F);          //��ֱ��ʾ�߶ȵ�8λ [0-7] ��VDHR0�� 
	RA8875_WriteReg(0x1A, 0x01);          //��ֱ��ʾ�߶�(����) = VDHR + 1 = 0x010F + 1 = 272
	RA8875_WriteReg(0x1B, 0x02);          //��ֱ����ʾ����λ[7:0]��VNDR0�� 
	RA8875_WriteReg(0x1C, 0x00);          //��ֱ����ʾ����λ[8] ��VNDR1��
	RA8875_WriteReg(0x1D, 0x07);          //��ֱͬ���ź���ʼ��ַ�߶�λ[7:0]  VSTR0 
	RA8875_WriteReg(0x1E, 0x00);          //��ֱͬ���ź���ʼ��ַ�߶�λ[8]  VSTR1  
	RA8875_WriteReg(0x1F, 0x09);          //��ֱͬ���ź�������� VPWR
	
	RA8875_WriteReg(0x20, 0x08);          //��ʾ���üĴ�����DPCR��
	RA8875_Delaly1ms();		
	
	/* ����TFT���� DISP  ����Ϊ�ߣ�ʹ�����. ģ���DISP�������ӵ�RA8875оƬ��GP0X�� */
	RA8875_WriteReg(0xC7, 0x01);	/* DISP = 1 */

	/* LCD ��ʾ/�ر�Ѷ�� (LCD Display on) */
	RA8875_WriteReg(0x01, 0x80);

	/* 	REG[40h] Memory Write Control Register 0 (MWCR0)

		Bit 7	��ʾģʽ�趨
			0 : ��ͼģʽ��
			1 : ����ģʽ��

		Bit 6	����д����/�ڴ�д�����趨
			0 : �趨����/�ڴ�д����Ϊ����ʾ��
			1 : �趨����/�ڴ�д����Ϊ��ʾ��

		Bit 5	����д����/�ڴ�д������˸�趨
			0 : �α겻��˸��
			1 : �α���˸��

		Bit 4   NA

		Bit 3-2  ��ͼģʽʱ���ڴ�д�뷽��
			00b : �� -> �ң�Ȼ���� -> �¡�
			01b : �� -> ��Ȼ���� -> �¡�
			10b : �� -> �£�Ȼ���� -> �ҡ�
			11b : �� -> �ϣ�Ȼ���� -> �ҡ�

		Bit 1 	�ڴ�д�����Զ����ӹ����趨
			0 : ���ڴ�д��ʱ���λ���Զ���һ��
			1 : ���ڴ�д��ʱ���λ�ò����Զ���һ��

		Bit 0 �ڴ��ȡ����Զ����ӹ����趨
			0 : ���ڴ��ȡʱ���λ���Զ���һ��
			1 : ���ڴ��ȡʱ���λ�ò����Զ���һ��
	*/
	RA8875_WriteReg(0x40, 0x00);	/* ѡ���ͼģʽ */


	/* 	REG[41h] Memory Write Control Register1 (MWCR1)
		д��Ŀ��λ�ã�ѡ��ͼ��1
	*/
	RA8875_WriteReg(0x41, 0x00);	/* ѡ���ͼģʽ, Ŀ��ΪCGRAM */

	LCD_SetDispWin(0, 0, 272, 480);
	
    /* ����Դ� */
	LCD_ClrScr(CL_BLACK);	  /* ��ɫ */
	
	LCD_SetBackLight(255);	
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_ClrScr
*	����˵��: �����������ɫֵ����
*	��    �Σ�_usColor : ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
  uint32_t i;
	
	#if 0	/* �����֧�Ĵ��룬������RA8875��Ӳ����ɫ��书�ܣ��ٶȽϿ� */
	
		/* ��ɫ��������, ����pdf ��162ҳ
		�˹���ʹ���ڽ�ѡ���ض�������������ǉ\�����ĳ��ǰ��ɫ��R8875 ����ĵ�ɫ�趨ΪBTE ǰ��ɫ��
	
		�������裺
			1. �趨Ŀ��ͼ���λ�� REG[58h], [59h], [5Ah], [5Bh]
			2. �趨BTE ��Ⱥ͸߶� REG[5Ch], [5Dh], [5Eh], [5Fh]
			3. �趨BTE ������͹�դ������  REG[51h] Bit[3:0] = 0Ch
			4. �趨ǰ��ɫ  REG[63h], [64h], [65h]
			5. ����BTE ����  REG[50h] Bit7 = 1
			6. ���״̬������ STSR Bit6��ȷ��BTE �Ƿ����
		*/
		BTE_SetTarBlock(0, 0, s_LcdHeight, s_LcdWidth, 0);	/* ����BTEλ�úͿ�ȸ߶� */
		BTE_SetOperateCode(0x0C);	/* �趨BTE ������͹�դ������  REG[51h] Bit[3:0] = 0Ch */
		BTE_SetFrontColor(_usColor);	/* ����BTEǰ��ɫ */
		BTE_Start();				/* ����BTE ���� */
		BTE_Wait();					/* �ȴ��������� */
	
	#else 	/* ����Ĵ����ǰ���ͨ��㷽ʽ�������ٶȽ��� */
	
		LCD_SetCursor(0, 0);	/* ���ù��λ�� */
	
		LCD_REG = 0x02; 	/* �����趨RA8875 �����ڴ�(DDRAM��CGRAM)��ȡ/д��ģʽ */
	
		for (i = 0; i < 272 * 480; i++)
		{
			LCD_RAM = _usColor;
			while ((LCD_REG & 0x80 ) == 0x80);	//����������æ��δ�������⡣
		}
	
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_SetDispWin
*	����˵��: ������ʾ���ڣ����봰����ʾģʽ��TFT����оƬ֧�ִ�����ʾģʽ�����Ǻ�һ���12864����LCD���������
*	��    �Σ�
*		_usX : ˮƽ����
*		_usY : ��ֱ����
*		_usHeight: ���ڸ߶�
*		_usWidth : ���ڿ��
*	�� �� ֵ: ��
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
*	�� �� ��: LCD_SetCursor
*	����˵��: ���ù��λ��
*	��    �Σ�_usX : X����; _usY: Y����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetCursor(uint16_t _usX, uint16_t _usY)
{
	/* �����ڴ�д�������� ��0x80-83 �ǹ��ͼ�ε����꡿ */
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
 * ��������LCD_SetTextColor()
 * ����  ��uint16_t Color ǰ��ɫ
 * ���  ��void
 * ����  ������LCD��ǰ��ɫ
 * ����  ���ⲿ����        
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
 * ��������LCD_SetBackColor()
 * ����  ��uint16_t Color ����ɫ
 * ���  ��void
 * ����  ������LCD�ı���ɫ
 * ����  ���ⲿ����        
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
 * ��������LCD_GPIO_Config()
 * ����  ��void
 * ���  ��void
 * ����  ������LCD��GPIO�ܽ���FSMC����
 * ����  ���ڲ�����        
 *****************************************************************************************************************/
void LCD_GPIO_Config(void)
{
	
/*******************Һ����Ӳ������******************************

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

 /* ���û���FSMC��Һ������������
 * Һ���������ߣ�FSMC-D0~D15: PD 14 15 0 1,PE 7 8 9 10 11 12 13 14 15,PD 8 9 10
 */	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;										  //���÷�ת�ٶ�Ϊ100Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;											      //���ùܽ�ģʽΪ����ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | 
																GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);													  
																																		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
																GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
																GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
	
 /* ���û���FSMC��Һ����������
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
   
  /*����GPIOD��Ӧ�ĹܽŸ������ӵ�FSMC*/
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);	    //����GPIOD��Pin0�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);			//����GPIOD��Pin1�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);			//����GPIOD��Pin4�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);			//����GPIOD��Pin5�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FSMC);			//����GPIOD��Pin7�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);			//����GPIOD��Pin8�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);			//����GPIOD��Pin9�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);		//����GPIOD��Pin10�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);		//����GPIOD��Pin11�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);		//����GPIOD��Pin14�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);		//����GPIOD��Pin15�������ӵ�FSMC
	
   /*����GPIOE��Ӧ�ĹܽŸ������ӵ�FSMC*/
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);			//����GPIOE��Pin7�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);			//����GPIOE��Pin8�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);			//����GPIOE��Pin9�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);		//����GPIOE��Pin10�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);		//����GPIOE��Pin11�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);		//����GPIOE��Pin12�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);	  //����GPIOE��Pin13�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);		//����GPIOE��Pin14�������ӵ�FSMC
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);		//����GPIOE��Pin15�������ӵ�FSMC	
}

/**************************************************************************************************************
 * ��������LCD_FSMC_Config()
 * ����  ��void
 * ���  ��void
 * ����  ������LCD��FSMC
 * ����  ���ڲ�����        
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
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* ע��ɿ��������Ա */
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
*	�� �� ��: SetBackLight_by8875
*	����˵��: ����RA8875оƬ��PWM1��ؼĴ���������LCD����
*	��    �Σ�_bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SetBackLight_by8875(uint8_t _bright)
{
	if (_bright == 0)
	{
		/* �ر�PWM, PWM1����ȱʡ���LOW
		��ע�����ֱ�ӽ�ռ�ձȼĴ���P1DCR����Ϊ0��ʵ�������PWM���λ��Ǵ��ں�խ�����壬�������0ֵ����
		���������Ļ����ر�PWMҲ������ʡ���Ŀ�ġ�
		 */
		RA8875_WriteReg(0x8A, 0);
	}
	else if (_bright == 255)	/* ������� */
	{
		/* �ر�PWM, PWM1����ȱʡ���HIGH */
		/* ʵ�ʲ���������255������������⴦��PWM����Ҳ�ܹ�����㶨�ߵ�ƽ */
		RA8875_WriteReg(0x8A, 1 << 6);
	}
	else
	{
		/* ʹ��PWM1, ����ռ�ձȵ��� */

		/* 	REG[8Ah] PWM1 Control Register (P1CR)

			Bit7 ������ȵ��� (PWM1) �趨
				0 : �رգ���״̬�£�PWM1 ���׼λ���մ˻�����Bit6 ������
				1 : ������

			Bit6 PWM1 �ر�ʱ��׼λ
				0 : ��PWM �رջ���˯��ģʽʱ��PWM1 ���Ϊ��Low�� ״̬��
				1 : ��PWM �رջ���˯��ģʽʱ��PWM1 ���Ϊ��High�� ״̬��

			Bit5 ����

			Bit4 PWM1 ����ѡ��
				0 : PWM1 ���ܡ�
				1 : PWM1 �̶����һƵ��Ϊ�ⲿ��������Clock (Fin) Ƶ�� 1 /16 ��Clock

			Bit3-0  PWM1 ��·��Ƶ����Դѡ��:
				0000b : SYS_CLK / 1 1000b : SYS_CLK / 256
				0001b : SYS_CLK / 2 1001b : SYS_CLK / 512
				0010b : SYS_CLK / 4 1010b : SYS_CLK / 1024
				0011b : SYS_CLK / 8 1011b : SYS_CLK / 2048
				0100b : SYS_CLK / 16 1100b : SYS_CLK / 4096
				0101b : SYS_CLK / 32 1101b : SYS_CLK / 8192
				0110b : SYS_CLK / 64 1110b : SYS_CLK / 16384
				0111b : SYS_CLK / 128 1111b : SYS_CLK / 32768

		*/

		RA8875_WriteReg(0x8A, (1 << 7) | 4);      //�޸Ĵ˾䣬�����޸ı���PWMƵ��

		/* REG[8Bh] PWM1 Duty Cycle Register (P1DCR) */

		RA8875_WriteReg(0x8B, _bright);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_SetBackLight
*	����˵��: ��ʼ������LCD�������GPIO,����ΪPWMģʽ��
*			���رձ���ʱ����CPU IO����Ϊ��������ģʽ���Ƽ�����Ϊ������������������͵�ƽ)����TIM3�ر� ʡ��
*	��    �Σ�_bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
  SetBackLight_by8875(_bright);
}

