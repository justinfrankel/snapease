#include "main.h"
#include "resource.h"
#ifdef _WIN32
#include <commctrl.h>
#endif

WDL_String g_ini_file;
char g_exepath[4096];
HINSTANCE g_hInst;
UINT Scroll_Message;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nShowCmd)
{
  g_hInst=hInstance;
  InitCommonControls();
  Scroll_Message = RegisterWindowMessage("MSWHEEL_ROLLMSG");
  WDL_VWnd_regHelperClass("SNAPeaseVwndHost");

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

  g_DecodeThreadQuit=false;


  int numCPU = 1; // todo: smp option?

  HANDLE hThread[4]={0,};
  if (numCPU>sizeof(hThread)/sizeof(hThread[0])) numCPU=sizeof(hThread)/sizeof(hThread[0]);

  int x;
  for(x=0;x<numCPU;x++)
  {
    DWORD tid;
    hThread[x] = CreateThread(NULL,0,DecodeThreadProc,0,NULL,&tid);
    SetThreadPriority(hThread[x],THREAD_PRIORITY_BELOW_NORMAL);
  }

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

    vvv = EditImageProcessMessage(&msg);
    if (vvv>0) continue;

    if (!vvv)
    {
      if (msg.hwnd == g_hwnd||IsChild(g_hwnd,msg.hwnd))
      {
        if (msg.message == WM_KEYDOWN||msg.message==WM_CHAR)
        {
          if (g_fullmode_item && msg.wParam == VK_ESCAPE)
          {
            RemoveFullItemView(true);
            continue;
          }
        }
      }
    }

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
  g_DecodeThreadQuit = true;
  for(x=0;x<sizeof(hThread)/sizeof(hThread[0]);x++)
  {
    if (hThread[x])
    {
      WaitForSingleObject(hThread[x],INFINITE);
      CloseHandle(hThread[x]);
      hThread[x]=0;
    }
  }
  return 0;
}


#ifndef _WIN32
// todo mac res
#endif