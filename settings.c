// Copyright (C) 2008 Andrey Gruber (aka lamer)

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

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include <strsafe.h>
#include "stringconstants.h"
#include "globalvars.h"
#include "numericconstants.h"
#include "shared.h"
#include "main.h"

static BOOL Settings_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void Settings_OnClose(HWND hwnd);
static void Settings_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void AddTabs(HWND hwnd);
static LRESULT CALLBACK Appearance_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void Appearance_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL Appearance_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void Appearance_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
static void Appearance_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
static BOOL CALLBACK Prefs_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void Prefs_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL Prefs_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void LoadAppearance(void);
static void EnableLineNumbersControls(BOOL bEnabled);
static void AddAlignmentStrings(wchar_t * lpLangFile);
static BOOL GetFont(wchar_t * lpCaption, P_TPFORMAT lptpf);
UINT APIENTRY CFHookProc(HWND hdlg,	UINT uiMsg,	WPARAM wParam, LPARAM lParam);
static void LoadPrefs(void);
static void SetDefaults(HWND hwnd);
static HWND CreateGCP(HWND hwnd, int idStatic, int idGCP, PGCOLORTYPE pgcp, int x, int cx, int cy);
static void CreateAllGCP(HWND hwnd);
static void EnableGCS(void);
static LRESULT CALLBACK PreviewEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreatePreviewEdit(void);
static void CleanSettings(void);
static void GetBrowser(void);

static HWND			m_hTab, m_hEditor, m_hPrefs, m_hColors, m_hGCP, m_hPreview, m_hStaticPreview;
static HIMAGELIST	m_ImlSettings;
static int			m_TempTextHeight, m_TempLineNumberHeight;
static HFONT		m_TempTextFont, m_TempLinesFont;
static HBITMAP		m_hTempCR, m_hTempLF;
static BOOL			m_Loaded = FALSE, m_HideBold = FALSE;

BOOL CALLBACK Settings_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLE_MSG (hwnd, WM_INITDIALOG, Settings_OnInitDialog);
		HANDLE_MSG (hwnd, WM_CLOSE, Settings_OnClose);
		HANDLE_MSG (hwnd, WM_COMMAND, Settings_OnCommand);
		case WM_NOTIFY:{
			LPNMHDR		lpnmhdr = (LPNMHDR)lParam; 
			if(lpnmhdr->code == TCN_SELCHANGE){
				switch(SendDlgItemMessageW(hwnd, IDC_TAB_SETTINGS, TCM_GETCURSEL, 0, 0)){
					case 0:
						ShowWindow(m_hEditor, SW_SHOW);
						ShowWindow(m_hPrefs, SW_HIDE);
						break;
					case 1:
						ShowWindow(m_hEditor, SW_HIDE);
						ShowWindow(m_hPrefs, SW_SHOW);
						break;
				}
			}
			return TRUE;
		}
		default: return FALSE;
	}
}
static void Settings_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDCANCEL:
		if(codeNotify == BN_CLICKED){
			CleanSettings();
			EndDialog(hwnd, IDCANCEL);
		}
		break;
	case IDOK:
		if(codeNotify == BN_CLICKED){
			CleanSettings();
			EndDialog(hwnd, IDOK);
		}
		break;
	case IDC_CMD_DEFAULT:
		if(codeNotify == BN_CLICKED){
			SetDefaults(hwnd);
		}
		break;
	}
}

static void Settings_OnClose(HWND hwnd)
{
	CleanSettings();
	EndDialog(hwnd, IDCANCEL);
}

static BOOL Settings_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	RECT		rc, rcButton, rcItem;

	g_hSettings = hwnd;

	m_hTab = GetDlgItem(hwnd, IDC_TAB_SETTINGS);
	GetClientRect(hwnd, &rc);
	GetWindowRect(GetDlgItem(hwnd, IDOK), &rcButton);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rcButton, 2);
	MoveWindow(GetDlgItem(hwnd, IDC_TAB_SETTINGS), 0, 0, rc.right - rc.left, rcButton.top - 4, TRUE);
	
	TabCtrl_GetItemRect(m_hTab, 0, &rcItem);
	m_hEditor = CreateDialogParamW(g_hInstance, MAKEINTRESOURCEW(DLG_TEXT_AREA), hwnd, (DLGPROC) Appearance_DlgProc, 0);
	m_hPrefs = CreateDialogParamW(g_hInstance, MAKEINTRESOURCEW(DLG_PREFS), hwnd, (DLGPROC) Prefs_DlgProc, 0);
	AddTabs(hwnd);
	SetRect(&rc, 0, 0, rc.right - rc.left, rcButton.top - 4);
	TabCtrl_AdjustRect(m_hTab, FALSE, &rc);
	MoveWindow(m_hEditor, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(m_hPrefs, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
	g_TempSettings = g_Settings;
	g_TempLineNumbersFormat = g_LineNumbersFormat;
	g_TempTextAreaFormat = g_TextAreaFormat;
	g_TempBreaksColor = g_BreaksColor;
	g_TempTabsColor = g_TabsColor;
	g_TempSpaceColor = g_SpaceColor;
	g_TempBMColor = g_BMColor;
	StringCchCopyW(g_sTempDefBrowser, ARRAYSIZE(g_sTempDefBrowser), g_sDefBrowser);
	m_TempTextFont = CreateFontIndirectW(&g_TempTextAreaFormat.lf);
	GetTextHeight(hwnd, &m_TempTextHeight, m_TempTextFont);
	m_TempLinesFont = CreateFontIndirectW(&g_TempLineNumbersFormat.lf);
	GetLineTextHeight(hwnd, &m_TempLineNumberHeight, m_TempLinesFont);
	m_ImlSettings = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR24, 0, 0);
	AddWrapBitmap(FALSE, g_TempLineNumbersFormat.cText, m_ImlSettings);
	LoadAppearance();
	LoadPrefs();
	CreateAllGCP(m_hEditor);
	CreateCRLF(hwnd, &m_hTempCR, &m_hTempLF, g_TempBreaksColor, g_TempTextAreaFormat);
	//create preview rich edit
	GetWindowRect(GetDlgItem(m_hEditor, IDC_ST_PREVIEW), &rc);
	MapWindowPoints(HWND_DESKTOP, m_hEditor, (LPPOINT)&rc, 2);
	m_hPreview = CreatePreviewEdit();
	ShowWindow(m_hEditor, SW_SHOW);
	m_Loaded = TRUE;
	return TRUE;
}

static void LoadAppearance(void){
	wchar_t			szLang[MAX_PATH];

	StringCchCopyW(szLang, ARRAYSIZE(szLang), g_Paths.sLangDir);
	StringCchCatW(szLang, ARRAYSIZE(szLang), g_Paths.sLangFile);

	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_LN, g_TempSettings.showLines ? TRUE : FALSE);
	EnableLineNumbersControls(g_TempSettings.showLines);
	SetDlgItemInt(m_hEditor, IDC_EDT_LINES_WIDTH, g_TempSettings.marginWidth, FALSE);
	AddAlignmentStrings(szLang);
	SendDlgItemMessageW(m_hEditor, IDC_CBO_LN_ALIGNMENT, CB_SETCURSEL, g_TempSettings.alignNumbers, 0);
	CheckDlgButton(m_hEditor, IDC_CHK_WORD_WRAP, g_TempSettings.wordWrap ? TRUE : FALSE);
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_WS, g_TempSettings.showWS ? TRUE : FALSE);
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_BRAKES, g_TempSettings.showCRLF ? TRUE : FALSE);
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_TABS, g_TempSettings.showTabs ? TRUE : FALSE);
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_SPACES, g_TempSettings.showSpaces ? TRUE : FALSE);
	SetDlgItemInt(m_hEditor, IDC_EDT_TAB_SIZE, g_TempSettings.tabStop, FALSE);
	CheckDlgButton(m_hEditor, IDC_CHK_AUTO_URL, IsBitOn(g_TempSettings.res1, SB_AUTO_URL) ? TRUE : FALSE);
}

static void LoadPrefs(void){
	wchar_t			szLang[MAX_PATH];

	StringCchCopyW(szLang, ARRAYSIZE(szLang), g_Paths.sLangDir);
	StringCchCatW(szLang, ARRAYSIZE(szLang), g_Paths.sLangFile);
	
	SendDlgItemMessageW(m_hPrefs, IDC_UPD_TAB_WIDTH, UDM_SETPOS, 0, MAKELONG(g_TempSettings.tabWidth, 0));
	CheckDlgButton(m_hPrefs, IDC_CHK_SINGLE_INST, g_TempSettings.singleInstance ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hPrefs, IDC_CHK_MIN_TO_TRAY, g_TempSettings.minTotray ? BST_CHECKED : BST_UNCHECKED);
	if(g_TempSettings.openNew)
		CheckDlgButton(m_hPrefs, IDC_OPT_NEW_TAB, BST_CHECKED);
	else
		CheckDlgButton(m_hPrefs, IDC_OPT_EXISTING_TAB, BST_CHECKED);
	if(g_TempSettings.lastSession)
		CheckDlgButton(m_hPrefs, IDC_OPT_LAST_SESSION, BST_CHECKED);
	else
		CheckDlgButton(m_hPrefs, IDC_OPT_BLANK, BST_CHECKED);
	CheckDlgButton(m_hPrefs, IDC_CHK_TRACK, g_TempSettings.trackChanges ? BST_CHECKED : BST_UNCHECKED);
	switch(g_TempSettings.preservePosition){
	case POS_SAVED:
		CheckDlgButton(m_hPrefs, IDC_CHK_PRESERVE_POSITION, BST_CHECKED);
		break;
	case POS_START:
		CheckDlgButton(m_hPrefs, IDC_CHK_JUMP_START, BST_CHECKED);
		break;
	default:
		CheckDlgButton(m_hPrefs, IDC_CHK_JUMP_END, BST_CHECKED);
		break;
	}
	CheckDlgButton(m_hPrefs, IDC_CHK_NEW_VERSION, g_TempSettings.checkNVOnStart ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemTextW(m_hPrefs, IDC_EDT_DEF_BROWSER, g_sTempDefBrowser);
}

static void EnableLineNumbersControls(BOOL bEnabled){
	EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_FONT), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_BCOLOR), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_FCOLOR), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_ST_LINES_FONT), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_ST_LINES_BCOLOR), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_ST_LINES_FCOLOR), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_EDT_LINES_WIDTH), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_UPD_LINES_WIDTH), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_ST_LINES_WIDTH), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_CBO_LN_ALIGNMENT), bEnabled);
	EnableWindow(GetDlgItem(m_hEditor, IDC_ST_LN_ALIGNMENT), bEnabled);
}

static void AddTabs(HWND hwnd){
	TCITEMW			ti;
	wchar_t			szLang[MAX_PATH], szBuffer[128];

	StringCchCopyW(szLang, ARRAYSIZE(szLang), g_Paths.sLangDir);
	StringCchCatW(szLang, ARRAYSIZE(szLang), g_Paths.sLangFile);

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_TEXT;
	ti.pszText = szBuffer;
	GetPrivateProfileStringW(S_TABS, L"0", L"Editor settings", szBuffer, 128, szLang);
	SendMessageW(m_hTab, TCM_INSERTITEMW, 0, (LPARAM)&ti);
	GetPrivateProfileStringW(S_TABS, L"1", L"Preferences", szBuffer, 128, szLang);
	SendMessageW(m_hTab, TCM_INSERTITEMW, 1, (LPARAM)&ti);
	GetPrivateProfileStringW(S_CONTROLS, L"1001", L"Settings", szBuffer, 128, szLang);
	SetWindowTextW(hwnd, szBuffer);
	SetControlText(hwnd, IDOK, L"OK", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);
	SetControlText(hwnd, IDC_CMD_DEFAULT, L"Default settings", szLang);
	//Editor dialog
	SetControlText(m_hEditor, IDC_GRP_TEXT_ARREA_APP, L"Text area", szLang);
	SetControlText(m_hEditor, IDC_GRP_LN_APPEARANCE, L"Line numbers area area", szLang);
	SetControlText(m_hEditor, IDC_CHK_SHOW_LN, L"Show line numbers", szLang);
	SetControlText(m_hEditor, IDC_ST_TEXT_FONT, L"Font", szLang);
	SetControlText(m_hEditor, IDC_ST_TEXT_BCOLOR, L"Background", szLang);
	SetControlText(m_hEditor, IDC_ST_TEXT_FCOLOR, L"Text color", szLang);
	SetControlText(m_hEditor, IDC_ST_LINES_FONT, L"Font", szLang);
	SetControlText(m_hEditor, IDC_ST_LINES_BCOLOR, L"Background", szLang);
	SetControlText(m_hEditor, IDC_ST_LINES_FCOLOR, L"Text color", szLang);
	SetControlText(m_hEditor, IDC_ST_LINES_WIDTH, L"Left margin width", szLang);
	SetControlText(m_hEditor, IDC_ST_LN_ALIGNMENT, L"Alignment", szLang);
	SetControlText(m_hEditor, IDC_CHK_WORD_WRAP, L"Word Wrap", szLang);
	SetControlText(m_hEditor, IDC_GRP_WHITE_SPACES, L"Whitespaces", szLang);
	SetControlText(m_hEditor, IDC_CHK_SHOW_WS, L"Show whitespaces", szLang);
	SetControlText(m_hEditor, IDC_CHK_SHOW_BRAKES, L"Line breaks", szLang);
	SetControlText(m_hEditor, IDC_CHK_SHOW_TABS, L"Tabs", szLang);
	SetControlText(m_hEditor, IDC_ST_TAB_LEN, L"Tab symbol size", szLang);
	SetControlText(m_hEditor, IDC_CHK_SHOW_SPACES, L"Spaces", szLang);
	SetControlText(m_hEditor, IDC_GRP_TEXT_MISC, L"Misc", szLang);
	SetControlText(m_hEditor, IDC_ST_LEFT_BAR, L"Selection bar", szLang);
	SetControlText(m_hEditor, IDC_CHK_AUTO_URL, L"Automatically detect hyperlinks", szLang);
	//Prefs dialog
	SetControlText(m_hPrefs, IDC_GRP_TABS, L"Tabs", szLang);
	SetControlText(m_hPrefs, IDC_ST_TAB_WIDTH, L"Tab width", szLang);
	SetControlText(m_hPrefs, IDC_GRP_BEHAVIOR, L"Behavior", szLang);
	SetControlText(m_hPrefs, IDC_CHK_SINGLE_INST, L"Single instance", szLang);
	SetControlText(m_hPrefs, IDC_CHK_MIN_TO_TRAY, L"Minimize to tray", szLang);
	SetControlText(m_hPrefs, IDC_GRP_OPEN_TAB, L"Open documents in:", szLang);
	SetControlText(m_hPrefs, IDC_OPT_NEW_TAB, L"New tab", szLang);
	SetControlText(m_hPrefs, IDC_OPT_EXISTING_TAB, L"Current tab", szLang);
	SetControlText(m_hPrefs, IDC_CMD_CLEAR_RECENT, L"Clear recent files list", szLang);
	SetControlText(m_hPrefs, IDC_CMD_CLEAR_SEARCH, L"Clear search history", szLang);
	SetControlText(m_hPrefs, IDC_GRP_NEW_SESSION, L"When program_starts:", szLang);
	SetControlText(m_hPrefs, IDC_OPT_BLANK, L"Show blank document", szLang);
	SetControlText(m_hPrefs, IDC_OPT_LAST_SESSION, L"Load last session", szLang);
	SetControlText(m_hPrefs, IDC_CHK_TRACK, L"Keep track of outer changes", szLang);
	SetControlText(m_hPrefs, IDC_GRP_ON_RELOAD, L"On document reload", szLang);
	SetControlText(m_hPrefs, IDC_CHK_PRESERVE_POSITION, L"Preserve text position", szLang);
	SetControlText(m_hPrefs, IDC_CHK_JUMP_START, L"Jump to start", szLang);
	SetControlText(m_hPrefs, IDC_CHK_JUMP_END, L"Jump to end", szLang);
	SetControlText(m_hPrefs, IDC_GRP_VEW_VERSION, L"New version", szLang);
	SetControlText(m_hPrefs, IDC_CHK_NEW_VERSION, L"Check on program start", szLang);
	SetControlText(m_hPrefs, IDC_CMD_NEW_VERSION, L"Check now", szLang);
	SetControlText(m_hPrefs, IDC_GRP_DEF_BROWSER, L"Default browser", szLang);
	SetControlText(m_hPrefs, IDC_ST_DEF_BROWSER, L"Leave blank to use system default", szLang);
}

static void AddAlignmentStrings(wchar_t * lpLangFile){
	wchar_t			szBuffer[128];

	GetPrivateProfileStringW(S_LINES_AL, L"0", L"Left", szBuffer, 128, lpLangFile);
	SendDlgItemMessageW(m_hEditor, IDC_CBO_LN_ALIGNMENT, CB_ADDSTRING, 0, (LPARAM)szBuffer);
	GetPrivateProfileStringW(S_LINES_AL, L"1", L"Center", szBuffer, 128, lpLangFile);
	SendDlgItemMessageW(m_hEditor, IDC_CBO_LN_ALIGNMENT, CB_ADDSTRING, 0, (LPARAM)szBuffer);
	GetPrivateProfileStringW(S_LINES_AL, L"2", L"Right", szBuffer, 128, lpLangFile);
	SendDlgItemMessageW(m_hEditor, IDC_CBO_LN_ALIGNMENT, CB_ADDSTRING, 0, (LPARAM)szBuffer);
}

static LRESULT CALLBACK Appearance_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHARFORMAT2		cr;

	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_COMMAND, Appearance_OnCommand);
	HANDLE_MSG (hwnd, WM_INITDIALOG, Appearance_OnInitDialog);
	HANDLE_MSG (hwnd, WM_DRAWITEM, Appearance_OnDrawItem);
	HANDLE_MSG (hwnd, WM_MEASUREITEM, Appearance_OnMeasureItem);

	case GCN_COLOR_SELECTED:
		switch(wParam){
		case IDC_CMD_TEXT_BCOLOR:
			memcpy(&g_TempTextAreaFormat.cBack, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			CreateCRLF(hwnd, &m_hTempCR, &m_hTempLF, g_TempBreaksColor, g_TempTextAreaFormat);
			if(g_TempTextAreaFormat.cBack.sysIndex != -1)
				SendMessageW(m_hPreview, EM_SETBKGNDCOLOR, 0, GetSysColor(g_TempTextAreaFormat.cBack.sysIndex));
			else
				SendMessageW(m_hPreview, EM_SETBKGNDCOLOR, 0, g_TempTextAreaFormat.cBack.crValue);
			break;
		case IDC_CMD_TEXT_FCOLOR:
			memcpy(&g_TempTextAreaFormat.cText, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			ZeroMemory(&cr, sizeof(cr));
			cr.cbSize = sizeof(cr);
			cr.dwMask = CFM_COLOR;
			if(g_TempTextAreaFormat.cText.sysIndex != -1)
				cr.crTextColor = GetSysColor(g_TempTextAreaFormat.cText.sysIndex);
			else
				cr.crTextColor = g_TempTextAreaFormat.cText.crValue;
			SendMessageW(m_hPreview, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cr);
			break;
		case IDC_CMD_LN_BCOLOR:
			memcpy(&g_TempLineNumbersFormat.cBack, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			break;
		case IDC_CMD_LN_FCOLOR:
			memcpy(&g_TempLineNumbersFormat.cText, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			AddWrapBitmap(TRUE, g_TempLineNumbersFormat.cText, m_ImlSettings);
			break;
		case IDC_GC_BREAKS:
			memcpy(&g_TempBreaksColor, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			CreateCRLF(hwnd, &m_hTempCR, &m_hTempLF, g_TempBreaksColor, g_TempTextAreaFormat);
			break;
		case IDC_GC_WS:
			memcpy(&g_TempTabsColor, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			break;
		case IDC_GC_SPACES:
			memcpy(&g_TempSpaceColor, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			break;
		case IDC_GC_LEFT_BAR:
			memcpy(&g_TempBMColor, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			break;
		}
		InvalidateRect(m_hPreview, NULL, FALSE);
		// RedrawWindow(GetDlgItem(m_hEditor, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
		return TRUE;
	default: return FALSE;
	}
}

static void Appearance_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	if(lpMeasureItem->CtlType == ODT_COMBOBOX){
		lpMeasureItem->itemHeight -= 2;
	}
}

static void Appearance_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	if(lpDrawItem->CtlType == ODT_COMBOBOX){
		DrawComboItem(lpDrawItem);
	}
}

static void Appearance_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	RECT		rc, rcStatic;
	int			len, value, tabStop;

	switch(id){
	case IDC_CHK_SHOW_LN:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				GetWindowRect(m_hStaticPreview, &rcStatic);
				if(IsDlgButtonChecked(hwnd, IDC_CHK_SHOW_LN) == BST_CHECKED){
					EnableLineNumbersControls(TRUE);
					g_TempSettings.showLines = TRUE;
				}
				else{
					EnableLineNumbersControls(FALSE);
					g_TempSettings.showLines = FALSE;
				}
				SendMessageW(m_hPreview, EM_GETRECT, 0, (LPARAM)&rc);
				if(g_TempSettings.showLines){
					rc.left += g_TempSettings.marginWidth;
					SetWindowPos(m_hStaticPreview, 0, 0, 0, g_TempSettings.marginWidth + L_BOOKMARK, rcStatic.bottom - rcStatic.top, SWP_NOMOVE | SWP_SHOWWINDOW);
					// ShowWindow(m_hStaticPreview, SW_SHOW);
				}
				else{
					rc.left -= g_TempSettings.marginWidth;
					SetWindowPos(m_hStaticPreview, 0, 0, 0, L_BOOKMARK, rcStatic.bottom - rcStatic.top, SWP_NOMOVE | SWP_SHOWWINDOW);
					// ShowWindow(m_hStaticPreview, SW_HIDE);
				}
				SendMessageW(m_hPreview, EM_SETRECTNP, 0, (LPARAM)&rc);
				InvalidateRect(m_hPreview, NULL, FALSE);
				// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
			}
		}
		break;
	case IDC_CHK_WORD_WRAP:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				if(IsDlgButtonChecked(hwnd, IDC_CHK_WORD_WRAP) == BST_CHECKED){
					g_TempSettings.wordWrap = TRUE;
					SendMessageW(m_hPreview, EM_SETTARGETDEVICE, 0, 0);
				}
				else{
					g_TempSettings.wordWrap = FALSE;
					SendMessageW(m_hPreview, EM_SETTARGETDEVICE, 0, 1);
				}
				InvalidateRect(m_hPreview, NULL, FALSE);
				// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
			}
		}
		break;
	case IDC_CHK_SHOW_WS:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				if(IsDlgButtonChecked(hwnd, IDC_CHK_SHOW_WS) == BST_CHECKED){
					g_TempSettings.showWS = TRUE;
				}
				else{
					g_TempSettings.showWS = FALSE;
				}
				EnableGCS();
				InvalidateRect(m_hPreview, NULL, FALSE);
			}
		}
		break;
	case IDC_CHK_AUTO_URL:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				if(IsDlgButtonChecked(hwnd, IDC_CHK_AUTO_URL) == BST_CHECKED){
					BitOn(&g_TempSettings.res1, SB_AUTO_URL);
				}
				else{
					BitOff(&g_TempSettings.res1, SB_AUTO_URL);
				}
			}
		}
		break;
	case IDC_CHK_SHOW_BRAKES:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				if(IsDlgButtonChecked(hwnd, IDC_CHK_SHOW_BRAKES) == BST_CHECKED){
					g_TempSettings.showCRLF = TRUE;
				}
				else{
					g_TempSettings.showCRLF = FALSE;
				}
				EnableGCS();
				InvalidateRect(m_hPreview, NULL, FALSE);
			}
		}
		break;
	case IDC_CHK_SHOW_TABS:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				if(IsDlgButtonChecked(hwnd, IDC_CHK_SHOW_TABS) == BST_CHECKED){
					g_TempSettings.showTabs = TRUE;
				}
				else{
					g_TempSettings.showTabs = FALSE;
				}
				EnableGCS();
				InvalidateRect(m_hPreview, NULL, FALSE);
			}
		}
		break;
	case IDC_CHK_SHOW_SPACES:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				if(IsDlgButtonChecked(hwnd, IDC_CHK_SHOW_SPACES) == BST_CHECKED){
					g_TempSettings.showSpaces = TRUE;
				}
				else{
					g_TempSettings.showSpaces = FALSE;
				}
				EnableGCS();
				InvalidateRect(m_hPreview, NULL, FALSE);
			}
		}
		break;
	case IDC_EDT_TAB_SIZE:
		if(m_Loaded){
			if(codeNotify == EN_UPDATE){
				value = GetDlgItemInt(hwnd, IDC_EDT_TAB_SIZE, FALSE, FALSE);
				len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_TAB_SIZE));
				if(len == 0){
					value = T_SIZE_MIN;
					SetDlgItemInt(hwnd, IDC_EDT_TAB_SIZE, value, FALSE);
				}
				if(len > 1){
					value /= 10;
					if(value > T_SIZE_MAX)
						value = T_SIZE_MAX;
					SetDlgItemInt(hwnd, IDC_EDT_TAB_SIZE, value, FALSE);
				}
				else if(value > T_SIZE_MAX){
					value = T_SIZE_MAX;
					SetDlgItemInt(hwnd, IDC_EDT_TAB_SIZE, value, FALSE);
				}
				else if(value < T_SIZE_MIN){
					value = T_SIZE_MIN;
					SetDlgItemInt(hwnd, IDC_EDT_TAB_SIZE, value, FALSE);
				}
				g_TempSettings.tabStop = value;
				tabStop = value * 4;
				SendMessageW(m_hPreview, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);
				InvalidateRect(m_hPreview, NULL, FALSE);
			}
		}
		break;
	case IDC_EDT_LINES_WIDTH:
		if(m_Loaded){
			if(codeNotify == EN_UPDATE){
				value = GetDlgItemInt(hwnd, IDC_EDT_LINES_WIDTH, FALSE, FALSE);
				len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_LINES_WIDTH));
				if(len > 1){
					if(value < L_MARGIN_MIN){
						value = L_MARGIN_MIN;
						SetDlgItemInt(hwnd, IDC_EDT_LINES_WIDTH, L_MARGIN_MIN, FALSE);
					}
					else if(value > L_MARGIN_MAX){
						value = L_MARGIN_MAX;
						SetDlgItemInt(hwnd, IDC_EDT_LINES_WIDTH, L_MARGIN_MAX, FALSE);
					}
				}
				else if(len == 0){
					value = L_MARGIN_MIN;
					SetDlgItemInt(hwnd, IDC_EDT_LINES_WIDTH, L_MARGIN_MIN, FALSE);
				}
				g_TempSettings.marginWidth = value;
				SendMessageW(m_hPreview, EM_GETRECT, 0, (LPARAM)&rc);
				if(g_TempSettings.showLines){
					rc.left = value + L_BOOKMARK;
					SetWindowPos(m_hStaticPreview, 0, 0, 0, value + L_BOOKMARK, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
				}
				else{
					rc.left = 1 + L_BOOKMARK;
					SetWindowPos(m_hStaticPreview, 0, 0, 0, 1 + L_BOOKMARK, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
				}
				SendMessageW(m_hPreview, EM_SETRECTNP, 0, (LPARAM)&rc);
				InvalidateRect(m_hPreview, NULL, FALSE);
				// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
			}
			else if(codeNotify == EN_KILLFOCUS){
				value = GetDlgItemInt(hwnd, IDC_EDT_LINES_WIDTH, FALSE, FALSE);
				len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_LINES_WIDTH));
				if(len < 2){
					value = L_MARGIN_MIN;
					SetDlgItemInt(hwnd, IDC_EDT_LINES_WIDTH, L_MARGIN_MIN, FALSE);
					g_TempSettings.marginWidth = value;
					SendMessageW(m_hPreview, EM_GETRECT, 0, (LPARAM)&rc);
					rc.top += 1;
					rc.bottom += 3;
					if(g_TempSettings.showLines){
						rc.left = value + L_BOOKMARK;
						SetWindowPos(m_hStaticPreview, 0, 0, 0, value + L_BOOKMARK, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
					}
					else{
						rc.left = 1 + L_BOOKMARK;
						SetWindowPos(m_hStaticPreview, 0, 0, 0, 1 + L_BOOKMARK, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
					}
					SendMessageW(m_hPreview, EM_SETRECTNP, 0, (LPARAM)&rc);
					InvalidateRect(m_hPreview, NULL, FALSE);
					// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
				}
			}
		}
		break;
	case IDC_CBO_LN_ALIGNMENT:
		if(m_Loaded){
			if(codeNotify == CBN_SELENDOK){
				g_TempSettings.alignNumbers = SendDlgItemMessageW(hwnd, IDC_CBO_LN_ALIGNMENT, CB_GETCURSEL, 0, 0);
				InvalidateRect(m_hPreview, NULL, FALSE);
				// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
			}
		}
		break;
	case IDC_CMD_TEXT_FONT:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				m_HideBold = TRUE;
				if(GetFont(g_Strings.sFontTextCaption, &g_TempTextAreaFormat)){
					if(m_TempTextFont)
						DeleteObject(m_TempTextFont);
					m_TempTextFont = CreateFontIndirectW(&g_TempTextAreaFormat.lf);
					SendMessageW(m_hPreview, WM_SETFONT, (WPARAM)m_TempTextFont, TRUE);
					GetTextHeight(hwnd, &m_TempTextHeight, m_TempTextFont);
					tabStop = g_TempSettings.tabStop * 4;
					SendMessageW(m_hPreview, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);
					InvalidateRect(m_hPreview, NULL, FALSE);
					// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
				}
			}
		}
		break;
	case IDC_CMD_LN_FONT:
		if(m_Loaded){
			if(codeNotify == BN_CLICKED){
				m_HideBold = FALSE;
				if(GetFont(g_Strings.sFontLinesCaption, &g_TempLineNumbersFormat)){
					if(m_TempLinesFont)
						DeleteObject(m_TempLinesFont);
					m_TempLinesFont = CreateFontIndirectW(&g_TempLineNumbersFormat.lf);
					GetLineTextHeight(hwnd, &m_TempLineNumberHeight, m_TempLinesFont);
					InvalidateRect(m_hPreview, NULL, FALSE);
					// RedrawWindow(GetDlgItem(hwnd, IDC_ST_PREVIEW), NULL, NULL, RDW_INVALIDATE);
				}
			}
		}
		break;
	}
}

static BOOL Appearance_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{

	SendDlgItemMessageW(hwnd, IDC_EDT_LINES_WIDTH, EM_LIMITTEXT, 2, 0);
	SendDlgItemMessageW(hwnd, IDC_UPD_LINES_WIDTH, UDM_SETRANGE, 0, MAKELONG(96, 48));
	SendDlgItemMessageW(hwnd, IDC_UPD_TAB_SIZE, UDM_SETRANGE, 0, MAKELONG(8, 2));
	return TRUE;
}

static BOOL CALLBACK Prefs_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwndDlg, WM_COMMAND, Prefs_OnCommand);
	HANDLE_MSG (hwndDlg, WM_INITDIALOG, Prefs_OnInitDialog);

	default: return FALSE;
	}
}
static void Prefs_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
		case IDC_CMD_DEF_BROWSER:
			if(codeNotify == BN_CLICKED){
				GetBrowser();
			}
			break;
		case IDC_CMD_NEW_VERSION:
			if(codeNotify == BN_CLICKED){
				g_CheckingFromButton = TRUE;
				StartUpdateProcess(g_hMain);
			}
			break;
		case IDC_EDT_DEF_BROWSER:
			if(m_Loaded){
				GetDlgItemTextW(hwnd, IDC_EDT_DEF_BROWSER, g_sTempDefBrowser, MAX_PATH);
			}
			break;
		case IDC_EDT_TAB_WIDTH:
			if(m_Loaded){
				if(codeNotify == EN_UPDATE){
					int		value = GetDlgItemInt(hwnd, IDC_EDT_TAB_WIDTH, FALSE, FALSE);
					int		len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_TAB_WIDTH));
					if(len > 2){
						if(value > T_WIDTH_MAX){
							value = T_WIDTH_MAX;
							SetDlgItemInt(hwnd, IDC_EDT_TAB_WIDTH, T_WIDTH_MAX, FALSE);
						}
					}
					else if(len == 0){
						value = T_WIDTH_MIN;
						SetDlgItemInt(hwnd, IDC_EDT_TAB_WIDTH, T_WIDTH_MIN, FALSE);
					}
					g_TempSettings.tabWidth = value;
				}
				else if(codeNotify == EN_KILLFOCUS){
					int		value = GetDlgItemInt(hwnd, IDC_EDT_TAB_WIDTH, FALSE, FALSE);
					int		len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_TAB_WIDTH));
					if(len < 2){
						value = T_WIDTH_MIN;
						SetDlgItemInt(hwnd, IDC_EDT_TAB_WIDTH, T_WIDTH_MIN, FALSE);
						g_TempSettings.tabWidth = value;
					}
					else if(len == 2){
						if(value < T_WIDTH_MIN){
							value = T_WIDTH_MIN;
							SetDlgItemInt(hwnd, IDC_EDT_TAB_WIDTH, T_WIDTH_MIN, FALSE);
							g_TempSettings.tabWidth = value;
						}
					}
				}
			}
			break;
		case IDC_CHK_SINGLE_INST:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_SINGLE_INST) == BST_CHECKED){
						g_TempSettings.singleInstance = TRUE;
					}
					else{
						g_TempSettings.singleInstance = FALSE;
					}
				}
			}
			break;
		case IDC_CHK_NEW_VERSION:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_NEW_VERSION) == BST_CHECKED){
						g_TempSettings.checkNVOnStart = TRUE;
					}
					else{
						g_TempSettings.checkNVOnStart = FALSE;
					}
				}
			}
			break;
		case IDC_CHK_PRESERVE_POSITION:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_PRESERVE_POSITION) == BST_CHECKED){
						g_TempSettings.preservePosition = POS_SAVED;
					}
				}
			}
			break;
		case IDC_CHK_JUMP_END:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_JUMP_END) == BST_CHECKED){
						g_TempSettings.preservePosition = POS_END;
					}
				}
			}
			break;
		case IDC_CHK_JUMP_START:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_JUMP_START) == BST_CHECKED){
						g_TempSettings.preservePosition = POS_START;
					}
				}
			}
			break;
		case IDC_CHK_MIN_TO_TRAY:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_MIN_TO_TRAY) == BST_CHECKED){
						g_TempSettings.minTotray = TRUE;
					}
					else{
						g_TempSettings.minTotray = FALSE;
					}
				}
			}
			break;
		case IDC_OPT_NEW_TAB:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_OPT_NEW_TAB) == BST_CHECKED){
						g_TempSettings.openNew = TRUE;
					}
				}
			}
			break;
		case IDC_OPT_EXISTING_TAB:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_OPT_EXISTING_TAB) == BST_CHECKED){
						g_TempSettings.openNew = FALSE;
					}
				}
			}
			break;
		case IDC_CMD_CLEAR_RECENT:
			if(codeNotify == BN_CLICKED){
				SendMessageW(g_hMain, TBNPM_CLEARRECENT, 0, 0);
			}
			break;
		case IDC_CMD_CLEAR_SEARCH:
			if(codeNotify == BN_CLICKED){
				WritePrivateProfileSectionW(S_FIND, NULL, g_Paths.sINI);
				WritePrivateProfileSectionW(S_REPLACE, NULL, g_Paths.sINI);
				*g_SearchString = '\0';
				*g_ReplaceString = '\0';
			}
			break;
		case IDC_OPT_BLANK:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_OPT_BLANK) == BST_CHECKED){
						g_TempSettings.lastSession = FALSE;
					}
				}
			}
			break;
		case IDC_OPT_LAST_SESSION:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_OPT_LAST_SESSION) == BST_CHECKED){
						g_TempSettings.lastSession = TRUE;
					}
				}
			}
			break;
		case IDC_CHK_TRACK:
			if(m_Loaded){
				if(codeNotify == BN_CLICKED){
					if(IsDlgButtonChecked(hwnd, IDC_CHK_TRACK) == BST_CHECKED){
						g_TempSettings.trackChanges = TRUE;
					}
					else{
						g_TempSettings.trackChanges = FALSE;
					}
				}
			}
			break;
	}
}

static BOOL Prefs_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SendDlgItemMessageW(hwnd, IDC_EDT_TAB_WIDTH, EM_LIMITTEXT, 3, 0);
	SendDlgItemMessageW(hwnd, IDC_UPD_TAB_WIDTH, UDM_SETRANGE, 0, MAKELONG(204, 96));
	return TRUE;
}

static void SetDefaults(HWND hwnd){
	HDC				hdc;
	CHARFORMAT2		cr;
	int				tabStop;

	hdc = GetDC(hwnd);
	//appearance dialog
	ZeroMemory(&g_TempTextAreaFormat.lf, sizeof(LOGFONTW));
	g_TempTextAreaFormat.fontHeight = 10;
	g_TempTextAreaFormat.lf.lfWeight = FW_NORMAL;
	StringCchCopyW(g_TempTextAreaFormat.lf.lfFaceName, ARRAYSIZE(g_TempTextAreaFormat.lf.lfFaceName), L"Lucida Console");
	g_TempTextAreaFormat.lf.lfHeight = -MulDiv(g_TempTextAreaFormat.fontHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	if(m_TempTextFont)
		DeleteObject(m_TempTextFont);
	m_TempTextFont = CreateFontIndirectW(&g_TempTextAreaFormat.lf);
	SendMessageW(m_hPreview, WM_SETFONT, (WPARAM)m_TempTextFont, FALSE);
	g_TempTextAreaFormat.cBack.sysIndex = COLOR_WINDOW;
	g_TempTextAreaFormat.cBack.crValue = GetSysColor(COLOR_WINDOW);
	SendMessageW(m_hPreview, EM_SETBKGNDCOLOR, 0, GetSysColor(g_TempTextAreaFormat.cBack.sysIndex));
	g_TempTextAreaFormat.cText.sysIndex = COLOR_WINDOWTEXT;
	g_TempTextAreaFormat.cText.crValue = GetSysColor(COLOR_WINDOWTEXT);
	ZeroMemory(&cr, sizeof(cr));
	cr.cbSize = sizeof(cr);
	cr.dwMask = CFM_COLOR;
	cr.crTextColor = GetSysColor(g_TempTextAreaFormat.cText.sysIndex);
	SendMessageW(m_hPreview, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cr);
	ZeroMemory(&g_TempLineNumbersFormat.lf, sizeof(LOGFONTW));
	g_TempLineNumbersFormat.fontHeight = 8;
	g_TempLineNumbersFormat.lf.lfWeight = FW_NORMAL;
	StringCchCopyW(g_TempLineNumbersFormat.lf.lfFaceName, ARRAYSIZE(g_TempLineNumbersFormat.lf.lfFaceName), L"Lucida Console");
	g_TempLineNumbersFormat.lf.lfHeight = -MulDiv(g_TempLineNumbersFormat.fontHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	if(m_TempLinesFont)
		DeleteObject(m_TempLinesFont);
	m_TempLinesFont = CreateFontIndirectW(&g_TempLineNumbersFormat.lf);
	g_TempLineNumbersFormat.cBack.sysIndex = -1;
	g_TempLineNumbersFormat.cBack.crValue = 0xc0c0c0;	//silver
	g_TempLineNumbersFormat.cText.sysIndex = COLOR_WINDOWTEXT;
	g_TempLineNumbersFormat.cText.crValue = GetSysColor(COLOR_WINDOWTEXT);
	AddWrapBitmap(TRUE, g_TempLineNumbersFormat.cText, m_ImlSettings);
	g_TempBMColor.sysIndex = -1;
	g_TempBMColor.crValue = 0xd3d3d3;	//light gray
	CheckDlgButton(m_hEditor, IDC_CHK_WORD_WRAP, BST_UNCHECKED);
	g_TempSettings.wordWrap = FALSE;
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_LN, BST_CHECKED);
	g_TempSettings.showLines = TRUE;
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_WS, BST_UNCHECKED);
	g_TempSettings.showWS = FALSE;
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_BRAKES, BST_UNCHECKED);
	g_TempSettings.showCRLF = FALSE;
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_TABS, BST_UNCHECKED);
	g_TempSettings.showTabs = FALSE;
	CheckDlgButton(m_hEditor, IDC_CHK_SHOW_SPACES, BST_UNCHECKED);
	g_TempSettings.showSpaces = FALSE;
	EnableGCS();
	EnableLineNumbersControls(TRUE);
	SetDlgItemInt(m_hEditor, IDC_EDT_LINES_WIDTH, L_MARGIN_MIN, FALSE);
	g_TempSettings.marginWidth = L_MARGIN_MIN;
	SendDlgItemMessageW(m_hEditor, IDC_CBO_LN_ALIGNMENT, CB_SETCURSEL, DT_RIGHT, 0);
	g_TempSettings.alignNumbers = DT_RIGHT;
	ReleaseDC(hwnd, hdc);
	g_TempBreaksColor.sysIndex = -1;
	g_TempBreaksColor.crValue = COLOR_B;
	g_TempTabsColor.sysIndex = -1;
	g_TempTabsColor.crValue = COLOR_TABS;
	EnableWindow(GetDlgItem(m_hEditor, IDC_GC_BREAKS), FALSE);
	EnableWindow(GetDlgItem(m_hEditor, IDC_GC_WS), FALSE);
	SendDlgItemMessageW(m_hEditor, IDC_CMD_TEXT_BCOLOR, GCM_NEW_COLOR, 0, (LPARAM)&g_TempTextAreaFormat.cBack);
	SendDlgItemMessageW(m_hEditor, IDC_CMD_TEXT_FCOLOR, GCM_NEW_COLOR, 0, (LPARAM)&g_TempTextAreaFormat.cText);
	SendDlgItemMessageW(m_hEditor, IDC_CMD_LN_BCOLOR, GCM_NEW_COLOR, 0, (LPARAM)&g_TempLineNumbersFormat.cBack);
	SendDlgItemMessageW(m_hEditor, IDC_CMD_LN_FCOLOR, GCM_NEW_COLOR, 0, (LPARAM)&g_TempLineNumbersFormat.cText);
	SendDlgItemMessageW(m_hEditor, IDC_GC_BREAKS, GCM_NEW_COLOR, 0, (LPARAM)&g_TempBreaksColor);
	SendDlgItemMessageW(m_hEditor, IDC_GC_WS, GCM_NEW_COLOR, 0, (LPARAM)&g_TempTabsColor);
	SendDlgItemMessageW(m_hEditor, IDC_GC_LEFT_BAR, GCM_NEW_COLOR, 0, (LPARAM)&g_TempBMColor);
	CreateCRLF(hwnd, &m_hTempCR, &m_hTempLF, g_TempBreaksColor, g_TempTextAreaFormat);
	SetDlgItemInt(m_hEditor, IDC_EDT_TAB_SIZE, 4, FALSE);
	g_TempSettings.tabStop = T_SIZE_DEF;
	tabStop = g_TempSettings.tabStop * 4;
	SendMessageW(m_hPreview, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);
	InvalidateRect(m_hPreview, NULL, FALSE);
	CheckDlgButton(m_hEditor, IDC_CHK_AUTO_URL, BST_UNCHECKED);
	BitOff(&g_TempSettings.res1, SB_AUTO_URL);
	//prefs dialog
	CheckDlgButton(m_hPrefs, IDC_OPT_NEW_TAB, BST_CHECKED);
	CheckDlgButton(m_hPrefs, IDC_OPT_EXISTING_TAB, BST_UNCHECKED);
	g_TempSettings.openNew = TRUE;
	SetDlgItemInt(m_hPrefs, IDC_EDT_TAB_WIDTH, T_WIDTH_DEF, FALSE);
	g_TempSettings.tabWidth = T_WIDTH_DEF;
	CheckDlgButton(m_hPrefs, IDC_OPT_BLANK, BST_CHECKED);
	CheckDlgButton(m_hPrefs, IDC_OPT_LAST_SESSION, BST_UNCHECKED);
	g_TempSettings.lastSession = FALSE;
	CheckDlgButton(m_hPrefs, IDC_CHK_MIN_TO_TRAY, BST_CHECKED);
	g_TempSettings.minTotray = TRUE;
	CheckDlgButton(m_hPrefs, IDC_CHK_SINGLE_INST, BST_CHECKED);
	g_TempSettings.singleInstance = TRUE;
	CheckDlgButton(m_hPrefs, IDC_CHK_TRACK, BST_UNCHECKED);
	g_TempSettings.trackChanges = FALSE;
	CheckDlgButton(m_hPrefs, IDC_CHK_PRESERVE_POSITION, BST_UNCHECKED);
	CheckDlgButton(m_hPrefs, IDC_CHK_JUMP_END, BST_UNCHECKED);
	CheckDlgButton(m_hPrefs, IDC_CHK_JUMP_END, BST_CHECKED);
	g_TempSettings.preservePosition = POS_END;
	CheckDlgButton(m_hPrefs, IDC_CHK_NEW_VERSION, BST_UNCHECKED);
	g_TempSettings.checkNVOnStart = FALSE;
	*g_sTempDefBrowser = '\0';
	SetDlgItemTextW(m_hPrefs, IDC_EDT_DEF_BROWSER, g_sTempDefBrowser);
}

static void CreateAllGCP(HWND hwnd){
	RECT		rc;
	int			x, cx, cy;
	HFONT		hFont;

	hFont = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0);
	GetWindowRect(GetDlgItem(hwnd, IDC_EDT_LINES_WIDTH), &rc);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);
	x = rc.left;
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;
	GetWindowRect(GetDlgItem(hwnd, IDC_UPD_LINES_WIDTH), &rc);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);
	cx += rc.right - rc.left;
	//back color of lines numbering area
	m_hGCP = CreateGCP(hwnd, IDC_ST_LINES_BCOLOR, IDC_CMD_LN_BCOLOR, &g_TempLineNumbersFormat.cBack, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	//text color of lines numbering area
	m_hGCP = CreateGCP(hwnd, IDC_ST_LINES_FCOLOR, IDC_CMD_LN_FCOLOR, &g_TempLineNumbersFormat.cText, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	//selection bar back color
	m_hGCP = CreateGCP(hwnd, IDC_ST_LEFT_BAR, IDC_GC_LEFT_BAR, &g_TempBMColor, x, cx, cy);
	//-----------	Text area	-----------
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	GetWindowRect(GetDlgItem(hwnd, IDC_CMD_TEXT_FONT), &rc);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);
	x = rc.left;
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;
	//background color
	m_hGCP = CreateGCP(hwnd, IDC_ST_TEXT_BCOLOR, IDC_CMD_TEXT_BCOLOR, &g_TempTextAreaFormat.cBack, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	//text color
	m_hGCP = CreateGCP(hwnd, IDC_ST_TEXT_FCOLOR, IDC_CMD_TEXT_FCOLOR, &g_TempTextAreaFormat.cText, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	//-----------	Whitespaces area	-----------
	//line breaks color
	m_hGCP = CreateGCP(hwnd, IDC_CHK_SHOW_BRAKES, IDC_GC_BREAKS, &g_TempBreaksColor, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	//tabs color
	m_hGCP = CreateGCP(hwnd, IDC_CHK_SHOW_TABS, IDC_GC_WS, &g_TempTabsColor, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	//spaces color
	m_hGCP = CreateGCP(hwnd, IDC_CHK_SHOW_SPACES, IDC_GC_SPACES, &g_TempSpaceColor, x, cx, cy);
	SendMessageW(m_hGCP, WM_SETFONT, (WPARAM)hFont, TRUE);
	EnableGCS();
}

static void EnableGCS(void){
	if(!g_TempSettings.showWS){
		EnableWindow(GetDlgItem(m_hEditor, IDC_GC_BREAKS), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_GC_WS), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_GC_SPACES), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CHK_SHOW_BRAKES), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CHK_SHOW_TABS), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CHK_SHOW_SPACES), FALSE);
	}
	else{
		EnableWindow(GetDlgItem(m_hEditor, IDC_CHK_SHOW_BRAKES), TRUE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CHK_SHOW_TABS), TRUE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CHK_SHOW_SPACES), TRUE);
		if(g_TempSettings.showCRLF)
			EnableWindow(GetDlgItem(m_hEditor, IDC_GC_BREAKS), TRUE);
		else
			EnableWindow(GetDlgItem(m_hEditor, IDC_GC_BREAKS), FALSE);
		if(g_TempSettings.showTabs)
			EnableWindow(GetDlgItem(m_hEditor, IDC_GC_WS), TRUE);
		else
			EnableWindow(GetDlgItem(m_hEditor, IDC_GC_WS), FALSE);
		if(g_TempSettings.showSpaces)
			EnableWindow(GetDlgItem(m_hEditor, IDC_GC_SPACES), TRUE);
		else
			EnableWindow(GetDlgItem(m_hEditor, IDC_GC_SPACES), FALSE);
	}
	if(!g_TempSettings.showLines){
		// EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_TEXT_BCOLOR), FALSE);
		// EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_TEXT_FCOLOR), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_BCOLOR), FALSE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_FCOLOR), FALSE);
	}
	else{
		// EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_TEXT_BCOLOR), TRUE);
		// EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_TEXT_FCOLOR), TRUE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_BCOLOR), TRUE);
		EnableWindow(GetDlgItem(m_hEditor, IDC_CMD_LN_FCOLOR), TRUE);
	}
}

static HWND CreateGCP(HWND hwnd, int idStatic, int idGCP, PGCOLORTYPE pgcp, int x, int cx, int cy){
	HWND		hGCP;
	RECT		rcStatic;
	int			y;

	GetWindowRect(GetDlgItem(hwnd, idStatic), &rcStatic);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rcStatic, 2);
	y = rcStatic.top - (cy - (rcStatic.bottom - rcStatic.top)) / 2;
	hGCP = CreateGCPickerWindowW(x, y, cx, cy, hwnd, idGCP, pgcp);
	BringWindowToTop(hGCP);

	return hGCP;
}

static BOOL GetFont(wchar_t * lpCaption, P_TPFORMAT lptpf){

	CHOOSEFONTW		cf;
	BOOL			fResult;
	HDC				hdc;

	hdc = GetDC(g_hMain);
	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hInstance = g_hInstance;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_ENABLEHOOK;
	cf.hwndOwner = m_hEditor;
	cf.lpLogFont = &lptpf->lf;
	cf.lCustData = (int)lpCaption;
	cf.lpfnHook = CFHookProc;
	fResult = ChooseFontW(&cf);
	lptpf->fontHeight = -MulDiv(lptpf->lf.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
	ReleaseDC(g_hMain, hdc);
	return fResult;
}

static HWND CreatePreviewEdit(void){
	HWND		hEdit, hStatic;
	int			staticStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD | SS_OWNERDRAW;
	RECT		rc;
	CHARFORMAT2	cr;
	SETTEXTEX	st;
	int			tabStop, staticWidth;

	GetWindowRect(GetDlgItem(m_hEditor, IDC_RICH_PREVIEW), &rc);
	MapWindowPoints(HWND_DESKTOP, m_hEditor, (LPPOINT)&rc, 2);
	hEdit = CreateWindowExW(WS_EX_STATICEDGE, RICHEDIT_CLASSW, NULL, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL | ES_SAVESEL | ES_WANTRETURN, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, m_hEditor, NULL, g_hInstance, NULL);
	GetClientRect(hEdit, &rc);
	SetWindowLongPtrW(hEdit, GWLP_USERDATA, (LONG_PTR)SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)PreviewEditProc));
	if(g_TempSettings.showLines)
		staticWidth = g_TempSettings.marginWidth + L_BOOKMARK;
	else
		staticWidth = L_BOOKMARK;
	hStatic = CreateWindowExW(0, L"STATIC", NULL, staticStyle, 0, 0, staticWidth, rc.bottom - rc.top, hEdit, NULL, g_hInstance, NULL);
	m_hStaticPreview = hStatic;
	SendMessageW(hEdit, EM_SETTEXTMODE, TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE, 0);
	SendMessageW(hEdit, EM_SETEDITSTYLE, SES_XLTCRCRLFTOCR, SES_XLTCRCRLFTOCR);
	SendMessageW(hEdit, EM_LIMITTEXT, -1, 0);
	SendMessageW(hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	rc.top += 2;
	if(g_TempSettings.showLines)
		rc.left += g_TempSettings.marginWidth + L_BOOKMARK;
	else
		rc.left += L_BOOKMARK;
	SendMessageW(hEdit, EM_SETRECTNP, 0, (LPARAM)&rc);
	SendMessageW(hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
	SendMessageW(hEdit, EM_SETMODIFY, FALSE, 0);
	SendMessageW(hEdit, WM_SETFONT, (WPARAM)m_TempTextFont, TRUE);
	if(g_TempTextAreaFormat.cBack.sysIndex != -1)
		SendMessageW(hEdit, EM_SETBKGNDCOLOR, 0, GetSysColor(g_TempTextAreaFormat.cBack.sysIndex));
	else
		SendMessageW(hEdit, EM_SETBKGNDCOLOR, 0, g_TempTextAreaFormat.cBack.crValue);
	ZeroMemory(&cr, sizeof(cr));
	cr.cbSize = sizeof(cr);
	cr.dwMask = CFM_COLOR;
	if(g_TempTextAreaFormat.cText.sysIndex != -1)
		cr.crTextColor = GetSysColor(g_TempTextAreaFormat.cText.sysIndex);
	else
		cr.crTextColor = g_TempTextAreaFormat.cText.crValue;
	SendMessageW(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cr);

	if(g_TempSettings.wordWrap)
		SendMessageW(hEdit, EM_SETTARGETDEVICE, 0, 0);

	tabStop = g_TempSettings.tabStop * 4;
	SendMessageW(hEdit, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);

	st.flags = 0;
	st.codepage = 1200;
	SendMessageW(hEdit, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)PREVIEW_STRING);

	return hEdit;
}

static LRESULT CALLBACK PreviewEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_PAINT:{
			HideCaret(hwnd);
			CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
			RedrawWindow(m_hStaticPreview, NULL, NULL, RDW_INVALIDATE);
			if(g_TempSettings.showWS && (g_TempSettings.showCRLF || g_TempSettings.showTabs || g_TempSettings.showSpaces))
				DrawCRLFWhiteSpace(hwnd, TRUE, g_TempSettings, g_TempTabsColor, g_TempSpaceColor, m_TempTextHeight, m_hTempCR, m_hTempLF);
			ShowCaret(hwnd);
			return FALSE;
		}
		case WM_DRAWITEM:{
			LPDRAWITEMSTRUCT	lpdi = (LPDRAWITEMSTRUCT)lParam;
			if(!g_TempSettings.wordWrap)
				DrawLineNumbersRegular(hwnd, lpdi->hDC, g_TempSettings, g_TempBMColor, g_TempLineNumbersFormat, m_TempLinesFont, m_TempTextHeight, m_TempLineNumberHeight, NULL);
			else
				DrawLineNumbersWrap(hwnd, lpdi->hDC, g_TempSettings, g_TempBMColor, g_TempLineNumbersFormat, m_TempLinesFont, m_TempTextHeight, m_TempLineNumberHeight, NULL, m_ImlSettings);
			
			return FALSE;
		}
		default:
			return CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
	}
}

static void CleanSettings(void){
	ImageList_Destroy(m_ImlSettings);
	g_hSettings = NULL;
	if(m_TempTextFont)
		DeleteObject(m_TempTextFont);
	if(m_TempLinesFont)
		DeleteObject(m_TempLinesFont);
	if(m_hColors){
		SendMessageW(m_hColors, GCM_CLOSE, 0, 0);
		m_hColors = NULL;
	}
	if(m_hGCP){
		SendMessageW(m_hGCP, GCM_CLOSE, 0, 0);
		m_hGCP = NULL;
	}
	if(m_hTempCR)
		DeleteObject(m_hTempCR);
	if(m_hTempLF)
		DeleteObject(m_hTempLF);
}

static void GetBrowser(void){
	OPENFILENAMEW		ofn;
	wchar_t				szPath[MAX_PATH];

	*szPath = '\0';

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hPrefs;
	ofn.hInstance = g_hInstance;
	ofn.lpstrTitle = g_Strings.sPrefBrowserCaption;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrFilter = EXE_FILTER;
	ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	if(GetOpenFileNameW(&ofn)){
		SetDlgItemTextW(m_hPrefs, IDC_EDT_DEF_BROWSER, szPath);
	}
}

UINT APIENTRY CFHookProc(HWND hdlg,	UINT uiMsg,	WPARAM wParam, LPARAM lParam){
	RECT			rc1, rc2, rc3, rc4;
	int				offset;

	if(uiMsg == WM_INITDIALOG){
		if(m_HideBold){
			GetWindowRect(GetDlgItem(hdlg, cmb1), &rc1);
			MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc1, 2);
			offset = rc1.left;
			GetWindowRect(GetDlgItem(hdlg, cmb3), &rc2);
			GetWindowRect(GetDlgItem(hdlg, IDOK), &rc3);
			GetWindowRect(GetDlgItem(hdlg, IDCANCEL), &rc4);
			MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc2, 2);
			MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc3, 2);
			MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc4, 2);
			SetWindowPos(GetDlgItem(hdlg, IDOK), 0, rc2.right - (rc3.right - rc3.left), rc3.top, 0, 0, SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hdlg, IDCANCEL), 0, rc2.right - (rc4.right - rc4.left), rc4.top, 0, 0, SWP_NOSIZE);
			GetWindowRect(GetDlgItem(hdlg, stc2), &rc1);
			GetWindowRect(GetDlgItem(hdlg, cmb2), &rc2);
			MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc1, 2);
			MapWindowPoints(HWND_DESKTOP, hdlg, (LPPOINT)&rc2, 2);
			SetWindowPos(GetDlgItem(hdlg, stc3), 0, rc1.left, rc1.top, 0, 0, SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hdlg, cmb3), 0, rc2.left, rc2.top, 0, 0, SWP_NOSIZE);
			ShowWindow(GetDlgItem(hdlg, cmb2), SW_HIDE);
			ShowWindow(GetDlgItem(hdlg, stc2), SW_HIDE);
			GetWindowRect(GetDlgItem(hdlg, IDOK), &rc3);
			GetWindowRect(hdlg, &rc1);
			SetWindowPos(hdlg, 0, 0, 0, rc3.right - rc1.left + offset, rc1.bottom - rc1.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		CHOOSEFONTW * lpcf = (CHOOSEFONTW *)lParam;
		SetWindowTextW(hdlg, (LPCWSTR)lpcf->lCustData);
	}
	return 0;
}
