#ifndef _SNAPEASE_MAIN_H_
#define _SNAPEASE_MAIN_H_

#ifdef _WIN32
  #include <windows.h>
#else
  #include "../WDL/swell/swell.h"
#endif

#include "../WDL/wdltypes.h"
#include "../WDL/wingui/virtwnd.h"
#include "../WDL/mutex.h"

extern HINSTANCE g_hInst;

WDL_DLGRET MainWindowProc(HWND, UINT, WPARAM, LPARAM);

extern bool g_DecodeThreadQuit, g_DecodeDidSomething;
DWORD WINAPI DecodeThreadProc(LPVOID v);


#include "imagerecord.h"

extern WDL_PtrList<ImageRecord> g_images;
extern WDL_Mutex g_images_mutex;


#endif