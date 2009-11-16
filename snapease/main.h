#ifndef _SNAPEASE_MAIN_H_
#define _SNAPEASE_MAIN_H_

#ifdef _WIN32
  #include <windows.h>
#else
  #include "../WDL/swell/swell.h"
#endif

#include "../WDL/wdltypes.h"
#include "../WDL/wingui/virtwnd.h"

extern HINSTANCE g_hInst;

WDL_DLGRET MainWindowProc(HWND, UINT, WPARAM, LPARAM);


#endif