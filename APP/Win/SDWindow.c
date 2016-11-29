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
#include "SDWindow.h"
#include "SDWindowFramewinDLG.h"
#include "exfuns.h"
#include "sd.h"
#include "stdlib.h"
//#include "main2sd.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0             (GUI_ID_USER + 0x00)
#define ID_LISTBOX_0             (GUI_ID_USER + 0x03)
#define ID_BUTTON_0             (GUI_ID_USER + 0x04)


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
//  { LISTBOX_CreateIndirect, "Listbox", ID_LISTBOX_0, 0, 0, 200, 240, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 350, 200, 90, 50, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

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
		LISTBOX_Handle hListBox;
	
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Window'
    //

	
    hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, 0x00FFFFFF);
	
//    hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_0);
		hItem = LISTBOX_CreateEx(0, 0, 300, 272, pMsg->hWin, WM_CF_SHOW, 0, ID_LISTBOX_0, NULL);
		SCROLLBAR_SetSkin(hItem, SCROLLBAR_SKIN_FLEX);
		LISTBOX_SetAutoScrollV(hItem, 1);
		LISTBOX_SetAutoScrollH(hItem, 1);
		LISTBOX_SetScrollStepH(hItem, 20);
		LISTBOX_SetScrollbarWidth(hItem, 30);
//		LISTVIEW_SetGridVis(hItem, 1);

		LISTBOX_AddString(hItem, info.lfname);
	
    LISTBOX_SetFont(hItem, GUI_FONT_24_1);

    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
		BUTTON_SetSkin(hItem, BUTTON_SKIN_FLEX);
    BUTTON_SetFont(hItem, GUI_FONT_20B_1);
    BUTTON_SetText(hItem, "Cancel");
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
	

		
    break;
		
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_LISTBOX_0: // Notifications sent by 'Listbox'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
		      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_0);
					LISTBOX_GetItemText(hItem, LISTBOX_GetSel(hItem), buf, 100);
					if(strlen(buf) > 7)
					{
							strcpy(showstring, "Print ");
							strcat(showstring, buf);
							strcat(showstring, " ?");
							CreateSDWindowFramewin();
					}
					strcpy(showstring, buf);
					
        // USER END
        break;
      case WM_NOTIFICATION_SEL_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
		  GUI_EndDialog(pMsg->hWin, 0);
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
WM_HWIN CreateSDWindow(void);
WM_HWIN CreateSDWindow(void) {
  WM_HWIN hWin;
		
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	GUI_Exec();
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
