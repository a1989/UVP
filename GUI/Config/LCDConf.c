/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2012     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/
#include "GUI.h"
#include "GUIDRV_Template.h"
#include "bsp.h"
#include "lcd_ra8875.h"

/*
********************************************************************
*
*       Layer configuration (to be modified)
*
********************************************************************
*/
// Physical display size
//
#define XSIZE_PHYS  480
#define YSIZE_PHYS  272
#define VXSIZE_PHYS 480
#define VYSIZE_PHYS 272
#define COLOR_CONVERSION  GUICC_M565
#define LCD_CONTROLLER       8875 
#define LCD_BITSPERPIXEL       16
#define LCD_USE_PARALLEL_16     0
//
// Color conversion
//
#define COLOR_CONVERSION GUICC_M565
//
// Display driver
//
#define DISPLAY_DRIVER &GUIDRV_Template_API
//
// Orientation
//
#define DISPLAY_ORIENTATION  GUI_SWAP_XY | GUI_MIRROR_X | GUI_MIRROR_Y
//
// Configures touch screen module
//
#define GUI_TOUCH_AD_LEFT 	 106
#define GUI_TOUCH_AD_RIGHT 	 980
#define GUI_TOUCH_AD_TOP 	 106
#define GUI_TOUCH_AD_BOTTOM  935
/*
**********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif

#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif

#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif

#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif

#ifndef   VRAM_ADDR
  #define VRAM_ADDR 0
#endif

#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif

#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif


/*
****************************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
****************************************************************************   
*/
void LCD_X_Config(void) 
{
 	/* Set display driver and color conversion for 1st layer */
    GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);

//	if (LCD_GetHeight() == 272)	         /*   4.3寸或者5寸  480x272 */
//	{
		LCD_SetSizeEx    (0, 480, 272);
		LCD_SetVSizeEx   (0, 480, 272);
//	}
		GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 480, GUI_TOUCH_AD_TOP, GUI_TOUCH_AD_BOTTOM);
		GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 272, GUI_TOUCH_AD_LEFT, GUI_TOUCH_AD_RIGHT);
//	else if(LCD_GetHeight() == 320)      /*   3.5寸  480*320 */
//	{
//		LCD_SetSizeEx    (0, 480, 320);
//		LCD_SetVSizeEx   (0, 480, 320);
//	}
//	else 	                            /*   7寸或者5寸  800*480 */
//	{
//		LCD_SetSizeEx    (0, XSIZE_PHYS, YSIZE_PHYS);
//		LCD_SetVSizeEx   (0, VXSIZE_PHYS, VYSIZE_PHYS);	
//	}
}

/*
*********************************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*******************************************************************************
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) 
{
	int r;
	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
		{
			//
			// Called during the initialization process in order to set up the
			// display controller and put it into operation. If the display
			// controller is not initialized by any external routine this needs
			// to be adapted by the customer...
			//
			// ...
			//  已经在前面初始化了，这里不再初始化
			LCD_InitHard();
			RA8875_TouchInit();
			return 0;
		}
		case LCD_X_SETVRAMADDR: 
		{
			//
			// Required for setting the address of the video RAM for drivers
			// with memory mapped video RAM which is passed in the 'pVRAM' element of p
			//
			//...
			return 0;
		}
		case LCD_X_SETORG: 
		{
			//
			// Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
			//

			//...
			return 0;
		}
		case LCD_X_SHOWBUFFER: 
		{
			//
			// Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
			//
			//...
			return 0;
		}
		case LCD_X_SETLUTENTRY: 
		{
			//
			// Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
			//
			//...
			return 0;
		}
		case LCD_X_ON: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			return 0;
		}
		case LCD_X_OFF: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			// ...
			return 0;
		}
		default:
		r = -1;
	}
	return r;
}
/*************************** End of file ****************************/
