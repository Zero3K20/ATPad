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

#define WIN32_LEAN_AND_MEAN  /* speed up compilations */
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <richedit.h>
#include <stdlib.h>
#include <limits.h>
#include <strsafe.h>

#include "find.h"
#include "stringconstants.h"
#include "numericconstants.h"
#include "globalvars.h"
#include "main.h"
#include "shared.h"

static BOOL Find_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void Find_OnClose(HWND hwnd);
static void Find_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void Find_OnDestroy(HWND hwnd);
static void Find_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
static void Find_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
static void PrepareFindControls(HWND hwnd);
static P_FRC_ITEM SearchInFile(HWND hEdit, FINDTEXTEXW * pftx, int params);
static void FindAll(HWND hwnd);
static int ReplaceEditText(HWND hEdit, int params);
static void ReplaceAll(HWND hwnd);
static void ShowReplaceResults(wchar_t * lpText, int number);

BOOL CALLBACK Find_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_INITDIALOG, Find_OnInitDialog);
	HANDLE_MSG (hwnd, WM_CLOSE, Find_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, Find_OnCommand);
	HANDLE_MSG (hwnd, WM_DESTROY, Find_OnDestroy);
	HANDLE_MSG (hwnd, WM_DRAWITEM, Find_OnDrawItem);
	HANDLE_MSG (hwnd, WM_MEASUREITEM, Find_OnMeasureItem);

	default: return FALSE;
	}
}

static void Find_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	lpMeasureItem->itemHeight -= 2;
}

static void Find_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	if(lpDrawItem->CtlType == ODT_COMBOBOX){
		DrawComboItem(lpDrawItem);
	}
}

static void Find_OnDestroy(HWND hwnd)
{
	g_hFind = NULL;
}

static void Find_OnClose(HWND hwnd)
{
	DestroyWindow(hwnd);
}

static void Find_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	wchar_t				szMessage[1024];

	switch(id){
	case IDCANCEL:
		DestroyWindow(hwnd);
		break;
	case IDC_CMD_FIND_ALL:
		FindAll(hwnd);
		break;
	case IDC_CMD_REPLACE_ALL:
		ReplaceAll(hwnd);
		break;
	case IDC_CMD_FIND_NEXT:
		if(codeNotify == BN_CLICKED){
			GetWindowTextW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT), g_SearchString, 512);
			if(SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_FINDSTRINGEXACT, -1, (LPARAM)g_SearchString) == CB_ERR){
				SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_INSERTSTRING, 0, (LPARAM)g_SearchString);
				AddToHistory(hwnd, IDC_CBO_FIND_WHAT);
			}
			g_FParam = 0;
			if(IsDlgButtonChecked(hwnd, IDC_OPT_DOWN) == BST_CHECKED){
				g_FParam |= FR_DOWN;
				g_SearchParams.updown = FR_DOWN;
			}
			else{
				g_SearchParams.updown = 0;
			}
			if(IsDlgButtonChecked(hwnd, IDC_CHK_MATCH_CASE) == BST_CHECKED){
				g_FParam |= FR_MATCHCASE;
				g_SearchParams.matchcase = FR_MATCHCASE;
			}
			else{
				g_SearchParams.matchcase = 0;
			}
			if(IsDlgButtonChecked(hwnd, IDC_OPT_ALL_FILES) == BST_CHECKED){
				g_SearchParams.alldocs = 1;
			}
			else{
				g_SearchParams.alldocs = 0;
			}
			if(IsDlgButtonChecked(hwnd, IDC_CHK_WHOLE_WORD) == BST_CHECKED){
				g_FParam |= FR_WHOLEWORD;
				g_SearchParams.wholeword = FR_WHOLEWORD;
			}
			else{
				g_SearchParams.wholeword = 0;
			}
			FindOnClick();
		}
		break;
	case IDC_CMD_REPLACE:
		if(codeNotify == BN_CLICKED){
			HWND			hEdit;
			int				params = FR_DOWN;
			P_TPEDIT		pE;
			CHARRANGE		chrg;
			GETTEXTLENGTHEX	gtx;
			BOOL			bRepeat = TRUE;

			GetWindowTextW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT), g_SearchString, 512);
			if(SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_FINDSTRINGEXACT, -1, (LPARAM)g_SearchString) == CB_ERR){
				SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_INSERTSTRING, 0, (LPARAM)g_SearchString);
				AddToHistory(hwnd, IDC_CBO_FIND_WHAT);
			}
			
			g_FParam = 0;
			if(IsDlgButtonChecked(hwnd, IDC_OPT_DOWN) == BST_CHECKED){
				g_FParam |= FR_DOWN;
				g_SearchParams.updown = FR_DOWN;
			}
			else{
				g_SearchParams.updown = 0; 
			}
			if(IsDlgButtonChecked(hwnd, IDC_CHK_MATCH_CASE) == BST_CHECKED){
				g_FParam |= FR_MATCHCASE;
				params |= FR_MATCHCASE;
				g_SearchParams.matchcase = FR_MATCHCASE;
			}
			else{
				g_SearchParams.matchcase = 0;
			}
			if(IsDlgButtonChecked(hwnd, IDC_CHK_WHOLE_WORD) == BST_CHECKED){
				g_FParam |= FR_WHOLEWORD;
				params |= FR_WHOLEWORD;
				g_SearchParams.wholeword = FR_WHOLEWORD;
			}
			else{
				g_SearchParams.wholeword = 0;
			}
			if(IsDlgButtonChecked(hwnd, IDC_OPT_ALL_FILES) == BST_CHECKED){
				g_SearchParams.alldocs = 1;
			}
			else{
				g_SearchParams.alldocs = 0;
			}
			GetWindowTextW(GetDlgItem(hwnd, IDC_CBO_REPLACE_WITH), g_ReplaceString, 512);
			if(SendDlgItemMessageW(hwnd, IDC_CBO_REPLACE_WITH, CB_FINDSTRINGEXACT, -1, (LPARAM)g_ReplaceString) == CB_ERR){
				SendDlgItemMessageW(hwnd, IDC_CBO_REPLACE_WITH, CB_INSERTSTRING, 0, (LPARAM)g_ReplaceString);
				AddToHistory(hwnd, IDC_CBO_REPLACE_WITH);
			}
			StringCchCopyW(szMessage, ARRAYSIZE(szMessage), g_Strings.sNoOccurrences);
			StringCchCatW(szMessage, ARRAYSIZE(szMessage), L"\n");
			StringCchCatW(szMessage, ARRAYSIZE(szMessage), g_SearchString);
			while(bRepeat){
				bRepeat = FALSE;
				hEdit = (HWND)SendMessageW(g_hMain, TBNPM_GETACTIVEEDIT, 0, 0);
				if(hEdit){
					if(ReplaceEditText(hEdit, -1) == -1){
						if(g_SearchParams.alldocs == 0){
							MessageBoxW(hwnd, szMessage, PROGRAM_NAME, MB_OK);
							g_RepIndex = 0;
						}
						else{
							pE = (P_TPEDIT)SendMessageW(g_hMain, TBNPM_GET_PE_BY_TAB, g_RepIndex++, 0);
							if(pE){
								if((g_FParam & FR_DOWN) == FR_DOWN){
									chrg.cpMin = 0;
								}
								else{
									//get total number of chars in control
									gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
									gtx.codepage = 1200;
									chrg.cpMin = SendMessageW(pE->hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0) - 1;
								}
								chrg.cpMax = chrg.cpMin;
								SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
								SendMessageW(g_hMain, TBNPM_ACT_BY_EDIT, (WPARAM)pE->hEdit, 0);
								bRepeat = TRUE;
							}
							else{
								MessageBoxW(hwnd, szMessage, PROGRAM_NAME, MB_OK);
								g_RepIndex = 0;
							}
						}
					}
				}
				else{
					MessageBoxW(hwnd, szMessage, PROGRAM_NAME, MB_OK);
					g_RepIndex = 0;
				}
			}
		}
		break;
	case IDC_CBO_FIND_WHAT:
		if(codeNotify == CBN_EDITCHANGE){
			if(GetWindowTextLengthW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT)) > 0){
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_NEXT), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_ALL), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), TRUE);
			}
			else{
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_NEXT), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_ALL), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), FALSE);
			}
			g_FindIndex = 0;
		}
		else if(codeNotify == CBN_SELENDOK){
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_NEXT), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_ALL), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), TRUE);
			g_FindIndex = 0;
		}
		break;
	case IDC_CBO_REPLACE_WITH:
		if(codeNotify == CBN_EDITCHANGE){
			if(GetWindowTextLengthW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT)) > 0){
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), TRUE);
			}
			else{
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), FALSE);
			}
		}
		else if(codeNotify == CBN_SELENDOK){
			if(GetWindowTextLengthW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT)) > 0){
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), TRUE);
			}
		}
		break;
	}
}

static BOOL Find_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	wchar_t		szCaption[128];
	wchar_t		szLang[MAX_PATH];

	g_hFind = hwnd;
	g_FindIndex = g_RepIndex = 0;

	StringCchCopyW(szLang, ARRAYSIZE(szLang), g_Paths.sLangDir);
	StringCchCatW(szLang, ARRAYSIZE(szLang), g_Paths.sLangFile);

	if(lParam == 0){
		PrepareFindControls(hwnd);
		GetPrivateProfileStringW(S_STRINGS, L"find", L"Find", szCaption, 128, szLang);
	}
	else if(lParam == 1){
		GetPrivateProfileStringW(S_STRINGS, L"replace", L"Replace", szCaption, 128, szLang);
	}

	if(g_SearchParams.matchcase){
		CheckDlgButton(hwnd, IDC_CHK_MATCH_CASE, BST_CHECKED);
	}
	if(g_SearchParams.wholeword){
		CheckDlgButton(hwnd, IDC_CHK_WHOLE_WORD, BST_CHECKED);
	}
	if(g_SearchParams.updown == FR_DOWN)
		CheckDlgButton(hwnd, IDC_OPT_DOWN, BST_CHECKED);
	else
		CheckDlgButton(hwnd, IDC_OPT_UP, BST_CHECKED);
	if(g_SearchParams.alldocs)
		CheckDlgButton(hwnd, IDC_OPT_ALL_FILES, BST_CHECKED);
	else
		CheckDlgButton(hwnd, IDC_OPT_CURR_FILE, BST_CHECKED);

	LoadFromHistory(hwnd, IDC_CBO_FIND_WHAT);
	LoadFromHistory(hwnd, IDC_CBO_REPLACE_WITH);

	SetWindowTextW(hwnd, szCaption);
	SetControlText(hwnd, IDC_ST_FIND_WHAT, L"Find what:", szLang);
	SetControlText(hwnd, IDC_ST_REPLACE_WITH, L"Replace with:", szLang);
	SetControlText(hwnd, IDC_CMD_FIND_NEXT, L"Find Next", szLang);
	SetControlText(hwnd, IDC_CMD_FIND_ALL, L"Find All", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);
	SetControlText(hwnd, IDC_CMD_REPLACE, L"Replace", szLang);
	SetControlText(hwnd, IDC_CMD_REPLACE_ALL, L"Replace All", szLang);
	SetControlText(hwnd, IDC_CHK_MATCH_CASE, L"Match case", szLang);
	SetControlText(hwnd, IDC_CHK_WHOLE_WORD, L"Whole word", szLang);
	SetControlText(hwnd, IDC_GRP_DIRECTION, L"Direction", szLang);
	SetControlText(hwnd, IDC_OPT_UP, L"Up", szLang);
	SetControlText(hwnd, IDC_OPT_DOWN, L"Down", szLang);
	SetControlText(hwnd, IDC_GRP_SEARCH_SCOPE, L"Find/Replace scope", szLang);
	SetControlText(hwnd, IDC_OPT_CURR_FILE, L"Current document", szLang);
	SetControlText(hwnd, IDC_OPT_ALL_FILES, L"All open documents", szLang);

	CheckForSelection(hwnd, lParam);

	SetFocus(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT));
	return FALSE;
}

static void PrepareFindControls(HWND hwnd){
	RECT				rc1, rc2, rc3;
	WINDOWPLACEMENT		wp;
	int					style1, style2, right, bottom;

	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	GetWindowPlacement(GetDlgItem(hwnd, IDC_CMD_REPLACE), &wp);
	SetWindowPlacement(GetDlgItem(hwnd, IDCANCEL), &wp);
	ShowWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), SW_HIDE);

	right = wp.rcNormalPosition.right + 4;

	GetWindowRect(GetDlgItem(hwnd, IDC_CHK_MATCH_CASE), &rc1);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc1, 2);
	GetWindowRect(GetDlgItem(hwnd, IDC_GRP_DIRECTION), &rc3);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc3, 2);
	SetWindowPos(GetDlgItem(hwnd, IDC_GRP_DIRECTION), 0, rc1.left, rc1.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hwnd, IDC_GRP_SEARCH_SCOPE), 0, rc3.left, rc3.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

	GetWindowRect(GetDlgItem(hwnd, IDC_ST_REPLACE_WITH), &rc1);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc1, 2);
	SetWindowPos(GetDlgItem(hwnd, IDC_CHK_MATCH_CASE), 0, rc1.left, rc1.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
	GetWindowRect(GetDlgItem(hwnd, IDC_CBO_REPLACE_WITH), &rc1);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc1, 2);
	SetWindowPos(GetDlgItem(hwnd, IDC_CHK_WHOLE_WORD), 0, rc1.left, rc1.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
	ShowWindow(GetDlgItem(hwnd, IDC_ST_REPLACE_WITH), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_CBO_REPLACE_WITH), SW_HIDE);

	GetWindowRect(GetDlgItem(hwnd, IDC_GRP_DIRECTION), &rc1);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc1, 2);
	GetWindowRect(GetDlgItem(hwnd, IDC_OPT_UP), &rc2);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc2, 2);
	SetWindowPos(GetDlgItem(hwnd, IDC_OPT_UP), 0, rc2.left, rc1.top + 20, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
	GetWindowRect(GetDlgItem(hwnd, IDC_OPT_DOWN), &rc2);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc2, 2);
	SetWindowPos(GetDlgItem(hwnd, IDC_OPT_DOWN), 0, rc2.left, rc1.top + 20, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

	GetWindowRect(GetDlgItem(hwnd, IDC_GRP_SEARCH_SCOPE), &rc3);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc3, 2);
	bottom = rc3.bottom + 16;
	GetWindowRect(GetDlgItem(hwnd, IDC_OPT_CURR_FILE), &rc2);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc2, 2);
	SetWindowPos(GetDlgItem(hwnd, IDC_OPT_CURR_FILE), 0, rc2.left, rc3.top + 20, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hwnd, IDC_OPT_ALL_FILES), 0, rc2.left, rc3.top + (rc2.bottom - rc2.top) + 24, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

	style1 = GetWindowLongPtrW(hwnd, GWL_STYLE);
	style2 = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	SetRect(&rc1, 0, 0, right, bottom);
	MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&rc1, 2);
	AdjustWindowRectEx(&rc1, style1, FALSE, style2);
	MoveWindow(hwnd, rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top, TRUE);
}

void CheckForSelection(HWND hwnd, int type){
	HWND		hEdit;
	CHARRANGE	chrg;
	
	hEdit = (HWND)SendMessageW(g_hMain, TBNPM_GETACTIVEEDIT, 0, 0);
	SendMessageW(hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
	if(chrg.cpMax > chrg.cpMin){
		long long	selectionLen;
		int			textLen;
		wchar_t		*szBuffer, *pw;

		selectionLen = (long long)chrg.cpMax - (long long)chrg.cpMin;
		if(selectionLen > INT_MAX - 1){
			return;
		}
		textLen = (int)selectionLen + 1;
		szBuffer = malloc(sizeof(wchar_t) * textLen);
		if(!szBuffer){
			return;
		}
		pw = malloc(sizeof(wchar_t) * textLen);
		if(!pw){
			free(szBuffer);
			return;
		}

		EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_NEXT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_ALL), TRUE);
		SendMessageW(hEdit, EM_GETSELTEXT, 0, (LPARAM)pw);
		
		for(int i = 0; i < textLen; i++){
			if(pw[i] != '\n' && pw[i] != '\r'){
				szBuffer[i] = pw[i];
			}
			else{
				szBuffer[i] = '\0';
				break;
			}
		}
		SetWindowTextW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT), szBuffer);
		free(szBuffer);
		free(pw);
	}
	else{
		if(wcslen(g_SearchString) > 0){
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_NEXT), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_FIND_ALL), TRUE);
			SetWindowTextW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT), g_SearchString);
		}
	}
	if(type == 1){
		if(wcslen(g_ReplaceString) > 0 || IsWindowEnabled(GetDlgItem(hwnd, IDC_CMD_FIND_NEXT))){
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDC_CMD_REPLACE_ALL), TRUE);
			SetWindowTextW(GetDlgItem(hwnd, IDC_CBO_REPLACE_WITH), g_ReplaceString);
		}
	}
}

int SearchText(HWND hEdit){
	FINDTEXTEXW		ftx;
	int				result;

	ZeroMemory(&ftx, sizeof(ftx));
	SendMessageW(hEdit, EM_EXGETSEL, 0, (LPARAM)&ftx.chrg);
	if((g_FParam & FR_DOWN) == FR_DOWN)
		ftx.chrg.cpMin = ftx.chrg.cpMax;
	ftx.chrg.cpMax = -1;
	ftx.lpstrText = g_SearchString;
	result = SendMessageW(hEdit, EM_FINDTEXTEXW, g_FParam, (LPARAM)&ftx);
	if(result != -1){
		result = SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&ftx.chrgText);
	}
	return result;
}

void AddToHistory(HWND hwnd, int id){
	int			count;
	wchar_t		szBuffer[512], szSection[32], szKey[12];

	count = SendDlgItemMessageW(hwnd, id, CB_GETCOUNT, 0, 0);
	if(id == IDC_CBO_FIND_WHAT){
		StringCchCopyW(szSection, ARRAYSIZE(szSection), S_FIND);
	}
	else{
		StringCchCopyW(szSection, ARRAYSIZE(szSection), S_REPLACE);
	}
	WritePrivateProfileSectionW(szSection, NULL, g_Paths.sINI);
	for(int i = 0; i < count; i++){
		SendDlgItemMessageW(hwnd, id, CB_GETLBTEXT, i, (LPARAM)szBuffer);
		_itow_s(i + 1, szKey, ARRAYSIZE(szKey), 10);
		WritePrivateProfileStringW(szSection, szKey, szBuffer, g_Paths.sINI);
	}
}

void LoadFromHistory(HWND hwnd, int id){
	wchar_t		szKeys[512], *pw, szBuffer[512], szSection[32];

	if(id == IDC_CBO_FIND_WHAT){
		StringCchCopyW(szSection, ARRAYSIZE(szSection), S_FIND);
	}
	else{
		StringCchCopyW(szSection, ARRAYSIZE(szSection), S_REPLACE);
	}
	GetPrivateProfileStringW(szSection, NULL, NULL, szKeys, 512, g_Paths.sINI);
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(szSection, pw, NULL, szBuffer, 512, g_Paths.sINI);
				SendDlgItemMessageW(hwnd, id, CB_INSERTSTRING, -1, (LPARAM)szBuffer);
			}
			while(*pw++)
				;
		}
	}
}

static void FindAll(HWND hwnd){
	int				count, total = 0;
	P_TPEDIT		pE;
	int				params = FR_DOWN;
	FINDTEXTEXW		ftx;
	P_FRC_ITEM		p = NULL;

	GetWindowTextW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT), g_SearchString, 512);
	if(SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_FINDSTRINGEXACT, -1, (LPARAM)g_SearchString) == CB_ERR){
		SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_INSERTSTRING, 0, (LPARAM)g_SearchString);
		AddToHistory(hwnd, IDC_CBO_FIND_WHAT);
	}
	if(IsDlgButtonChecked(hwnd, IDC_CHK_MATCH_CASE) == BST_CHECKED){
		params |= FR_MATCHCASE;
		g_SearchParams.matchcase = FR_MATCHCASE;
	}
	else{
		g_SearchParams.matchcase = 0;
	}
	if(IsDlgButtonChecked(hwnd, IDC_CHK_WHOLE_WORD) == BST_CHECKED){
		params |= FR_WHOLEWORD;
		g_SearchParams.wholeword = FR_WHOLEWORD;
	}
	else{
		g_SearchParams.wholeword = 0;
	}
	if(IsDlgButtonChecked(hwnd, IDC_OPT_ALL_FILES) == BST_CHECKED){
		g_SearchParams.alldocs = 1;
	}
	else{
		g_SearchParams.alldocs = 0;
	}
	
	count = SendMessageW(g_hMain, TBNPM_GET_TABS_CNT, 0, 0);
	SendMessageW(g_hSplit, TBNPM_CLEAR_LIST, 0, 0);
	
	if(IsDlgButtonChecked(hwnd, IDC_OPT_ALL_FILES) == BST_CHECKED){
		for(int i = 0; i < count; i++){
			ZeroMemory(&ftx, sizeof(ftx));
			pE = (P_TPEDIT)SendMessageW(g_hMain, TBNPM_GET_PE_BY_TAB, i, 0);
			do{
				p = SearchInFile(pE->hEdit, &ftx, params);
				if(p){
					total++;
					SendMessageW(g_hSplit, TBNPM_INS_FIND_STR, (WPARAM)pE->szLongName, (LPARAM)p);
				}
			}while(p);
		}
	}
	else{
		ZeroMemory(&ftx, sizeof(ftx));
		pE = (P_TPEDIT)SendMessageW(g_hMain, TBNPM_GET_ACT_HANDLE, 0, 0);
		do{
			p = SearchInFile(pE->hEdit, &ftx, params);
			if(p){
				total++;
				SendMessageW(g_hSplit, TBNPM_INS_FIND_STR, (WPARAM)pE->szLongName, (LPARAM)p);
			}
		}while(p);
	}
	SendMessageW(g_hSplit, TBNPM_INS_RES_STR, total, 0);
	if(!IsWindowVisible(g_hSplit)){
		ShowWindow(g_hSplit, SW_SHOW);
		SendMessageW(g_hMain, TBNPM_HSP_SIZED, 0, 0);
	}
	SendMessageW(g_hSplit, TBNPM_SELECT_ASS_TAB, IDM_F_R_FILES, 6);
	SendMessageW(hwnd, WM_CLOSE, 0, 0);
}

static void ReplaceAll(HWND hwnd){
	int				count, total = 0;
	P_TPEDIT		pE;
	int				params = FR_DOWN;

	GetWindowTextW(GetDlgItem(hwnd, IDC_CBO_FIND_WHAT), g_SearchString, 512);
	if(SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_FINDSTRINGEXACT, -1, (LPARAM)g_SearchString) == CB_ERR){
		SendDlgItemMessageW(hwnd, IDC_CBO_FIND_WHAT, CB_INSERTSTRING, 0, (LPARAM)g_SearchString);
		AddToHistory(hwnd, IDC_CBO_FIND_WHAT);
	}
	GetWindowTextW(GetDlgItem(hwnd, IDC_CBO_REPLACE_WITH), g_ReplaceString, 512);
	if(SendDlgItemMessageW(hwnd, IDC_CBO_REPLACE_WITH, CB_FINDSTRINGEXACT, -1, (LPARAM)g_ReplaceString) == CB_ERR){
		SendDlgItemMessageW(hwnd, IDC_CBO_REPLACE_WITH, CB_INSERTSTRING, 0, (LPARAM)g_ReplaceString);
		AddToHistory(hwnd, IDC_CBO_REPLACE_WITH);
	}
	if(IsDlgButtonChecked(hwnd, IDC_CHK_MATCH_CASE) == BST_CHECKED){
		params |= FR_MATCHCASE;
		g_SearchParams.matchcase = FR_MATCHCASE;
	}
	else{
		g_SearchParams.matchcase = 0;
	}
	if(IsDlgButtonChecked(hwnd, IDC_CHK_WHOLE_WORD) == BST_CHECKED){
		params |= FR_WHOLEWORD;
		g_SearchParams.matchcase = FR_WHOLEWORD;
	}
	else{
		g_SearchParams.wholeword = 0;
	}
	if(IsDlgButtonChecked(hwnd, IDC_OPT_ALL_FILES) == BST_CHECKED){
		g_SearchParams.alldocs = 1;
	}
	else{
		g_SearchParams.alldocs = 0;
	}
	// WritePrivateProfileStructW(S_PREFERENCES, K_SEARCH_PARAMS, &g_SearchParams, sizeof(g_SearchParams), g_Paths.sINI);
	count = SendMessageW(g_hMain, TBNPM_GET_TABS_CNT, 0, 0);
	for(int i = 0; i < count; i++){
		pE = (P_TPEDIT)SendMessageW(g_hMain, TBNPM_GET_PE_BY_TAB, i, 0);
		CHARRANGE	chrg;
		
		chrg.cpMin = 0;
		chrg.cpMax = 0;
		SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
		SendMessageW(g_hMain, TBNPM_TOGGLESP, 0, 0);
		while(ReplaceEditText(pE->hEdit, params) != -1)
			total++;
		SendMessageW(g_hMain, TBNPM_TOGGLESP, 0, 0);
	}
	ShowReplaceResults(g_SearchString, total);
	SendMessageW(hwnd, WM_CLOSE, 0, 0);
}

static int ReplaceEditText(HWND hEdit, int params){
	FINDTEXTEXW		ftx;
	long long		selectionLen;
	int				result;
	int				textLen;
	wchar_t			*szBuffer;

	if(params == -1)
		params = g_FParam;

	ZeroMemory(&ftx, sizeof(ftx));
	SendMessageW(hEdit, EM_EXGETSEL, 0, (LPARAM)&ftx.chrg);

	if(ftx.chrg.cpMax < ftx.chrg.cpMin){
		return -1;
	}
	selectionLen = (long long)ftx.chrg.cpMax - (long long)ftx.chrg.cpMin;
	if(selectionLen > INT_MAX - 1){
		return -1;
	}
	textLen = (int)selectionLen + 1;
	szBuffer = malloc(sizeof(wchar_t) * textLen);
	if(!szBuffer){
		return -1;
	}
	SendMessageW(hEdit, EM_GETSELTEXT, 0, (LPARAM)szBuffer);
	if(wcscmp(szBuffer, g_SearchString) == 0){
		SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)g_ReplaceString);
	}
	free(szBuffer);
	if((g_FParam & FR_DOWN) == FR_DOWN && *g_ReplaceString)
		ftx.chrg.cpMin = ftx.chrg.cpMax;
	ftx.chrg.cpMax = -1;
	ftx.lpstrText = g_SearchString;
	result = SendMessageW(hEdit, EM_FINDTEXTEXW, params, (LPARAM)&ftx);
	if(result != -1){
		result = SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&ftx.chrgText);
	}
	return result;
}

static P_FRC_ITEM SearchInFile(HWND hEdit, FINDTEXTEXW * pftx, int params){
	P_FRC_ITEM		p;
	int				result;

	pftx->chrg.cpMin = pftx->chrgText.cpMax;
	pftx->chrg.cpMax = -1;
	pftx->lpstrText = g_SearchString;
	result = SendMessageW(hEdit, EM_FINDTEXTEXW, params, (LPARAM)pftx);
	if(result != -1){
		p = malloc(sizeof(FRC_ITEM));
		if(g_Settings.wordWrap){
			GetCurrentPositionWrap(hEdit, pftx->chrgText, &p->row, &p->col);
		}
		else{
			GetCurrentPositionRegular(hEdit, pftx->chrgText, &p->row, &p->col);
		}
		p->min = pftx->chrgText.cpMin;
		p->max = pftx->chrgText.cpMax;
		p->hEdit = hEdit;
		return p;
	}
	return NULL;
}

static void ShowReplaceResults(wchar_t * lpText, int number){
	wchar_t			szBuffer[1024], szNumber[12];

	StringCchCopyW(szBuffer, ARRAYSIZE(szBuffer), g_Strings.sTotalFound);
	StringCchCatW(szBuffer, ARRAYSIZE(szBuffer), L" \"");
	StringCchCatW(szBuffer, ARRAYSIZE(szBuffer), lpText);
	StringCchCatW(szBuffer, ARRAYSIZE(szBuffer), L"\": ");
	_ltow_s(number, szNumber, ARRAYSIZE(szNumber), 10);
	StringCchCatW(szBuffer, ARRAYSIZE(szBuffer), szNumber);
	MessageBoxW(g_hMain, szBuffer, PROGRAM_NAME, MB_OK);
}

void FindOnClick(void){
	HWND			hEdit;
	CHARRANGE		chrg;
	P_TPEDIT		pE;
	GETTEXTLENGTHEX	gtx;
	wchar_t			szMessage[1024];
	BOOL			bRepeat = TRUE;

	if(wcslen(g_SearchString) == 0)
		return;
	StringCchCopyW(szMessage, ARRAYSIZE(szMessage), g_Strings.sNoOccurrences);
	StringCchCatW(szMessage, ARRAYSIZE(szMessage), L"\n");
	StringCchCatW(szMessage, ARRAYSIZE(szMessage), g_SearchString);
	while(bRepeat){
		bRepeat = FALSE;
		hEdit = (HWND)SendMessageW(g_hMain, TBNPM_GETACTIVEEDIT, 0, 0);
		if(hEdit){
			if(SearchText(hEdit) == -1){
				if(g_SearchParams.alldocs == 0){
					MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_OK);
					g_FindIndex = 0;
				}
				else{
					pE = (P_TPEDIT)SendMessageW(g_hMain, TBNPM_GET_PE_BY_TAB, g_FindIndex++, 0);
					if(pE){
						if((g_FParam & FR_DOWN) == FR_DOWN){
							chrg.cpMin = 0;
						}
						else{
							//get total number of chars in control
							gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
							gtx.codepage = 1200;
							chrg.cpMin = SendMessageW(pE->hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0) - 1;
						}
						chrg.cpMax = chrg.cpMin;
						SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
						SendMessageW(g_hMain, TBNPM_ACT_BY_EDIT, (WPARAM)pE->hEdit, 0);
						bRepeat = TRUE;
					}
					else{
						MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_OK);
						g_FindIndex = 0;
					}
				}
			}
		}
		else{
			MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_OK);
			g_FindIndex = 0;
		}
	}
}
