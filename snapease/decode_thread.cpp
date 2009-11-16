#include "main.h"

#include "imagerecord.h"

#include "../WDL/lice/lice.h"

#define DESIRED_PREVIEW_CACHEDIM 192

bool g_DecodeDidSomething;
bool g_DecodeThreadQuit;


static bool DoProcessBitmap(LICE_IBitmap *bmOut, const char *fn, LICE_IBitmap *workBM)
{
  bool success=false;
#ifdef _WIN32
  __try
  {
#endif

    LICE_IBitmap *b = LICE_LoadImage(fn,workBM,false);

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
        bmOut->resize(outw,outh);

        LICE_ScaledBlit(bmOut,b,0,0,outw,outh,0,0,b->getWidth(),b->getHeight(),1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);

        success=true;
      }

    }

#ifdef _WIN32
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
  }
#endif
  return success;
}

DWORD WINAPI DecodeThreadProc(LPVOID v)
{
  LICE_MemBitmap bm;
  WDL_String curfn;
  LICE_IBitmap *bmOut=NULL;

  int scanpos=0;
  while (!g_DecodeThreadQuit)
  {
    int sleepAmt=1;
    g_images_mutex.Enter();
    ImageRecord *rec = g_images.Get(scanpos++);
    if (!rec) 
    {
      scanpos=0;
      sleepAmt = 30;
    }
    else
    {
      if (rec->m_state == IR_STATE_NEEDLOAD)
      {
        LICE_IBitmap *bmDel = NULL;
        if (rec->m_preview_image)
        {
          bmDel = bmOut;
          bmOut = rec->m_preview_image;
          rec->m_preview_image=0;
        }

        rec->m_state=IR_STATE_DECODING;
        curfn.Set(rec->m_fn.Get());

        g_DecodeDidSomething=true;

        g_images_mutex.Leave();

        if (!bmOut) bmOut = new LICE_MemBitmap;
        delete bmDel;

        // load/process image
        bool success = DoProcessBitmap(bmOut, curfn.Get(),&bm);


        g_images_mutex.Enter();

        if (g_images.Find(rec)>=0 && rec->m_state == IR_STATE_DECODING)
        {
          if (!success) rec->m_state = IR_STATE_ERROR;
          else if (strcmp(rec->m_fn.Get(),curfn.Get())) rec->m_state = IR_STATE_ERROR;
          else
          {
            rec->m_state = IR_STATE_LOADED;
            rec->m_preview_image = bmOut;
            bmOut= 0;
          }
          g_DecodeDidSomething=true;
        }
      }
    }
    g_images_mutex.Leave();
    Sleep(sleepAmt);
  }
  delete bmOut;

  return 0;
}
