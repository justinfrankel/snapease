/*
    SnapEase
    label_edit.cpp -- code for managing renaming of images
    Copyright (C) 2009 and onward Cockos Incorporated

    SnapEase is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    SnapEase is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SnapEase; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "main.h"

#ifndef _WIN32
#include "../WDL/swell/swell-dlggen.h"
#endif
static HWND s_wnd;
static ImageRecord *s_rec;

int EditImageProcessMessage(MSG *msg)
{
  if (s_wnd && (msg->hwnd == s_wnd || IsChild(s_wnd,msg->hwnd)))
  {
    if (msg->message == WM_KEYDOWN || msg->message == WM_CHAR)
    {
      if (msg->wParam == VK_RETURN||msg->wParam==VK_ESCAPE)
      {
        EditImageLabelEnd(msg->wParam==VK_ESCAPE);
        return 1;
      }
      else if (msg->wParam == VK_TAB)
      {
        int a = g_images.Find(s_rec);
        if (a>=0)
        {
          a += (GetAsyncKeyState(VK_SHIFT)&0x8000) ? -1 : 1;

          if (g_images.Get(a)) EditImageLabel(g_images.Get(a));
        }

        return 1;
      }
    }
    return -1;
  }
  return 0;
}

void EditImageRunTimer()
{
  if (s_wnd)
  {
    HWND foc = GetFocus();
    if (!foc || (foc != s_wnd && !IsChild(s_wnd,foc)))
      EditImageLabelEnd();
  }
}

void EditImageLabel(ImageRecord *rec)
{
  EditImageLabelEnd();

  if (!rec) return;

  EnsureImageRecVisible(rec);

  UpdateWindow(g_hwnd); // make sure our rect is valid

  s_rec=rec;

  RECT r;
  rec->GetPosition(&r);
  RECT tr=rec->m_lastlbl_rect;
  
  tr.left=r.left;
  tr.right=r.right;
  tr.top += r.top;
  tr.bottom += r.top;
  tr.top -= 4;


#ifdef _WIN32
  s_wnd = CreateWindowEx(WS_EX_NOPARENTNOTIFY,"Edit","",
      WS_CHILDWINDOW|WS_TABSTOP|ES_CENTER|ES_AUTOHSCROLL|ES_WANTRETURN,
      tr.left,tr.top,tr.right-tr.left,tr.bottom-tr.top,
      g_hwnd,NULL,g_hInst,0);
  SendMessage(s_wnd,WM_SETFONT,SendMessage(g_hwnd,WM_GETFONT,0,0),0);
#else
  SWELL_MakeSetCurParms(1,1,0,0,g_hwnd,false,false);
  s_wnd = SWELL_MakeEditField(0,tr.left,tr.top,tr.right-tr.left,tr.bottom-tr.top,ES_AUTOHSCROLL|ES_WANTRETURN);
  SWELL_MakeSetCurParms(1,1,0,0,NULL,false,false);
  
#endif
  SetWindowText(s_wnd,rec->m_outname.Get());

  SendMessage(s_wnd,EM_SETSEL,0,-1);
  ShowWindow(s_wnd,SW_SHOW);
  SetFocus(s_wnd);

}

void EditImageLabelEnd(bool ignoreData)
{
  if (s_wnd)
  {
    if (!ignoreData)
    {
      char buf[1024];
      buf[0]=0;
      GetWindowText(s_wnd,buf,sizeof(buf));
      if (s_rec)
      {
        if (buf[0]) s_rec->m_outname.Set(buf);
        else s_rec->SetDefaultTitle();
      }
      s_rec=0;
    }
    HWND ow = s_wnd;
    s_wnd=0;
    DestroyWindow(ow);
    InvalidateRect(g_hwnd,NULL,FALSE);
  }
  else s_rec=0;
}
