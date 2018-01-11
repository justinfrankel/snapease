/*
    SnapEase
    imagerecord.cpp -- image thumbnail/button/info/etc entry interface
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

#ifndef _IMAGERECORD_H_
#define _IMAGERECORD_H_

#include "../WDL/wdlstring.h"

enum { EDIT_MODE_NONE=0, EDIT_MODE_CROP, EDIT_MODE_TRANSFORM, EDIT_MODE_BCHSV}; 
extern int g_edit_mode;

class ImageRecord : public WDL_VWnd
{
public:
  ImageRecord(const char *srcfn, time_t timestamp=0);
  ~ImageRecord();

  ImageRecord *Duplicate();
  ////// WDL_VWnd impl

  virtual const char *GetType() { return "ImageRecord"; }
  virtual void OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect, int rscale);
  virtual void SetPosition(const RECT *r);
  virtual INT_PTR SendCommand(int command, INT_PTR parm1, INT_PTR parm2, WDL_VWnd *src);
  virtual int OnMouseDown(int xpos, int ypos);
  virtual bool OnMouseDblClick(int xpos, int ypos);
  virtual void OnMouseMove(int xpos, int ypos);
  virtual void OnMouseUp(int xpos, int ypos);
  virtual int UpdateCursor(int xpos, int ypos);
  virtual bool GetToolTipString(int xpos, int ypos, char *bufOut, int bufOutSz);

  //////

  int UserIsDraggingImageToPosition(int *typeOut); // typeOut=0 for none, 1 for move, 2 for copy

  bool ProcessImageToBitmap(LICE_IBitmap *srcimage, LICE_IBitmap *destimage, int max_w, int max_h); // resizes destimage, return false on error

  void SetIsFullscreen(bool isFS);

  void GetCropRectForScreen(int w, int h, RECT *cr); // scales + rotates m_croprect for output to w,h
  bool SetCropRectFromScreen(int w, int h, const RECT *cr); // return true on update

  void GetSizeInfoString(char *buf, int bufsz); // WxH or WxH cropped to WxH

  void SetDefaultTitle();
  void UpdateButtonStates();

  bool ProcessRect(LICE_IBitmap *destimage, int x, int y, int w, int h);
  ///

  WDL_FastString m_fn;
  WDL_FastString m_outname;

  enum { IR_STATE_NEEDLOAD=0, IR_STATE_DECODING, IR_STATE_LOADED, IR_STATE_ERROR };

  float m_bchsv[5];
  char m_rot; // 90deg steps (0..3)
  bool m_bw;
  bool m_need_rotchk; // check file for rotation EXIF info
  bool m_cache_has_thumbnail;

  RECT m_croprect;

  class TransformTriangle
  {
    public:
      TransformTriangle(double x1,double y1,double x2, double y2, double x3, double y3) 
      { 
        x[0]=u[0]=x1;  
        y[0]=v[0]=y1;
        x[1]=u[1]=x2;  
        y[1]=v[1]=y2;
        x[2]=u[2]=x3;  
        y[2]=v[2]=y3;
        cap[0]=cap[1]=cap[2]=false;
        memset(cap_offs,0,sizeof(cap_offs));
      }
      ~TransformTriangle() { }
      double x[3],y[3]; // vertices

      double u[3],v[3]; 

      bool cap[3]; // whether each vertex is in mouse capture
      POINT cap_offs[3];
  };
  WDL_PtrList<TransformTriangle> m_transform;

  // state
  int m_state;
  int m_srcimage_w, m_srcimage_h;

  LICE_IBitmap *m_preview_image;

  LICE_IBitmap *m_fullimage;

  LICE_IBitmap *m_fullimage_scaled, 
               *m_fullimage_final;
  char m_fullimage_cachevalid; // 1=final, 2=intermediate(scaled)
  bool m_is_fs;

  RECT m_last_drawrect; // set by drawing
  RECT m_last_crop_drawrect; // set by drawing, read by UI code
  POINT m_crop_capmode_lastpos; // offsets from actual pt

  // used depending on capture state.
  // cropping: 1=left,2=top,4=right,8=bottom, only certain combinations will be used, obviously
  // drag and drop -- last index of where it would end up (or -1 if invalid)
  int m_capture_state; 
  
  RECT m_lastlbl_rect; // last rect of text label (for editing)

  time_t m_file_timestamp;

  static int sortByFN(const void *a, const void *b)
  {
    ImageRecord *r1 = *(ImageRecord**)a;
    ImageRecord *r2 = *(ImageRecord**)b;
    return stricmp(r1->m_fn.Get(),r2->m_fn.Get());
  }
};

int get_lice_bitmap_size(LICE_IBitmap *); //kb

#endif
