#include "main.h"

#include "../WDL/ptrlist.h"
#include "../WDL/lice/lice.h"
#include "../WDL/dirscan.h"

#include "../jmde/coolsb/coolscroll.h"

#include "resource.h"

WDL_PtrList<ImageRecord> g_images;
WDL_Mutex g_images_mutex;

HWND g_hwnd;
WDL_VWnd_Painter g_hwnd_painter;
WDL_VWnd g_vwnd; // owns all children windows

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

void UpdateMainWindowWithSizeChanged()
{
  HWND hwndDlg=g_hwnd;
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

void AddImage(const char *fn)
{
  ImageRecord *w = new ImageRecord(fn);
  g_vwnd.AddChild(w);
  g_images_mutex.Enter();
  g_images.Add(w);
  g_images_mutex.Leave();
 
}

WDL_DLGRET MainWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:

      g_vwnd_scrollpos=0;
#ifndef _WIN32 // need to fix some coolsb top level window bugs
      InitializeCoolSB(hwndDlg);
#endif
      g_hwnd = hwndDlg;
      g_vwnd.SetRealParent(hwndDlg);
      
      UpdateMainWindowWithSizeChanged();
      ShowWindow(hwndDlg,SW_SHOW);

      SetTimer(hwndDlg,1,100,NULL);
    return 0;
    case WM_DESTROY:

      g_images_mutex.Enter();
      g_images.Empty(); // does not free -- g_vwnd owns all images!
      g_images_mutex.Leave();

      g_vwnd.RemoveAllChildren(true);

#ifndef _WIN32
      UninitializeCoolSB(hwndDlg);
#endif
      PostQuitMessage(0);
    return 0;
    case WM_TIMER:
      if (wParam==1)
      {
        if (g_DecodeDidSomething)
        {
          g_DecodeDidSomething=false;
          InvalidateRect(hwndDlg,NULL,FALSE);
        }
      }
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDCANCEL:
          DestroyWindow(hwndDlg);
        break;
        case ID_IMPORT:

          {
            static char cwd[4096];

            if (!cwd[0]) GetPrivateProfileString("snapease","cwd","",cwd,sizeof(cwd),g_ini_file.Get());
            if (!cwd[0]) GetCurrentDirectory(sizeof(cwd),cwd);

            char *extlist=LICE_GetImageExtensionList();

          #ifdef _WIN32
            int temp_size=256*1024;
            char *temp=(char *)calloc(temp_size,1);
            OPENFILENAME l={sizeof(l),};
            l.hwndOwner = hwndDlg;
            l.lpstrFilter = (char*)extlist;
            l.lpstrFile = temp;
            l.nMaxFile = temp_size-1;
            l.lpstrTitle = "Load images:";
            l.lpstrDefExt = "jpg";
            l.lpstrInitialDir = cwd;
            l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT|OFN_NOCHANGEDIR;
            if (GetOpenFileName(&l))          
          #else
            char *temp=BrowseForFiles("Load image resource:", dir, fn, false, extlist);
            if (temp)
          #endif     
            {
              WDL_String path;
              path.Set(temp); 
              if (!temp[strlen(temp)+1]) // if single file, remove filename portion
              {
                char *p=path.Get();
                while (*p) p++;
                while (p > path.Get() && *p != '\\' && *p != '/') p--;
                *p=0;
              }            
              WritePrivateProfileString("snapease","cwd",path.Get(),g_ini_file.Get());

              if (!temp[strlen(temp)+1]) AddImage(temp);
              else
              {
                char *p = temp+strlen(temp)+1;

                if (temp[0] && p[-2]==PREF_DIRCH) p[-2]=0;

                while (*p)
                {
                  path.Set(temp);
                  path.Append(PREF_DIRSTR);
                  path.Append(p);
                  p+=strlen(p)+1;
                  AddImage(path.Get());
                }
              }

              free(temp);

              UpdateMainWindowWithSizeChanged();
            }
        
          }

        break;
      }
    return 0;
    case WM_SIZE:

      if (wParam != SIZE_MINIMIZED)
      {
        UpdateMainWindowWithSizeChanged();
      }

    return 0;
    case WM_LBUTTONDOWN:
      if (g_vwnd.OnMouseDown(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)))
      {
        SetCapture(hwndDlg);
      }
    return 0;
    case WM_MOUSEMOVE:
      g_vwnd.OnMouseMove(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
    return 0;
    case WM_LBUTTONUP:
      g_vwnd.OnMouseUp(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
      ReleaseCapture();
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

        UpdateMainWindowWithSizeChanged();
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
            if (!LICE_ImageIsSupported(buf))
            {
              WDL_String tmp;
              WDL_DirScan ds;
              if (!ds.First(buf))
              {
                WDL_PtrList<WDL_String> dirstack;
                for (;;)
                {
                  if (ds.GetCurrentFN()[0] != '.')
                  {
                    ds.GetCurrentFullFN(&tmp);
                    if (ds.GetCurrentIsDirectory())
                    {
                      WDL_String *s = new WDL_String;
                      s->Set(tmp.Get());
                      dirstack.Add(s);
                    }
                    else if (LICE_ImageIsSupported(tmp.Get()))
                    {
                      AddImage(tmp.Get());
                    }
                  }

                  if (!ds.Next()) continue;

                  bool didNew = false;

                  while (dirstack.GetSize() && !didNew)
                  {
                    WDL_String *s = dirstack.Get(0);
                    dirstack.Delete(0);

                    didNew = !ds.First(s->Get());

                    delete s;
                  }
                  
                  if (!didNew) break;
                }
              
              }
            }
            else // image
            {
              AddImage(buf);
            }
          }
        }
        DragFinish(hDrop);
        if (n)
          UpdateMainWindowWithSizeChanged();
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