#include "main.h"

#include "resource.h"

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

static struct
{
  bool overwrite;

  int constrain_w,constrain_h; // zero if unconstrained
  int fmt;
  bool jpg_baseline;
  int jpg_level;
  bool png_alpha;
  char formatstr[256];

  char disk_out[1024]; // empty if not writing to disk


} exportConfig;

static WDL_DLGRET ExportConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      if (config_readint("export_constrainsize",1))
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (config_readint("export_overwrite",1))
        CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
      SetDlgItemInt(hwndDlg,IDC_EDIT1,config_readint("export_maxw",800),FALSE);
      SetDlgItemInt(hwndDlg,IDC_EDIT2,config_readint("export_maxh",800),FALSE);

      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"JPEG");
      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"PNG");
      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,config_readint("export_fmt",0),0);
      SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO2,CBN_SELCHANGE),0);

      if (config_readint("export_jpg_baseline",0))
        CheckDlgButton(hwndDlg,IDC_CHECK5,BST_CHECKED);
      SetDlgItemInt(hwndDlg,IDC_EDIT3,config_readint("export_jpg_level",75),FALSE);

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

          config_writeint("export_overwrite",exportConfig.overwrite = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK2));

          {
            BOOL t;
            int w = GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,FALSE);
            int h = GetDlgItemInt(hwndDlg,IDC_EDIT2,&t,FALSE);
            config_writeint("export_maxw",w);
            config_writeint("export_maxh",h);
            if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1))
            {
              config_writeint("export_constrainsize",1);
              exportConfig.constrain_h=h;
              exportConfig.constrain_w=w;
            }
            else
            {
              config_writeint("export_constrainsize",0);
              exportConfig.constrain_w = 0;
              exportConfig.constrain_h = 0;
            }

            exportConfig.fmt = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
            if (exportConfig.fmt>=0) config_writeint("export_fmt",exportConfig.fmt);

            config_writeint("export_jpg_baseline",exportConfig.jpg_baseline = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK5));

            exportConfig.jpg_level = GetDlgItemInt(hwndDlg,IDC_EDIT3,&t,FALSE);
            if (!t) exportConfig.jpg_level=75;
            if (exportConfig.jpg_level<0)exportConfig.jpg_level=0;
            else if (exportConfig.jpg_level>120)exportConfig.jpg_level=120;

            config_writeint("export_jpg_level",exportConfig.jpg_level);
      
            config_writeint("export_png_alpha",exportConfig.png_alpha = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK6));

            char buf[4096];
            GetDlgItemText(hwndDlg,IDC_COMBO1,buf,sizeof(buf));

            if (IsDlgButtonChecked(hwndDlg,IDC_CHECK4))
            {
              config_writeint("export_todisk",1);
              lstrcpyn(exportConfig.disk_out,buf,sizeof(exportConfig.disk_out));
            }
            else 
            {
              config_writeint("export_todisk",0);
              exportConfig.disk_out[0]=0;
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
            lstrcpyn(exportConfig.formatstr,buf,sizeof(exportConfig.formatstr));

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
  }
}