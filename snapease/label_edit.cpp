#include "main.h"

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


  s_wnd = CreateWindowEx(WS_EX_NOPARENTNOTIFY,"Edit","",
      WS_CHILDWINDOW|WS_TABSTOP|ES_CENTER|ES_AUTOHSCROLL|ES_WANTRETURN,
      tr.left,tr.top,tr.right-tr.left,tr.bottom-tr.top,
      g_hwnd,NULL,g_hInst,0);
  SendMessage(s_wnd,WM_SETFONT,SendMessage(g_hwnd,WM_GETFONT,0,0),0);
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
    }
    DestroyWindow(s_wnd);
    InvalidateRect(g_hwnd,NULL,FALSE);
  }
  s_rec=0;
}