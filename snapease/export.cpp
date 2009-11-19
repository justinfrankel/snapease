#include "main.h"

#include "resource.h"

#define FORMAT_JPG 0
#define FORMAT_PNG 1

static WDL_DLGRET ExportConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      if (config_readint("export_constrainsize",1))
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      SetDlgItemInt(hwndDlg,IDC_EDIT1,config_readint("export_maxw",800),FALSE);
      SetDlgItemInt(hwndDlg,IDC_EDIT2,config_readint("export_maxh",800),FALSE);

      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"JPEG");
      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"PNG");
      SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,config_readint("export_fmt",0),0);
      SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO2,CBN_SELCHANGE),0);

      if (config_readint("export_jpg_baseline",0))
        CheckDlgButton(hwndDlg,IDC_CHECK5,BST_CHECKED);
      SetDlgItemInt(hwndDlg,IDC_EDIT3,config_readint("export_jpg_level",75),FALSE);

      if (config_readint("export_png_trans",0))
        CheckDlgButton(hwndDlg,IDC_CHECK6,BST_CHECKED);

      {
        char buf[1024];
        buf[0]=0;
        config_readstr("export_fmt",buf,sizeof(buf));
        if (!buf[0]) strcpy(buf,">");
        SetDlgItemText(hwndDlg,IDC_EDIT4,buf);
      }

      //IDC_CHECK4, IDC_COMBO1, IDC_BUTTON1 = save to folder
      // IDC_CHECK8. IDC_COMBO3 = upload w/ IDC_RECT 


    return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
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

          // save config state

          EndDialog(hwndDlg,1);
        break;
      }
  }
  return 0;
}

void DoExportDialog(HWND hwndDlg)
{
  DialogBox(g_hInst,MAKEINTRESOURCE(IDD_EXPORT_CONFIG),hwndDlg,ExportConfigDialogProc);
}