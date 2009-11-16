#include "main.h"

#include "../WDL/lice/lice.h"
#include "../WDL/lice/lice_text.h"

#include "imagerecord.h"

LICE_CachedFont g_imagerecord_font;

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
  LICE_DrawRect(drawbm,origin_x+r.left,origin_y+r.top,r.right-r.left,r.bottom-r.top,LICE_RGBA(32,32,32,32),1.0f,LICE_BLIT_MODE_COPY);
}
