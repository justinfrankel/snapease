#include "main.h"

#include "../WDL/ptrlist.h"
#include "../WDL/lice/lice.h"

#include "imagerecord.h"

HWND g_hwnd;
WDL_VWnd_Painter g_hwnd_painter;
WDL_VWnd g_vwnd; // owns all children windows
WDL_PtrList<ImageRecord> g_images;


void OrganizeWindow(HWND hwndDlg)
{
  RECT r;
  GetClientRect(hwndDlg,&r);

  int border_size_x = 8, border_size_y=8;

  int preview_w = r.right-border_size_x;

  if (preview_w > 256) preview_w = 256;
  if (preview_w < 32) preview_w = 32;

  int preview_h = (preview_w*3) / 4;

  // decide whether we will need a scrollbar?


  int xpos=border_size_x/2, ypos=border_size_y/2;
  int x;
  for (x = 0; x < g_images.GetSize(); x ++)
  {
    ImageRecord *rec = g_images.Get(x);
    if (rec)
    {
      RECT r = {xpos,ypos,xpos+preview_w,ypos+preview_h};
      rec->SetPosition(&r);
    }
    xpos += preview_w + border_size_x;

    if (xpos + preview_w > r.right)
    {
      xpos=border_size_x/2;
      ypos += preview_h + border_size_y;
    }   
  }
}

WDL_DLGRET MainWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      g_hwnd = hwndDlg;
      g_vwnd.SetRealParent(hwndDlg);
      
      int x;
      for(x=0;x<64;x++)
      {
        ImageRecord *w = new ImageRecord("C:/Users/Justin/Desktop/dmw cropped.jpg");
        g_images.Add(w);
        g_vwnd.AddChild(w);

      }
      ShowWindow(hwndDlg,SW_SHOW);
    return 0;
    case WM_DESTROY:
      g_images.Empty(); // do not free -- g_vwnd owns all images!
      g_vwnd.RemoveAllChildren(true);
      PostQuitMessage(0);
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDCANCEL:
          DestroyWindow(hwndDlg);
        break;
      }
    return 0;
    case WM_SIZE:
      OrganizeWindow(hwndDlg);
      InvalidateRect(hwndDlg,NULL,FALSE);
    return 0;
    case WM_PAINT:

      {
        RECT r;
        GetClientRect(hwndDlg,&r);
        g_vwnd.SetPosition(&r);
        g_hwnd_painter.PaintBegin(hwndDlg,RGB(0,0,0));
        g_hwnd_painter.PaintVirtWnd(&g_vwnd);
        g_hwnd_painter.PaintEnd();

      }

    return 0;
  }

  return 0;
}





// an app should implement these
int WDL_STYLE_WantGlobalButtonBorders() { return 0; }
bool WDL_STYLE_WantGlobalButtonBackground(int *col) { return false; }
int WDL_STYLE_GetSysColor(int p) { return GetSysColor(p); }
void WDL_STYLE_ScaleImageCoords(int *x, int *y) { }
bool WDL_Style_WantTextShadows(int *col) { return false; }
bool WDL_STYLE_GetBackgroundGradient(double *gradstart, double *gradslope) { return false; } // return values 0.0-1.0 for each, return false if no gradient desired

LICE_IBitmap *WDL_STYLE_GetSliderBitmap2(bool vert) { return NULL; }
bool WDL_STYLE_AllowSliderMouseWheel() { return true; }
int WDL_STYLE_GetSliderDynamicCenterPos() { return 500; }
