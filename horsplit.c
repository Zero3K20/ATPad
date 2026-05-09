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

#include "horsplit.h"
#include "main.h"
#include "globalvars.h"
#include "numericconstants.h"
#include "stringconstants.h"
#include "toolbar.h"
#include "menu.h"
#include "shared.h"

#define	HSPLIT_CLASS			L"__atp_hspl_class__"
#define	HSPPLCHOLDER_CLASS		L"__atp_hspplchldr_class__"

#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))

typedef struct _LVSORT_MEMBER {
	int		column;
	int		order;
}LVSORT_MEMBER, *P_LVSORT_MEMBER;

static LRESULT CALLBACK HSplit_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL HSplit_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void HSplit_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
static void HSplit_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void HSplit_OnDestroy(HWND hwnd);
static LRESULT CALLBACK PlaceHolder_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL PlaceHolder_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void PlaceHolder_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void PlaceHolder_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void PlaceHolder_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
static void PlaceHolder_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
static void InsertTab(int id, wchar_t * lpLang, int imgID);
static void DeleteTab(int id);
static void CloseAssTab(void);
static void CreateImageList(HIMAGELIST *lpIml, int idBmp, COLORREF crMask, int x, int y, int imgCount);
static void EnableButtons(int tab);
static void PrepareAssMenuRecursive(HMENU hMenu, wchar_t * lpLangFile);
static void PrepareAssMenu(wchar_t * lpLang);
static BOOL CALLBACK Snippets_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void Snippets_OnClose(HWND hwnd);
static void Snippets_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL Snippets_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void ReplaceCRLFForward(wchar_t * lpText);
static void ReplaceCRLFBackward(wchar_t * lpText);
static void ReplaceCRLFBySpace(wchar_t * lpText);
static BOOL SnippetExists(wchar_t * lpName);
static void LoadSnippets(void);
static void DeleteSnippet(void);
static void InsertSnippet(void);
static int CALLBACK StringCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static HWND				m_hTabHSplit, m_hPlaceHolder, m_hTbrPane, m_hListFind, m_hListSnippets;
static HIMAGELIST		m_ImlPane, m_ImlPaneGray;
static BOOL				m_StopMessage;
static HMENU			m_hAssMenu, m_hAssPopUpMenu;
static HBITMAP			m_hAssBmpNormal, m_hAssBmpGray;
static WNDPROC			m_OldEditProc;
static LVSORT_MEMBER	m_lvsm;
static int				m_SortOrderName = LVS_SORTASCENDING, m_SortOrderSnippet = LVS_SORTASCENDING;

static TBBUTTON		 	m_TBPane[] = {{1,IDM_ADD_SNIPPET,0,TBSTYLE_BUTTON,0,0,0,0},
										{2,IDM_DEL_SNIPPET,0,TBSTYLE_BUTTON,0,0,0,0},
										{3,IDM_INS_SNIPPET,0,TBSTYLE_BUTTON,0,0,0,0},
										{4,IDM_EDIT_SNIPPET,0,TBSTYLE_BUTTON,0,0,0,0},
										{0,IDM_CLOSE_PANE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0}};
static MItem			m_AssMenus[] = {
						{IDM_ADD_SNIPPET, 1, 0, -1, -1, MT_REGULARITEM, FALSE, L"Add Snippet", L""}, 
						{IDM_DEL_SNIPPET, 2, 0, -1, -1, MT_REGULARITEM, FALSE, L"Delete Snippet", L""}, 
						{IDM_INS_SNIPPET, 3, 0, -1, -1, MT_REGULARITEM, FALSE, L"Insert Snippet", L""},
						{IDM_EDIT_SNIPPET, 4, 0, -1, -1, MT_REGULARITEM, FALSE, L"Edit Snippet", L""},
						{IDM_CLOSE_PANE, 0, 0, -1, -1, MT_REGULARITEM, FALSE, L"Close", L""}};

HWND CreateHSplit(HWND hParent, int t, int h){
	WNDCLASSEXW			wx;
	RECT				rc;

	ZeroMemory(&wx, sizeof(wx));
	wx.cbSize = sizeof(wx);
	//register placeholder class
	wx.lpszClassName = HSPPLCHOLDER_CLASS;
    wx.style = CS_VREDRAW|CS_HREDRAW;
	wx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wx.lpfnWndProc = PlaceHolder_WndProc;
	if(!RegisterClassExW(&wx)){
		return NULL;
	}
	//register splitter class
	wx.lpszClassName = HSPLIT_CLASS;
	wx.hCursor = LoadCursor(g_hInstance, MAKEINTRESOURCE(ID_CUR_HOR_SPLIT));
	wx.lpfnWndProc = HSplit_WndProc;

	if(!RegisterClassExW(&wx)){
		return NULL;
	}
	GetClientRect(hParent, &rc);
	g_hSplit = CreateWindowExW(0, HSPLIT_CLASS, NULL, WS_CHILD, 0, rc.bottom - h - t, rc.right - rc.left, h, hParent, NULL, g_hInstance, 0);
	return g_hSplit;
}

static LRESULT CALLBACK HSplit_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCITEMW			tci;
	int				count;

	switch(msg)
	{
		HANDLE_MSG (hwnd, WM_SIZE, HSplit_OnSize);
		HANDLE_MSG (hwnd, WM_MOUSEMOVE, HSplit_OnMouseMove);
		HANDLE_MSG (hwnd, WM_CREATE, HSplit_OnCreate);
		HANDLE_MSG (hwnd, WM_DESTROY, HSplit_OnDestroy);
		
		case TBNPM_DEL_ASS_TAB:
			DeleteTab(wParam);
			return TabCtrl_GetItemCount(m_hTabHSplit);
		case TBNPM_GET_ASS_TAB:
			return (LRESULT)m_hTabHSplit;
		case TBNPM_INSERT_ASS_TAB:{
			wchar_t		szLang[MAX_PATH];

			wcscpy(szLang, g_Paths.sLangDir);
			wcscat(szLang, g_Paths.sLangFile);
			InsertTab(wParam, szLang, lParam);
			return TRUE;
		}
		case TBNPM_SELECT_ASS_TAB:{
			ZeroMemory(&tci, sizeof(tci));
			tci.mask = TCIF_PARAM;
			count = TabCtrl_GetItemCount(m_hTabHSplit);
			for(int i = 0; i < count; i++){
				SendMessageW(m_hTabHSplit, TCM_GETITEMW, i, (LPARAM)&tci);
				if(tci.lParam == wParam){
					NMHDR		nmhdr;
			
					nmhdr.code = TCN_SELCHANGE;
					nmhdr.hwndFrom = m_hTabHSplit;
					TabCtrl_SetCurSel(m_hTabHSplit, i);
					SendMessageW(m_hPlaceHolder, WM_NOTIFY, 0, (LPARAM)&nmhdr);
					return TRUE;
				}
			}
			SendMessageW(hwnd, TBNPM_INSERT_ASS_TAB, wParam, lParam);
			return TRUE;
		}
		case TBNPM_MOVE_HSP:{
			RECT		rc;

			GetClientRect(hwnd, &rc);
			m_StopMessage = TRUE;
			MoveWindow(hwnd, 0, (int)wParam - (rc.bottom - rc.top), (int)lParam, rc.bottom - rc.top, TRUE);
			m_StopMessage = FALSE;
			return TRUE;
		}
		case TBNPM_CLEAR_LIST:{
			int			count;
			P_FRC_ITEM	p;

			count = SendMessageW(m_hListFind, LB_GETCOUNT, 0, 0);
			for(int i = 0; i < count; i++){
				p = (P_FRC_ITEM)SendMessageW(m_hListFind, LB_GETITEMDATA, i, 0);
				if(p){
					free(p);
				}
			}
			SendMessageW(m_hListFind, LB_RESETCONTENT, 0, 0);
			return TRUE;
		}
		case TBNPM_INS_RES_STR:{
			wchar_t			szBuffer[1024], szNumber[12];

			wcscpy(szBuffer, g_Strings.sTotalFound);
			wcscat(szBuffer, L" \"");
			wcscat(szBuffer, g_SearchString);
			wcscat(szBuffer, L"\": ");
			_ltow(wParam, szNumber, 10);
			wcscat(szBuffer, szNumber);
			SendMessageW(m_hListFind, LB_INSERTSTRING, 0, (LPARAM)szBuffer);
			return TRUE;
		}
		case TBNPM_INS_FIND_STR:{
			wchar_t			szBuffer[MAX_LINE_SIZE], szLine[MAX_LINE_SIZE], szNumber[12];
			P_FRC_ITEM		p;
			int				index, count;

			p = (P_FRC_ITEM)lParam;
			wcscpy(szBuffer, L"\"");
			wcscat(szBuffer, (wchar_t *)wParam);
			wcscat(szBuffer, L"\" (");
			_ltow(p->row, szNumber, 10);
			wcscat(szBuffer, szNumber);
			wcscat(szBuffer, L", ");
			_ltow(p->col, szNumber, 10);
			wcscat(szBuffer, szNumber);
			wcscat(szBuffer, L")");
			*szLine = MAX_LINE_SIZE;
			count = SendMessageW(p->hEdit, EM_GETLINE, SendMessageW(p->hEdit, EM_EXLINEFROMCHAR, 0, p->min), (LPARAM)szLine);
			if(szLine[count - 1] == '\r' || szLine[count - 1] == '\n')
				szLine[count - 1] = '\0';
			wcscat(szBuffer, L"\t");
			wcscat(szBuffer, szLine);
			index = SendMessageW(m_hListFind, LB_ADDSTRING, 0, (LPARAM)szBuffer);
			SendMessageW(m_hListFind, LB_SETITEMDATA, index, (LPARAM)p);
			return TRUE;
		}
		case TBNPM_REMOVE_RESULTS:{
			P_FRC_ITEM		p;
			HWND			hEdit = (HWND)wParam;
			int				count = SendMessageW(m_hListFind, LB_GETCOUNT, 0, 0);
			for(int i = count - 1; i > 0; i--){
				p = (P_FRC_ITEM)SendMessageW(m_hListFind, LB_GETITEMDATA, i, 0);
				if(p && p->hEdit){
					if(p->hEdit == hEdit){
						free(p);
						SendMessageW(m_hListFind, LB_DELETESTRING, i, 0);
					}
				}
			}
			count = SendMessageW(m_hListFind, LB_GETCOUNT, 0, 0);
			if(count > 0){
				if(count == 1){
					SendMessageW(m_hListFind, LB_RESETCONTENT, 0, 0);
				}
				else{
					SendMessageW(m_hListFind, LB_DELETESTRING, 0, 0);
					SendMessageW(g_hSplit, TBNPM_INS_RES_STR, count - 1, 0);
				}
			}
			return TRUE;
		}
		default: return DefWindowProc (hwnd, msg, wParam, lParam);
	}
}

static void HSplit_OnDestroy(HWND hwnd)
{
	if(m_ImlPane)
		ImageList_Destroy(m_ImlPane);
	if(m_ImlPaneGray)
		ImageList_Destroy(m_ImlPaneGray);
	if(m_hAssMenu){
		FreeMenus(m_hAssMenu);
		DestroyMenu(m_hAssMenu);
	}
	if(m_hAssBmpNormal)
		DeleteObject(m_hAssBmpNormal);
	if(m_hAssBmpGray)
		DeleteObject(m_hAssBmpGray);
}

static BOOL HSplit_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	m_hPlaceHolder = CreateWindowExW(0, HSPPLCHOLDER_CLASS, NULL, WS_CHILD | WS_VISIBLE, 0, 4, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	m_hAssBmpNormal = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_IML_ASS_NORMAL));
	m_hAssBmpGray = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_IML_ASS_GRAY));
	return TRUE;
}

static void HSplit_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if(keyFlags == MK_LBUTTON){
		SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTTOP, 0);
	}
}

static void HSplit_OnSize(HWND hwnd, UINT state, int cx, int cy)
{	
	RECT			rc;

	GetClientRect(hwnd, &rc);
	MoveWindow(m_hPlaceHolder, 0, 4, rc.right - rc.left, rc.bottom - rc.top - 4, TRUE);
	if(!m_StopMessage){
		SendMessageW(GetParent(hwnd), TBNPM_HSP_SIZED, 0, 0);
	}
}

static LRESULT CALLBACK PlaceHolder_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR		lpnmhdr;
	int			tab;

	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_CREATE, PlaceHolder_OnCreate);
	HANDLE_MSG (hwnd, WM_SIZE, PlaceHolder_OnSize);
	HANDLE_MSG (hwnd, WM_COMMAND, PlaceHolder_OnCommand);
	HANDLE_MSG (hwnd, WM_DRAWITEM, PlaceHolder_OnDrawItem);
	HANDLE_MSG (hwnd, WM_MEASUREITEM, PlaceHolder_OnMeasureItem);
	case WM_NOTIFY:
		lpnmhdr = (LPNMHDR)lParam;
		if(lpnmhdr->code == TCN_SELCHANGE){
			TCITEMW			tci;
			ZeroMemory(&tci, sizeof(tci));
			tci.mask = TCIF_PARAM;
			tab = TabCtrl_GetCurSel(m_hTabHSplit);
			SendMessageW(m_hTabHSplit, TCM_GETITEMW, tab, (LPARAM)&tci);
			switch(tci.lParam){
			case IDM_F_R_FILES:
				ShowWindow(m_hListFind, SW_SHOW);
				ShowWindow(m_hListSnippets, SW_HIDE);
				break;
			case IDM_SNIPPETS:
				ShowWindow(m_hListFind, SW_HIDE);
				ShowWindow(m_hListSnippets, SW_SHOW);
				break;
			}
			EnableButtons(tci.lParam);
		}
		else if(lpnmhdr->code == TTN_NEEDTEXTW){
			//show toolbar tooltips
			wchar_t			szTooltip[128];
			TOOLTIPTEXTW 	* ttp;

			switch(lpnmhdr->idFrom){
			case IDM_ADD_SNIPPET:
			case IDM_DEL_SNIPPET:
			case IDM_INS_SNIPPET:
			case IDM_EDIT_SNIPPET:
				GetTooltip(szTooltip, m_AssMenus, NELEMS(m_AssMenus), lpnmhdr->idFrom);
				ttp = (TOOLTIPTEXTW *)lParam;
				wcscpy(ttp->szText, szTooltip);
				break;
			case IDM_CLOSE_PANE:{
				wchar_t		szLang[MAX_PATH], szId[32];
				
				_itow(lpnmhdr->idFrom, szId, 10);
				wcscpy(szLang, g_Paths.sLangDir);
				wcscat(szLang, g_Paths.sLangFile);
				GetPrivateProfileStringW(S_MENU, szId, L"Close", szTooltip, 128, szLang);
				ttp = (TOOLTIPTEXTW *)lParam;
				wcscpy(ttp->szText, szTooltip);
				break;
			}
			}	
		}
		else if(lpnmhdr->code == NM_RCLICK){
			if(lpnmhdr->hwndFrom == m_hListSnippets){
				ShowPopUp(m_hPlaceHolder, m_hAssPopUpMenu);
			}
		}
		else if(lpnmhdr->code == NM_DBLCLK){
			if(lpnmhdr->hwndFrom == m_hListSnippets){
				SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDM_INS_SNIPPET, 0), 0);
			}
		}
		else if(lpnmhdr->code == LVN_ITEMCHANGED){
			LPNMLISTVIEW	pnmv = (LPNMLISTVIEW)lParam;
			if(pnmv->uNewState == 3 || pnmv->uNewState == 2){
				SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_DEL_SNIPPET, TRUE);
				SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_INS_SNIPPET, TRUE);
				SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_EDIT_SNIPPET, TRUE);
				EnableMenuItem(m_hAssPopUpMenu, IDM_DEL_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(m_hAssPopUpMenu, IDM_INS_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(m_hAssPopUpMenu, IDM_EDIT_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
			}
			else{
				SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_DEL_SNIPPET, FALSE);
				SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_INS_SNIPPET, FALSE);
				SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_EDIT_SNIPPET, FALSE);
				EnableMenuItem(m_hAssPopUpMenu, IDM_DEL_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(m_hAssPopUpMenu, IDM_INS_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(m_hAssPopUpMenu, IDM_EDIT_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
			}
		}
		else if(lpnmhdr->code == LVN_COLUMNCLICK){
			LPNMLISTVIEW	pnmv = (LPNMLISTVIEW)lParam;
			switch(pnmv->iSubItem){
			case 0:
				m_lvsm.column = 0;
				if(m_SortOrderName == LVS_SORTASCENDING)
					m_SortOrderName = LVS_SORTDESCENDING;
				else
					m_SortOrderName = LVS_SORTASCENDING;
				SendMessageW(m_hListSnippets, LVM_SORTITEMSEX, (WPARAM)m_SortOrderName, (LPARAM)StringCompareFunc);
				break;
			case 1:
				m_lvsm.column = 1;
				if(m_SortOrderSnippet == LVS_SORTASCENDING)
					m_SortOrderSnippet = LVS_SORTDESCENDING;
				else
					m_SortOrderSnippet = LVS_SORTASCENDING;
				SendMessageW(m_hListSnippets, LVM_SORTITEMSEX, (WPARAM)m_SortOrderSnippet, (LPARAM)StringCompareFunc);
				break;
			}
		}
		return FALSE;
	default: return DefWindowProc (hwnd, msg, wParam, lParam);
	}
}

static void PlaceHolder_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDM_INS_SNIPPET:
		InsertSnippet();
		break;
	case IDM_DEL_SNIPPET:
		if(MessageBoxW(g_hMain, g_Strings.sDelSnipQuestion, PROGRAM_NAME, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DeleteSnippet();
		break;
	case IDM_ADD_SNIPPET:
		DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_SNIPPETS), g_hMain, Snippets_DlgProc, -1);
		break;
	case IDM_EDIT_SNIPPET:
		DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_SNIPPETS), g_hMain, Snippets_DlgProc, ListView_GetNextItem(m_hListSnippets, -1, LVNI_ALL | LVNI_SELECTED));
		break;
	case IDM_CLOSE_PANE:
		CloseAssTab();
		break;
	default:
		if(hwndCtl == m_hListFind && codeNotify == LBN_DBLCLK){
			P_FRC_ITEM		p;
			int				index;
			CHARRANGE		chrg;

			index = SendMessageW(m_hListFind, LB_GETCURSEL, 0, 0);
			if(index > 0){
				p = (P_FRC_ITEM)SendMessageW(m_hListFind, LB_GETITEMDATA, index, 0);
				if(p){
					chrg.cpMin = p->min;
					chrg.cpMax = p->max;
					SendMessageW(g_hMain, TBNPM_ACT_BY_EDIT, (WPARAM)p->hEdit, 0);
					SendMessageW(p->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
				}
			}
		}
		break;
	}
}

static BOOL PlaceHolder_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	LVCOLUMNW		lvc;
	wchar_t			szBuffer[256];

	m_hTabHSplit = CreateWindowExW(0, WC_TABCONTROLW, NULL, WS_CHILD | WS_VISIBLE, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	SendMessageW(m_hTabHSplit, WM_SETFONT, (WPARAM)g_hMenuFont, MAKELPARAM(TRUE, 0));
	
	m_hTbrPane = CreateWindowExW(0, TOOLBARCLASSNAMEW, NULL, WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_TRANSPARENT | CCS_NORESIZE | CCS_NODIVIDER, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	CreateImageList(&m_ImlPane, IDB_IML_ASS_NORMAL, CLR_MASK, 16, 16, 0);
	CreateImageList(&m_ImlPaneGray, IDB_IML_ASS_GRAY, CLR_MASK, 16, 16, 0);
	SendMessageW(m_hTbrPane, TB_SETBUTTONSIZE, 0, MAKELONG(16, 16));
	DoToolbarW(m_hTbrPane, m_ImlPane, m_ImlPaneGray, NELEMS(m_TBPane), m_TBPane);
	
	SendMessageW(m_hTabHSplit, TCM_SETIMAGELIST, 0, (LPARAM)m_ImlPane);

	m_hListFind = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | LBS_USETABSTOPS | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_HSCROLL | WS_VSCROLL, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	SendMessageW(m_hListFind, WM_SETFONT, (WPARAM)g_hMenuFont, MAKELPARAM(TRUE, 0));

	m_hListSnippets = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	SendMessageW(m_hListFind, WM_SETFONT, (WPARAM)g_hMenuFont, MAKELPARAM(TRUE, 0));
	ListView_SetImageList(m_hListSnippets, m_ImlPane, LVSIL_SMALL);
	ListView_SetExtendedListViewStyleEx(m_hListSnippets, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
	lvc.pszText = szBuffer;

	lvc.cx = 240;
	wcscpy(szBuffer, L"Name");
	SendMessageW(m_hListSnippets, LVM_INSERTCOLUMNW, 0, (LPARAM)&lvc);
	wcscpy(szBuffer, L"Text");
	lvc.iSubItem = 1;
	lvc.cx = 480;
	SendMessageW(m_hListSnippets, LVM_INSERTCOLUMNW, 1, (LPARAM)&lvc);
	ShowWindow(m_hListSnippets, SW_HIDE);
	LoadSnippets();

	ApplyPaneLanguage();
	return TRUE;
}

static void PlaceHolder_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	RECT			rc;

	GetClientRect(hwnd, &rc);
	MoveWindow(m_hTabHSplit, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(m_hTbrPane, rc.right - 120, rc.top - 2, 120, 18, TRUE);
	TabCtrl_AdjustRect(m_hTabHSplit, FALSE, &rc);
	MoveWindow(m_hListFind, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(m_hListSnippets, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

static void PlaceHolder_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	if(lpDrawItem->CtlType == ODT_MENU)
		//draw menu item
		DrawMItem(lpDrawItem,m_hAssBmpNormal, m_hAssBmpGray, CLR_MASK, g_VSEnabled);
}

static void PlaceHolder_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	//measure menu item
	MeasureMItem(g_hMenuFont, lpMeasureItem);
}

static void InsertTab(int id, wchar_t * lpLang, int imgID){
	TCITEMW		tci;
	wchar_t		szBuffer[128], szDefault[128], szKey[32];
	RECT		rc;

	_itow(id, szKey, 10);
	ZeroMemory(&tci, sizeof(tci));
	tci.mask = TCIF_PARAM | TCIF_IMAGE | TCIF_TEXT;
	tci.iImage = imgID;
	tci.lParam = id;
	tci.pszText = szBuffer;
	SendMessageW(g_hMain, TBNPM_GET_MENU_TXT, id, (LPARAM)szDefault);
	GetPrivateProfileStringW(S_MENU, szKey, szDefault, szBuffer, 128, lpLang);
	SendMessageW(m_hTabHSplit, TCM_INSERTITEMW, 0, (LPARAM)&tci);
	GetClientRect(m_hPlaceHolder, &rc);
	TabCtrl_AdjustRect(m_hTabHSplit, FALSE, &rc);
	switch(id){
		case IDM_F_R_FILES:
			MoveWindow(m_hListFind, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
			break;
		case IDM_SNIPPETS:
			MoveWindow(m_hListSnippets, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
			break;
	}
	SendMessageW(g_hSplit, TBNPM_SELECT_ASS_TAB, id, imgID);
	RedrawWindow(m_hPlaceHolder, NULL, NULL, RDW_INVALIDATE);
}

static void DeleteTab(int id){
	TCITEMW		tci;
	int			count, next = -1;

	ZeroMemory(&tci, sizeof(tci));
	tci.mask = TCIF_PARAM;
	count = TabCtrl_GetItemCount(m_hTabHSplit);
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabHSplit, TCM_GETITEMW, i, (LPARAM)&tci);
		if(tci.lParam == id){
			if(count > 1){
				if(i < count -1){
					next = i + 1;
				}
				else if(i == count - 1){
					next = i - 1;
				}
			}
			if(next >= 0){
				SendMessageW(m_hTabHSplit, TCM_GETITEMW, next, (LPARAM)&tci);
				SendMessageW(g_hSplit, TBNPM_SELECT_ASS_TAB, tci.lParam, 0);
			}
			TabCtrl_DeleteItem(m_hTabHSplit, i);
			RedrawWindow(m_hPlaceHolder, NULL, NULL, RDW_INVALIDATE);
			break;
		}
	}
}

static void CloseAssTab(void){
	TCITEMW		tci;

	ZeroMemory(&tci, sizeof(tci));
	tci.mask = TCIF_PARAM;
	SendMessageW(m_hTabHSplit, TCM_GETITEMW, TabCtrl_GetCurSel(m_hTabHSplit), (LPARAM)&tci);
	SendMessageW(g_hMain, WM_COMMAND, MAKEWPARAM(tci.lParam, 0), 0);
}

static void CreateImageList(HIMAGELIST *lpIml, int idBmp, COLORREF crMask, int x, int y, int imgCount){
	HBITMAP			hBmp;

	*lpIml = ImageList_Create(x, y, ILC_MASK | ILC_COLOR24, imgCount, 0);
	hBmp = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(idBmp));
	ImageList_AddMasked(*lpIml, hBmp, crMask);
	DeleteObject(hBmp);
	DeleteObject((void*)crMask);
}

void ApplyPaneLanguage(void){
	LVCOLUMNW	lvc;
	TCITEMW		ti;
	int			count;
	wchar_t		szLang[MAX_PATH], szBuffer[256], szID[12];

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);
	count = TabCtrl_GetItemCount(m_hTabHSplit);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_TEXT | TCIF_PARAM;
	ti.pszText = szBuffer;
	ti.cchTextMax = 128;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabHSplit, TCM_GETITEMW, i, (LPARAM)&ti);
		_itow(ti.lParam, szID, 10);
		GetPrivateProfileStringW(S_MENU, szID, szBuffer, szBuffer, 128, szLang);
		SendMessageW(m_hTabHSplit, TCM_SETITEMW, i, (LPARAM)&ti);
	}

	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	lvc.cchTextMax = 256;
	lvc.pszText = szBuffer;
	for(int i = 0; i < 2; i++){
		lvc.iSubItem = i;
		SendMessageW(m_hListSnippets, LVM_GETCOLUMNW, i, (LPARAM)&lvc);
		_itow(i, szID, 10);
		GetPrivateProfileStringW(S_SNIPPETS_COLS, szID, L"Column", szBuffer, 256, szLang);
		SendMessageW(m_hListSnippets, LVM_SETCOLUMNW, i, (LPARAM)&lvc);
	}
	PrepareAssMenu(szLang);
	RedrawWindow(m_hPlaceHolder, NULL, NULL, RDW_INVALIDATE);
}

static void EnableButtons(int tab){
	switch(tab){
	case IDM_F_R_FILES:
		SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_ADD_SNIPPET, FALSE);
		SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_DEL_SNIPPET, FALSE);
		SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_INS_SNIPPET, FALSE);
		SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_EDIT_SNIPPET, FALSE);
		EnableMenuItem(m_hAssPopUpMenu, IDM_ADD_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(m_hAssPopUpMenu, IDM_DEL_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(m_hAssPopUpMenu, IDM_INS_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(m_hAssPopUpMenu, IDM_EDIT_SNIPPET, MF_BYCOMMAND | MF_GRAYED);
		break;
	case IDM_SNIPPETS:
		SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_ADD_SNIPPET, TRUE);
		EnableMenuItem(m_hAssPopUpMenu, IDM_ADD_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
		if(ListView_GetSelectedCount(m_hListSnippets) > 0){
			SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_DEL_SNIPPET, TRUE);
			SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_INS_SNIPPET, TRUE);
			SendMessageW(m_hTbrPane, TB_ENABLEBUTTON, IDM_EDIT_SNIPPET, TRUE);
			EnableMenuItem(m_hAssPopUpMenu, IDM_DEL_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(m_hAssPopUpMenu, IDM_INS_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(m_hAssPopUpMenu, IDM_EDIT_SNIPPET, MF_BYCOMMAND | MF_ENABLED);
		}
		break;
	}
}

static void PrepareAssMenuRecursive(HMENU hMenu, wchar_t * lpLangFile){
	int				count;
	MENUITEMINFOW	mi;
	wchar_t			szText[128], szDefault[128];
	PMItem			pmi;

	count = GetMenuItemCount(hMenu);
	for(int i = 0; i < count; i++){
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_FTYPE;
		GetMenuItemInfoW(hMenu, i, TRUE, &mi);
		if((mi.fType & MFT_SEPARATOR) != MFT_SEPARATOR){
			if(mi.hSubMenu){
				PrepareAssMenuRecursive(mi.hSubMenu, lpLangFile);
			}

			GetMIText(m_AssMenus, NELEMS(m_AssMenus), mi.wID, szDefault);
			SetMenuText(mi.wID, S_MENU, lpLangFile, szDefault, szText);
			SetMIText(m_AssMenus, NELEMS(m_AssMenus), mi.wID, szText);
			pmi = GetMItem(m_AssMenus, NELEMS(m_AssMenus), mi.wID);
			if(pmi){
				SetMenuItemProperties(pmi, hMenu, i);
			}
		}
		if((mi.fType & MFT_OWNERDRAW) != MFT_OWNERDRAW){
			mi.fType |= MFT_OWNERDRAW;
			SetMenuItemInfoW(hMenu, i, TRUE, &mi);
		}
	}
}

static void PrepareAssMenu(wchar_t * lpLang){

	if(m_hAssMenu)
		DestroyMenu(m_hAssMenu);
	m_hAssMenu = LoadMenuW(g_hInstance, MAKEINTRESOURCEW(IDR_MENU_ASS_PANEL));
	m_hAssPopUpMenu = GetSubMenu(m_hAssMenu, 0);

	PrepareAssMenuRecursive(m_hAssPopUpMenu, lpLang);
}

static BOOL CALLBACK Snippets_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_CLOSE, Snippets_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, Snippets_OnCommand);
	HANDLE_MSG (hwnd, WM_INITDIALOG, Snippets_OnInitDialog);

	default: return FALSE;
	}
}

static void Snippets_OnClose(HWND hwnd)
{
	RemovePropW(hwnd, L"edit");
	EndDialog(hwnd, IDCANCEL);
}

static void Snippets_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	wchar_t			szName[128];

	switch(id){
	case IDCANCEL:
		RemovePropW(hwnd, L"edit");
		EndDialog(hwnd, IDCANCEL);
		break;
	case IDOK:
		GetDlgItemTextW(hwnd, IDC_EDT_SNP_NAME, szName, 128);
		if((int)GetPropW(hwnd, L"edit") > 0 || !SnippetExists(szName)){
			LVITEMW		lvi;
			int			size = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_SNP_TEXT));
			wchar_t		*szText, szSize[256];
			int			index = ListView_GetItemCount(m_hListSnippets);

			szText = malloc(sizeof(wchar_t) * (size + 1));
			if(!szText)
				return;
			_itow(size, szSize, 10);
			GetDlgItemTextW(hwnd, IDC_EDT_SNP_TEXT, szText, size + 1);
			ReplaceCRLFForward(szText);
			WritePrivateProfileStringW(szName, L"size", szSize, g_Paths.sSnippetsPath);
			WritePrivateProfileStringW(szName, L"text", szText, g_Paths.sSnippetsPath);
			ZeroMemory(&lvi, sizeof(lvi));
			lvi.mask = LVIF_IMAGE | LVIF_TEXT;
			lvi.iImage = 5;
			lvi.pszText = szName;
			if((int)GetPropW(hwnd, L"edit") == 0){
				lvi.iItem = index;
				SendMessageW(m_hListSnippets, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
			}
			else{
				lvi.iItem = ListView_GetNextItem(m_hListSnippets, -1, LVNI_ALL | LVNI_SELECTED);
				SendMessageW(m_hListSnippets, LVM_SETITEMW, 0, (LPARAM)&lvi);
			}
			lvi.iSubItem = 1;
			lvi.pszText = szText;
			ReplaceCRLFBySpace(szText);
			SendMessageW(m_hListSnippets, LVM_SETITEMW, 0, (LPARAM)&lvi);
			free(szText);
			RemovePropW(hwnd, L"edit");
			EndDialog(hwnd, IDOK);
		}
		else{
			MessageBoxW(g_hMain, g_Strings.sSnippetExists, PROGRAM_NAME, MB_OK | MB_ICONSTOP);
			SetFocus(GetDlgItem(hwnd, IDC_EDT_SNP_NAME));
		}
		break;
	case IDC_EDT_SNP_NAME:
	case IDC_EDT_SNP_TEXT:
		if(GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_SNP_NAME)) > 0 && GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDT_SNP_TEXT)) > 0){
			EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
		}
		else{
			EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
		}
		break;
	}
}

static BOOL Snippets_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	wchar_t			szLang[MAX_PATH], szBuffer[128];
	LVITEMW			lvi;
	int				size;

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);
	GetPrivateProfileStringW(S_CONTROLS, L"1009", L"Add/Edit Snippet", szBuffer, 128, szLang);
	SetWindowTextW(hwnd, szBuffer);
	SetControlText(hwnd, IDOK, L"OK", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);
	SetControlText(hwnd, IDC_ST_SNP_NAME, L"Name", szLang);
	SetControlText(hwnd, IDC_ST_SNP_TEXT, L"Snippet", szLang);
	SendDlgItemMessageW(hwnd, IDC_EDT_SNP_NAME, EM_LIMITTEXT, 127, 0);
	SendDlgItemMessageW(hwnd, IDC_EDT_SNP_TEXT, EM_LIMITTEXT, 1024 * 64 - 1, 0);
	if(lParam >= 0){
		SetPropW(hwnd, L"edit", (HANDLE)1);
		SendDlgItemMessageW(hwnd, IDC_EDT_SNP_NAME, EM_SETREADONLY, TRUE, 0);
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_TEXT;
		lvi.iItem = lParam;
		lvi.cchTextMax = 128;
		lvi.pszText = szBuffer;
		SendMessageW(m_hListSnippets, LVM_GETITEMW, 0, (LPARAM)&lvi);
		SetDlgItemTextW(hwnd, IDC_EDT_SNP_NAME, szBuffer);
		size = GetPrivateProfileIntW(szBuffer, L"size", 0, g_Paths.sSnippetsPath);
		if(size > 0){
			wchar_t		*szText;
			szText = malloc(sizeof(wchar_t) * (size + 1));
			if(!szText)
				return FALSE;
			GetPrivateProfileStringW(szBuffer, L"text", NULL, szText, size + 1, g_Paths.sSnippetsPath);
			ReplaceCRLFBackward(szText);
			SetDlgItemTextW(hwnd, IDC_EDT_SNP_TEXT, szText);
			free(szText);
		}
	}
	else{
		SetPropW(hwnd, L"edit", (HANDLE)0);
	}
	//subclass text edit box - for processing TAB characters
	m_OldEditProc = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwnd, IDC_EDT_SNP_TEXT), GWLP_WNDPROC, (LONG_PTR)EditProc); 
	return FALSE;
}

static void ReplaceCRLFForward(wchar_t * lpText){
	while(*lpText++){
		if(*lpText == '\n')
			*lpText = 6;
		else if(*lpText == '\r')
			*lpText = 7;
		else if(*lpText == '\t')
			*lpText = 8;
	}
}

static void ReplaceCRLFBackward(wchar_t * lpText){
	while(*lpText++){
		if(*lpText == 6)
			*lpText = '\n';
		else if(*lpText == 7)
			*lpText = '\r';
		else if(*lpText == 8)
			*lpText = '\t';
	}
}

static void ReplaceCRLFBySpace(wchar_t * lpText){
	while(*lpText++){
		if(*lpText == 6 || *lpText == 7 || *lpText == 8)
			*lpText = ' ';
	}
}

static BOOL SnippetExists(wchar_t * lpName){
	wchar_t			szSections[1024 * 10], *pw;

	szSections[0] = '\0';
	GetPrivateProfileSectionNamesW(szSections, 1024 * 10, g_Paths.sSnippetsPath);
	if(*szSections){
		pw = szSections;
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				if(_wcsicmp(pw, lpName) == 0)
					return TRUE;
			}
			while(*pw++)
				;
		}
	}
	return FALSE;
}

static void LoadSnippets(void){
	wchar_t			szSections[1024 * 10], *pw;
	int				size, index;
	LVITEMW			lvi;

	szSections[0] = '\0';
	GetPrivateProfileSectionNamesW(szSections, 1024 * 10, g_Paths.sSnippetsPath);
	if(*szSections){
		pw = szSections;
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				size = GetPrivateProfileIntW(pw, L"size", 0, g_Paths.sSnippetsPath);
				if(size > 0){
					wchar_t		*szText;
					szText = malloc(sizeof(wchar_t) * (size + 1));
					if(szText){
						GetPrivateProfileStringW(pw, L"text", NULL, szText, size + 1, g_Paths.sSnippetsPath);
						index = ListView_GetItemCount(m_hListSnippets);
						ZeroMemory(&lvi, sizeof(lvi));
						lvi.mask = LVIF_IMAGE | LVIF_TEXT;
						lvi.iImage = 5;
						lvi.pszText = pw;
						lvi.iItem = index;
						SendMessageW(m_hListSnippets, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
						lvi.iSubItem = 1;
						lvi.pszText = szText;
						ReplaceCRLFBySpace(szText);
						SendMessageW(m_hListSnippets, LVM_SETITEMW, 0, (LPARAM)&lvi);
						free(szText);
					}
				}
			}
			while(*pw++)
				;
		}
	}
}

static void DeleteSnippet(void){
	LVITEMW			lvi;
	wchar_t			szBuffer[128];
	int				index;

	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT;
	lvi.cchTextMax = 128;
	lvi.pszText = szBuffer;
	index = ListView_GetNextItem(m_hListSnippets, -1, LVNI_ALL | LVNI_SELECTED);
	lvi.iItem = index;
	SendMessageW(m_hListSnippets, LVM_GETITEMW, 0, (LPARAM)&lvi);
	WritePrivateProfileSectionW(szBuffer, NULL, g_Paths.sSnippetsPath);
	ListView_DeleteItem(m_hListSnippets, index);
}

static void InsertSnippet(void){
	HWND			hEdit;
	LVITEMW			lvi;
	wchar_t			szBuffer[128];
	int				size;

	hEdit = (HWND)SendMessageW(g_hMain, TBNPM_GETACTIVEEDIT, 0, 0);
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT;
	lvi.cchTextMax = 128;
	lvi.pszText = szBuffer;
	lvi.iItem = ListView_GetNextItem(m_hListSnippets, -1, LVNI_ALL | LVNI_SELECTED);
	SendMessageW(m_hListSnippets, LVM_GETITEMW, 0, (LPARAM)&lvi);
	size = GetPrivateProfileIntW(szBuffer, L"size", 0, g_Paths.sSnippetsPath);
	if(size > 0){
		wchar_t		*szText;
		szText = malloc(sizeof(wchar_t) * (size + 1));
		if(!szText)
			return;
		GetPrivateProfileStringW(szBuffer, L"text", NULL, szText, size + 1, g_Paths.sSnippetsPath);
		ReplaceCRLFBackward(szText);
		SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szText);
		SetFocus(hEdit);
		free(szText);
	}
}

static int CALLBACK StringCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){

	wchar_t		szString1[1024 * 64], szString2[1024 * 64];
	LVITEMW		lvi;

	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT;
	lvi.cchTextMax = 1024 * 64;
	lvi.pszText = szString1;
	lvi.iSubItem = m_lvsm.column;
	SendMessageW(m_hListSnippets, LVM_GETITEMTEXTW, (WPARAM)lParam1, (LPARAM)&lvi);
	lvi.pszText = szString2;
	SendMessageW(m_hListSnippets, LVM_GETITEMTEXTW, (WPARAM)lParam2, (LPARAM)&lvi);
	if((int)lParamSort == LVS_SORTASCENDING)
		return _wcsicmp(szString1, szString2);
	else
		return _wcsicmp(szString2, szString1);
}

static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	if(msg == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;
	return CallWindowProcW(m_OldEditProc, hwnd, msg, wParam, lParam);
}
