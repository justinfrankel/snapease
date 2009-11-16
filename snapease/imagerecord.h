#ifndef _IMAGERECORD_H_
#define _IMAGERECORD_H_

#include "../WDL/wdlstring.h"

class ImageRecord : public WDL_VWnd
{
public:
  ImageRecord(const char *srcfn);
  ~ImageRecord();

  ////// WDL_VWnd impl

  virtual void OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);

  //////

  WDL_String m_fn;
  WDL_String m_outname;

  LICE_IBitmap *m_preview_image;
};


#endif