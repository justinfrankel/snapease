#include "main.h"

#include "../WDL/lice/lice.h"
#include "../WDL/lice/lice_text.h"
#include "../WDL/wingui/virtwnd-controls.h"

#include "imagerecord.h"

enum
{

  BUTTONID_BASE=1000,
  BUTTONID_CLOSE=BUTTONID_BASE,
  BUTTONID_ROTCW,
  BUTTONID_ROTCCW,
  BUTTONID_BW,
  BUTTONID_END
};
#define NUM_BUTTONS (BUTTONID_END-BUTTONID_BASE)


LICE_CachedFont g_imagerecord_font;

static int GetButtonSize(int idx)
{
  switch (idx)
  {
    case BUTTONID_CLOSE: return 10;
    case BUTTONID_ROTCCW:
    case BUTTONID_ROTCW: return 16;
    case BUTTONID_BW: return 16;
  }
  return 16;
}

static WDL_VirtualIconButton_SkinConfig *GetButtonIcon(int idx, char state=0)
{
  int sz = GetButtonSize(idx);
  switch (idx)
  {
    case BUTTONID_BW:
      {
        static WDL_VirtualIconButton_SkinConfig img_[2];
        WDL_VirtualIconButton_SkinConfig *img = &img_[!!state];

        if (!img->image)
        {
          img->image = new LICE_MemBitmap(sz*3,sz);
          LICE_Clear(img->image,LICE_RGBA(0,0,0,64));
          int x;
          for(x=0;x<3;x++)
          {
            if (x==2)
              LICE_FillRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(0,64,64,128),1.0f,LICE_BLIT_MODE_COPY);
            if (state)
              LICE_FillRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(0,128,192,255),0.65f,LICE_BLIT_MODE_COPY);

            LICE_DrawRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(255,255,255,128),1.0f,LICE_BLIT_MODE_COPY);

            int col = x==2 ? LICE_RGBA(255,255,255,128) :LICE_RGBA(255,255,255,128);
            LICE_DrawText(img->image,sz*x+sz/2-8/2,sz/2-8/2,"G",col,1.0f,LICE_BLIT_MODE_COPY);

          }
        }

        return img;
      }
    return 0;
    case BUTTONID_ROTCCW:
    case BUTTONID_ROTCW:
      {
        static WDL_VirtualIconButton_SkinConfig img_[2];
        WDL_VirtualIconButton_SkinConfig *img = &img_[idx==BUTTONID_ROTCCW];

        if (!img->image)
        {
          img->image = new LICE_MemBitmap(sz*3,sz);
          LICE_Clear(img->image,LICE_RGBA(0,0,0,64));
          int x;
          for(x=0;x<3;x++)
          {
            if (x==2)
              LICE_FillRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(0,64,64,128),1.0f,LICE_BLIT_MODE_COPY);

            LICE_DrawRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(255,255,255,128),1.0f,LICE_BLIT_MODE_COPY);

            int col = x==2 ? LICE_RGBA(255,255,255,128) :LICE_RGBA(255,255,255,128);
            LICE_Arc(img->image,sz*x + sz/2,sz/2,sz/2-4,
              idx==BUTTONID_ROTCCW?-2.0:-2.5,
              idx==BUTTONID_ROTCCW?2.5:2.0,col,1.0f,LICE_BLIT_MODE_COPY,true);

            int x1 = (idx==BUTTONID_ROTCCW) ? sz*x + 3 : sz*(x+1) - 3;
            int arsz=5;
            LICE_Line(img->image,x1, sz/2 + 1, 
                                 x1, sz/2 + 1 - arsz,
                                 col,1.0f,LICE_BLIT_MODE_COPY,true);
            LICE_Line(img->image,x1, sz/2 + 1, 
                                 x1 + (idx==BUTTONID_ROTCCW ? arsz : -arsz), sz/2 + 1,
                                 col,1.0f,LICE_BLIT_MODE_COPY,true);
          }
        }

        return img;
      }
    break;
    case BUTTONID_CLOSE:
      {
        static WDL_VirtualIconButton_SkinConfig img;
        if (!img.image)
        {
          img.image = new LICE_MemBitmap(sz*3,sz);
          LICE_Clear(img.image,LICE_RGBA(0,0,0,64));
          int x;
          for(x=0;x<3;x++)
          {
            if (x==2)
              LICE_FillRect(img.image,sz*x,0,sz-1,sz-1,LICE_RGBA(255,64,64,128),1.0f,LICE_BLIT_MODE_COPY);

            LICE_DrawRect(img.image,sz*x,0,sz-1,sz-1,LICE_RGBA(255,255,255,128),1.0f,LICE_BLIT_MODE_COPY);

            int edg  = 3;
            int col = x==2 ? LICE_RGBA(255,255,255,128) :LICE_RGBA(255,255,255,128);
            LICE_Line(img.image,sz*x + edg, edg, sz*x + sz - edg - 1, sz - edg -1,col,1.0f,LICE_BLIT_MODE_COPY,TRUE);
            LICE_Line(img.image,sz*x + edg, sz - edg - 1, sz*x + sz - edg - 1, edg,col,1.0f,LICE_BLIT_MODE_COPY,TRUE);

          }
        }
        return &img;
      }
  }
  return NULL;
}


ImageRecord::ImageRecord(const char *fn)
{
  m_bw=false;
  m_rot=0;
  m_state=IR_STATE_NEEDLOAD;
  m_preview_image=NULL;
  m_fn.Set(fn);
  {
    const char *p = fn;
    while (*p) p++;
    while (p >= fn && *p != '\\' && *p != '/') p--;
    m_outname.Set(++p);
  }
  {
    char *p = m_outname.Get();
    while (*p) p++;
    while (p >= m_outname.Get() && *p != '.') p--;
    if (p > m_outname.Get()) *p=0;
  }

  // add our button children
  int x;
  for(x=0;x<NUM_BUTTONS;x++)
  {
    WDL_VirtualIconButton *b = new WDL_VirtualIconButton;
    b->SetID(x+BUTTONID_BASE);
    char st=0;
    if (x+BUTTONID_BASE == BUTTONID_BW) st = m_bw;
    b->SetIcon(GetButtonIcon(x+BUTTONID_BASE,st));
    AddChild(b);
  }
}

ImageRecord::~ImageRecord()
{
  delete m_preview_image;
}

INT_PTR ImageRecord::SendCommand(int command, INT_PTR parm1, INT_PTR parm2, WDL_VWnd *src)
{
  if (command == WM_COMMAND && src)
  {
    switch (src->GetID())
    {
      case BUTTONID_BW:
        {
          m_bw=!m_bw;
          ((WDL_VirtualIconButton *)src)->SetIcon(GetButtonIcon(src->GetID(),!!m_bw));
        }
        RequestRedraw(NULL);
      break;

      case BUTTONID_ROTCCW:
      case BUTTONID_ROTCW:

        m_rot= (m_rot+ (src->GetID() == BUTTONID_ROTCCW ? -1 : 1 ))&3;

        RequestRedraw(NULL);

      break;
      case BUTTONID_CLOSE:
        {
          WDL_VWnd *par = GetParent();
          if (par)
          {
            g_images_mutex.Enter();
            g_images.Delete(g_images.Find(this));
            g_images_mutex.Leave();

            par->RemoveChild(this,true);
            // do nothing after this, "this" not valid anymore!
            UpdateMainWindowWithSizeChanged();
          }
        }

      break;
    }   
  }
  // dont allow anything upstream
  return 0; // WDL_VWnd::SendCommand(command,parm1,parm2,src);
}

void ImageRecord::SetPosition(const RECT *r)
{
  if ((r->right-r->left) != (m_position.right-m_position.left) ||
      (r->bottom-r->top) != (m_position.bottom-m_position.top))
  {
    // reposition images on size change
    int x;
    int rightpos = (r->right-r->left) - 2;
    int toppos = 2;
    for (x = 0; x < NUM_BUTTONS; x ++)
    {
      WDL_VWnd *b = GetChildByID(x+BUTTONID_BASE);
      if (b)
      {
        int sz= GetButtonSize(x+BUTTONID_BASE);
        RECT tr={rightpos - sz, toppos, rightpos, toppos+sz};
        b->SetPosition(&tr);
        rightpos -= sz + 4;
        if (x+BUTTONID_BASE == BUTTONID_CLOSE) rightpos -= 15;
        if (rightpos < 0)
        {
          rightpos = (r->right-r->left) - 2;
          toppos += sz+4;
        }
      }
    }
  }

  WDL_VWnd::SetPosition(r);
}

static void makeBWFunc(LICE_pixel *p, void *parm)
{
  LICE_pixel pix = *p;
  int v = (LICE_GETR(pix) + LICE_GETG(pix) + LICE_GETB(pix))/3;
  *p = LICE_RGBA(v,v,v,255);
}

void ImageRecord::OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect)
{
  if (!g_imagerecord_font.GetHFont())
  {
    bool doOutLine = true;
    LOGFONT lf = 
    {
        14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
          OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,doOutLine ? NONANTIALIASED_QUALITY : DEFAULT_QUALITY,DEFAULT_PITCH,
	    #ifdef _WIN32
        "MS Shell Dlg"
	    #else
	    "Arial"
	    #endif
    };

    g_imagerecord_font.SetFromHFont(CreateFontIndirect(&lf),LICE_FONT_FLAG_OWNS_HFONT|(doOutLine?LICE_FONT_FLAG_FX_OUTLINE:0));
  }

  RECT r;
  GetPosition(&r);
  r.left += origin_x;
  r.top += origin_y;
  r.right += origin_x;
  r.bottom += origin_y;
  LICE_DrawRect(drawbm,r.left,r.top,r.right-r.left,r.bottom-r.top,LICE_RGBA(32,32,32,32),1.0f,LICE_BLIT_MODE_COPY);

  if (m_preview_image)
  {
    int srcw=m_preview_image->getWidth();
    int srch=m_preview_image->getHeight();


    int rot = m_rot&3;
    if (rot&1)
    {
      int a= srcw;
      srcw=srch;
      srch=a;
    }

    int destw = r.right-r.left-4;
    int desth = r.bottom-r.top-4;
    // center image

    int w=destw,h=desth;
    if (srcw*desth > destw*srch) // src image wider than tall
    {
      h = (destw * srch) / max(srcw,1);
    }
    else
    {
      w = (desth * srcw) / max(srch,1);
    }

    int yoffs = (desth-h)/2;
    int xoffs = (destw-w)/2;


    if (!rot)
      LICE_ScaledBlit(drawbm,m_preview_image,r.left+xoffs + 2,r.top+yoffs + 2,w,h,0,0,srcw,srch,1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
    else
    {

      double dsdx=0, dsdy=0,dtdx=0,dtdy=0;

      int sx = rot != 1 ? m_preview_image->getWidth() - 1 : 0;
      int sy = rot != 3 ? m_preview_image->getHeight() - 1 : 0;

      if (rot!=2)
      {
        dtdx = m_preview_image->getHeight() / (double) w;
        dsdy = m_preview_image->getWidth() / (double) h;
        if (rot==1) dtdx=-dtdx;
        else dsdy=-dsdy;
      }
      else // flip
      {
        dsdx=-m_preview_image->getWidth() / (double) w;
        dtdy=-m_preview_image->getHeight() / (double) h;
      }

      LICE_DeltaBlit(drawbm,m_preview_image,r.left+xoffs+2,r.top+yoffs+2,w,h,
                sx,sy, // start x,y
            m_preview_image->getWidth(),m_preview_image->getHeight(),
              dsdx, dtdx,
              dsdy, dtdy,
              0,0,false,1.0f,LICE_BLIT_MODE_COPY);

    }
    if (m_bw)
    {
      LICE_ProcessRect(drawbm,r.left+xoffs+2,r.top+yoffs+2,w,h,makeBWFunc,NULL);
      //LICE_FillRect(drawbm,r.left+xoffs+2,r.top+yoffs+2,w,h,LICE_RGBA(128,0,128,255),1.0f,LICE_BLIT_MODE_HSVADJ);
    }
  }
  g_imagerecord_font.SetTextColor(LICE_RGBA(255,255,255,0));
  g_imagerecord_font.SetEffectColor(LICE_RGBA(0,0,0,0));

  if (m_state != IR_STATE_LOADED)
  {
    const char *str= "ERROR";
    switch (m_state)
    {
      case IR_STATE_NEEDLOAD:
        str="loading soon";
      break;
      case IR_STATE_DECODING:
        str="decoding now";
      break;
    }

    if (str[0])
    {
      g_imagerecord_font.SetCombineMode(LICE_BLIT_MODE_COPY,1.0f);
      g_imagerecord_font.DrawText(drawbm,str,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE);
    }

  }

  g_imagerecord_font.SetCombineMode(LICE_BLIT_MODE_COPY,0.5f);
  g_imagerecord_font.DrawText(drawbm,m_outname.Get(),-1,&r,DT_BOTTOM|DT_CENTER|DT_SINGLELINE);

  WDL_VWnd::OnPaint(drawbm,origin_x,origin_y,cliprect);
}
