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
  virtual void SetPosition(const RECT *r);
  virtual INT_PTR SendCommand(int command, INT_PTR parm1, INT_PTR parm2, WDL_VWnd *src);

  //////

  void GetCropRectForScreen(int w, int h, RECT *cr); // scales + rotates m_croprect for output to w,h

  ///

  WDL_String m_fn;
  WDL_String m_outname;

#define IR_STATE_NEEDLOAD 0
#define IR_STATE_DECODING 1
#define IR_STATE_LOADED 2
#define IR_STATE_ERROR 3

  int m_state;
  LICE_IBitmap *m_preview_image;
  int m_srcimage_w, m_srcimage_h;

  bool m_want_fullimage;
  LICE_IBitmap *m_fullimage;


  bool m_bw;
  char m_rot; // 90deg steps (0..3)
  bool m_crop_active;

  RECT m_croprect;

  static int sortByFN(const void *a, const void *b)
  {
    ImageRecord *r1 = *(ImageRecord**)a;
    ImageRecord *r2 = *(ImageRecord**)b;
    return stricmp(r1->m_fn.Get(),r2->m_fn.Get());
  }
};


#endif