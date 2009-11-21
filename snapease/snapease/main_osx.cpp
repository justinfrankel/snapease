
#include "../../WDL/swell/swell.h"

#include "../resource.h"
#include "../../WDL/swell/swell-dlggen.h"

#include "../main.h"

extern HMENU SWELL_app_stocksysmenu;

char g_exepath[4096];

void SWELL_app_initialstartup()
{
  {
    GetModuleFileName(NULL,g_exepath,sizeof(g_exepath));
    char *p=g_exepath;
    while (*p) p++;
    while (p > g_exepath && *p != '/') p--; *p=0;
  }
  g_ini_file.Set(g_exepath);
  g_ini_file.Append("/snapease.ini");

  
}
void SWELL_app_startupcompleted()
{
  if (SWELL_app_stocksysmenu)
  {
    HMENU menu = CreatePopupMenu();    
    HMENU nm=SWELL_DuplicateMenu(SWELL_app_stocksysmenu);
    if (nm)
    {
      MENUITEMINFO mi={sizeof(mi),MIIM_STATE|MIIM_SUBMENU|MIIM_TYPE,MFT_STRING,0,0,nm,NULL,NULL,0,"SnapEase"};
      InsertMenuItem(menu,0,TRUE,&mi);           
    }    
    SWELL_SetDefaultModalWindowMenu(menu);
  }
  
  
  HWND h=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_MAIN),NULL,MainWindowProc);
  HMENU menu = LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU1));
  if (SWELL_app_stocksysmenu)
  {
    HMENU nm=SWELL_DuplicateMenu(SWELL_app_stocksysmenu);
    if (nm)
    {
      MENUITEMINFO mi={sizeof(mi),MIIM_STATE|MIIM_SUBMENU|MIIM_TYPE,MFT_STRING,0,0,nm,NULL,NULL,0,"SnapEase"};
      InsertMenuItem(menu,0,TRUE,&mi);           
    }
  }
  SetMenu(h,menu);
  
  
}
void SWELL_app_onmenucommand(int tag)
{
  if (g_hwnd) SendMessage(g_hwnd,WM_COMMAND,tag&0xffff,0);
}

void SWELL_app_abouttoexit()
{
  if (g_hwnd) DestroyWindow(g_hwnd);
}

bool SWELL_app_hookkb(MSG *msg)
{
  if (MainProcessMessage(msg)>0) return true;
  return false;
}


#include "../snapease.rc_mac_dlg"

#undef BEGIN
#undef END
#include "../../WDL/swell/swell-menugen.h"

#include "../snapease.rc_mac_menu"
