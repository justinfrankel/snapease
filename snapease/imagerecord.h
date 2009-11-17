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
  virtual int OnMouseDown(int xpos, int ypos);
  virtual void OnMouseMove(int xpos, int ypos);
  virtual void OnMouseUp(int xpos, int ypos);

  //////

  void GetCropRectForScreen(int w, int h, RECT *cr); // scales + rotates m_croprect for output to w,h
  void SetCropRectFromScreen(int w, int h, const RECT *cr);

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

  RECT m_last_drawrect; // set by drawing
  RECT m_last_crop_drawrect; // set by drawing, read by UI code
  int m_crop_capmode; // 1=left,2=top,4=right,8=bottom, only certain combinations will be used, obviously
  POINT m_crop_capmode_lastpos; // offsets from actual pt

  static int sortByFN(const void *a, const void *b)
  {
    ImageRecord *r1 = *(ImageRecord**)a;
    ImageRecord *r2 = *(ImageRecord**)b;
    return stricmp(r1->m_fn.Get(),r2->m_fn.Get());
  }
};


#endif