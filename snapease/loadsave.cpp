#include "main.h"
#include "../WDL/projectcontext.h"

bool g_imagelist_fn_dirty; // need save
WDL_String g_imagelist_fn;

/*
    WDL_String projdir(g_imagelist_fn.Get());
    char *p=projdir.Get();
    while (*p) p++;
    while (p > projdir.Get() && *p != '\\' && *p != '/') p--;
    p[0]=0;
*/


WDL_INT64 file_size(const char *filename)
{
#ifdef _WIN32
  HANDLE hFile=CreateFile(filename,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hFile == INVALID_HANDLE_VALUE) return -1;
  DWORD l,h;
  l=GetFileSize(hFile,&h);
  CloseHandle(hFile);
  return ((WDL_INT64)h)<<32 | l;
#else
#if 1
  struct stat sb;
  if (!stat(filename,&sb)) return sb.st_size; // this is 64-bit on osx, yay!
  return -1;
#else
  FILE *fp=fopen(filename,"rb");
  if (!fp) return -1;
  fseek(fp,0,SEEK_END);
  int pos=ftell(fp);
  fclose(fp);
  return pos;
#endif
#endif
}

bool file_exists(const char *filename)
{
#ifdef _WIN32
  HANDLE hFile=CreateFile(filename,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hFile == INVALID_HANDLE_VALUE) return false;
  CloseHandle(hFile);
#else
#if 1
  struct stat sb;
  if (stat(filename,&sb)) return false;
#else
  FILE *fp=fopen(filename,"rb");
  if (!fp) return false;
  fclose(fp);
#endif
#endif

  return true;
}


static void make_fn_relative(const char *leadpath, const char *in, char *out, int outlen) // leadpath must NOT have any trailing \\ or /
{
  unsigned int pdlen=0;
  if (1)
  {
    pdlen=strlen(leadpath);

    if (pdlen && (strlen(in)<=pdlen || strnicmp(leadpath,in,pdlen) || (in[pdlen] != '\\' && in[pdlen] != '/'))) pdlen=0;
  }

  lstrcpyn(out,in+pdlen,outlen);
}

static void resolve_fn_fromrelative(const char *leadpath, const char *in, char *out, int outlen)
{
  if (!file_exists(in)) 
  {
    WDL_String tmp(leadpath);
    tmp.Append(PREF_DIRSTR);
    tmp.Append(in);
    if (file_exists(tmp.Get()))
    {
      lstrcpyn(out,tmp.Get(),outlen);
      return;
    }
  }

  lstrcpyn(out,in,outlen);
}

bool importImageListFromFile(const char *fn, bool addToCurrent)
{
  ProjectStateContext *ctx = ProjectCreateFileRead(fn);
  if (!ctx) return false;
  
  // if we get a "NEW_IMAGE_LIST", call ClearImageList()

}



void LoadImageList(bool addToCurrent)
{
}

bool SaveImageList(bool forceSaveAs)
{
  if (forceSaveAs || !g_imagelist_fn.Get()[0])
  {
  }

  g_imagelist_fn_dirty=false;
  UpdateCaption();

  return true;
}

bool SavePromptForClose(const char *promptmsg)
{
  if (!g_imagelist_fn_dirty) return true;
  char buf[4096];
  sprintf(buf,"%s\r\n\r\n(Selecting \"no\" will result in unsaved changes being lost!)",promptmsg);
  int a = MessageBox(g_hwnd,buf,"Save changes?",MB_YESNOCANCEL);

  if (a == IDCANCEL) return false;

  return a == IDNO || SaveImageList(false); // if saved, proceed, otherwise, cancel
}

void SetImageListIsDirty(bool isDirty)
{
  if (g_imagelist_fn_dirty!=isDirty)
  {
    g_imagelist_fn_dirty=isDirty;
    UpdateCaption();
  }
}

void UpdateCaption()
{
  WDL_String tmp;
  char *p=g_imagelist_fn.Get();
  if (p[0])
  {
    while (*p) p++;
    while (*p != '/' && *p != '\\' && p >= g_imagelist_fn.Get()) p--;
    tmp.Set(++p);
    p=tmp.Get();
    while (*p) p++;
    while (*p != '.' && p >= tmp.Get()) p--;
    if (p > tmp.Get()) *p=0;

    if (g_imagelist_fn_dirty) tmp.Append(" [modified]");

  }
  else
  {
    if (g_imagelist_fn_dirty) tmp.Set("[unsaved project]");
  }
  
  if (tmp.Get()[0]) tmp.Append(" - ");

  tmp.Append("SnapEase");

  SetWindowText(g_hwnd,tmp.Get());

  #ifndef _WIN32
  void Mac_SetMainWindowRepre(); // unsaved repre (move to swell?)
  Mac_SetMainWindowRepre();
  #endif

}