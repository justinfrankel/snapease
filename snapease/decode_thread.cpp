#include "main.h"

#include "imagerecord.h"

#include "../WDL/lice/lice.h"

#define DESIRED_PREVIEW_CACHEDIM 256

bool g_DecodeDidSomething;
bool g_DecodeThreadQuit;

static bool LoadFullBitmap(LICE_IBitmap *bmOut, const char *fn)
{
  bool success=false;
#ifdef _WIN32
  __try
  {
#endif

    if (LICE_LoadImage(fn,bmOut,false)) success=true;

#ifdef _WIN32
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
  }
#endif
  return success;
}

static bool DoProcessBitmap(LICE_IBitmap *bmOut, const char *fn, LICE_IBitmap *workBM)
{
  if (!LoadFullBitmap(workBM,fn)) return false;

  int outw = workBM->getWidth();
  int outh = workBM->getHeight();
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

    LICE_ScaledBlit(bmOut,workBM,0,0,outw,outh,0,0,workBM->getWidth(),workBM->getHeight(),1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
    return true;
  }
  return false;
}

static DWORD WINAPI DecodeThreadProc(LPVOID v)
{
  LICE_MemBitmap bm;
  WDL_String curfn;
  LICE_IBitmap *bmOut=NULL;

  int scanpos=0;
  while (!g_DecodeThreadQuit)
  {
    int sleepAmt=1;
    g_images_mutex.Enter();

    if (g_fullmode_item && g_images.Find(g_fullmode_item)>=0) // prioritize any full image loads
    {
      if (g_fullmode_item->m_want_fullimage && !g_fullmode_item->m_fullimage)
      {
        curfn.Set(g_fullmode_item->m_fn.Get());
        g_fullmode_item->m_want_fullimage=false;
        g_images_mutex.Leave();

        if (!bmOut) bmOut = new LICE_MemBitmap;

        bool suc = LoadFullBitmap(bmOut,curfn.Get());

        g_images_mutex.Enter();

        if (suc && g_fullmode_item && g_images.Find(g_fullmode_item)>=0 && !g_fullmode_item->m_fullimage &&
            !strcmp(g_fullmode_item->m_fn.Get(),curfn.Get()))
        {
          g_fullmode_item->m_srcimage_w = bmOut->getWidth();
          g_fullmode_item->m_srcimage_h = bmOut->getHeight();
          g_fullmode_item->m_fullimage = bmOut;
          g_fullmode_item->m_fullimage_rendercached_valid=false;
          bmOut=NULL;
        }
        g_DecodeDidSomething=true;
      }
    }

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
          rec->m_srcimage_w = bm.getWidth();
          rec->m_srcimage_h = bm.getHeight();
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


static HANDLE hThread[4]={0,};

void DecodeThread_Init()
{
  g_DecodeThreadQuit=false;


  int numCPU = 1; // todo: smp option?

  if (numCPU>sizeof(hThread)/sizeof(hThread[0])) numCPU=sizeof(hThread)/sizeof(hThread[0]);

  int x;
  for(x=0;x<numCPU;x++)
  {
    DWORD tid;
    if (!hThread[x])
    {
      hThread[x] = CreateThread(NULL,0,DecodeThreadProc,0,NULL,&tid);
      SetThreadPriority(hThread[x],THREAD_PRIORITY_BELOW_NORMAL);
    }
  } 
}

void DecodeThread_Quit()
{
  int x;
  g_DecodeThreadQuit = true;
  for(x=0;x<sizeof(hThread)/sizeof(hThread[0]);x++)
  {
    if (hThread[x])
    {
      WaitForSingleObject(hThread[x],INFINITE);
      CloseHandle(hThread[x]);
      hThread[x]=0;
    }
  }
}