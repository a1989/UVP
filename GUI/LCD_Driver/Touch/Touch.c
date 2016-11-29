#include "stm32f4xx.h"
//#include "ssd1298_lcd.h"
#include "lcd_ra8875.h"
#include "Touch.h"
//#include "Nvic.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "Systick.h"


Pen_Holder Pen_Point;	/* �����ʵ�� */

static void Delay(uint16_t counter)
{
    unsigned int i,j;
    for(i=0;i<counter;i++)
        for(j=0;j<10000;j++);
}

//����IO�ڳ�ʼ��
static void Touch_Gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOI, ENABLE);
    
    GPIO_PinAFConfig(GPIOI,GPIO_PinSource1,GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOI,GPIO_PinSource2,GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOI,GPIO_PinSource3,GPIO_AF_SPI2);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
    /*!< SPI SCK pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOI, &GPIO_InitStructure);
    
    /*!< SPI MOSI pin configuration */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
    GPIO_Init(GPIOI, &GPIO_InitStructure);
    
    /*!< SPI MISO pin configuration */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_Init(GPIOI, &GPIO_InitStructure);
 
    /*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_Init(GPIOG, &GPIO_InitStructure);

    //SD_CS_DISABLE();
    NotSelect_TOUCH_CS();
    NotSelect_FLASH_CS();
    NotSelect_SD_CS();

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;	/*����ʱ���ȶ�״̬�Ǹߵ�ƽ*/
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;/* ʱ�ӻ�Ծ���أ�����*/
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; /*SPI������Ԥ��Ƶ*/
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; /*��λ��ǰ*/ 
	SPI_InitStructure.SPI_CRCPolynomial = 7; /*the polynomial used for the CRC calculation*/
	SPI_Init(SPI2, &SPI_InitStructure);

	/* SPI2ʹ�� */
	SPI_Cmd(SPI2, ENABLE);     
}

///*SPIX��ʼ��*/
//static void  TouchSPIx_Init(void)
//{  
//	SPI_InitTypeDef  SPI_InitStructure;
//	
//	NotSelect_TOUCH_CS();
//	
//	/* SPI1 configuration */
//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//	 SPI_BaudRatePrescaler_32
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//	SPI_InitStructure.SPI_CRCPolynomial = 7;
//	SPI_Init(TOUCH_SPI, &SPI_InitStructure);
//	
//	/* Enable SPI1  */
//	SPI_Cmd(TOUCH_SPI, ENABLE);
//}
//�����ж�ʵʼ��
static void TOUCH_EXTI_Configuration(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE); //ʹ��IOʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ��SYSCFGʱ��

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;          //ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //�����Ͻ�
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;             //IO��Ϊ0
  GPIO_Init(GPIOG, &GPIO_InitStructure);                //��������gpio

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource8);//ʵʼ���ж���0

  EXTI_InitStructure.EXTI_Line = EXTI_Line8;                   //�����ж���Ϊ�ж���0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;          //�����ж�ģʽ
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;      //����Ϊ�½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;                    //�����ж���ʹ��
  EXTI_Init(&EXTI_InitStructure);                              

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;             
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  EXTI_GenerateSWInterrupt(EXTI_Line8);   //�����ж� ��һ������
}

/*SPI��дһ�ֽ�*/
uchar SPI_SendByte(unsigned char byte)
{
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(TOUCH_SPI, SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(TOUCH_SPI, byte);
	
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(TOUCH_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(TOUCH_SPI);
}

//2�ζ�ȡADS7846,������ȡ2����Ч��ADֵ,�������ε�ƫ��ܳ���
//50,��������,����Ϊ������ȷ,�����������.	   
//�ú����ܴ�����׼ȷ��
#define ERR_RANGE 50 //��Χ 
u8 Read_ADS2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=Read_ADS(&x1,&y1);   
    if(flag==0)return(0);
    flag=Read_ADS(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 
/*****************************************************************************
** ��������: Pen_Int_Set
** ��������: PEN�ж�����
				EN=1�������ж�
					EN=0: �ر��ж�
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/	 
void Pen_Int_Set(u8 en)
{
	if(en)EXTI->IMR|=1<<8;   //����line13�ϵ��ж�	  	
	else EXTI->IMR&=~(1<<8); //�ر�line13�ϵ��ж�	   
}
//��ȡһ������ֵ	
//������ȡһ��,֪��PEN�ɿ��ŷ���!					   
u8 Read_TP_Once(void)
{
//	u8 t=0;	    
	Pen_Int_Set(0);//�ر��ж�


	Pen_Point.Key_Sta=Key_Up;

    Delay(2);

	if(Read_ADS2(&Pen_Point.X,&Pen_Point.Y))
	{
	    Delay(500);
		Pen_Int_Set(1);//�����ж�
	    return 1;
	}
	else
	{ 
		Pen_Int_Set(1);//�����ж�
	    return 0;
	}


//	while(t<=150)	 // PEN==0&&t<=250
//	{
//		t++;
//		Delay(1);
//	};
//	Pen_Int_Set(1);//�����ж�		 
//	if(t>=50)		//250
//	    return 0;//����2.5s ��Ϊ��Ч
//	else 
//    	return 1;


//	if(Read_ADS2(&Pen_Point.X,&Pen_Point.Y) == 1)
//	    return 1;
//	else
//	    return 0;	
}
//////////////////////////////////////////////////
//��LCD�����йصĺ���  
//��һ��������
//����У׼�õ�
void Drow_Touch_Point(u8 x,u16 y)
{
	Draw_Circle(x,y,6,Red);//������Ȧ
}
//������У׼����
//�õ��ĸ�У׼����
void Touch_Adjust(void)
{								 
	u16 pos_temp[4][2];//���껺��ֵ
	u8  cnt=0;	
	u16 d1,d2;
	u32 tem1,tem2;
	double fac; 	   
	cnt=0;				 
	lcd_clear(White);//���� 
	Drow_Touch_Point(20,20);//����1 
	Pen_Point.Key_Sta=Key_Up;//���������ź� 
	Pen_Point.xfac=0;//xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������	 
	while(1)
	{
		if(Pen_Point.Key_Sta==Key_Down)//����������
		{
			if(Read_TP_Once())//�õ����ΰ���ֵ
			{  								   
				pos_temp[cnt][0]=Pen_Point.X;
				pos_temp[cnt][1]=Pen_Point.Y;

                printf(" \r %d ",Pen_Point.X);
                printf(" %d \n",Pen_Point.Y);

				cnt++;
			}			 
			switch(cnt)
			{			   
				case 1:
					lcd_clear(White);//���� 
					Drow_Touch_Point(220,20);//����2
					break;
				case 2:
					lcd_clear(White);//���� 
					Drow_Touch_Point(20,300);//����3
					break;
				case 3:
					lcd_clear(White);//���� 
					Drow_Touch_Point(220,300);//����4
					break;
				case 4:	 //ȫ���ĸ����Ѿ��õ�
	    		    	//�Ա����
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,2�ľ���
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
					{
						cnt=0;
						lcd_clear(White);//���� 
						Drow_Touch_Point(20,20);
						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,3�ľ���
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
						lcd_clear(White);//���� 
						Drow_Touch_Point(20,20);
						continue;
					}//��ȷ��
								   
					//�Խ������
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,4�ľ���
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
						lcd_clear(White);//���� 
						Drow_Touch_Point(20,20);
						continue;
					}//��ȷ��
					//������
					Pen_Point.xfac=(float)200/(pos_temp[1][0]-pos_temp[0][0]);//�õ�xfac		 
					Pen_Point.xoff=(240-Pen_Point.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//�õ�xoff
						  
					Pen_Point.yfac=(float)280/(pos_temp[2][1]-pos_temp[0][1]);//�õ�yfac
					Pen_Point.yoff=(320-Pen_Point.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//�õ�yoff  
//					POINT_COLOR=BLUE;
					lcd_clear(White);//����
                    LCD_ShowString(60, 18, "Touch Screen Adjust OK!", Red, Black); //У�����
					Delay(20);
					lcd_clear(Magenta);//����   
					return;//У�����				 
			}
		}
	} 
}
//��������ʼ��
void Touch_Init(void)
{
    Touch_Gpio_Init();
	//TouchSPIx_Init();
//	NVIC_Configuration();
	//TOUCH_EXTI_Configuration();

	//Touch_Adjust();
}
/*****************************************************************************
** ��������: void Touch_IRQ(void)
** ��������: �����жϺ���
** ��  ����: 
** �ա�  ��: 
*****************************************************************************/
void Touch_IRQ(void)
{
	//u16 i;
  	if(EXTI_GetITStatus(TOUCH_IRQ_GPIO_Pin) != RESET)
	{
		//if(Read_TP_Once() == 1)
		Pen_Point.Key_Sta=Key_Down;//�������� 
	
        EXTI_ClearITPendingBit(TOUCH_IRQ_LINE);			 		  				 
	}
}






//SPI������ 
//��7846/7843/XPT2046/UH7843/UH7846��ȡadcֵ	   
u16 ADS_Read_AD(u8 CMD)	  
{ 	 
//	u8 i;	  
	u16 Num,Date; 

    NotSelect_SD_CS(); 
    NotSelect_FLASH_CS();
	Select_TOUCH_CS();

    SPI_SendByte(CMD); 
	//for(i=100;i>0;i--); 
	Num=0;
	Date=0;
	Num = SPI_SendByte(0x00);
	Date = Num << 8;
	Num = SPI_SendByte(0x00);

	Date |= Num; 

    Date >>=4;
    Date &=0XFFF;

	NotSelect_TOUCH_CS();
	return(Date); 
}

//��ȡһ������ֵ
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ 
#define READ_TIMES 20 //��ȡ����
#define LOST_VAL 1	  //����ֵ
u16 ADS_Read_XY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)
	{				 
		buf[i]=ADS_Read_AD(xy);	    
	}				    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

//���˲��������ȡ
//��Сֵ��������100.
u8 Read_ADS(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=ADS_Read_XY(CMD_RDX);
	ytemp=ADS_Read_XY(CMD_RDY);
//	xtemp=ADS_Read_AD(CMD_RDX);
//	ytemp=ADS_Read_AD(CMD_RDY);
	  												   
	if(xtemp<10||ytemp<10)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}

//ת�����
//���ݴ�������У׼����������ת����Ľ��,������X0,Y0��
void Convert_Pos(void)
{		 	  
	if(Read_ADS2(&Pen_Point.X,&Pen_Point.Y))
	{
		Pen_Point.X0=Pen_Point.xfac*Pen_Point.X+Pen_Point.xoff;
		Pen_Point.Y0=Pen_Point.yfac*Pen_Point.Y+Pen_Point.yoff;  
	}
}

/*****************************************************************************
** ��������: Refreshes_Screen
** ��������: ˢ����Ļ				
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/
void Refreshes_Screen(void)
{
	lcd_clear(White);//����  
	LCD_ShowString(216,0,"RST",Red,Black);
//  	POINT_COLOR=BLUE;//���û�����ɫ 
}

//��һ�����
//2*2�ĵ�			   
void Draw_Big_Point(u8 x,u16 y)
{	    
	LCD_DrawPoint(x,y,Red);//���ĵ� 
	LCD_DrawPoint(x+1,y,Red);
	LCD_DrawPoint(x,y+1,Red);
	LCD_DrawPoint(x+1,y+1,Red);	 	  	
}
