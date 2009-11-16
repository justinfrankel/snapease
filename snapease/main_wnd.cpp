#include "main.h"

#include "../WDL/ptrlist.h"
#include "../WDL/lice/lice.h"

#include "../jmde/coolsb/coolscroll.h"

#include "imagerecord.h"

HWND g_hwnd;
WDL_VWnd_Painter g_hwnd_painter;
WDL_VWnd g_vwnd; // owns all children windows
WDL_PtrList<ImageRecord> g_images;
int g_vwnd_scrollpos;

int OrganizeWindow(HWND hwndDlg)
{
  RECT r;
  GetClientRect(hwndDlg,&r);

  bool hasScrollBar=false;
  // decide whether we will need a scrollbar?

  int border_size_x = 8, border_size_y=8;

  int preview_w = r.right-border_size_x;

  if (preview_w > 256) preview_w = 256;
  if (preview_w < 32) preview_w = 32;

  int preview_h = (preview_w*3) / 4;

  int xpos=border_size_x/2, ypos=border_size_y/2;
  int x;
  for (x = 0; x < g_images.GetSize(); x ++)
  {
    ImageRecord *rec = g_images.Get(x);
    if (rec)
    {
      if (xpos + preview_w > r.right)
      {
        xpos=border_size_x/2;
        ypos += preview_h + border_size_y;
      }   

      RECT r = {xpos,ypos - g_vwnd_scrollpos,xpos+preview_w,ypos+preview_h - g_vwnd_scrollpos};
      rec->SetPosition(&r);
    }
    xpos += preview_w + border_size_x;
  }
  int maxPos = ypos + preview_h + border_size_y/2;
  if (maxPos < 0) maxPos=0;

  return maxPos;
}
void DoWindowSizeScrollUpdate(HWND hwndDlg)
{
  RECT r;
  GetClientRect(hwndDlg,&r);
  int wh = OrganizeWindow(hwndDlg);
  if (g_vwnd_scrollpos<0)
  {
    g_vwnd_scrollpos=0;
    OrganizeWindow(hwndDlg);
  }
  else if (g_vwnd_scrollpos > wh - r.bottom && wh > r.bottom) 
  {
    g_vwnd_scrollpos=wh - r.bottom;
    OrganizeWindow(hwndDlg);
  }

  SCROLLINFO si={sizeof(si),SIF_PAGE|SIF_POS|SIF_RANGE,0,wh, r.bottom, g_vwnd_scrollpos,};
  CoolSB_SetScrollInfo(hwndDlg,SB_VERT,&si,TRUE);
  InvalidateRect(hwndDlg,NULL,FALSE);
}

WDL_DLGRET MainWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:

      g_vwnd_scrollpos=0;
      InitializeCoolSB(hwndDlg);
      g_hwnd = hwndDlg;
      g_vwnd.SetRealParent(hwndDlg);
      
      DoWindowSizeScrollUpdate(hwndDlg);
      ShowWindow(hwndDlg,SW_SHOW);

    return 0;
    case WM_DESTROY:
      g_images.Empty(); // do not free -- g_vwnd owns all images!
      g_vwnd.RemoveAllChildren(true);
      UninitializeCoolSB(hwndDlg);
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

      DoWindowSizeScrollUpdate(hwndDlg);

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
    case WM_VSCROLL:
    {
      SCROLLINFO si = { sizeof(SCROLLINFO), SIF_POS|SIF_PAGE|SIF_RANGE, 0 };
      CoolSB_GetScrollInfo(hwndDlg, SB_VERT, &si);
      int opos = si.nPos;
      switch (LOWORD(wParam))
      {
        case SB_THUMBTRACK:        
          si.nPos = HIWORD(wParam);
        break;
        case SB_LINEUP:
          si.nPos -= 30;
        break;
        case SB_LINEDOWN:
          si.nPos += 30;
        break;
        case SB_PAGEUP:
          si.nPos -= si.nPage;
        break;
        case SB_PAGEDOWN:
          si.nPos += si.nPage;
        break;
        case SB_TOP:
          si.nPos = 0;
        break;
        case SB_BOTTOM:
          si.nPos = si.nMax-si.nPage;
        break;
      }
      if (si.nPos < 0) si.nPos = 0;
      else if (si.nPos > si.nMax-si.nPage) si.nPos = si.nMax-si.nPage;
      if (si.nPos != opos)
      {
        g_vwnd_scrollpos=si.nPos;

        DoWindowSizeScrollUpdate(hwndDlg);
      }
    }
    return 0;
    case WM_DROPFILES:

      {
        HDROP hDrop = (HDROP) wParam;
        int x;
        int n=DragQueryFile(hDrop,-1,NULL,0);
        int cnt=0;
        for (x = 0; x < n; x ++)
        {
          char buf[4096];
          buf[0]=0;
          DragQueryFile(hDrop,x,buf,sizeof(buf));
          if (buf[0])
          {
            ImageRecord *w = new ImageRecord(buf);
            g_images.Add(w);
            g_vwnd.AddChild(w);
          }
        }
        DragFinish(hDrop);
        if (n)
          DoWindowSizeScrollUpdate(hwndDlg);
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



// for coolsb
void *GetIconThemePointer(const char *name)
{
  if (!strcmp(name,"scrollbar"))
  {
    // todo: load/cache scrollbar image
  }
  return NULL;
}

extern "C" {
int GSC_mainwnd(int p) { return GetSysColor(p); }
};