/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.22                          *
*        Compiled Jul  4 2013, 15:16:01                              *
*        (c) 2013 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END

//#include "DIALOG.h"
#include "PrintingWindowDLG.h"
#include "PrintingFramewinDLG.h"
#include "defines.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0     (GUI_ID_USER + 0x00)
#define ID_PROGBAR_0     (GUI_ID_USER + 0x01)
#define ID_PROGBAR_1     (GUI_ID_USER + 0x02)
#define ID_TEXT_0     (GUI_ID_USER + 0x03)
#define ID_TEXT_1     (GUI_ID_USER + 0x04)
#define ID_TEXT_2     (GUI_ID_USER + 0x05)
#define ID_TEXT_3     (GUI_ID_USER + 0x06)
#define ID_PROGBAR_2     (GUI_ID_USER + 0x07)
#define ID_BUTTON_0     (GUI_ID_USER + 0x08)
#define ID_BUTTON_1     (GUI_ID_USER + 0x09)
#define ID_TEXT_4     (GUI_ID_USER + 0x0a)
WM_HWIN hWin;



extern GUI_HWIN flag;
// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 480, 272, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "Progbar", ID_PROGBAR_0, 115, 42, 200, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "Progbar", ID_PROGBAR_1, 116, 81, 200, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text", ID_TEXT_0, 80, 12, 300, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text", ID_TEXT_1, 20, 42, 83, 20, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text", ID_TEXT_2, 23, 81, 80, 20, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text", ID_TEXT_3, 15, 115, 151, 20, 0, 0x64, 0 },
  { PROGBAR_CreateIndirect, "Progbar", ID_PROGBAR_2, 16, 145, 320, 20, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 48, 181, 80, 45, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_1, 189, 180, 80, 45, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "Text", ID_TEXT_4, 80, 240, 300, 20, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

extern float current_temperature[EXTRUDERS];
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Window'
    //
    hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, 0x00000000);
    //
    // Initialization of 'Text'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_20_1);
		
    TEXT_SetTextColor(hItem, 0x00FFFFFF);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
    TEXT_SetFont(hItem, GUI_FONT_20_1);
		
    TEXT_SetTextColor(hItem, 0x00FFFFFF);
    //
    // Initialization of 'Text'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_20B_1);
    TEXT_SetText(hItem, "Extruder");
    TEXT_SetTextColor(hItem, 0x00FFFFFF);

    //
    // Initialization of 'Text'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
    TEXT_SetFont(hItem, GUI_FONT_20B_1);
    TEXT_SetText(hItem, "Hot Bed");
    TEXT_SetTextColor(hItem, 0x00FFFFFF);
    //
    // Initialization of 'Text'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
    TEXT_SetFont(hItem, GUI_FONT_20B_1);
    TEXT_SetText(hItem, "Rate of Progress");
    TEXT_SetTextColor(hItem, 0x00FFFFFF);
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetFont(hItem, GUI_FONT_20B_1);
    BUTTON_SetText(hItem, "Pause");
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
    BUTTON_SetFont(hItem, GUI_FONT_20B_1);
    BUTTON_SetText(hItem, "Stop");
    // USER START (Optionally insert additional code for further widget initialization)
		
		hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_0);
		PROGBAR_SetBarColor(hItem, 0, GUI_GREEN);
		PROGBAR_SetSkin(hItem, PROGBAR_SKIN_FLEX);
		PROGBAR_SetMinMax(hItem, 0, 230);
		PROGBAR_SetValue(hItem, current_temperature[0]);
    // USER END
    break;
		
	case WM_TIMER:
			hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_0);
			PROGBAR_SetValue(hItem, current_temperature[0]);
			WM_RestartTimer(pMsg->Data.v, 1000);
		break;
		
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
		  flag = pMsg->hWin;
		  CreatePrintingFramewin();
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateWindow
*/
WM_HWIN CreatePrintingWindow(void);
WM_HWIN CreatePrintingWindow(void) {
//  WM_HWIN hWin;
		WM_HTIMER hTimer;
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
//	hTimer = WM_CreateTimer(hWin, 0, 1000, 0);
//	GUI_Exec();
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
