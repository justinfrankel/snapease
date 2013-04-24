/*
    SnapEase
    decode_thread.cpp -- background image loading thread
    Copyright (C) 2009-2010  Cockos Incorporated

    PathSync is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PathSync is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PathSync; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "main.h"

#include "imagerecord.h"

#include "../WDL/lice/lice.h"

#define DESIRED_PREVIEW_CACHEDIM 256

bool g_DecodeDidSomething;
static bool g_DecodeThreadQuit=true;


#define FORCE_THREADS 1
#define MAX_THREADS 4

#ifdef _WIN32
//#define USE_SEH
#endif

bool LoadFullBitmap(LICE_IBitmap *bmOut, const char *fn)
{
  bool success=false;
#ifdef USE_SEH
  __try
  {
#endif

    if (LICE_LoadImage(fn,bmOut,false)) success=true;

#ifdef USE_SEH
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

class DecodeThreadContext
{
public:
  DecodeThreadContext() { bmOut=NULL; }
  ~DecodeThreadContext() { delete bmOut; }
  LICE_MemBitmap bm;
  WDL_String curfn;
  LICE_IBitmap *bmOut;
  int last_visstart;
  int last_listsize;
  int scanpos;
};

static int RunWork(DecodeThreadContext &ctx)
{
  int sleepAmt=1;
  g_images_mutex.Enter();

  if (ctx.last_visstart != g_firstvisible_startitem || 
      ctx.last_listsize != g_images.GetSize())
  {
    ctx.last_listsize = g_images.GetSize();
    ctx.last_visstart=g_firstvisible_startitem;
    ctx.scanpos=0;
  }
  bool didProc=false;

  int fmi;
  if (g_fullmode_item && (fmi=g_images.Find(g_fullmode_item))>=0) // prioritize any full image loads
  {
    int x;
    const int divpt1 = fmi-2,divpt2=fmi+2; // try to keep 5 images loaded, plus allow another 10 of history (before/after current 5), but don't actively try to load them
    for (x=0;x<divpt1 - 5;x++)
    {
      ImageRecord *it = g_images.Get(x);
      if (it && it->m_fullimage)
      {
        delete it->m_fullimage;
        it->m_fullimage=0;
      }
    }
    for (x=divpt1;x<=divpt2; x++)
    {
      const int dpos = x - divpt1;
      ImageRecord *it = g_images.Get(fmi + ((dpos&1)?(dpos+1)/2 : -(dpos/2))); // +0, +1, -1, +2, -2
      if (it && !it->m_fullimage)
      {
        ctx.curfn.Set(it->m_fn.Get());
        g_images_mutex.Leave();

        if (!ctx.bmOut) ctx.bmOut = new LICE_MemBitmap;

        bool suc = LoadFullBitmap(ctx.bmOut,ctx.curfn.Get());

        g_images_mutex.Enter();

        if (suc && g_images.Find(it)>=0 && !strcmp(it->m_fn.Get(),ctx.curfn.Get()))
        {
          it->m_srcimage_w = ctx.bmOut->getWidth();
          it->m_srcimage_h = ctx.bmOut->getHeight();
          it->m_fullimage = ctx.bmOut;
          it->m_fullimage_cachevalid=0;
          ctx.bmOut=NULL;
          didProc=true;
        }

        g_DecodeDidSomething=true;
      }
    }
    x+=5;
    for (;x<g_images.GetSize();x++)
    {
      ImageRecord *it = g_images.Get(x);
      if (it && it->m_fullimage)
      {
        delete it->m_fullimage;
        it->m_fullimage=0;
      }
    }
  }

  int i;
  for (i= 0; i < 100 && !didProc; i ++)
  {
    if (ctx.scanpos>=g_images.GetSize()*2)
    {
      ctx.scanpos=0;
      sleepAmt=30;
      didProc=true;
    }


    int center = ctx.last_visstart;
    if (center<0) center=0;
    else if (center>g_images.GetSize()) center=g_images.GetSize();

    if (1)
    {
      if ((ctx.scanpos&3)==3) // every 4th, go backwards
        center -= (ctx.scanpos+1)/4;
      else
        center += ctx.scanpos - (ctx.scanpos+1)/4;
    }
    else center+=ctx.scanpos;
  
    ctx.scanpos++;

    ImageRecord *rec = g_images.Get(center);

    if (rec) 
    {
      if (rec->m_state == ImageRecord::IR_STATE_NEEDLOAD)
      {
        LICE_IBitmap *bmDel = NULL;
        if (rec->m_preview_image)
        {
          bmDel = ctx.bmOut;
          ctx.bmOut = rec->m_preview_image;
          rec->m_preview_image=0;
        }

        rec->m_state=ImageRecord::IR_STATE_DECODING;
        ctx.curfn.Set(rec->m_fn.Get());

        g_DecodeDidSomething=true;

        g_images_mutex.Leave();

        if (!ctx.bmOut) ctx.bmOut = new LICE_MemBitmap;
        delete bmDel;

        // load/process image
        bool success = DoProcessBitmap(ctx.bmOut, ctx.curfn.Get(),&ctx.bm);

        didProc=true;

        g_images_mutex.Enter();

        if (g_images.Find(rec)>=0 && rec->m_state == ImageRecord::IR_STATE_DECODING)
        {
          rec->m_srcimage_w = ctx.bm.getWidth();
          rec->m_srcimage_h = ctx.bm.getHeight();
          if (!success) rec->m_state = ImageRecord::IR_STATE_ERROR;
          else if (strcmp(rec->m_fn.Get(),ctx.curfn.Get())) rec->m_state = ImageRecord::IR_STATE_ERROR;
          else
          {
            rec->m_state = ImageRecord::IR_STATE_LOADED;
            rec->m_preview_image = ctx.bmOut;
            ctx.bmOut= 0;
          }
          g_DecodeDidSomething=true;
        }
      }
    }
  }
  g_images_mutex.Leave();

  return sleepAmt;
}

static DWORD WINAPI DecodeThreadProc(LPVOID v)
{
  DecodeThreadContext ctx;

  ctx.last_visstart=g_firstvisible_startitem;
  ctx.scanpos=0;
  while (!g_DecodeThreadQuit)
  {
    Sleep(RunWork(ctx));
  }

  return 0;
}


static HANDLE hThread[MAX_THREADS]={0,};

void DecodeThread_Init()
{
  if (!g_DecodeThreadQuit) return;

  g_DecodeThreadQuit=false;


  int numCPU = FORCE_THREADS; // todo: smp option?

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
  if (g_DecodeThreadQuit) return;

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

void DecodeThread_RunTimer()
{
  if (!hThread[0] && !g_DecodeThreadQuit)
  {
    static bool reent; // in case something runs the message loop in this bitch
    if (!reent)
    {
      reent=true;

      static DecodeThreadContext *p;
      if (!p) p=new DecodeThreadContext;
      if (p) RunWork(*p);

      reent=false;
    }
  }
}
