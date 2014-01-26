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

#include "../WDL/win32_utf8.h"

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
extern WDL_String g_list_path;
extern char g_exepath[4096];
extern HWND g_hwnd;



WDL_DLGRET MainWindowProc(HWND, UINT, WPARAM, LPARAM);


extern bool g_DecodeDidSomething;
void DecodeThread_Init();
void DecodeThread_Quit();
void DecodeThread_RunTimer();

void UpdateMainWindowWithSizeChanged();
bool RemoveFullItemView(bool refresh=true); // if in full view, removes full view (and returns true)
void OpenFullItemView(ImageRecord *w);
void EnsureImageRecVisible(ImageRecord *rec);
void SetMainScrollPosition(float pos, int relative=0); // relative=1 for lines, 2 for pages

LICE_IBitmap *LoadThemeElement(int idx, const char *name);

bool saveImageListToFile(const char *fn);
bool importImageListFromFile(const char *fn, bool addToCurrent);

void DoExportDialog(HWND hwndDlg);

#include "imagerecord.h"

extern WDL_PtrList<ImageRecord> g_images;
extern int g_images_cnt_err, g_images_cnt_ok, g_images_statcnt;
extern INT_PTR g_ram_use_preview, g_ram_use_full, g_ram_use_fullscaled, g_ram_use_fullfinal;
extern WDL_Mutex g_images_mutex;
extern ImageRecord *g_fullmode_item;


extern int g_config_smp, g_config_statusline;

extern int g_firstvisible_startitem;

void config_readstr(const char *what, char *out, int outsz);
int config_readint(const char *what, int def);
void config_writestr(const char *what, const char *value);
void config_writeint(const char *what, int value);

int MainProcessMessage(MSG *msg);

int EditImageProcessMessage(MSG *msg); // >0=eat, <0 =own but default processing
void EditImageRunTimer();
void EditImageLabel(ImageRecord *rec);
void EditImageLabelEnd(bool ignoreData=false);

void SetImageListIsDirty(bool isDirty=true);
void UpdateCaption();

void AddImageRec(ImageRecord *rec, int idx=-1);

extern bool g_imagelist_fn_dirty; // need save
extern WDL_String g_imagelist_fn;

void LoadImageList(bool addToCurrent);
bool SaveImageList(bool forceSaveAs);
bool SavePromptForClose(const char *promptmsg);
void ClearImageList();

bool file_exists(const char *filename);
WDL_INT64 file_size(const char *filename);

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20a
#endif
extern UINT Scroll_Message;

#endif