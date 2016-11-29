/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.16 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_TOUCH_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/
#include "GUI.h"
//#include "touch.h"
#include "lcd_ra8875.h"

extern unsigned char touch_flag;


void GUI_TOUCH_X_ActivateX(void) 
{

}


void GUI_TOUCH_X_ActivateY(void)
{

}


int  GUI_TOUCH_X_MeasureX(void) 
{
//				GUI_DispDecAt(1, 30, 80, 4);
//		return ADS_Read_XY(CMD_RDX);
//	return ADS_Read_XY(CMD_RDY);
	
		unsigned short X;
	
//	 X = 1086-RA8875_TouchReadX();	//ADC XMIN = 106  XMAX = 980
	X = RA8875_TouchReadX();

	return(X); 
}


int  GUI_TOUCH_X_MeasureY(void) 
{
//				GUI_DispDecAt(1 , 30, 90, 4);
//		return ADS_Read_XY(CMD_RDY);
//	return ADS_Read_XY(CMD_RDX);
	
			unsigned short Y;	
 
//    Y = 1041-RA8875_TouchReadY();  //ADC YMIN = 106  YMAX = 935
		Y = RA8875_TouchReadY();
	
		return(Y); 
}


