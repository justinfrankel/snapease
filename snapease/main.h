#ifndef _SNAPEASE_MAIN_H_
#define _SNAPEASE_MAIN_H_

#ifdef _WIN32
  #include <windows.h>
  #include <windowsx.h>
#else
  #include "../WDL/swell/swell.h"
#endif

#include "../WDL/wdltypes.h"
#include "../WDL/wingui/virtwnd.h"
#include "../WDL/mutex.h"
#include "../WDL/wdlstring.h"

#ifdef _WIN32

#define PREF_DIRCH '\\'
#define PREF_DIRSTR "\\"

#else
#define PREF_DIRCH '/'
#define PREF_DIRSTR "/"

#endif

extern HINSTANCE g_hInst;
extern WDL_String g_ini_file;
extern char g_exepath[4096];

WDL_DLGRET MainWindowProc(HWND, UINT, WPARAM, LPARAM);

extern bool g_DecodeThreadQuit, g_DecodeDidSomething;
DWORD WINAPI DecodeThreadProc(LPVOID v);

void UpdateMainWindowWithSizeChanged();

#include "imagerecord.h"

extern WDL_PtrList<ImageRecord> g_images;
extern WDL_Mutex g_images_mutex;


#endif