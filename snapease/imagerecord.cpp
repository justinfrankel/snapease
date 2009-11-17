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
  BUTTONID_CROP,
  BUTTONID_END
};

static WDL_VirtualIconButton_SkinConfig *GetButtonIcon(int idx, char state=0);

ImageRecord::ImageRecord(const char *fn)
{
  m_crop_active=false;
  memset(&m_croprect,0,sizeof(m_croprect));
  m_fullimage=0;
  m_want_fullimage=0;
  m_bw=false;
  m_rot=0;
  m_state=IR_STATE_NEEDLOAD;
  m_preview_image=NULL;
  m_srcimage_w=m_srcimage_h=0;
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
  for(x=BUTTONID_BASE;x<BUTTONID_END;x++)
  {
    WDL_VirtualIconButton *b = new WDL_VirtualIconButton;
    b->SetID(x);
    char st=0;
    switch (x)
    {
      case BUTTONID_BW: st = m_bw; break;
      case BUTTONID_CROP: st = m_crop_active; break;
    }
    b->SetIcon(GetButtonIcon(x,st));
    AddChild(b);
  }
}

ImageRecord::~ImageRecord()
{
  delete m_preview_image;
  delete m_fullimage;
}






LICE_CachedFont g_imagerecord_font;

static int GetButtonSize(int idx)
{
  switch (idx)
  {
    case BUTTONID_CLOSE: return 10;
    case BUTTONID_ROTCCW:
    case BUTTONID_ROTCW: 
    case BUTTONID_CROP:
    case BUTTONID_BW: return 16;
  }
  return 16;
}

static WDL_VirtualIconButton_SkinConfig *GetButtonIcon(int idx, char state)
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
    case BUTTONID_CROP:
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
              LICE_FillRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(64,250,64,128),1.0f,LICE_BLIT_MODE_COPY);
            if (state)
              LICE_FillRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(0,255,0,255),0.5f,LICE_BLIT_MODE_COPY);

            LICE_DrawRect(img->image,sz*x,0,sz-1,sz-1,LICE_RGBA(255,255,255,128),1.0f,LICE_BLIT_MODE_COPY);

            int edg  = 3;
            int col = x==2 ? LICE_RGBA(255,255,255,128) :LICE_RGBA(255,255,255,128);
            LICE_Line(img->image,sz*x + sz/2 - edg, edg, sz*x + sz/2 - edg, sz - edg -1,col,1.0f,LICE_BLIT_MODE_COPY,TRUE);
            LICE_Line(img->image,sz*x + sz/2 + edg - 1, edg, sz*x + sz/2 + edg -1, sz - edg -1,col,1.0f,LICE_BLIT_MODE_COPY,TRUE);

            LICE_Line(img->image,sz*x + edg, sz/2 - edg, sz*x + sz  -1 -edg, sz/2 - edg,col,1.0f,LICE_BLIT_MODE_COPY,TRUE);
            LICE_Line(img->image,sz*x + edg, sz/2 + edg - 1, sz*x + sz  -1 -edg, sz/2 + edg - 1,col,1.0f,LICE_BLIT_MODE_COPY,TRUE);

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
    break;
  }
  return NULL;
}




INT_PTR ImageRecord::SendCommand(int command, INT_PTR parm1, INT_PTR parm2, WDL_VWnd *src)
{
  if (command == WM_COMMAND && src)
  {
    switch (src->GetID())
    {
      case BUTTONID_CROP:
        m_crop_active=!m_crop_active;
        ((WDL_VirtualIconButton *)src)->SetIcon(GetButtonIcon(src->GetID(),!!m_crop_active));
        RequestRedraw(NULL);
      break;

      case BUTTONID_BW:
        m_bw=!m_bw;
        ((WDL_VirtualIconButton *)src)->SetIcon(GetButtonIcon(src->GetID(),!!m_bw));
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
            if (!RemoveFullItemView())
            {
              g_images_mutex.Enter();
              g_images.Delete(g_images.Find(this));
              g_images_mutex.Leave();

              par->RemoveChild(this,true);
              // do nothing after this, "this" not valid anymore!
              UpdateMainWindowWithSizeChanged();
            }
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
    for (x = BUTTONID_BASE; x < BUTTONID_END; x ++)
    {
      WDL_VWnd *b = GetChildByID(x);
      if (b)
      {
        int sz= GetButtonSize(x);
        RECT tr={rightpos - sz, toppos, rightpos, toppos+sz};
        b->SetPosition(&tr);
        rightpos -= sz + 4;
        if (x == BUTTONID_CLOSE) rightpos -= 15;
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



void ImageRecord::SetCropRectFromScreen(int w, int h, const RECT *cr)
{
  if (w<1) w=1;
  if (h<1) h=1;

  int sw=m_srcimage_w;
  int sh=m_srcimage_h;
  RECT tr;

  switch (m_rot & 3)
  {
    case 0:
      tr.left = (cr->left * (double)sw) / w + 0.5;
      tr.right = (cr->right * (double)sw) / w + 0.5;
      tr.top = (cr->top * (double)sh) / h + 0.5;
      tr.bottom = (cr->bottom * (double)sh) /h + 0.5;
    break;
    case 1: // 90cw
      tr.left = (cr->top * (double)sw) / h + 0.5;
      tr.right = (cr->bottom * (double)sw) / h + 0.5;
      tr.top = sh - (cr->right * (double)sh) / w + 0.5;
      tr.bottom = sh - (cr->left * (double)sh) / w + 0.5;
    break;
    case 2:
      tr.left = (sw - (cr->right * (double)sw) / w) + 0.5;
      tr.right = (sw - (cr->left * (double)sw) / w) + 0.5;
      tr.top = (sh - (cr->bottom * (double)sh) / h) + 0.5;
      tr.bottom = (sh -  (cr->top * (double)sh) /h) + 0.5;
    break;
    case 3: // 90ccw
      tr.left = sw - (cr->bottom * (double)sw) / h + 0.5;
      tr.right = sw - (cr->top * (double)sw) / h + 0.5;
      tr.top = (cr->left * (double)sh) / w + 0.5;
      tr.bottom = (cr->right * (double)sh) / w + 0.5;
    break;
  }

  if (tr.left<0) tr.left=0; else if (tr.left > sw) tr.left = sw;
  if (tr.right<0) tr.right=0; else if (tr.right > sw) tr.right = sw;
  if (tr.top<0) tr.top=0; else if (tr.top > sh) tr.top = sh;
  if (tr.bottom<0) tr.bottom=0; else if (tr.bottom > sh) tr.bottom = sh;

  m_croprect = tr;
}


void ImageRecord::GetCropRectForScreen(int w, int h, RECT *cr)
{
  RECT inrect = m_croprect;
  if (inrect.left >= inrect.right)
  {
    inrect.right = m_srcimage_w;
    if (inrect.left >= inrect.right) inrect.left=0;
  }
  if (inrect.top >= inrect.bottom)
  {
    inrect.bottom =m_srcimage_h;
    if (inrect.top >= inrect.bottom) inrect.top=0;
  }

  // rotate
  int sw=max(m_srcimage_w,1);
  int sh=max(m_srcimage_h,1);
  switch  (m_rot&3)
  {
    case 0: 
      cr->left = (inrect.left * (double)w) / sw + 0.5;
      cr->top = (inrect.top * (double)h) / sh + 0.5;
      cr->right = (inrect.right * (double)w) / sw + 0.5;
      cr->bottom = (inrect.bottom * (double)h) / sh + 0.5;      
    break;
    case 1: // 90cw
      cr->left = w - (inrect.bottom * (double)w) / sh + 0.5;
      cr->right = w - (inrect.top * (double)w) / sh + 0.5;
      cr->top = (inrect.left * (double)h) / sw + 0.5;
      cr->bottom = (inrect.right * (double)h) / sw + 0.5;
    break;
    case 2: // 180
      cr->left = (w - (inrect.right * (double)w) / sw) + 0.5;
      cr->top = (h - (inrect.bottom * (double)h) / sh) + 0.5;
      cr->right = (w - (inrect.left * (double)w) / sw) + 0.5;
      cr->bottom = (h - (inrect.top * (double)h) / sh) + 0.5;      
    break;
    case 3: // 90ccw
      cr->left = (inrect.top * (double)w) / sh + 0.5;
      cr->right = (inrect.bottom * (double)w) / sh + 0.5;
      cr->top = h - (inrect.right * (double)h) / sw + 0.5;
      cr->bottom = h - (inrect.left * (double)h) / sw + 0.5;
    break;

  }

//  if (cr->left>cr->right) { int a=cr->left; cr->left=cr->right; cr->right=a; }
  //if (cr->top>cr->bottom) { int a=cr->top; cr->top=cr->bottom; cr->bottom=a; }

  // crop 
  if (cr->left<0) cr->left=0; else if (cr->left > w) cr->left = w;
  if (cr->right<0) cr->right=0; else if (cr->right > w) cr->right = w;
  if (cr->top<0) cr->top=0; else if (cr->top > h) cr->top = h;
  if (cr->bottom<0) cr->bottom=0; else if (cr->bottom > h) cr->bottom = h;
}


enum
{
  LOCAL_CAP_NIL=-1000,
  LOCAL_CAP_CROP,
  LOCAL_CAP_CROPMOVE,

};
int ImageRecord::OnMouseDown(int xpos, int ypos)
{
  int a = WDL_VWnd::OnMouseDown(xpos,ypos);
  if (!a && m_crop_active)
  {
    m_crop_capmode = 0;
    int f=0;
    if (ypos >= m_last_crop_drawrect.top-2 && ypos <= m_last_crop_drawrect.bottom+2)
    {
      if (xpos >= m_last_crop_drawrect.left-2 && xpos <= m_last_crop_drawrect.left+2) m_crop_capmode|=1;
      else if (xpos >= m_last_crop_drawrect.right-2 && xpos <= m_last_crop_drawrect.right+2) m_crop_capmode|=4;
      else f|=1;
    }
    if (xpos >= m_last_crop_drawrect.left-2 && xpos <= m_last_crop_drawrect.right+2)
    {
      if (ypos >= m_last_crop_drawrect.top-2 && ypos <= m_last_crop_drawrect.top+2) m_crop_capmode|=2;
      else if (ypos >= m_last_crop_drawrect.bottom-2 && ypos <= m_last_crop_drawrect.bottom+2) m_crop_capmode|=8;
      else f|=2;
    }

    if (f==3)  m_crop_capmode=0xf;
    m_crop_capmode_lastpos.x = xpos - ((m_crop_capmode & 1) ? m_last_crop_drawrect.left : m_last_crop_drawrect.right);
    m_crop_capmode_lastpos.y = ypos - ((m_crop_capmode & 2) ? m_last_crop_drawrect.top : m_last_crop_drawrect.bottom);
    
    if (m_crop_capmode)
    {
      m_captureidx = LOCAL_CAP_CROP;
      return 1;
    }
    // hit test
  }
  return a;
}

void ImageRecord::OnMouseMove(int xpos, int ypos)
{
  switch (m_captureidx)
  {
    case LOCAL_CAP_NIL:
    return;
    case LOCAL_CAP_CROP:
      if (m_crop_capmode)
      {
        RECT r;
        GetCropRectForScreen(m_last_drawrect.right-m_last_drawrect.left,m_last_drawrect.bottom-m_last_drawrect.top,&r);

        RECT or = m_croprect;

        if (m_crop_capmode==0xf) // move all
        {
          int dx = (xpos - m_crop_capmode_lastpos.x) - r.left  - m_last_drawrect.left;
          if (r.left+dx<0) dx=-r.left;
          else if (r.right+dx>m_last_drawrect.right-m_last_drawrect.left) dx = m_last_drawrect.right-m_last_drawrect.left - r.right;

          int dy = (ypos - m_crop_capmode_lastpos.y) - r.top - m_last_drawrect.top;
          if (r.top+dy<0) dy=-r.top;
          else if (r.bottom+dy>m_last_drawrect.bottom-m_last_drawrect.top) dy = m_last_drawrect.bottom-m_last_drawrect.top- r.bottom;

          r.left += dx;
          r.top += dy;
          r.right += dx;
          r.bottom += dy;
        }
        else
        {
          if (m_crop_capmode&1) r.left = xpos - m_crop_capmode_lastpos.x - m_last_drawrect.left;
          if (m_crop_capmode&4) r.right = xpos - m_crop_capmode_lastpos.x - m_last_drawrect.left;
          if (m_crop_capmode&2) r.top = ypos - m_crop_capmode_lastpos.y - m_last_drawrect.top;
          if (m_crop_capmode&8) r.bottom = ypos - m_crop_capmode_lastpos.y - m_last_drawrect.top;
        }
        SetCropRectFromScreen(m_last_drawrect.right-m_last_drawrect.left,m_last_drawrect.bottom-m_last_drawrect.top,&r);

        if (memcmp(&or,&m_croprect,sizeof(RECT))) RequestRedraw(NULL);
      }
    return;
  }

  WDL_VWnd::OnMouseMove(xpos,ypos);
}

void ImageRecord::OnMouseUp(int xpos, int ypos)
{
  m_captureidx= -1;
  switch (m_captureidx)
  {
    case LOCAL_CAP_NIL:
      m_captureidx= -1;
    return;
    case LOCAL_CAP_CROP:

      // todo: notify that edit finished?
      m_captureidx= -1;

    return; 
  }
  WDL_VWnd::OnMouseUp(xpos,ypos);
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

  LICE_IBitmap *srcimage = (m_fullimage &&!m_want_fullimage) ? m_fullimage : m_preview_image;
  if (srcimage)
  {
    int cropw=0,croph=0,cropl=0,cropt=0;

    if (!m_crop_active && m_croprect.right > m_croprect.left && m_croprect.bottom > m_croprect.top)
    {
      // scale coordinates to match this image

      cropl = (m_croprect.left * srcimage->getWidth()) / max(m_srcimage_w,1);
      cropt = (m_croprect.top * srcimage->getHeight()) / max(m_srcimage_h,1);
      cropw = ((m_croprect.right * srcimage->getWidth()) / max(m_srcimage_w,1)) - cropl;
      croph = ((m_croprect.bottom * srcimage->getHeight()) / max(m_srcimage_h,1)) - cropt;

      if (cropw < 1) cropw=1;
      if (croph < 1) croph=1;

    }

    LICE_SubBitmap subbm(srcimage, cropl,cropt,cropw,croph); //unused if not cropping
    if (cropw>0&&croph>0)srcimage=&subbm;

    int srcw=srcimage->getWidth();
    int srch=srcimage->getHeight();


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

    int yoffs = (desth-h)/2 + r.top + 2;
    int xoffs = (destw-w)/2 + r.left + 2;

    m_last_drawrect.left=xoffs - r.top;
    m_last_drawrect.top = yoffs - r.left;
    m_last_drawrect.right = m_last_drawrect.left + w;
    m_last_drawrect.bottom = m_last_drawrect.top + h;


    // todo: cache scaled/rotated version in global cache if srcimage == m_fullimage?

    if (!rot)
      LICE_ScaledBlit(drawbm,srcimage,xoffs,yoffs,w,h,0,0,srcw,srch,1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
    else
    {

      double dsdx=0, dsdy=0,dtdx=0,dtdy=0;

      int sx = rot != 1 ? srcimage->getWidth() - 1 : 0;
      int sy = rot != 3 ? srcimage->getHeight() - 1 : 0;

      if (rot!=2)
      {
        dtdx = srcimage->getHeight() / (double) w;
        dsdy = srcimage->getWidth() / (double) h;
        if (rot==1) dtdx=-dtdx;
        else dsdy=-dsdy;
      }
      else // flip
      {
        dsdx=-srcimage->getWidth() / (double) w;
        dtdy=-srcimage->getHeight() / (double) h;
      }

      LICE_DeltaBlit(drawbm,srcimage,xoffs,yoffs,w,h,
                sx,sy, // start x,y
            srcimage->getWidth(),srcimage->getHeight(),
              dsdx, dtdx,
              dsdy, dtdy,
              0,0,false,1.0f,LICE_BLIT_MODE_COPY);

    }
    if (m_bw)
    {
      LICE_ProcessRect(drawbm,xoffs,yoffs,w,h,makeBWFunc,NULL);
      //LICE_FillRect(drawbm,xoffs,yoffs,w,h,LICE_RGBA(128,0,128,255),1.0f,LICE_BLIT_MODE_HSVADJ);
    }


    if (m_crop_active)
    {
#if 0
        m_croprect.left=100;
        m_croprect.top=100;
        m_croprect.right=400;
        m_croprect.bottom=400;
#endif
      RECT cr;
      GetCropRectForScreen(w,h,&cr);

      cr.left += xoffs;
      cr.right += xoffs;
      cr.bottom += yoffs;
      cr.top += yoffs;

      m_last_crop_drawrect = cr;
      m_last_crop_drawrect.top -= r.top;
      m_last_crop_drawrect.bottom -= r.top;
      m_last_crop_drawrect.left -= r.left;
      m_last_crop_drawrect.right -= r.left;

      float al = 0.85f;
      LICE_pixel col = 0;
      if (cr.left > xoffs) LICE_FillRect(drawbm,xoffs,yoffs,cr.left-xoffs,h,col,al,LICE_BLIT_MODE_COPY);
      if (cr.right < xoffs+w)  LICE_FillRect(drawbm,cr.right,yoffs,xoffs+w - cr.right,h,col,al,LICE_BLIT_MODE_COPY);

      int vsw=min(xoffs+w,cr.right)-max(xoffs,cr.left),vsx=max(xoffs,cr.left);
      if (cr.top > yoffs) LICE_FillRect(drawbm,vsx,yoffs,vsw,cr.top - yoffs,col,al,LICE_BLIT_MODE_COPY);
      if (cr.bottom < yoffs+h) LICE_FillRect(drawbm,vsx,cr.bottom,vsw,yoffs+h-cr.bottom,col,al,LICE_BLIT_MODE_COPY);

      col=LICE_RGBA(128,128,128,255);
      al = 0.25f;
      LICE_Line(drawbm,cr.left,yoffs,cr.left,yoffs+h,col,al,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,cr.right,yoffs,cr.right,yoffs+h,col,al,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,xoffs,cr.top,xoffs+w,cr.top,col,al,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,xoffs,cr.bottom,xoffs+w,cr.bottom,col,al,LICE_BLIT_MODE_COPY,false);

      LICE_DrawRect(drawbm,cr.left,cr.top,cr.right-cr.left,cr.bottom-cr.top,LICE_RGBA(255,255,255,255),1.0f,LICE_BLIT_MODE_COPY);

    }

  }
  g_imagerecord_font.SetTextColor(LICE_RGBA(255,255,255,0));
  g_imagerecord_font.SetEffectColor(LICE_RGBA(0,0,0,0));

  if (m_state != IR_STATE_LOADED && !srcimage)
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
