#include "main.h"
#include "resource.h"
#ifdef _WIN32
#include <commctrl.h>
#endif

HINSTANCE g_hInst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nShowCmd)
{
  g_hInst=hInstance;
  InitCommonControls();
  WDL_VWnd_regHelperClass("SNAPeaseVwndHost");

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