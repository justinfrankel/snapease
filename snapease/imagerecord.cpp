#include "main.h"

#include "../WDL/lice/lice.h"
#include "../WDL/lice/lice_text.h"

#include "imagerecord.h"

LICE_CachedFont g_imagerecord_font;

#define DESIRED_PREVIEW_CACHEDIM 192

ImageRecord::ImageRecord(const char *fn)
{
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


  static LICE_MemBitmap bm;

  LICE_IBitmap *b = LICE_LoadImage(m_fn.Get(),&bm,false);

  if (b)
  {
    int outw = b->getWidth();
    int outh = b->getHeight();
    if (outw > DESIRED_PREVIEW_CACHEDIM)
    {
      outh = (outh * DESIRED_PREVIEW_CACHEDIM) / outw;
      outw=DESIRED_PREVIEW_CACHEDIM;
    }
    if (outh > DESIRED_PREVIEW_CACHEDIM)
    {
      outw = (outw * DESIRED_PREVIEW_CACHEDIM) / outh;
      outh = DESIRED_PREVIEW_CACHEDIM;
    }
    if (outw > 0 && outh > 0)
    {
      m_preview_image = new LICE_MemBitmap(outw,outh);
      LICE_ScaledBlit(m_preview_image,b,0,0,outw,outh,0,0,b->getWidth(),b->getHeight(),1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
    }

  }
  // load image etc
}

ImageRecord::~ImageRecord()
{
  delete m_preview_image;
}

void ImageRecord::OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect)
{
  if (!g_imagerecord_font.GetHFont())
  {
    static LOGFONT lf = 
    {
        14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,
	    #ifdef _WIN32
        "MS Shell Dlg"
	    #else
	    "Arial"
	    #endif
    };

    g_imagerecord_font.SetFromHFont(CreateFontIndirect(&lf),LICE_FONT_FLAG_OWNS_HFONT);
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


    LICE_ScaledBlit(drawbm,m_preview_image,r.left+xoffs + 2,r.top+yoffs + 2,w,h,0,0,srcw,srch,1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
  }

  g_imagerecord_font.SetTextColor(LICE_RGBA(255,255,255,0));
  g_imagerecord_font.SetCombineMode(LICE_BLIT_MODE_COPY,0.25f);
  g_imagerecord_font.DrawText(drawbm,m_outname.Get(),-1,&r,DT_BOTTOM|DT_CENTER|DT_SINGLELINE);

}
