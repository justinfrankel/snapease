#include "main.h"

#include "resource.h"


#include "../WDL/lice/lice.h"

#define FORMAT_JPG 0
#define FORMAT_PNG 1

#define MAX_RECENT_DIRS 10

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x40
#endif
#ifdef _WIN32
#include <shlobj.h>
#include <commctrl.h>

static int WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			if (lpData && ((char *)lpData)[0]) 
      {
        SendMessage(hwnd,BFFM_SETSELECTION,1,(long)lpData);
      }
    return 0;

	}
	return 0;
}
#endif

static void FNFilterAppend(WDL_String *out, const char *in, int inlen)
{
  while (inlen-->0 && *in)
  {    
    char b[2]={*in,0};
    switch (b[0])
    {
      case '<': b[0]='('; break;
      case '>': b[0]=')'; break;
      case '"': 
      case '?': 
      case '*': b[0]='_'; break;
      case '|': 
      case ':': b[0]=' '; break;
      
    }
    if (b[0])
      out->Append(b);
    in++;
  }
}

static void DoImageOutputFileCalculation(const char *infn, const char *outname, int index, 
                                  const char *imagelistname, const char *diskoutpath, 
                                  const char *fmt,
                                  WDL_String *nameOut)
{
  nameOut->Set("");
  while (*fmt)
  {
    switch (*fmt)
    {
      case '*':
      case '<':
        {
          int oldsz = strlen(nameOut->Get());

          // add filename portion
          {
            const char *srcstr = *fmt == '<' ? infn : imagelistname;
            const char *p = srcstr;
            while (*p) p++;
            while (p >= srcstr && *p != '\\' && *p != '/') p--;
            p++;
            FNFilterAppend(nameOut,p,strlen(p));
          }
          
          // remove extension
          char *p=nameOut->Get();
          char *st = nameOut->Get() + oldsz;
          while (*p) p++;
          while (p > st && *p != '.') p--;
          if (p > st) *p=0;
        }
      break;
      case '>':
        {
          const char *p = outname;
          while (*p)
          {
            if (*p == '/' || *p == '\\')
            {
              WDL_String temp(diskoutpath);
              temp.Append(PREF_DIRSTR);
              temp.Append(nameOut->Get());
              CreateDirectory(temp.Get(),NULL);                       
            }

            FNFilterAppend(nameOut,p,1);
            p++;
          }
        }
      break;
      case '\\':
      case '/':
        if (diskoutpath[0])
        {
          WDL_String temp(diskoutpath);
          temp.Append(PREF_DIRSTR);
          temp.Append(nameOut->Get());
          CreateDirectory(temp.Get(),NULL);          
        }
        nameOut->Append(PREF_DIRSTR);
      break;
      case '$':
        {
          int n=1;
          while (fmt[1] == '$') { fmt++; n++; }
          char s[32];
          sprintf(s,"%%0%dd",n); // whats that syntax for doing this directly? I forget...
          nameOut->AppendFormatted(256,s,index);
        }
      break;
      default: 
        FNFilterAppend(nameOut,fmt,1);
      break;
    }
    fmt++;
  }
}


class imageExporter
{
public:
  imageExporter() { }
  ~imageExporter() { }

  void DisplayMessage(HWND hwndDlg, bool isLog, const char *fmt, ...);
  void RunExportTimer(HWND hwndDlg);

  void Reset()
  {
    m_messages.Set("");
    m_runpos=0;
    m_isFinished=0;
    m_total_files_out=0;
    m_total_bytes_out=0;
  }

  int m_overwrite; // 0=skip, 1=overwrite, 2= output to (2)

  int m_constrain_w,m_constrain_h; // zero if unconstrained
  int m_fmt;
  bool m_jpg_baseline;
  int m_jpg_level;
  bool m_png_alpha;
  char m_formatstr[256];

  char m_disk_out[1024]; // empty if not writing to disk


  // export run state

  bool m_isFinished;

private:
  int m_runpos;

  WDL_String m_messages;

  int m_total_files_out;
  WDL_INT64 m_total_bytes_out;


  // cur state
  WDL_String m_outname; // without any leading path
  WDL_String m_tmpfn;


};

void imageExporter::DisplayMessage(HWND hwndDlg, bool isLog, const char *fmt, ...)
{
  char b[4096];
  b[0]=0;

  va_list arglist;
	va_start(arglist, fmt);
  #ifdef _WIN32
	int written = _vsnprintf(b, sizeof(b)-1, fmt, arglist);
  #else
	int written = vsnprintf(b, sizeof(b)-1, fmt, arglist);
  #endif
  if (written < 0) written = 0;
	va_end(arglist);
  b[written] = '\0';

  SetDlgItemText(hwndDlg,IDC_STATUS,b);
  if (isLog)
  {
    m_messages.Append(b);
    m_messages.Append("\r\n");
    SetDlgItemText(hwndDlg,IDC_EDIT1,m_messages.Get());
  }

  UpdateWindow(hwndDlg);
}


void imageExporter::RunExportTimer(HWND hwndDlg)
{
  ImageRecord *rec;
  
  if (g_fullmode_item && g_images.Find(g_fullmode_item)>=0) rec = m_runpos ? 0 : g_fullmode_item;
  else rec = g_images.Get(m_runpos);
  if (!rec)
  {
    DisplayMessage(hwndDlg,false,"Processing %d/%d images completed!\r\n"
        "Total size: %.2fMB, average image size: %.2fMB",
        m_total_files_out,m_runpos,
      (m_total_bytes_out/1024.0/1024.0),
      (m_total_bytes_out/1024.0/1024.0)/(double)max(1,m_total_files_out)
      );
    SetDlgItemText(hwndDlg,IDCANCEL,"Close");
    m_isFinished=true;
    return;
  }
  
  const char *extension = m_fmt == FORMAT_JPG ? ".jpg" : m_fmt == FORMAT_PNG ? ".png" : ".unknown";
  // calculate output file
  
  DoImageOutputFileCalculation(rec->m_fn.Get(),
                               rec->m_outname.Get(),
                               g_images.Find(rec)+1,
                               g_imagelist_fn.Get()[0] ? g_imagelist_fn.Get() : "Untitled",
                               m_disk_out,
                               m_formatstr[0]?m_formatstr:"<",
                               &m_outname);

  bool preventDiskOutput=false;

  if (m_overwrite!=1 && m_disk_out[0]) // change if needed
  {
    int x;
    const int maxtries=1000;
    WDL_String s;
    
    for (x=0;x<maxtries;x++)
    {
      s.Set(m_disk_out);
      s.Append(PREF_DIRSTR);
      s.Append(m_outname.Get());
      char apstr[256];            
      if (x) sprintf(apstr," (%d)",x+1);
      else apstr[0]=0;

      s.Append(apstr);
      s.Append(extension);
      if (!file_exists(s.Get()))
      {
        m_outname.Append(apstr);
        break;
      }
      if (m_overwrite==0)
      {
        preventDiskOutput=true;
        break;
      }
    }

    if (x>=maxtries&&m_overwrite>1)
    {

      DisplayMessage(hwndDlg,true,"Could not find suitable unused output filename for:\r\n"
                                  "\t%.200s\r\n"
                                  "Last try was: %.200s\r\n",
                                  rec->m_fn.Get(),
                                  s.Get());

      preventDiskOutput=true;
    }
  }


  if (m_disk_out[0])
  {
    m_tmpfn.Set(m_disk_out);
    m_tmpfn.Append(PREF_DIRSTR);
    m_tmpfn.Append(m_outname.Get());
    m_tmpfn.Append(".SnapEase-temp");
  }
  else
  {
    char fn[2048];
#ifdef _WIN32
    GetTempPath(sizeof(fn)-128, fn);
#else
    char *p = getenv("TEMP");
    if (!p || !*p) p="/tmp";
    lstrcpyn(fn, p, 512);
    strcat(fn,"/");
#endif

    sprintf(fn+strlen(fn),"snapease-temp-%08x-%08x.tmp",
#ifdef _WIN32
      GetCurrentProcessId(),
#else
      0, // todo
#endif
      GetTickCount());

    m_tmpfn.Set(fn);
  }

  m_outname.Append(extension);

  SetDlgItemText(hwndDlg,IDC_UPLOADSTATUS,"");
  double avg_imgsize=(m_total_bytes_out/1024.0/1024.0)/(double)max(1,m_total_files_out);
  DisplayMessage(hwndDlg,false,"Processing %d/%d - %.2fMB/%.2fMB (est), average image size = %.2fMB\r\n"
                               "Source: %.100s\r\n"
                               "Destination: %.100s%s%.100s%s\r\n"
                               ,
                               m_runpos + 1,
                               g_fullmode_item && g_images.Find(g_fullmode_item)>=0 ? 1 : g_images.GetSize(),

                                (m_total_bytes_out/1024.0/1024.0),
                                avg_imgsize * (g_fullmode_item && g_images.Find(g_fullmode_item)>=0 ? 1 : g_images.GetSize()),
                                avg_imgsize,

                               rec->m_fn.Get(),
                               m_disk_out[0] ? m_disk_out : "<nowhere>/",
                               m_disk_out[0] ? PREF_DIRSTR: "",
                               m_outname.Get(),
                               "" // if upload, " (upload to blah)" : ""
                               
                               );


  bool hadError=false;

  LICE_IBitmap *srcimage = LICE_LoadImage(rec->m_fn.Get(),NULL,false);
  if (!srcimage)
  {
    hadError=true;
    DisplayMessage(hwndDlg,true,"Failed loading image:\r\n\t%.200s\r\n",rec->m_fn.Get());
  }

  if (!hadError)
  {
    LICE_MemBitmap tempimage;
    if (!rec->ProcessImageToBitmap(srcimage,&tempimage,
                              m_constrain_w,
                              m_constrain_h))
    {
      DisplayMessage(hwndDlg,true,"Failed processing image:\r\n\t%.200s\r\n",rec->m_fn.Get());
      hadError=true;
    }          
    else
    {            
      if (m_fmt == FORMAT_JPG)
      {
        if (!LICE_WriteJPG(m_tmpfn.Get(),&tempimage,m_jpg_level,m_jpg_baseline))
          hadError=true;
        else 
        {
          m_total_files_out++;
          m_total_bytes_out += file_size(m_tmpfn.Get());
        }
      }
      else if (m_fmt == FORMAT_PNG)
      {
        if (!LICE_WritePNG(m_tmpfn.Get(),&tempimage,m_png_alpha))
          hadError=true;
        else 
        {
          m_total_files_out++;
          m_total_bytes_out += file_size(m_tmpfn.Get());
        }
      }
      else
      {
        DisplayMessage(hwndDlg,true,"Unknown format selected");
        hadError=true;
      }
      if (hadError) DisplayMessage(hwndDlg,true,"Failed writing image to:\r\n\t%.200s\r\n",m_tmpfn.Get());
    }
  }


  
  delete srcimage;


  // todo: if uploading, upload m_tmpfn.Get(), using m_outname.Get() as the "name"


  if (!hadError && m_disk_out[0] && !preventDiskOutput)
  {
    WDL_String s;
    s.Set(m_disk_out);
    s.Append(PREF_DIRSTR);
    s.Append(m_outname.Get());
    if (m_overwrite==1) DeleteFile(s.Get());
    if (!MoveFile(m_tmpfn.Get(),s.Get()))
    {
      DisplayMessage(hwndDlg,true,"Failed moving:\r\n\t%.200s\r\nto:\r\n\t%.200s\r\n",m_tmpfn.Get(),s.Get());
    }
  }

  DeleteFile(m_tmpfn.Get());
  m_runpos++;
}

static imageExporter exportConfig;


static WDL_DLGRET ExportRunDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:

      exportConfig.Reset();


      if (exportConfig.m_disk_out[0]) CreateDirectory(exportConfig.m_disk_out,NULL);
      
      SetTimer(hwndDlg,1,10,NULL);
    return 1;
    case WM_TIMER:
      if (wParam==1 && !exportConfig.m_isFinished)
      {
        static bool reent; // in case something runs the message loop in this bitch
        if (!reent)
        {
          reent=true;
          exportConfig.RunExportTimer(hwndDlg);
          reent=false;
        }
      }
    break;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDCANCEL:
          if (!exportConfig.m_isFinished)
          {
            if (MessageBox(hwndDlg,"Abort conversion?","Abort",MB_YESNO)==IDNO) return 0;
          }
          EndDialog(hwndDlg,0);
        break;
      }
    break;
    case WM_DESTROY:
      // handle any cleanup needed

      exportConfig.Reset();

    break;
  }
  return 0;
}

static WDL_DLGRET ExportConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      SetWindowText(hwndDlg,g_fullmode_item&&g_images.Find(g_fullmode_item)>=0?"Export one image" : "Export all images");

      WDL_UTF8_HookComboBox(GetDlgItem(hwndDlg,IDC_COMBO1));
      WDL_UTF8_HookComboBox(GetDlgItem(hwndDlg,IDC_COMBO2));

      if (config_readint("export_constrainsize",1))
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);

      SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_ADDSTRING,0,(LPARAM)"Skip files that exist");
      SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_ADDSTRING,0,(LPARAM)"Overwrite existing files");
      SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_ADDSTRING,0,(LPARAM)"Output to filename (n)");
      SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_SETCURSEL,config_readint("export_overwrite",1),0);
      
      SetDlgItemInt(hwndDlg,IDC_EDIT1,config_readint("export_maxw",800),FALSE);
      SetDlgItemInt(hwndDlg,IDC_EDIT2,config_readint("export_maxh",800),FALSE);

      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"JPEG");
      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"PNG");
      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,config_readint("export_fmt",0),0);
      SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO2,CBN_SELCHANGE),0);

      if (config_readint("export_jpg_baseline",0))
        CheckDlgButton(hwndDlg,IDC_CHECK5,BST_CHECKED);
      SetDlgItemInt(hwndDlg,IDC_EDIT3,config_readint("export_jpg_level",90),FALSE);

      if (config_readint("export_png_alpha",0))
        CheckDlgButton(hwndDlg,IDC_CHECK6,BST_CHECKED);

      {
        char buf[1024];
        buf[0]=0;
        config_readstr("export_fnstr",buf,sizeof(buf));
        if (!buf[0]) strcpy(buf,">");
        SetDlgItemText(hwndDlg,IDC_EDIT4,buf);
      }
      if (config_readint("export_todisk",0))
        CheckDlgButton(hwndDlg,IDC_CHECK4,BST_CHECKED);

      {
        int x;
        for (x=0;x<MAX_RECENT_DIRS;x++)
        {
          char fmt[32];
          sprintf(fmt,"export_dir%d",x);
          char buf[1024];
          buf[0]=0;
          config_readstr(fmt,buf,sizeof(buf));
          if (!buf[0]) break;
          SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)buf);
        }
        if (x) SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,0,0);       
      }

      if (config_readint("export_upload",0))
        CheckDlgButton(hwndDlg,IDC_CHECK8,BST_CHECKED);

      // todo populate IDC_COMBO3, ui in IDC_RECT

      // IDC_COMBO3 = upload w/ IDC_RECT 


    return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_BUTTON1:
          {
			      char name[2048];
            int idx=IDC_COMBO1;
            GetDlgItemText(hwndDlg,idx,name,sizeof(name));
#ifdef _WIN32
            BROWSEINFO bi={0,};
			      LPITEMIDLIST idlist;
			      bi.hwndOwner = hwndDlg;
			      bi.pszDisplayName = name;
			      bi.lpszTitle = "Select a directory:";
			      bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
            bi.lpfn=BrowseCallbackProc;
            bi.lParam=(LPARAM)name;
			      idlist = SHBrowseForFolder( &bi );
			      if (idlist) 
            {
				      SHGetPathFromIDList( idlist, name );        
	            IMalloc *m;
	            SHGetMalloc(&m);
	            m->Free(idlist);
				      SetDlgItemText(hwndDlg,idx,name);
			      }
#else
            if (BrowseForDirectory("Select a directory:",name,name,sizeof(name)))
            {
				      SetDlgItemText(hwndDlg,idx,name);
            }
#endif            
          }
        break;
        case IDC_COMBO2:
          {
            int fmt = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
            ShowWindow(GetDlgItem(hwndDlg,IDC_JPGLBL),fmt==FORMAT_JPG);
            ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT3),fmt==FORMAT_JPG);
            ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK5),fmt==FORMAT_JPG);
            ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK6),fmt==FORMAT_PNG);
          }
        break;
        case IDCANCEL:
          EndDialog(hwndDlg,0);
        break;
        case IDOK:

          {
            int a = SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_GETCURSEL,0,0);
            if (a>=0)
            {
              exportConfig.m_overwrite = a;
              config_writeint("export_overwrite", a);
            }
          }

          {
            BOOL t;
            int w = GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,FALSE);
            int h = GetDlgItemInt(hwndDlg,IDC_EDIT2,&t,FALSE);
            config_writeint("export_maxw",w);
            config_writeint("export_maxh",h);
            if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1))
            {
              config_writeint("export_constrainsize",1);
              exportConfig.m_constrain_h=h;
              exportConfig.m_constrain_w=w;
            }
            else
            {
              config_writeint("export_constrainsize",0);
              exportConfig.m_constrain_w = 0;
              exportConfig.m_constrain_h = 0;
            }

            exportConfig.m_fmt = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
            if (exportConfig.m_fmt>=0) config_writeint("export_fmt",exportConfig.m_fmt);

            config_writeint("export_jpg_baseline",exportConfig.m_jpg_baseline = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK5));

            exportConfig.m_jpg_level = GetDlgItemInt(hwndDlg,IDC_EDIT3,&t,FALSE);
            if (!t) exportConfig.m_jpg_level=75;
            if (exportConfig.m_jpg_level<0)exportConfig.m_jpg_level=0;
            else if (exportConfig.m_jpg_level>120)exportConfig.m_jpg_level=120;

            config_writeint("export_jpg_level",exportConfig.m_jpg_level);
      
            config_writeint("export_png_alpha",exportConfig.m_png_alpha = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK6));

            char buf[4096];
            GetDlgItemText(hwndDlg,IDC_COMBO1,buf,sizeof(buf));

            if (IsDlgButtonChecked(hwndDlg,IDC_CHECK4))
            {
              config_writeint("export_todisk",1);
              lstrcpyn(exportConfig.m_disk_out,buf,sizeof(exportConfig.m_disk_out));
            }
            else 
            {
              config_writeint("export_todisk",0);
              exportConfig.m_disk_out[0]=0;
            }

            int x,oidx=0;
            for (x = 0; x < MAX_RECENT_DIRS + 3; x ++)
            {
              char fmt[32];
              sprintf(fmt,"export_dir%d",oidx);
              if (!x && buf[0]) 
              {
                config_writestr(fmt,buf);
                oidx++;
              }
              else
              {
                char buf2[2048];
                buf2[0]=0;
                if (SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETLBTEXT,x - !!buf[0],(LPARAM)buf2)<0 || !buf2[0]) break;

                if (stricmp(buf,buf2))
                {
                  config_writestr(fmt,buf2);
                  oidx++;
                }
              }
            }
            char fmt[32];
            sprintf(fmt,"export_dir%d",oidx);
            config_writestr(fmt,"");

            buf[0]=0;
            GetDlgItemText(hwndDlg,IDC_EDIT4,buf,sizeof(buf));
            if (!buf[0]) strcpy(buf,">");
            config_writestr("export_fnstr",buf);
            lstrcpyn(exportConfig.m_formatstr,buf,sizeof(exportConfig.m_formatstr));

            if (strstr(exportConfig.m_formatstr,"*") && !g_imagelist_fn.Get()[0])
            {
              if (MessageBox(hwndDlg,"Note: format string contains \"*\", which represents the current imagelist file name.\r\n\r\n"
                    "The current imagelist is not saved, so the string \"Untitled\" will be used in its place.", "Conversion warning",MB_OKCANCEL) == IDCANCEL) return 0;
            }

          }

          // save config state

          EndDialog(hwndDlg,1);
        break;
      }
  }
  return 0;
}

void DoExportDialog(HWND hwndDlg)
{
  if (DialogBox(g_hInst,MAKEINTRESOURCE(IDD_EXPORT_CONFIG),hwndDlg,ExportConfigDialogProc))
  {
    DecodeThread_Quit();

    DialogBox(g_hInst,MAKEINTRESOURCE(IDD_EXPORT_RUN),hwndDlg,ExportRunDialogProc);

    DecodeThread_Init();
  }
}