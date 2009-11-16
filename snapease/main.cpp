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

  return 0;
}


#ifndef _WIN32
// todo mac res
#endif