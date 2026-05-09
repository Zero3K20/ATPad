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
#include <commctrl.h>
#include <richedit.h>
#include <stdlib.h>

#include "globalvars.h"
#include "shared.h"
#include "main.h"
#include "numericconstants.h"
#include "stringconstants.h"
#include "hotkeys.h"

static void Hotdlg_OnClose(HWND hwnd);
static void Hotdlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL Hotdlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void CreateImageList(HIMAGELIST *lpIml, int idBmp, COLORREF crMask, int x, int y, int imgCount);
static void CleanUp(HWND hwnd, BOOL withArray);
static void InsertCommands(HMENU hMenu, HTREEITEM htiParent);
static void FreeCommands(BOOL withArray, HTREEITEM hti);
static BOOL CALLBACK AccSet_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void AccSet_OnClose(HWND hwnd);
static void AccSet_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL AccSet_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL KeyExists(P_TP_TEMPACCEL pA, HTREEITEM hti);

static HIMAGELIST		m_ImlAcc;
static HWND				m_hTree;
static TP_TEMPACCEL		m_Acc;

BOOL CALLBACK Hotdlg_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	LPNMHDR			pnmh;
	P_TP_TEMPACCEL	pAcc;

	switch(msg){
		HANDLE_MSG (hwnd, WM_CLOSE, Hotdlg_OnClose);
		HANDLE_MSG (hwnd, WM_COMMAND, Hotdlg_OnCommand);
		HANDLE_MSG (hwnd, WM_INITDIALOG, Hotdlg_OnInitDialog);
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if(pnmh->code == TVN_SELCHANGEDW){
				LPNMTREEVIEW	pnmpt = (LPNMTREEVIEW)lParam;
				pAcc = (P_TP_TEMPACCEL)pnmpt->itemNew.lParam;
				if(pAcc){
					if(pAcc->acc.cmd){
						EnableWindow(GetDlgItem(hwnd, IDC_CMD_CHANGE_HK), TRUE);
						if(pAcc->acc.key){
							EnableWindow(GetDlgItem(hwnd, IDC_CMD_DELETE), TRUE);
						}
						else{
							EnableWindow(GetDlgItem(hwnd, IDC_CMD_DELETE), FALSE);
						}
					}
					else{
						EnableWindow(GetDlgItem(hwnd, IDC_CMD_CHANGE_HK), FALSE);
						EnableWindow(GetDlgItem(hwnd, IDC_CMD_DELETE), FALSE);
					}
					SetDlgItemTextW(hwnd, IDC_ST_HK_TEXT, pAcc->text);
				}
			}
			return FALSE;
		default:
			return FALSE;
	}
}

static void Hotdlg_OnClose(HWND hwnd)
{
	CleanUp(hwnd, FALSE);
	EndDialog(hwnd, IDCANCEL);
}

static void Hotdlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	P_TP_TEMPACCEL	pAcc;
	TVITEMW			tvi;
	int  			result;

	switch(id){
		case IDCANCEL:
			if(codeNotify == BN_CLICKED){
				CleanUp(hwnd, FALSE);
				EndDialog(hwnd, IDCANCEL);
			}
			break;
		case IDC_CMD_DEFAULT:
			if(codeNotify == BN_CLICKED){
				CleanUp(hwnd, FALSE);
				EndDialog(hwnd, IDC_CMD_DEFAULT);
			}
			break;
		case IDC_CMD_ADD:
			if(codeNotify == BN_CLICKED){
				CleanUp(hwnd, TRUE);
				EndDialog(hwnd, IDC_CMD_ADD);
			}
			break;
		case IDC_CMD_CHANGE_HK:
			if(codeNotify == BN_CLICKED){
				ZeroMemory(&tvi, sizeof(tvi));
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = TreeView_GetSelection(m_hTree);
				SendMessageW(m_hTree, TVM_GETITEMW, 0, (LPARAM)&tvi);
				pAcc = (P_TP_TEMPACCEL)tvi.lParam;
				memcpy(&m_Acc, pAcc, sizeof(m_Acc));
				result = DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_SET_ACC), hwnd, AccSet_DlgProc, 0);
				if(result == IDOK){
					if(!KeyExists(&m_Acc, TreeView_GetRoot(m_hTree))){
						if(m_Acc.acc.fVirt != pAcc->acc.fVirt || m_Acc.acc.key != pAcc->acc.key){
							memcpy(pAcc, &m_Acc, sizeof(m_Acc));
						}
					}
					else{
						MessageBoxW(hwnd, g_Strings.sKeyExists, PROGRAM_NAME, MB_OK);
					}
				}
				TreeView_Select(m_hTree, NULL, TVGN_CARET);
				TreeView_Select(m_hTree, tvi.hItem, TVGN_CARET);
				SetFocus(m_hTree);
			}
			break;
		case IDC_CMD_DELETE:
			if(codeNotify == BN_CLICKED){
				ZeroMemory(&tvi, sizeof(tvi));
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = TreeView_GetSelection(m_hTree);
				SendMessageW(m_hTree, TVM_GETITEMW, 0, (LPARAM)&tvi);
				pAcc = (P_TP_TEMPACCEL)tvi.lParam;
				pAcc->acc.fVirt = 0;
				pAcc->acc.key = 0;
				*pAcc->text = '\0';
				TreeView_Select(m_hTree, NULL, TVGN_CARET);
				TreeView_Select(m_hTree, tvi.hItem, TVGN_CARET);
				SetFocus(m_hTree);
			}
			break;
	}
}

static BOOL Hotdlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	wchar_t		szCaption[128];
	wchar_t		szLang[MAX_PATH];

	g_TempAccCount = 0;
	if(g_TempAcc)
		free(g_TempAcc);
	g_TempAcc = 0;

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);
	GetPrivateProfileStringW(S_CONTROLS, L"1007", L"Hot keys", szCaption, 128, szLang);
	SetWindowTextW(hwnd, szCaption);
	SetControlText(hwnd, IDC_ST_HK_COMMAND, L"Menu commands:", szLang);
	SetControlText(hwnd, IDC_ST_HK_KEYS, L"Keys:", szLang);
	SetControlText(hwnd, IDC_CMD_CHANGE_HK, L"Change keys", szLang);
	SetControlText(hwnd, IDC_CMD_DELETE, L"Delete keys", szLang);
	SetControlText(hwnd, IDC_CMD_ADD, L"Apply changes", szLang);
	SetControlText(hwnd, IDC_CMD_DEFAULT, L"Default settings", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);
	CreateImageList(&m_ImlAcc, IDB_ACC_IML, CLR_MASK, 16, 16, 2);
	m_hTree = GetDlgItem(hwnd, IDC_TVW_HK);
	TreeView_SetImageList(m_hTree, m_ImlAcc, TVSIL_NORMAL);
	InsertCommands(g_hMenu, NULL);
	return FALSE;
}

static void CleanUp(HWND hwnd, BOOL withArray){
	if(m_ImlAcc)
		ImageList_Destroy(m_ImlAcc);
	FreeCommands(withArray, TreeView_GetRoot(m_hTree));
}

static void CreateImageList(HIMAGELIST *lpIml, int idBmp, COLORREF crMask, int x, int y, int imgCount){
	HBITMAP			hBmp;

	*lpIml = ImageList_Create(x, y, ILC_MASK | ILC_COLOR24, imgCount, 0);
	hBmp = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(idBmp));
	ImageList_AddMasked(*lpIml, hBmp, crMask);
	DeleteObject(hBmp);
	DeleteObject((void*)crMask);
}

static void InsertCommands(HMENU hMenu, HTREEITEM htiParent){
	TVINSERTSTRUCTW	tvs;
	HTREEITEM		hti;
	MENUITEMINFOW	mi;
	int				count;
	wchar_t			szBuffer[128], *ptr;
	TP_TEMPACCEL	acc;
	P_TP_TEMPACCEL	pAcc;

	ZeroMemory(&tvs, sizeof(tvs));
	tvs.hParent = htiParent;
	tvs.hInsertAfter = TVI_LAST;
	tvs.itemex.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	count = GetMenuItemCount(hMenu);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_ID | MIIM_SUBMENU;
	for(int i = 0; i < count; i++){
		*szBuffer = '\0';
		GetMenuItemInfoW(hMenu, i, TRUE, &mi);
		if(mi.wID > 0){
			SendMessageW(g_hMain, TBNPM_GET_MENU_TXT, mi.wID, (LPARAM)szBuffer);
			if(wcslen(szBuffer) > 0){
				ZeroMemory(&acc, sizeof(acc));
				wcstok(szBuffer, L"\t", &ptr);
				acc.acc.cmd = mi.wID;
				SendMessageW(g_hMain, TBNPM_GET_ACC, 0, (LPARAM)&acc);
				if(mi.hSubMenu){
					acc.acc.cmd = 0;
					tvs.itemex.iImage = 0;
					tvs.itemex.iSelectedImage = 0;
				}
				else{
					tvs.itemex.iImage = 1;
					tvs.itemex.iSelectedImage = 1;
				}
				wcscpy(acc.text, ptr);
				tvs.itemex.pszText = szBuffer;
				pAcc = malloc(sizeof(TP_TEMPACCEL));
				memcpy(pAcc, &acc, sizeof(TP_TEMPACCEL));
				tvs.itemex.lParam = (long)pAcc;
				hti = (HTREEITEM)SendMessageW(m_hTree, TVM_INSERTITEMW, 0, (LPARAM)&tvs);
				if(mi.hSubMenu)
					InsertCommands(mi.hSubMenu, hti);
				TreeView_Expand(m_hTree, hti, TVE_EXPAND);
			}
		}
	}
	TreeView_SelectItem(m_hTree, TreeView_GetRoot(m_hTree));
	TreeView_EnsureVisible(m_hTree, TreeView_GetRoot(m_hTree));
}

static BOOL KeyExists(P_TP_TEMPACCEL pA, HTREEITEM hti){
	TVITEMW			tvi;
	P_TP_TEMPACCEL	pAcc;
	BOOL			result;

	ZeroMemory(&tvi, sizeof(tvi));
	tvi.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_HANDLE;
	while(hti){
		tvi.hItem = hti;
		SendMessageW(m_hTree, TVM_GETITEMW, 0, (LPARAM)&tvi);
		if(tvi.cChildren > 0){
			result = KeyExists(pA, TreeView_GetChild(m_hTree, hti));
			if(result)
				return TRUE;
		}
		if(tvi.lParam){
			pAcc = (P_TP_TEMPACCEL)tvi.lParam;
			if(pAcc->acc.cmd != pA->acc.cmd){
				if(pAcc->acc.fVirt == pA->acc.fVirt && pAcc->acc.key == pA->acc.key){
					return TRUE;
				}
			}
		}
		hti = TreeView_GetNextSibling(m_hTree, hti);
	}
	return FALSE;
}

static void FreeCommands(BOOL withArray, HTREEITEM hti){
	TVITEMW			tvi;
	P_TP_TEMPACCEL	pAcc;
	LPACCEL			p;

	ZeroMemory(&tvi, sizeof(tvi));
	tvi.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_HANDLE;
	while(hti){
		tvi.hItem = hti;
		SendMessageW(m_hTree, TVM_GETITEMW, 0, (LPARAM)&tvi);
		if(tvi.cChildren > 0){
			FreeCommands(withArray, TreeView_GetChild(m_hTree, hti));
		}
		if(tvi.lParam){
			pAcc = (P_TP_TEMPACCEL)tvi.lParam;
			if(pAcc->acc.key && withArray){
				g_TempAccCount++;
				if(g_TempAcc == NULL){
					g_TempAcc = malloc(sizeof(ACCEL));
					memcpy(g_TempAcc, &pAcc->acc, sizeof(ACCEL));
				}
				else{
					g_TempAcc = realloc(g_TempAcc, g_TempAccCount * sizeof(ACCEL));
					p = g_TempAcc;
					p += (g_TempAccCount - 1);
					memcpy(p, &pAcc->acc, sizeof(ACCEL));
				}
			}
			free(pAcc);
		}
		hti = TreeView_GetNextSibling(m_hTree, hti);
	}
}

static BOOL CALLBACK AccSet_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_CLOSE, AccSet_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, AccSet_OnCommand);
	HANDLE_MSG (hwnd, WM_INITDIALOG, AccSet_OnInitDialog);

	default: return FALSE;
	}
}

static void AccSet_OnClose(HWND hwnd)
{
	EndDialog(hwnd, IDCANCEL);
}

static void AccSet_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
		case IDCANCEL:
			if(codeNotify == BN_CLICKED){
				EndDialog(hwnd, IDCANCEL);
			}
			break;
		case IDOK:
			if(codeNotify == BN_CLICKED){
				EndDialog(hwnd, IDOK);
			}
			break;
	}
}

static BOOL AccSet_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	// wchar_t		szCaption[128];
	wchar_t		szLang[MAX_PATH];
	HWND		hEdit;

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);
	// GetPrivateProfileStringW(S_CONTROLS, L"1007", L"Hot keys", szCaption, 128, szLang);
	// SetWindowTextW(hwnd, szCaption);
	SetControlText(hwnd, IDC_ST_ACC_SET, L"Type the keys to be used as hot key for selected command", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);
	hEdit = GetDlgItem(hwnd, IDC_EDT_ACC_SET);
	SetDlgItemTextW(hwnd, IDC_EDT_ACC_SET, m_Acc.text);
	SetWindowLongPtrW(hEdit, GWLP_USERDATA, (LONG_PTR)SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)EditProc));
	return FALSE;
}

static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	wchar_t			szModifiers[24], szKey[8], szBuffer[24];
	int				idMod = 0, key = 0;
	BOOL			proceed = FALSE;

	switch(msg){
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			//get modifiers
			idMod = GetModifiers(szModifiers);
			//get the very first other key pressed
			key = GetOtherKey(szKey);
			if(idMod == 0 && (key >= VK_F1 && key <= VK_F24)){
				proceed = TRUE;
			}
			else if(idMod && key){
				proceed = TRUE;
			}
			if(proceed){
				wcscpy(szBuffer, szModifiers);
				wcscat(szBuffer, szKey);
				m_Acc.acc.fVirt = FVIRTKEY | idMod;
				m_Acc.acc.key = key;
				wcscpy(m_Acc.text, szBuffer);
				EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), TRUE);
				SetWindowTextW(hwnd, szBuffer);
			}
			return FALSE;
		default:
			return CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
	}
}
