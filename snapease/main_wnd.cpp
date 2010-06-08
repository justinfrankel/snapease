#define VERSTRING "0.2"
/*
    SnapEase
    main_wnd.cpp -- main window dialogness
    Copyright (C) 2009-2010  Cockos Incorporated

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



/*

  Todo: 
    (tabs for image lists along top ?)
    disk-based thumbnail cache? sqlite?

      facebook?



                                      http://api.facebook.com/restserver.php

                                      if no session key:

                                        auth.createToken
                                        launch browser to  login.php with this token
                                        when user hits OK, Auth.getSession

                                        upload.
                                           photos.getAlbums...
                                           Photos.createAlbum

                                           Photos.upload : aid : caption 

                                          - fail, reset session key, repeat

                                      save session key in ini?

    
    slideshows etc?
    confirm to remove item from set?


*/


#include "main.h"
#include <math.h>

#include "../WDL/ptrlist.h"
#include "../WDL/lice/lice.h"
#include "../WDL/dirscan.h"
#include "../WDL/lice/lice_text.h"

#include "../WDL/wingui/scrollbar/coolscroll.h"

#include "resource.h"

WDL_String g_ini_file;
WDL_String g_list_path;


WDL_PtrList<ImageRecord> g_images;
WDL_Mutex g_images_mutex;

HWND g_hwnd;
WDL_VWnd_Painter g_hwnd_painter;


class MainWindowVwnd : public WDL_VWnd
{
public:
  MainWindowVwnd() { }
  ~MainWindowVwnd()  { }

  virtual void OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect)
  {
    WDL_VWnd::OnPaint(drawbm,origin_x,origin_y,cliprect);
    if (!m_children) return;

    WDL_VWnd *capwnd = m_children->Get(m_captureidx);
    if (capwnd && !strcmp(capwnd->GetType(),"ImageRecord"))
    {
      int to=0;
      int a = ((ImageRecord*)capwnd)->UserIsDraggingImageToPosition(&to);
      if (to>0 && a>=0)
      {
        bool doright=false;
        WDL_VWnd *nextwnd = m_children->Get(a);
        if (!nextwnd) 
        {
          nextwnd = m_children->Get(a-1);
          doright=true;
        }
        if (nextwnd)
        {
          RECT r;
          nextwnd->GetPosition(&r);
          int xp = doright ? r.right : r.left;

          if (!doright && xp > 4) xp-=4;

          bool wantDraw=true;

          if (to != 2)
          {
            if (nextwnd==capwnd) wantDraw=false;
            else
              if (m_children->Get(a+ (doright?1:-1))==capwnd) wantDraw=false;
          }

          if (wantDraw)
          {
            LICE_pixel col= to==2 ? LICE_RGBA(0,255,0,255) : LICE_RGBA(255,255,0,255);
            LICE_Line(drawbm,origin_x+xp-3,origin_y+r.top,origin_x+xp+3,origin_y+r.top,col,1.0f,LICE_BLIT_MODE_COPY,false);
            LICE_Line(drawbm,origin_x+xp-3,origin_y+r.bottom,origin_x+xp+3,origin_y+r.bottom,col,1.0f,LICE_BLIT_MODE_COPY,false);
            LICE_Line(drawbm,origin_x+xp,origin_y+r.top,origin_x+xp,origin_y+r.bottom,col,1.0f,LICE_BLIT_MODE_COPY,false);
            if (to == 2)
            {
              int cx = origin_x+xp+5;
              int cy = origin_y+(r.top+r.bottom)/2;
              LICE_Line(drawbm,cx,cy-2,cx,cy+2,col,1.0f,LICE_BLIT_MODE_COPY,false);
              LICE_Line(drawbm,cx-2,cy,cx+2,cy,col,1.0f,LICE_BLIT_MODE_COPY,false);
            }
          }
        }
      }
    }
  }
  virtual bool OnMouseDblClick(int xpos, int ypos)
  {
    if (WDL_VWnd::OnMouseDblClick(xpos,ypos)) return true;
    return RemoveFullItemView();
  }
};

MainWindowVwnd g_vwnd; // owns all children windows

int g_firstvisible_startitem;

bool g_aboutwindow_open;

char g_hwnd_tooltip[256];
POINT g_hwnd_tooltip_pt;

#define GENERAL_TIMER 1
#define TOOLTIP_TIMER 2
#define TOOLTIP_TIMEOUT 350

void ClearImageList()
{
  int x;
  WDL_PtrList<ImageRecord> r = g_images;
  g_images_mutex.Enter();
  g_images.Empty(); // does not free -- g_vwnd owns all images!
  g_images_mutex.Leave();

  for(x=0;x<r.GetSize();x++)
    g_vwnd.RemoveChild(r.Get(x),true);
}


// this should likely go into WDL
void DrawTooltipForPoint(LICE_IBitmap *bm, POINT mousePt, RECT *wndr, const char *text)
{
  if (!bm || !text || !text[0]) return;

    static LICE_CachedFont tmpfont;
    if (!tmpfont.GetHFont())
    {
      bool doOutLine = true;
      LOGFONT lf = 
      {
          14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,
	      #ifdef _WIN32
          "MS Shell Dlg"
	      #else
	      "Arial"
	      #endif
      };

      tmpfont.SetFromHFont(CreateFontIndirect(&lf),LICE_FONT_FLAG_OWNS_HFONT);                 
    }
    tmpfont.SetBkMode(TRANSPARENT);
    LICE_pixel col1 = LICE_RGBA(0,0,0,255);
    LICE_pixel col2 = LICE_RGBA(255,255,192,255);

    tmpfont.SetTextColor(col1);
    RECT r={0,};
    tmpfont.DrawText(bm,text,-1,&r,DT_CALCRECT);


    int xo = min(max(mousePt.x,wndr->left),wndr->right);
    int yo = min(max(mousePt.y + 24,wndr->top),wndr->bottom);

    if (yo + r.bottom > wndr->bottom-4) // too close to bottom, move up if possible
    {
      if (mousePt.y - r.bottom - 12 >= wndr->top)
        yo = mousePt.y - r.bottom - 12;
      else
        yo = wndr->bottom -4 - r.bottom;
    }

    if (xo + r.right > wndr->right - 4)
    {
      xo = wndr->right - 4 - r.right;
    }


    r.left += xo;
    r.top += yo;
    r.right += xo;
    r.bottom += yo;
    

    int border = 3;
    LICE_FillRect(bm,r.left-border,r.top-border,r.right-r.left+border*2,r.bottom-r.top+border*2,col2,1.0f,LICE_BLIT_MODE_COPY);
    LICE_DrawRect(bm,r.left-border,r.top-border,r.right-r.left+border*2,r.bottom-r.top+border*2,col1,1.0f,LICE_BLIT_MODE_COPY);
    
    tmpfont.DrawText(bm,text,-1,&r,0);

}


ImageRecord *g_fullmode_item;

int g_vwnd_scrollpos;

int OrganizeWindow(HWND hwndDlg)
{
  RECT r;
  GetClientRect(hwndDlg,&r);

  bool hasScrollBar=false;
  // decide whether we will need a scrollbar?

  int border_size_x = 8, border_size_y=8;

  int preview_w = (r.right-border_size_x);

  if (preview_w > 256) preview_w = 256;
  if (preview_w < 32) preview_w = 32;

  int preview_h = (preview_w*3) / 4;

  int xpos=border_size_x/2, ypos=border_size_y/2;
  int x;
  bool hadFullItem=false;
  bool hadVis=false;
  for (x = 0; x < g_images.GetSize(); x ++)
  {
    ImageRecord *rec = g_images.Get(x);
    if (rec)
    {
      if (g_fullmode_item&&g_fullmode_item!=rec)
      {
        rec->SetVisible(false);
      }
      else
      {
        rec->SetVisible(true);
        if (g_fullmode_item)
        {
          hadFullItem=true;
          rec->SetIsFullscreen(true);

          RECT rr={12,12,r.right-12,r.bottom-12};
          rec->SetPosition(&rr);

          g_firstvisible_startitem = max(0,x-5); 
        }
        else
        {
          rec->SetIsFullscreen(false);

          if (xpos + preview_w > r.right)
          {
            xpos=border_size_x/2;
            ypos += preview_h + border_size_y;
          }   

          RECT rr = {xpos,ypos - g_vwnd_scrollpos,xpos+preview_w,ypos+preview_h - g_vwnd_scrollpos};
          if (!hadVis && rr.top + (rr.bottom-rr.top)*2/3 >=0 )
          {
            hadVis=true;
            g_firstvisible_startitem=x;
          }
          rec->SetPosition(&rr);
        }
      }
    }
    xpos += preview_w + border_size_x;
  }
  if (!hadFullItem&&g_fullmode_item)
  {
    g_images_mutex.Enter();
    g_fullmode_item=0;
    g_images_mutex.Leave();
    return OrganizeWindow(hwndDlg);
  }

  if (hadFullItem) return r.bottom - 4;

  int maxPos = ypos + preview_h + border_size_y/2;
  if (maxPos < 0) maxPos=0;

  return maxPos;
}

void KillTooltip(bool doRefresh=false)
{
  KillTimer(g_hwnd,TOOLTIP_TIMER);
  bool had=!!g_hwnd_tooltip[0];
  g_hwnd_tooltip[0]=0;
  if (had && doRefresh) InvalidateRect(g_hwnd,NULL,FALSE);
}

void UpdateMainWindowWithSizeChanged()
{
  KillTooltip();

  g_aboutwindow_open=false;

  EditImageLabelEnd();

  HWND hwndDlg=g_hwnd;
  RECT r;
  GetClientRect(hwndDlg,&r);
  if (g_vwnd_scrollpos<0)g_vwnd_scrollpos=0;
  int wh = OrganizeWindow(hwndDlg);

  if (!g_fullmode_item)
  {
    if (g_vwnd_scrollpos > wh - r.bottom && wh > r.bottom) 
    {
      g_vwnd_scrollpos=wh - r.bottom;
      OrganizeWindow(hwndDlg);
    }

    SCROLLINFO si={sizeof(si),SIF_PAGE|SIF_POS|SIF_RANGE,0,wh, r.bottom, g_vwnd_scrollpos,};
    CoolSB_SetScrollInfo(hwndDlg,SB_VERT,&si,TRUE);
  }
  else
  {
    SCROLLINFO si={sizeof(si),SIF_PAGE|SIF_POS|SIF_RANGE,0,0,0, 0,};
    CoolSB_SetScrollInfo(hwndDlg,SB_VERT,&si,TRUE);
  }
  InvalidateRect(hwndDlg,NULL,FALSE);
}


void OpenFullItemView(ImageRecord *w)
{
  RemoveFullItemView(false);

  g_images_mutex.Enter();
  w->m_want_fullimage=true;
  LICE_IBitmap *old = w->m_fullimage;
  w->m_fullimage = 0;
  g_fullmode_item=w;
  g_images_mutex.Leave();
  delete old;
  delete w->m_fullimage_rendercached;
  w->m_fullimage_rendercached=0;
  UpdateMainWindowWithSizeChanged();

  // tell thread to background-load full quality imagea for this
}

bool RemoveFullItemView(bool refresh)
{
  if (g_fullmode_item)
  {
    g_images_mutex.Enter();
    ImageRecord *r = g_fullmode_item;
    if (g_images.Find(g_fullmode_item)>=0)
    {
      LICE_IBitmap *old = g_fullmode_item->m_fullimage;
      g_fullmode_item->m_want_fullimage = false;
      g_fullmode_item->m_fullimage = 0;
      delete old;
      delete g_fullmode_item->m_fullimage_rendercached;
      g_fullmode_item->m_fullimage_rendercached=0;

    }
    g_fullmode_item=0;
    g_images_mutex.Leave();
    if (refresh) 
    {
      UpdateMainWindowWithSizeChanged();
      if (g_images.Find(r)>=0) EnsureImageRecVisible(r);
    }
    return true;
  }
  return false;
}


void SetMainScrollPosition(float pos, int relative=0) // relative=1 for lines, 2 for pages
{
  if (relative)
  {
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_POS|SIF_PAGE|SIF_RANGE|SIF_TRACKPOS, 0 };
    CoolSB_GetScrollInfo(g_hwnd, SB_VERT, &si);

    if (relative == 2) pos *= (int)si.nPage;
    pos += si.nPos;

    if (pos > si.nMax-(int)si.nPage) pos = si.nMax-(int)si.nPage;
    if (pos < 0) pos = 0;
  }
  if (pos != g_vwnd_scrollpos)
  {
    g_vwnd_scrollpos=(int)(pos+0.5);

    UpdateMainWindowWithSizeChanged();
  }
}

void EnsureImageRecVisible(ImageRecord *rec)
{
  if (g_fullmode_item) 
  {
    if (rec!=g_fullmode_item)
      OpenFullItemView(rec);
  }
  else
  {
    RECT cr,r;
    GetClientRect(g_hwnd,&r);
    rec->GetPosition(&cr);

    int pos = g_vwnd_scrollpos;
    if (cr.bottom > r.bottom) pos += cr.bottom-r.bottom;
    else if (cr.top < r.top) pos -= r.top-cr.top;

    if (pos<0)pos=0;
    SetMainScrollPosition(pos);
    
    // do any necessary scrolling
  }
}

void AddImageRec(ImageRecord *rec, int idx)
{
  g_vwnd.AddChild(rec,idx);
  g_images_mutex.Enter();
  if (idx<0) g_images.Add(rec);
  else g_images.Insert(idx,rec);
  g_images_mutex.Leave();
}

void AddImage(const char *fn)
{
  ImageRecord *w = new ImageRecord(fn);

  AddImageRec(w);
 
  SetImageListIsDirty(true);
  if (g_fullmode_item) EnsureImageRecVisible(w);
}

static RECT g_lastSplashRect;


static void DrawAboutWindow(WDL_VWnd_Painter *painter, RECT r)
{
  static LICE_IBitmap *splash=  NULL;
  if (!splash)
    splash = LoadThemeElement(IDR_SPLASH,"snapease.png");
  if (splash)
  {
    int xo=0,yo=0;
    LICE_IBitmap *bm = painter->GetBuffer(&xo,&yo);

    g_lastSplashRect.left = r.right/2 - splash->getWidth()/2;
    g_lastSplashRect.top = r.bottom/2 - splash->getHeight()/2 - splash->getHeight()/4;
    if (g_lastSplashRect.top<0)g_lastSplashRect.top=0;
    g_lastSplashRect.right = g_lastSplashRect.left + splash->getWidth();
    g_lastSplashRect.bottom = g_lastSplashRect.top + splash->getHeight();

    xo += g_lastSplashRect.left;
    yo += g_lastSplashRect.top;
    if (bm) 
    {
      float xsc = 1.0/splash->getWidth();
      float ysc = 1.0/splash->getHeight();

      static float a[9]={0,};
      int x;
      static double t;
      
      t += ((rand()+GetTickCount()/10000)%100)/1000.0;
      for(x=0;x<9;x++)
        a[x]=a[x]*0.9 + 0.1 * ((0.5+sin(t*(0.2*x+0.2)))*0.3 - ((x/3)&1)*0.2);
      
      LICE_GradRect(bm,xo,yo,splash->getWidth(),splash->getHeight(),a[6],a[7],a[8],1,   a[0]*xsc,a[1]*xsc,a[2]*xsc,0*xsc,a[3]*ysc,a[4]*ysc,a[5]*ysc,0*ysc,LICE_BLIT_MODE_COPY);

      LICE_Blit(bm,splash,xo, yo,NULL,1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_USE_ALPHA);

      //if (g_aboutwindow_open)
      {
        static LICE_CachedFont tmpfont;
        if (!tmpfont.GetHFont())
        {
          bool doOutLine = true;
          LOGFONT lf = 
          {
              14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,
	          #ifdef _WIN32
              "MS Shell Dlg"
	          #else
	          "Arial"
	          #endif
          };

          tmpfont.SetFromHFont(CreateFontIndirect(&lf),LICE_FONT_FLAG_OWNS_HFONT);                 
        }
        tmpfont.SetBkMode(TRANSPARENT);
        tmpfont.SetTextColor(LICE_RGBA(255,255,255,255));

        RECT tr={xo - g_lastSplashRect.left,yo+splash->getHeight()+5,xo - g_lastSplashRect.left + r.right,yo+r.bottom};
        int h = tmpfont.DrawText(bm,
              "Version " VERSTRING " - "
              "Copyright (C) 2009-2010 Cockos Incorporated",-1,&tr,DT_CENTER|DT_TOP);

        tr.top += h + 32;

        if (!g_aboutwindow_open)
        {
          tmpfont.DrawText(bm,
              "(...drag image files here, if you like...)"
              ,-1,&tr,DT_CENTER|DT_TOP);

        }
        else
        {
          // credits etc
        }

      }
    }
  }
}


WDL_DLGRET MainWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
  if (Scroll_Message && uMsg == Scroll_Message)
  {
    uMsg=WM_MOUSEWHEEL;
    wParam<<=16; 
  }
#endif
  switch (uMsg)
  {
    case WM_INITDIALOG:
#ifdef _WIN32
      SetClassLongPtr(hwndDlg,GCLP_HICON,(INT_PTR)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,32,32,LR_SHARED));
      SetClassLongPtr(hwndDlg,GCLP_HICONSM,(INT_PTR)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,16,16,LR_SHARED));
#endif
      g_vwnd_scrollpos=0;
#ifndef _WIN32 // need to fix some coolsb top level window bugs
      InitializeCoolSB(hwndDlg);
#endif


      {
        g_list_path.Set(g_ini_file.Get());
        char *p=g_list_path.Get();
        while (*p) p++;
        while (p > g_list_path.Get() && *p != '\\' && *p != '/') p--;
        *p=0;

        g_list_path.Append(PREF_DIRSTR "lists");
        CreateDirectory(g_list_path.Get(),NULL);
    
      }

      g_hwnd = hwndDlg;
      g_vwnd.SetRealParent(hwndDlg);
      
      UpdateMainWindowWithSizeChanged();

      {
        RECT r={config_readint("wndx",15),config_readint("wndy",15),};

        r.right = r.left + config_readint("wndw",0);
        r.bottom = r.top + config_readint("wndh",0);
       
        if (r.bottom != r.top && r.right != r.left)
          SetWindowPos(hwndDlg,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE);         
      }

#ifdef _WIN32
      if (config_readint("wndmax",0))
        ShowWindow(hwndDlg,SW_SHOWMAXIMIZED);
      else
#endif
        ShowWindow(hwndDlg,SW_SHOW);
 
      DecodeThread_Init();

      SetTimer(hwndDlg,GENERAL_TIMER,30,NULL);
    return 0;
    case WM_DESTROY:

      DecodeThread_Quit();


      {
        RECT r;
        GetWindowRect(hwndDlg,&r);

#ifdef _WIN32
        WINDOWPLACEMENT wp={sizeof(wp),};
        if (GetWindowPlacement(hwndDlg,&wp)) r=wp.rcNormalPosition;
        config_writeint("wndmax",wp.showCmd == SW_SHOWMAXIMIZED);
#endif
        config_writeint("wndx",r.left);
        config_writeint("wndy",r.top);
        config_writeint("wndw",r.right-r.left);
        config_writeint("wndh",abs(r.bottom-r.top));

      }

      g_images_mutex.Enter();
      g_images.Empty(); // does not free -- g_vwnd owns all images!
      g_images_mutex.Leave();

      g_vwnd.RemoveAllChildren(true);

#ifndef _WIN32
      UninitializeCoolSB(hwndDlg);
#endif
      
      g_hwnd=0; // prevent multidelete on osx :)
#ifdef _WIN32
      PostQuitMessage(0);
#else
      SWELL_PostQuitMessage(hwndDlg);
#endif
    return 0;
    case WM_TIMER:
      if (wParam == TOOLTIP_TIMER)
      {
        KillTimer(hwndDlg,wParam);
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(hwndDlg,&p);

        RECT r;
        GetClientRect(hwndDlg,&r);

        char buf[256];
        buf[0]=0;
        if (!PtInRect(&r,p) || !g_vwnd.GetToolTipString(p.x,p.y,buf,sizeof(buf))) buf[0]=0;

        if (strcmp(buf,g_hwnd_tooltip))
        {
          g_hwnd_tooltip_pt = p;
          lstrcpyn(g_hwnd_tooltip,buf,sizeof(g_hwnd_tooltip));
          InvalidateRect(hwndDlg,NULL,FALSE);
        }
      }
      else if (wParam==GENERAL_TIMER)
      {
        DecodeThread_RunTimer();

        if (!g_images.GetSize()||g_aboutwindow_open)
          InvalidateRect(hwndDlg,&g_lastSplashRect,FALSE);

        EditImageRunTimer();
        if (g_DecodeDidSomething)
        {
          g_DecodeDidSomething=false;
          InvalidateRect(hwndDlg,NULL,FALSE);
        }
      }
    return 0;
    case WM_CLOSE:
      SendMessage(hwndDlg,WM_COMMAND,ID_QUIT,0);
    break;
    case WM_INITMENUPOPUP:
      {
        HMENU hm=(HMENU)wParam;
        EnableMenuItem(hm,ID_SAVE,MF_BYCOMMAND|(g_imagelist_fn.Get()[0]? MF_ENABLED:MF_GRAYED));
        EnableMenuItem(hm,ID_EXPORT,MF_BYCOMMAND|(g_images.GetSize()? MF_ENABLED:MF_GRAYED));
      }
    break;
#ifdef _WIN32
    case WM_ENDSESSION:
      DestroyWindow(hwndDlg);
    break;
    case WM_QUERYENDSESSION:

      if (SavePromptForClose("Save image list before shutting down?"))
        SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,TRUE);
      else 
        SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,0);

    return TRUE;
#endif
    case WM_COMMAND:
      if (g_aboutwindow_open && LOWORD(wParam) != ID_ABOUT) 
      {
        g_aboutwindow_open=false;
        InvalidateRect(hwndDlg,NULL,FALSE);
      }

      switch (LOWORD(wParam))
      {
        case ID_ABOUT:
          g_aboutwindow_open=!g_aboutwindow_open;
          InvalidateRect(hwndDlg,NULL,FALSE);
        break;
        case ID_LOAD_ADD:
        case ID_LOAD:
          if (LOWORD(wParam) == ID_LOAD_ADD || SavePromptForClose("Save image list before loading?"))
          {
            LoadImageList(LOWORD(wParam) == ID_LOAD_ADD);
          }
        break;
          
        case ID_SAVE:
          SaveImageList(false);
        break;
        case ID_SAVEAS:
          SaveImageList(true);
        break;
        case ID_QUIT:
          if (SavePromptForClose("Save image list before exit?"))
          {
            DestroyWindow(hwndDlg);
          }
        break;
        case ID_NEWLIST:
          if (SavePromptForClose("Save image list before creating new list?"))
          {
            ClearImageList();
            g_imagelist_fn.Set("");
            g_imagelist_fn_dirty=false;

            UpdateMainWindowWithSizeChanged();

            UpdateCaption();
          }
        break;
        case ID_EXPORT:
          if (g_images.GetSize()) DoExportDialog(hwndDlg);
        break;
        case ID_IMPORT:

          {
            static char cwd[4096];

            if (!cwd[0]) config_readstr("cwd",cwd,sizeof(cwd));
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
            l.lpstrTitle = "Add images:";
            l.lpstrDefExt = "jpg";
            l.lpstrInitialDir = cwd;
            l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT|OFN_NOCHANGEDIR;
            if (GetOpenFileName(&l))          
          #else
            char *temp=BrowseForFiles("Add images:", cwd, NULL, true, extlist);
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
              config_writestr("cwd",path.Get());
              lstrcpyn(cwd,path.Get(),sizeof(cwd));

              if (!temp[strlen(temp)+1])
              {
                AddImage(temp);
                UpdateMainWindowWithSizeChanged();
              }
              else
              {
                char *p = temp+strlen(temp)+1;

                if (temp[0] && p[-2]==PREF_DIRCH) p[-2]=0;

                WDL_PtrList<ImageRecord> newimages;
                while (*p)
                {
                  path.Set(temp);
                  path.Append(PREF_DIRSTR);
                  path.Append(p);
                  p+=strlen(p)+1;
                  newimages.Add(new ImageRecord(path.Get()));
                }
                if (newimages.GetSize()>1) qsort(newimages.GetList(),newimages.GetSize(),sizeof(ImageRecord *),ImageRecord::sortByFN);

                g_images_mutex.Enter();
                int x;
                for(x=0;x<newimages.GetSize();x++)
                {
                  ImageRecord *w = newimages.Get(x);
                  g_vwnd.AddChild(w);
                  g_images.Add(w);
                }
                g_images_mutex.Leave();
  
                if (g_fullmode_item && newimages.Get(0)) EnsureImageRecVisible(newimages.Get(0));
                else if (newimages.GetSize()) UpdateMainWindowWithSizeChanged();
              }
  
              SetImageListIsDirty(true);

              free(temp);

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
    case WM_LBUTTONDBLCLK:
      g_vwnd.OnMouseDblClick(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
    return 0;
#ifndef _WIN32 // win32 happens in our message proc
    case WM_KEYDOWN:
    case WM_KEYUP:
      if (wParam == VK_CONTROL)
      {
        if (GetCapture()==hwndDlg)
          InvalidateRect(hwndDlg,NULL,FALSE);
      }
    return 0;
#endif
    case WM_SETCURSOR:
      {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(hwndDlg,&p);
        if (g_vwnd.UpdateCursor(p.x,p.y)>0)
        {
          return TRUE;
        }
        else 
        {
          SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW)));
          return TRUE;
        }
      }
    return 0;
    case WM_LBUTTONDOWN:
      if (g_aboutwindow_open)
      {
        g_aboutwindow_open=false;
        InvalidateRect(hwndDlg,NULL,FALSE);
        return 0;
      }
      KillTooltip(true);
      EditImageLabelEnd();
      SetFocus(hwndDlg);
      if (g_vwnd.OnMouseDown(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)))
      {
        SetCapture(hwndDlg);
      }
    return 0;
    case WM_MOUSEMOVE:
      if (GetCapture()==hwndDlg)
      {
        g_vwnd.OnMouseMove(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
      }
      else
      {
        KillTooltip(true);
        SetTimer(g_hwnd,TOOLTIP_TIMER,TOOLTIP_TIMEOUT,NULL);
      }

    return 0;
    case WM_LBUTTONUP:
      if (GetCapture()==hwndDlg)
      {
        g_vwnd.OnMouseUp(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
        ReleaseCapture();
      }
      KillTooltip(true);
    return 0;
    case WM_PAINT:
      {
        RECT r;
        GetClientRect(hwndDlg,&r);
        g_vwnd.SetPosition(&r);
        g_hwnd_painter.PaintBegin(hwndDlg,RGB(0,0,0));

        if (!g_images.GetSize()||g_aboutwindow_open)
          DrawAboutWindow(&g_hwnd_painter,r);
        else
        {
          g_hwnd_painter.PaintVirtWnd(&g_vwnd);

          if (g_hwnd_tooltip[0])
          {
            int xo=0,yo=0;
            LICE_IBitmap *bm = g_hwnd_painter.GetBuffer(&xo,&yo);
            POINT p ={ g_hwnd_tooltip_pt.x + xo, g_hwnd_tooltip_pt.y + yo};
            RECT rr = { r.left+xo,r.top+yo,r.right+xo,r.bottom+yo};
            DrawTooltipForPoint(bm,p,&rr,g_hwnd_tooltip);
          }
        }

        g_hwnd_painter.PaintEnd();
      }
    return 0;
    case WM_MOUSEWHEEL:
      if (!g_fullmode_item)
      {
        int l=(short)HIWORD(wParam);
        SetMainScrollPosition(-l/400.0,2);
      }
      else
      {
        // todo some fullmode thing?
      }
    return -1;
    case WM_VSCROLL:
      if (!g_fullmode_item)
      {
        SCROLLINFO si = { sizeof(SCROLLINFO), SIF_POS|SIF_PAGE|SIF_RANGE|SIF_TRACKPOS, 0 };
        CoolSB_GetScrollInfo(hwndDlg, SB_VERT, &si);
        switch (LOWORD(wParam))
        {
          case SB_THUMBTRACK:        
            si.nPos = si.nTrackPos;
          break;
          case SB_LINEUP:
            si.nPos -= 30;
          break;
          case SB_LINEDOWN:
            si.nPos += 30;
          break;
          case SB_PAGEUP:
            si.nPos -= (int)si.nPage;
          break;
          case SB_PAGEDOWN:
            si.nPos += (int)si.nPage;
          break;
          case SB_TOP:
            si.nPos = 0;
          break;
          case SB_BOTTOM:
            si.nPos = si.nMax-(int)si.nPage;
          break;
        }
        if (si.nPos > si.nMax-(int)si.nPage) si.nPos = si.nMax-(int)si.nPage;
        if (si.nPos < 0) si.nPos = 0;
        SetMainScrollPosition(si.nPos);
      }
    return 0;
    case WM_DROPFILES:

      {
        WDL_PtrList<ImageRecord> newimages;
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
            const char *listext = ".snapeaselist";
            if (strlen(buf)>strlen(listext) &&
                !stricmp(buf+strlen(buf)-strlen(listext),listext))
            {
              bool addTo = n!=1 || (GetAsyncKeyState(VK_CONTROL)&0x8000);
              if (addTo ||SavePromptForClose("Save current project before loading new image list?"))
                importImageListFromFile(buf,addTo);
            }
            else if (!LICE_ImageIsSupported(buf))
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
                      newimages.Add(new ImageRecord(tmp.Get()));
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
              newimages.Add(new ImageRecord(buf));
            }
          }
        }

        DragFinish(hDrop);

        if (newimages.GetSize()>1) qsort(newimages.GetList(),newimages.GetSize(),sizeof(ImageRecord *),ImageRecord::sortByFN);

        g_images_mutex.Enter();
        for(x=0;x<newimages.GetSize();x++)
        {
          ImageRecord *w = newimages.Get(x);
          g_vwnd.AddChild(w);
          g_images.Add(w);
        }
        g_images_mutex.Leave();

        if (newimages.GetSize())
          SetImageListIsDirty(true);

        if (g_fullmode_item && newimages.Get(0)) EnsureImageRecVisible(newimages.Get(0));
        else
        {
          if (newimages.GetSize())
            UpdateMainWindowWithSizeChanged();
        }
      }

    return 0;
  }

  return 0;
}


int MainProcessMessage(MSG *msg)
{
  int a = EditImageProcessMessage(msg);
  if (a) return a;;

  if (msg->hwnd == g_hwnd||IsChild(g_hwnd,msg->hwnd))
  {
    if (msg->wParam == VK_CONTROL && (msg->message == WM_KEYDOWN || msg->message == WM_KEYUP))
    {
      // win32-only, OSX checks in wndproc
      if (GetCapture()==g_hwnd) InvalidateRect(g_hwnd,NULL,FALSE);
    }
    if (msg->message == WM_KEYDOWN||msg->message==WM_CHAR)
    {
      // todo: kbd table etc? :)
      if (msg->wParam == 'A' || (msg->wParam == VK_INSERT && msg->message == WM_KEYDOWN))
      {
        if (!(GetAsyncKeyState(VK_CONTROL)&0x8000) && 
            !(GetAsyncKeyState(VK_MENU)&0x8000) &&
            !(GetAsyncKeyState(VK_SHIFT)&0x8000))
        {
          if (!GetCapture()) SendMessage(g_hwnd,WM_COMMAND,ID_IMPORT,0);
          return 1;
        }
      }
      else if (msg->wParam == 'Q')
      {
        if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && 
            !(GetAsyncKeyState(VK_MENU)&0x8000) &&
            !(GetAsyncKeyState(VK_SHIFT)&0x8000))
        {
          if (!GetCapture()) SendMessage(g_hwnd,WM_COMMAND,ID_QUIT,0);
          return 1;
        }
      }
      else if (msg->wParam == 'N')
      {
        if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && 
            !(GetAsyncKeyState(VK_MENU)&0x8000) &&
            !(GetAsyncKeyState(VK_SHIFT)&0x8000))
        {
          if (!GetCapture()) SendMessage(g_hwnd,WM_COMMAND,ID_NEWLIST,0);
          return 1;
        }
      }
      else if (msg->wParam == 'E')
      {
        if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && 
            !(GetAsyncKeyState(VK_MENU)&0x8000) &&
            !(GetAsyncKeyState(VK_SHIFT)&0x8000))
        {
          if (!GetCapture()) SendMessage(g_hwnd,WM_COMMAND,ID_EXPORT,0);
          return 1;
        }
      }      else if (msg->wParam == 'O')
      {
        if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && 
            !(GetAsyncKeyState(VK_MENU)&0x8000))
        {
          if (!GetCapture()) SendMessage(g_hwnd,WM_COMMAND,(GetAsyncKeyState(VK_SHIFT)&0x8000) ? ID_LOAD_ADD : ID_LOAD,0);
          return 1;
        }
      }
      else if (msg->wParam == 'S')
      {
        if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && 
            !(GetAsyncKeyState(VK_MENU)&0x8000))
        {
          if (!GetCapture()) SendMessage(g_hwnd,WM_COMMAND,(GetAsyncKeyState(VK_SHIFT)&0x8000) ? ID_SAVEAS : ID_SAVE,0);
          return 1;
        }
      }

      if (msg->wParam == VK_HOME || msg->wParam == VK_END)
      {
        int a = msg->wParam == VK_HOME ? 0 : g_images.GetSize()-1;
        if (g_images.Get(a)) EnsureImageRecVisible(g_images.Get(a));
        return 1;
      }


      if (g_fullmode_item)
      {
        if (msg->wParam == VK_ESCAPE)
        {
          RemoveFullItemView(true);
          return 1;
        }

        if (msg->wParam == VK_NEXT || msg->wParam == VK_PRIOR)
        {
          int a = g_images.Find(g_fullmode_item);
          if (a>=0)
          {
            a += (msg->wParam == VK_PRIOR) ? -1 : 1;
            if (g_images.Get(a)) EnsureImageRecVisible(g_images.Get(a));
          }
          return 1;
        }
      }
      else
      {
        if (msg->wParam == VK_NEXT || msg->wParam == VK_PRIOR)
        {
          SetMainScrollPosition(msg->wParam == VK_NEXT ? 0.5 : -0.5, 2);
          return 1;
        }
      }

    }
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
extern "C" void *GetIconThemePointer(const char *name)
{
  if (!strcmp(name,"scrollbar"))
  {
#ifndef _WIN32
    static LICE_IBitmap *sb;
    if (!sb) sb = LoadThemeElement(0,"scrollbar.png");
    return &sb;
#endif
  }
  return NULL;
}

extern "C" int GSC_mainwnd(int p) { return GetSysColor(p); }

