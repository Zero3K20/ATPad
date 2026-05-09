// Copyright (C) 2007 Andrey Gruber (aka lamer)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>

#include "linklabel.h"
#include "about.h"
#include "aboutresource.h"

/** Defines ********************************************************/
#define	HOME_PAGE				"http://pnotes.sf.net"
#define	EMAIL_ADDRESS			"andrey.gruber@gmail.com"

/** Prototypes ********************************************************/
static void CenterWindow(HWND hwnd);
static void CreateAboutString(HINSTANCE hInstance);
static LRESULT CALLBACK About_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void About_OnClose(HWND hwnd);
static void About_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL About_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void About_OnPaint(HWND hwnd);

#define	FORMAT_STRING		"\\StringFileInfo\\%04x%04x\\%s"

/** Module variables ********************************************************/
static HICON				m_hAboutIcon;
static HFONT				m_hAboutFont;
static char					m_sAbout[1024], m_sName[128];

/*-@@+@@------------------------------------------------------------------
 Procedure: CreateAboutString
 Created  : Sat May 19 12:29:25 2007
 Modified : Sat May 19 12:29:25 2007

 Synopsys : Creates "About" dialog text
 Input    : Application instance
 Output   : Nothing
 Errors   : 
 ------------------------------------------------------------------@@-@@-*/

static void CreateAboutString(HINSTANCE hInstance){

	char szPath[MAX_PATH], szBuffer[MAX_PATH], szLicense[512];
	DWORD dwSize, dwBytes = 0;
	HGLOBAL hMem;
	UINT cbLang;
	LPVOID lpt;
	UINT cbBuffSize;
	WORD * langInfo;

	strcpy(m_sName, _T("About - "));
	GetModuleFileName(hInstance, szPath, MAX_PATH);
	dwBytes = GetFileVersionInfoSize(szPath, &dwSize);
	if(dwBytes){
		hMem = GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, dwBytes);
		if(GetFileVersionInfo(szPath, 0, dwBytes, hMem)){
			if(VerQueryValue(hMem, _T("\\VarFileInfo\\Translation"), (LPVOID*)&langInfo, &cbLang)){
				//InternalName
				wsprintf(szBuffer, FORMAT_STRING, langInfo[0], langInfo[1], _T("InternalName"));
				if(VerQueryValue(hMem, szBuffer, &lpt, &cbBuffSize)){
					strcpy(m_sAbout, (LPTSTR)lpt);
					strcat(m_sAbout, _T(" - "));
					strcat(m_sName, (LPTSTR)lpt);
				}
				//FileVersion
				wsprintf(szBuffer, FORMAT_STRING, langInfo[0], langInfo[1], _T("FileVersion"));
				if(VerQueryValue(hMem, szBuffer, &lpt, &cbBuffSize)){
					strcat(m_sAbout, (LPTSTR)lpt);
					strcat(m_sAbout, _T("\n\n"));
				}
				//FileDescription
				wsprintf(szBuffer, FORMAT_STRING, langInfo[0], langInfo[1], _T("FileDescription"));
				if(VerQueryValue(hMem, szBuffer, &lpt, &cbBuffSize)){
					strcat(m_sAbout, (LPTSTR)lpt);
					strcat(m_sAbout, _T("\n\n"));
				}
				//LegalCopyright
				wsprintf(szBuffer, FORMAT_STRING, langInfo[0], langInfo[1], _T("LegalCopyright"));
				if(VerQueryValue(hMem, szBuffer, &lpt, &cbBuffSize)){
					strcat(m_sAbout, (LPTSTR)lpt);
					strcat(m_sAbout, _T("\n\n"));
				}
				// //Comments
				// wsprintf(szBuffer, FORMAT_STRING, langInfo[0], langInfo[1], _T("Comments"));
				// if(VerQueryValue(hMem, szBuffer, &lpt, &cbBuffSize)){
					// strcat(m_sAbout, (LPTSTR)lpt);
					// strcat(m_sAbout, _T("\n\n"));
				// }
				//License
				LoadString(hInstance, IDS_LICENSE, szLicense, 512);
				strcat(m_sAbout, szLicense);
			}
		}
		GlobalFree(hMem);
	}

}
/*-@@+@@------------------------------------------------------------------
 Procedure: CreateAboutDialog
 Created  : Sat May 19 12:32:42 2007
 Modified : Sat May 19 12:32:42 2007

 Synopsys : Creates "About" dialog
 Input    : Parent window handle, application instance, iconf for 
            displaying on "About" dialog
 Output   : Nothing
 Errors   : 
 ------------------------------------------------------------------@@-@@-*/

void CreateAboutDialog(HWND hwnd, HINSTANCE hInstance, HICON hIcon){
	CreateAboutString(hInstance);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)About_DlgProc, (LPARAM)hIcon);
}

static LRESULT CALLBACK About_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_CLOSE, About_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, About_OnCommand);
	HANDLE_MSG (hwnd, WM_INITDIALOG, About_OnInitDialog);
	HANDLE_MSG (hwnd, WM_PAINT, About_OnPaint);
	case WM_NOTIFY:{
		LPNMHDR lpnmh = (LPNMHDR)lParam;
		char	szAddress[128];
		if(lpnmh->idFrom == IDC_EMAIL && lpnmh->code == NM_CLICK){
			strcpy(szAddress, "mailto:");
			strcat(szAddress, LLGetText(GetDlgItem(hwnd, IDC_EMAIL)));
			ShellExecute(hwnd, _T("open"), szAddress, NULL, NULL, SW_SHOWDEFAULT);
		}
		return TRUE;
	}
	default: return FALSE;
	}
}

static void About_OnClose(HWND hwnd)
{
	DeleteObject(m_hAboutFont);
	EndDialog(hwnd, 0);
}

static void About_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id){
		case IDOK:
		case IDCANCEL:
			SendMessageW(hwnd, WM_CLOSE, 0, 0);
			break;
		case IDC_SHOW_LICENSE:
			ShellExecute(hwnd, "open", "License.txt", NULL, NULL, SW_SHOWNORMAL);
			break;
	}
}

static BOOL About_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	RECT		rc, rc1, rc2;
	HWND		hLabel;

	m_hAboutIcon = (HICON)lParam;
	m_hAboutFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	SetWindowText(hwnd, m_sName);
	CenterWindow(hwnd);
	GetClientRect(hwnd, &rc);
	GetWindowRect(GetDlgItem(hwnd, IDOK), &rc1);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc1, 2);
	hLabel = CreateLinkLable(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), EMAIL_ADDRESS, 0, 0, 20, 20, IDC_EMAIL);
	GetWindowRect(hLabel, &rc2);
	SetWindowPos(hLabel, 0, ((rc.right - rc.left) - (rc2.right - rc2.left)) / 2, rc1.top - (rc2.bottom - rc2.top) - 12, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	return TRUE;
}

static void About_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	RECT rc;
	HFONT hOldFont;

	BeginPaint(hwnd, &ps);
	SetBkMode(ps.hdc, TRANSPARENT);
	hOldFont = SelectObject(ps.hdc, m_hAboutFont);
	DrawIconEx(ps.hdc, 8, 8, m_hAboutIcon, 32, 32, 0, NULL, DI_NORMAL);
	GetClientRect(hwnd, &rc);
	rc.left +=8;
	rc.top += 8;
	rc.right -=8;
	DrawText(ps.hdc, m_sAbout, -1, &rc, DT_CENTER | DT_WORDBREAK);
	SelectObject(ps.hdc, hOldFont);
	EndPaint(hwnd, &ps);
}
/*-@@+@@------------------------------------------------------------------
 Procedure: CenterWindow
 Created  : Sat May 19 12:30:14 2007
 Modified : Sat May 19 12:30:14 2007

 Synopsys : Centers window on screen
 Input    : Window handle
 Output   : Nothing
 Errors   : 
 ------------------------------------------------------------------@@-@@-*/

static void CenterWindow(HWND hwnd){
	RECT		rcD, rcW;

	GetWindowRect(GetDesktopWindow(), &rcD);
	GetWindowRect(hwnd, &rcW);
	MoveWindow(hwnd, ((rcD.right - rcD.left) - (rcW.right - rcW.left)) / 2, \
		((rcD.bottom - rcD.top) - (rcW.bottom - rcW.top)) / 2, \
		rcW.right - rcW.left, rcW.bottom - rcW.top, TRUE);
}

