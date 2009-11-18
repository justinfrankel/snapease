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

class ImageRecord;

extern HINSTANCE g_hInst;
extern WDL_String g_ini_file;
extern char g_exepath[4096];
extern HWND g_hwnd;

WDL_DLGRET MainWindowProc(HWND, UINT, WPARAM, LPARAM);

extern bool g_DecodeThreadQuit, g_DecodeDidSomething;
DWORD WINAPI DecodeThreadProc(LPVOID v);

void UpdateMainWindowWithSizeChanged();
bool RemoveFullItemView(bool refresh=true); // if in full view, removes full view (and returns true)
void OpenFullItemView(ImageRecord *w);

#include "imagerecord.h"

extern WDL_PtrList<ImageRecord> g_images;
extern WDL_Mutex g_images_mutex;
extern ImageRecord *g_fullmode_item;

void config_readstr(const char *what, char *out, int outsz);
int config_readint(const char *what, int def);
void config_writestr(const char *what, const char *value);
void config_writeint(const char *what, int value);


bool EditImageProcessMessage(MSG *msg);
void EditImageRunTimer();
void EditImageLabel(ImageRecord *rec);
void EditImageLabelEnd(bool ignoreData=false);

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20a
#endif
extern UINT Scroll_Message;

#endif