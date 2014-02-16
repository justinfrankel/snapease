/*
    SnapEase
    decode_thread.cpp -- background image loading thread
    Copyright (C) 2009 and onward  Cockos Incorporated

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

#include "main.h"

#ifdef __APPLE__
#import <Carbon/Carbon.h>
#endif

#include "imagerecord.h"

#include "../WDL/lice/lice.h"
#include "../WDL/zlib/zlib.h"

#include "../WDL/fnv64.h"
#include "../WDL/wdlcstring.h"

#define DESIRED_PREVIEW_CACHEDIM 256
#define FILE_CACHE_BLOB_HEADERSIZE 16

bool g_DecodeDidSomething;
static bool g_DecodeThreadQuit=true;

static char GetRotationForImage(const char *fn);

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

static bool DoProcessBitmap(LICE_IBitmap *bmOut, const char *fn, LICE_IBitmap *workBM, char *want_rot_calc, sqlite3 *database, WDL_HeapBuf *workspace, sqlite3_stmt **stmts)
{
  WDL_UINT64 fnhash = WDL_FNV64_IV;
  bool fnhash_valid = false;
  if (database)
  {
    struct stat sb = { 0, };
    if (!statUTF8(fn,&sb))
    {
      const char *p = fn;
      while (*p) p++;
      while (p >= fn && *p != '\\' && *p != '/') p--;
      p++;
      while (*p)
      {
        unsigned char c = (unsigned char)tolower(*p);
        fnhash = WDL_FNV64(fnhash, &c, 1);
        p++;
      }
      WDL_INT64 t = sb.st_mtime;
      fnhash = WDL_FNV64(fnhash, (const unsigned char *)&t, sizeof(t));
      t = sb.st_size;
      fnhash = WDL_FNV64(fnhash, (const unsigned char *)&t, sizeof(t));
      fnhash_valid = true;
    }


    if (fnhash_valid)
    {
      bool got_res = false;
	    sqlite3_stmt *stmt = stmts ? stmts[0] : NULL;
      bool stmt_needrel = false;

      if (!stmt) 
      {
        if (sqlite3_prepare_v2(database, "SELECT DATA FROM THUMB WHERE HASH = ?1", -1, &stmt, NULL) == SQLITE_OK)
        {
          stmt_needrel = true;
        }
      }
      if (stmt)
      {
        sqlite3_bind_int64(stmt, 1, fnhash);
        int res,try_cnt=0;
        do
        {
          res = sqlite3_step(stmt);
          if (res != SQLITE_BUSY) break;
          Sleep(1);
        } while (!g_DecodeThreadQuit && try_cnt++ < 2000);

        if (res==SQLITE_ROW)
        {
          const void *blob = sqlite3_column_blob(stmt, 0);
          const int blob_bytes = sqlite3_column_bytes(stmt, 0);

          if (blob && blob_bytes>0)
          {
            z_stream stream;
            memset(&stream, 0, sizeof(stream));
            if (inflateInit(&stream) == Z_OK)
            {
              stream.avail_in = blob_bytes;
              stream.next_in = (unsigned char *)blob;
              int wroffs = 0;
              for (;;)
              {
                const int chunksz = 256 * 1024;
                workspace->Resize(wroffs + chunksz, false);
                if (workspace->GetSize() != wroffs + chunksz) break;

                stream.total_out = 0;
                stream.avail_out = workspace->GetSize()-wroffs;
                stream.next_out = (unsigned char*)workspace->Get() + wroffs;
                int res=inflate(&stream, Z_SYNC_FLUSH);
                wroffs += stream.total_out;
                if (res != Z_OK||!stream.total_out) break;                
              }
              if (workspace->GetSize()>=wroffs && wroffs > FILE_CACHE_BLOB_HEADERSIZE)
              {
                const char *rd = (const char *)workspace->Get();
                if (want_rot_calc) *want_rot_calc = *rd;
                
                const int w = *(int *)(rd + 4);
                const int h = *(int *)(rd + 8);
                const int extra = *(int *)(rd + 12);
                if (!rd[1] && w>0&&h>0 && wroffs >= FILE_CACHE_BLOB_HEADERSIZE + w*h*3)
                {
                  rd += FILE_CACHE_BLOB_HEADERSIZE;
                  bmOut->resize(w, h);
                  if (bmOut->getWidth() == w && bmOut->getHeight()==h)
                  {
                    int y;
                    for (y = 0; y < h; y ++)
                    {
                      LICE_pixel_chan *po = (LICE_pixel_chan *)(bmOut->getBits() + bmOut->getRowSpan()*y);
                      int x;
                      for (x = 0; x < w; x ++)
                      {
                        po[LICE_PIXEL_R] = *rd++;
                        po[LICE_PIXEL_G] = *rd++;
                        po[LICE_PIXEL_B] = *rd++;
                        po[LICE_PIXEL_A] = 0;
                        po += 4;
                      }

                    }
                    got_res = true;
                  }
                }
              }
              inflateEnd(&stream);
            }
          }
        }        
        if (stmt_needrel) sqlite3_finalize(stmt);
        else 
        {
          sqlite3_reset(stmt);
          sqlite3_clear_bindings(stmt);
        }
      }
      if (got_res)
        return true;
    }
  }
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
    outw = bmOut->getWidth();
    outh = bmOut->getHeight();

    LICE_ScaledBlit(bmOut,workBM,0,0,outw,outh,0,0,(float)workBM->getWidth(),(float)workBM->getHeight(),1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);

    if (want_rot_calc)
    {
      *want_rot_calc = GetRotationForImage(fn);
    }

    if (database && fnhash_valid)
    {
      // compress to buffer
      z_stream stream;
      memset(&stream, 0, sizeof(stream));

      const int alloc_sz = outw * outh * 7 + 4096 + FILE_CACHE_BLOB_HEADERSIZE;
      workspace->Resize(alloc_sz,false);
      if (workspace->GetSize() == alloc_sz && deflateInit(&stream, 1) == Z_OK)
      {
        char *srcbuf = (char *)workspace->Get() + 4096 + outw*outh * 4;
        srcbuf[0] = want_rot_calc?*want_rot_calc:0;
        srcbuf[1] = 0;
        srcbuf[2] = 0;
        srcbuf[3] = 0;
        *(int *)(srcbuf + 4) = outw;
        *(int *)(srcbuf + 8) = outh;
        *(int *)(srcbuf + 12) = 0;

        char *dest = srcbuf + FILE_CACHE_BLOB_HEADERSIZE;
        int y;
        for (y = 0; y < outh; y ++)
        {
          const LICE_pixel_chan *s = (LICE_pixel_chan *)(bmOut->getBits() + y * bmOut->getRowSpan());
          int x=outw;
          while (x--)
          {
            *dest++ = s[LICE_PIXEL_R] & 0xf8;
            *dest++ = s[LICE_PIXEL_G] & 0xf8;
            *dest++ = s[LICE_PIXEL_B] & 0xf8;
            s+=4;
          }
        }

        // srcbuf, FILE_CACHE_BLOB_HEADERSIZE + outw*outh*3 bytes, deflate
        stream.avail_in = FILE_CACHE_BLOB_HEADERSIZE + outw*outh * 3;
        stream.next_in = (unsigned char*)srcbuf;
        stream.avail_out = outw*outh * 4 + 4096;
        stream.next_out = (unsigned char *)workspace->Get();
        deflate(&stream, Z_FINISH);
        const int blob_sz = stream.total_out;
     
        deflateEnd(&stream);

	      sqlite3_stmt *stmt = stmts ? stmts[1] : NULL;
        bool stmt_needrel = false;
        if (!stmt)
        {
  	      if (sqlite3_prepare_v2(database, "INSERT OR REPLACE INTO THUMB (HASH, DATA) VALUES(?1, ?2)", -1, &stmt, NULL) == SQLITE_OK)
          {
            stmt_needrel = true;
          }
        }

        if (stmt)
        {
          sqlite3_bind_int64(stmt, 1, fnhash);
          sqlite3_bind_blob(stmt, 2, workspace->Get(), blob_sz,SQLITE_STATIC);
          int res,try_cnt=0;
          do
          {
            res = sqlite3_step(stmt);
            if (res != SQLITE_BUSY) break;
            Sleep(1);
          } while (!g_DecodeThreadQuit && try_cnt++ < 2000);

          if (stmt_needrel)
            sqlite3_finalize(stmt);
          else
          {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
          }
        }        
      }
    }

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
  WDL_FastString curfn;
  LICE_IBitmap *bmOut;
  int last_visstart;
  int last_listsize;
  int scanpos;
};

static unsigned int __exif_getint(const unsigned char *buf, int sz, unsigned char byteorder)
{
  unsigned int res = 0;
  if (byteorder == 'M')
  {
    while (sz--) res = (res << 8) + *buf++;
  }
  else 
  {
    int sh = 0;
    while (sz--)
    {
      res += *buf++ << sh;
      sh += 8;
    }
  }
  return res;
}
static bool __exif_process_dir(const unsigned char *base, int len, const unsigned char *cur, unsigned char byteorder, char *rotOut)
{
  int nument = __exif_getint(cur, 2, byteorder);
  if (cur + 2 + nument * 12 > base + len) return false;
  cur += 2;
  while (nument-- > 0)
  {
    const int tag = __exif_getint(cur, 2, byteorder);
    const int fmt = __exif_getint(cur + 2, 2, byteorder);
    const int comp = __exif_getint(cur + 4, 4, byteorder);
    const unsigned char *val = cur + 8;

    const char bpf[] = { 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };
    if (!fmt || fmt >= sizeof(bpf)-1) return false; // illegal format

    const int bc = bpf[fmt - 1] * comp;
    if (bc > 4)
    {
      val = base + __exif_getint(val, 4, byteorder);
      if (val < base || val + bc > base + len) return false; // out of range
    }
    if (tag == 0x0112) // orientation
    {
      int rdsz = 1;
      if (fmt == 3 || fmt == 8) rdsz = 2;
      else if (fmt == 4 || fmt == 9) rdsz = 4;
      else return false;

      const int rot = __exif_getint(val, rdsz, byteorder);
      if (rot == 8) *rotOut = 3;
      else if (rot == 3) *rotOut = 2;
      else if (rot == 6) *rotOut = 1;

      return true;
    }
    if (tag == 0x8769 || tag == 0xa005)
    {
      const unsigned char *d = base + __exif_getint(val, 4, byteorder);
      if (d >= base && d < base + len)
        if (__exif_process_dir(base, len, d, byteorder, rotOut)) return true;
    }
    
    cur += 12;
  }
  return false;
}

static bool __exif_process_tag(const unsigned char *buf, int buflen, char *rotOut)
{
  const unsigned char sig[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
  unsigned char byteorder = buf[6];
  if (!memcmp(sig, buf, 6) &&
    buf[7] == byteorder &&
    (byteorder == 'M' || byteorder == 'I') &&
    __exif_getint(buf + 8, 2, byteorder) == 0x2a &&
    __exif_getint(buf + 10, 4, byteorder) == 0x8)
  {
    return __exif_process_dir(buf + 6, buflen - 4, buf + 14, byteorder, rotOut);
  }
  return false;
}

char GetRotationForImage(const char *fn)
{
  FILE *fp = fopenUTF8(fn, "rb");
  if (!fp) return 0;
  char ret = 0;
  if (fgetc(fp) == 0xff && fgetc(fp) == 0xd8)
  {
    int cnt;
    for (cnt = 0; cnt < 3; cnt++)
    {
      int scan;
      int type;
      for (scan = 0; scan < 7 && 0xff == (type = fgetc(fp)); scan++);
      if (type == 0xff || type == 0xda || type == 0xd9) break;
      int l = fgetc(fp) << 8;
      l += fgetc(fp);
      l -= 2;
      if (l < 0) break;
      if (type == 0xe1 && l > 16)
      {
        unsigned char buf[65534];
        if (fread(buf, 1, l, fp) != l) break;

        if (__exif_process_tag(buf, l, &ret)) break;
      }
      else
        fseek(fp, l, SEEK_CUR);
    }
  }

  fclose(fp);
  return ret;
}


static int RunWork(DecodeThreadContext &ctx, bool allowFullMode, sqlite3 *database, WDL_HeapBuf *workspace, sqlite3_stmt **stmts)
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
  if (allowFullMode && g_fullmode_item && (fmi = g_images.Find(g_fullmode_item)) >= 0) // prioritize any full image loads
  {
    int x;
    const int divpt1 = fmi-2,divpt2=fmi+2; // try to keep 5 images loaded, plus allow another 10 of history (before/after current 5), but don't actively try to load them
    for (x=0;x<divpt1 - 5;x++)
    {
      ImageRecord *it = g_images.Get(x);
      if (it && it->m_fullimage)
      {
        g_ram_use_full -= get_lice_bitmap_size(it->m_fullimage);
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
        const bool calc_rot = it->m_need_rotchk;
        char calculated_rot = 0;

        g_images_mutex.Leave();

        if (!ctx.bmOut) ctx.bmOut = new LICE_MemBitmap(0,0,0);

        bool suc = LoadFullBitmap(ctx.bmOut,ctx.curfn.Get());
        if (suc && calc_rot) calculated_rot = GetRotationForImage(ctx.curfn.Get());

        g_images_mutex.Enter();

        if (suc && g_images.Find(it)>=0 && !strcmp(it->m_fn.Get(),ctx.curfn.Get()))
        {
          if (it->m_need_rotchk && calc_rot)
          {
            it->m_rot = calculated_rot;
            it->m_need_rotchk = false;
          }
          it->m_srcimage_w = ctx.bmOut->getWidth();
          it->m_srcimage_h = ctx.bmOut->getHeight();
          it->m_fullimage = ctx.bmOut;
          g_ram_use_full += get_lice_bitmap_size(it->m_fullimage);
          it->m_fullimage_cachevalid = 0;
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
        g_ram_use_full -= get_lice_bitmap_size(it->m_fullimage);
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
          g_ram_use_preview -= get_lice_bitmap_size(rec->m_preview_image);

          bmDel = ctx.bmOut;
          ctx.bmOut = rec->m_preview_image;
          rec->m_preview_image=0;
        }

        rec->m_state=ImageRecord::IR_STATE_DECODING;
        ctx.curfn.Set(rec->m_fn.Get());

        const bool calc_rot = rec->m_need_rotchk;
        char calculated_rot = 0;
        g_DecodeDidSomething=true;

        g_images_mutex.Leave();

        if (!ctx.bmOut) ctx.bmOut = new LICE_MemBitmap(0,0,0);
        delete bmDel;

        // load/process image
        const bool success = DoProcessBitmap(ctx.bmOut, ctx.curfn.Get(),&ctx.bm, calc_rot ? &calculated_rot : NULL,database, workspace,stmts);

        didProc=true;

        g_images_mutex.Enter();

        if (g_images.Find(rec)>=0 && rec->m_state == ImageRecord::IR_STATE_DECODING)
        {
          if (rec->m_need_rotchk && calc_rot)
          {
            rec->m_need_rotchk = false;
            rec->m_rot = calculated_rot;
          }
          rec->m_srcimage_w = ctx.bm.getWidth();
          rec->m_srcimage_h = ctx.bm.getHeight();
          if (!success)
          {
            rec->m_state = ImageRecord::IR_STATE_ERROR;
            g_images_cnt_err++;
          }
          else if (strcmp(rec->m_fn.Get(),ctx.curfn.Get())) rec->m_state = ImageRecord::IR_STATE_ERROR;
          else
          {
            g_images_cnt_ok++;
            g_images_statcnt++;
            rec->m_state = ImageRecord::IR_STATE_LOADED;
            rec->m_preview_image = ctx.bmOut;
            g_ram_use_preview += get_lice_bitmap_size(rec->m_preview_image);
            ctx.bmOut = 0;
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

  sqlite3 *thisdb = NULL;

  if (!g_config_nodb) sqlite3_open(g_db_file.Get(), &thisdb);

  ctx.last_visstart=g_firstvisible_startitem;
  ctx.scanpos=0;

  sqlite3_stmt *stmts[2]={0,};

  if (thisdb)
  {
    if (sqlite3_prepare_v2(thisdb, "SELECT DATA FROM THUMB WHERE HASH = ?1", -1, &stmts[0], NULL) != SQLITE_OK)
      stmts[0] = 0;
    if (sqlite3_prepare_v2(thisdb, "INSERT OR REPLACE INTO THUMB (HASH, DATA) VALUES(?1, ?2)", -1, &stmts[1], NULL) == SQLITE_OK)
      stmts[1] = 0;
  }
          
  WDL_HeapBuf hb;
  while (!g_DecodeThreadQuit)
  {
    Sleep(RunWork(ctx,!v, thisdb,&hb,stmts));
  }

  if (thisdb) 
  {
    if (stmts[0]) sqlite3_finalize(stmts[0]);
    if (stmts[1]) sqlite3_finalize(stmts[1]);
    sqlite3_close(thisdb);
  }

  return 0;
}


static HANDLE hThread[MAX_THREADS]={0,};

static int getCPUcount()
{

#ifdef WIN32
  const int max_cpus = 16;

  static int init = 0;
  static int nbcpu = 1;
  if (init) return nbcpu;
  HKEY k;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor", 0, KEY_READ, &k) == ERROR_SUCCESS)
  {
    int i;
    for (i = 0; i<max_cpus; i++)
    {
      char keyname[512];
      if (RegEnumKey(k, i, keyname, 511) != ERROR_SUCCESS) break;
    }
    if (i>0) nbcpu = i;
    RegCloseKey(k);
  }

  init = 1;
  return nbcpu;
#elif defined(__APPLE__)

  return MPProcessors();
#else
  int np = sysconf(_SC_NPROCESSORS_ONLN);
  if (!np) np = 1;
  return np;
#endif
}

void DecodeThread_Init()
{
  if (!g_DecodeThreadQuit) return;

  g_DecodeThreadQuit=false;


  int numCPU = g_config_smp ? getCPUcount() : 1;

  if (numCPU<1) numCPU = 1;
  if (numCPU>sizeof(hThread)/sizeof(hThread[0])) numCPU=sizeof(hThread)/sizeof(hThread[0]);

  int x;
  for(x=0;x<numCPU;x++)
  {
    DWORD tid;
    if (!hThread[x])
    {
      hThread[x] = CreateThread(NULL,0,DecodeThreadProc,(LPVOID)x,NULL,&tid);
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
  g_DecodeThreadQuit = false;
}

void DecodeThread_RunTimer(void *db)
{
  if (!hThread[0] && !g_DecodeThreadQuit)
  {
    static bool reent; // in case something runs the message loop in this bitch
    if (!reent)
    {
      reent=true;

      static DecodeThreadContext *p;
      if (!p) p=new DecodeThreadContext;
      static WDL_HeapBuf hb;
      if (p) RunWork(*p,true, (sqlite3*)db,&hb,NULL);

      reent=false;
    }
  }
}
