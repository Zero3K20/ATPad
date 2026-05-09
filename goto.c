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

#include "goto.h"
#include "stringconstants.h"
#include "numericconstants.h"
#include "globalvars.h"
#include "main.h"
#include "shared.h"

static void Goto_OnClose(HWND hwnd);
static void Goto_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL Goto_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

BOOL CALLBACK Goto_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
switch(msg)
	{
	HANDLE_MSG (hwnd, WM_CLOSE, Goto_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, Goto_OnCommand);
	HANDLE_MSG (hwnd, WM_INITDIALOG, Goto_OnInitDialog);

	default: return FALSE;
	}
}

static void Goto_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

static void Goto_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDCANCEL:
		if(codeNotify == BN_CLICKED){
			EndDialog(hwnd, 0);
		}
		break;
	case IDOK:
		if(codeNotify == BN_CLICKED){
			long		line = GetDlgItemInt(hwnd, IDC_EDT_GOTO, FALSE, FALSE);
			HWND		hEdit = (HWND)SendMessageW(g_hMain, TBNPM_GETACTIVEEDIT, 0, 0);
			long		count = SendMessageW(hEdit, EM_GETLINECOUNT, 0, 0);
			CHARRANGE	chrg;

			if(count >= line){
				if(!g_Settings.wordWrap){
					line--;
					chrg.cpMin = SendMessageW(hEdit, EM_LINEINDEX, line, 0);
					chrg.cpMax = chrg.cpMin;
					SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
				}
				else{
					if(line == 1){
						chrg.cpMin = 0;
						chrg.cpMax = 0;
					}
					else{
						GETTEXTLENGTHEX	gtx;
						void			* pTemp;
						long			charCount, charIndex = 0;
						TEXTRANGEW		trg;

						gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
						gtx.codepage = 1200;
						charCount = SendMessageW(hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
						trg.chrg.cpMin = 0;
						trg.chrg.cpMax = charCount - 1;
						trg.lpstrText = (wchar_t *)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, charCount * sizeof(wchar_t));
						pTemp = (void *)trg.lpstrText;
						SendMessageW(hEdit, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
						for(int i = 0; i < charCount && line > 1; i++, charIndex++){
							if(*trg.lpstrText++ == 13){
								line--;
							}
						}
						HeapFree(g_hHeap, 0, pTemp);
						line = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, charIndex);
						// line--;
						chrg.cpMin = SendMessageW(hEdit, EM_LINEINDEX, line, 0);
						chrg.cpMax = chrg.cpMin;
					}
					SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
				}

			}
			EndDialog(hwnd, 0);
		}
		break;
	case IDC_EDT_GOTO:
		if(codeNotify == EN_CHANGE){
			if(GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_GOTO)) == 0){
				SetDlgItemInt(hwnd, IDC_EDT_GOTO, 1, FALSE);
			}
		}
		break;
	}
}

static BOOL Goto_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	wchar_t		szCaption[128];
	wchar_t		szLang[MAX_PATH];
	HWND		hEdit;
	CHARRANGE	chrg;

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);
	GetPrivateProfileStringW(S_CONTROLS, L"1005", L"Go To", szCaption, 128, szLang);
	SetWindowTextW(hwnd, szCaption);
	SetControlText(hwnd, IDC_ST_GOTO, L"Go to line:", szLang);
	SetControlText(hwnd, IDOK, L"OK", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);

	hEdit = (HWND)SendMessageW(g_hMain, TBNPM_GETACTIVEEDIT, 0, 0);
	SendMessageW(hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
	if(!g_Settings.marginWidth){
		SetDlgItemInt(hwnd, IDC_EDT_GOTO, SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin) + 1, FALSE);
	}
	else{
		void			* pTemp;
		long			charCount = 1;
		TEXTRANGEW		trg;

		trg.chrg.cpMin = 0;
		trg.chrg.cpMax = chrg.cpMin;
		trg.lpstrText = (wchar_t *)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, (chrg.cpMin + 1) * sizeof(wchar_t));
		pTemp = (void *)trg.lpstrText;
		SendMessageW(hEdit, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
		for(int i = 0; i < chrg.cpMin; i++){
			if(*trg.lpstrText++ == 13)
				charCount++;
		}
		HeapFree(g_hHeap, 0, pTemp);
		SetDlgItemInt(hwnd, IDC_EDT_GOTO, charCount, FALSE);
	}
	return FALSE;
}


