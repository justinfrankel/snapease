/*
    SnapEase
    main.cpp -- win32 app initialization
    Copyright (C) 2009  Cockos Incorporated

    PathSync is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PathSync is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PathSync; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "main.h"
#include "resource.h"
#ifdef _WIN32
#include <commctrl.h>
#endif

char g_exepath[4096];
HINSTANCE g_hInst;

UINT Scroll_Message;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nShowCmd)
{
  g_hInst=hInstance;
  InitCommonControls();
  Scroll_Message = RegisterWindowMessage("MSWHEEL_ROLLMSG");

  const char *mainClassName = "SNAPeaseVwndHost";
  WDL_VWnd_regHelperClass(mainClassName);


  {
    GetModuleFileName(g_hInst,g_exepath,sizeof(g_exepath));
    char *p=g_exepath;
    while (*p) p++;
    while (p > g_exepath && *p != '\\') p--; *p=0;
  }
  g_ini_file.Set(g_exepath);
  g_ini_file.Append("\\snapease.ini");

  HKEY k;
  if (RegOpenKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0,KEY_READ,&k) == ERROR_SUCCESS)
  {
    char buf[1024];
    DWORD b=sizeof(buf);
    DWORD t=REG_SZ;
    if (RegQueryValueEx(k,"AppData",0,&t,(unsigned char *)buf,&b) == ERROR_SUCCESS && t == REG_SZ)
    {
      g_ini_file.Set(buf);
      g_ini_file.Append("\\snapease");

      CreateDirectory(g_ini_file.Get(),NULL);
      g_ini_file.Append("\\snapease.ini");
      

    }
    RegCloseKey(k);
  }
   
  CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_MAIN),GetDesktopWindow(),MainWindowProc);

  for(;;)
	{	      
    MSG msg={0,};
    int vvv = GetMessage(&msg,NULL,0,0);
    if (!vvv)  break;

    if (vvv<0)
    {
      Sleep(10);
      continue;
    }
    if (!msg.hwnd)
    {
		  DispatchMessage(&msg);
      continue;
    }

    vvv = MainProcessMessage(&msg);
    if (vvv>0) continue;

    if (IsDialogMessage(g_hwnd,&msg)) continue;

    HWND hWndParent=NULL;
    HWND temphwnd = msg.hwnd;
    do
    { 
      if (GetClassLong(temphwnd, GCW_ATOM) == (INT)32770) 
      {
        hWndParent=temphwnd;
        if (!(GetWindowLong(temphwnd,GWL_STYLE)&WS_CHILD)) break; // not a child, exit 
      }
    }
    while (temphwnd = GetParent(temphwnd));
    
    if (hWndParent && IsDialogMessage(hWndParent,&msg)) continue;

		TranslateMessage(&msg);
   	DispatchMessage(&msg);
 
  }
  return 0;
}


#ifndef _WIN32
// todo mac res
#endif