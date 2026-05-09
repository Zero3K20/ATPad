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

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include "gcpicker.h"
#include <gcolors.h>

#define	GCPICKER_CLASS_W			L"GCPICKER_CLASS_Wnd"
#define	GCPICKER_CLASS				"GCPICKER_CLASS_Wnd"
#define	GCP_ID_W					L"GCP_ID"
#define	GCP_ID						"GCP_ID"

#define	GetButton(hwnd)				(HWND)GetWindowLongPtr(hwnd, O_CMDHWND)
#define	GetButtonW(hwnd)			(HWND)GetWindowLongPtrW(hwnd, O_CMDHWND)
#define	GetOwner(hwnd)				(HWND)GetWindowLongPtr(hwnd, O_PHWND)
#define	GetOwnerW(hwnd)				(HWND)GetWindowLongPtrW(hwnd, O_PHWND)
#define	GetGCWindow(hwnd)			(HWND)GetWindowLongPtr(hwnd, O_CHWND)
#define	GetGCWindowW(hwnd)			(HWND)GetWindowLongPtrW(hwnd, O_CHWND)

typedef enum {O_CSTRUCT = 0, O_PHWND = 4, O_CMDHWND = 8};

static BOOL RegisterGCPickerWindowW(void);
static BOOL RegisterGCPickerWindow(void);
static LRESULT CALLBACK GCPicker_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void GCPicker_OnPaint(HWND hwnd);
static BOOL GCPicker_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void GCPicker_OnDestroy(HWND hwnd);
static void GCPicker_OnSetFocus(HWND hwnd, HWND hwndOldFocus);
static void GCPicker_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void GCPicker_OnSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
static void GCPicker_OnEnable(HWND hwnd, BOOL fEnable);
static void GCPicker_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static HWND InnerCreateWindow(int x, int y, int cx, int cy, HWND hParentWnd, int id, PGCOLORTYPE pgcType, BOOL unicode);

static HWND		m_hGColor;

HWND CreateGCPickerWindowW(int x, int y, int cx, int cy, HWND hParentWnd, int id, PGCOLORTYPE pgcType){
	return InnerCreateWindow(x, y, cx, cy, hParentWnd, id, pgcType, TRUE);
}

HWND CreateGCPickerWindow(int x, int y, int cx, int cy, HWND hParentWnd, int id, PGCOLORTYPE pgcType){
	return InnerCreateWindow(x, y, cx, cy, hParentWnd, id, pgcType, FALSE);
}

static BOOL RegisterGCPickerWindowW(void){

    WNDCLASSEXW 			wcx;

	ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
	wcx.cbWndExtra = 12;
    wcx.hInstance = GetModuleHandleW(0);
	wcx.lpfnWndProc = GCPicker_WndProc;
    wcx.lpszClassName = GCPICKER_CLASS_W;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    if (!RegisterClassExW(&wcx)){
		if(GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
			return TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL RegisterGCPickerWindow(void){

    WNDCLASSEX 			wcx;

	ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
	wcx.cbWndExtra = 12;
    wcx.hInstance = GetModuleHandle(0);
	wcx.lpfnWndProc = GCPicker_WndProc;
    wcx.lpszClassName = GCPICKER_CLASS;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    if (!RegisterClassEx(&wcx)){
		if(GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
			return TRUE;
		return FALSE;
	}
	return TRUE;
}

static LRESULT CALLBACK GCPicker_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_PAINT, GCPicker_OnPaint);
	HANDLE_MSG (hwnd, WM_CREATE, GCPicker_OnCreate);
	HANDLE_MSG (hwnd, WM_DESTROY, GCPicker_OnDestroy);
	HANDLE_MSG (hwnd, WM_SETFOCUS, GCPicker_OnSetFocus);
	HANDLE_MSG (hwnd, WM_COMMAND, GCPicker_OnCommand);
	HANDLE_MSG (hwnd, WM_SETFONT, GCPicker_OnSetFont);
	HANDLE_MSG (hwnd, WM_ENABLE, GCPicker_OnEnable);
	HANDLE_MSG (hwnd, WM_LBUTTONDBLCLK, GCPicker_OnLButtonDblClk);

	case GCM_NEW_COLOR:{
		BOOL			unicode = IsWindowUnicode(hwnd);
		PGCOLORTYPE		pgct;
		if(unicode){
			pgct = (PGCOLORTYPE)GetWindowLongPtrW(hwnd, O_CSTRUCT);
			memcpy(pgct, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
		}
		else{
			pgct = (PGCOLORTYPE)GetWindowLongPtr(hwnd, O_CSTRUCT);
			memcpy(pgct, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));	
		}
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		return TRUE;
	}
	case GCN_COLOR_SELECTED:{
		BOOL			unicode = IsWindowUnicode(hwnd);
		PGCOLORTYPE		pgct;
		if(unicode){
			pgct = (PGCOLORTYPE)GetWindowLongPtrW(hwnd, O_CSTRUCT);
			memcpy(pgct, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));
			PostMessageW(GetOwnerW(hwnd), GCN_COLOR_SELECTED, (WPARAM)GetPropW(hwnd, GCP_ID_W), (LPARAM)pgct);
		}
		else{
			pgct = (PGCOLORTYPE)GetWindowLongPtr(hwnd, O_CSTRUCT);
			memcpy(pgct, (PGCOLORTYPE)lParam, sizeof(GCOLORTYPE));	
			PostMessage(GetOwner(hwnd), GCN_COLOR_SELECTED, (WPARAM)GetProp(hwnd, GCP_ID), (LPARAM)pgct);
		}
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		return TRUE;
	}
	case GCM_CLOSE:
		if(m_hGColor){
			SendMessageW(m_hGColor, GCM_CLOSE, 0, 0);
		}
		return TRUE;
	default: return DefWindowProc (hwnd, msg, wParam, lParam);
	}
}

static void GCPicker_OnEnable(HWND hwnd, BOOL fEnable)
{
	BOOL			unicode = IsWindowUnicode(hwnd);

	if(unicode){
		EnableWindow(GetButtonW(hwnd), fEnable);
	}
	else{
		EnableWindow(GetButton(hwnd), fEnable);
	}
	InvalidateRect(hwnd, NULL, FALSE);
}

static void GCPicker_OnSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
{
	BOOL			unicode = IsWindowUnicode(hwndCtl);

	if(unicode){
		SendMessageW(GetButtonW(hwndCtl), WM_SETFONT, (WPARAM)hfont, TRUE);
	}
	else{
		SendMessage(GetButton(hwndCtl), WM_SETFONT, (WPARAM)hfont, TRUE);
	}
}

static void GCPicker_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	BOOL			unicode = IsWindowUnicode(hwnd);
	RECT			rc;
	PGCOLORTYPE		pgct;

	GetWindowRect(hwnd, &rc);
	if(unicode){
		pgct = (PGCOLORTYPE)GetWindowLongPtrW(hwnd, O_CSTRUCT);
		m_hGColor = CreateGColorsWindowW(rc.left, rc.bottom, hwnd, pgct);
	}
	else{
		pgct = (PGCOLORTYPE)GetWindowLongPtr(hwnd, O_CSTRUCT);
		m_hGColor = CreateGColorsWindow(rc.left, rc.bottom, hwnd, pgct);
	}
}

static void GCPicker_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL			unicode = IsWindowUnicode(hwnd);
	RECT			rc;
	PGCOLORTYPE		pgct;

	GetWindowRect(hwnd, &rc);
	if(unicode){
		if(hwndCtl == GetButtonW(hwnd)){
			pgct = (PGCOLORTYPE)GetWindowLongPtrW(hwnd, O_CSTRUCT);
			m_hGColor = CreateGColorsWindowW(rc.left, rc.bottom, hwnd, pgct);
		}
	}
	else{
		if(hwndCtl == GetButtonW(hwnd)){
			pgct = (PGCOLORTYPE)GetWindowLongPtr(hwnd, O_CSTRUCT);
			m_hGColor = CreateGColorsWindow(rc.left, rc.bottom, hwnd, pgct);
		}
	}
}

static void GCPicker_OnSetFocus(HWND hwnd, HWND hwndOldFocus)
{
	BOOL			unicode = IsWindowUnicode(hwnd);

	if(unicode)
		SetFocus(GetButtonW(hwnd));
	else
		SetFocus(GetButton(hwnd));
}

static BOOL GCPicker_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	RECT			rc;
	BOOL			unicode = IsWindowUnicode(hwnd);
	HWND			hCmd;
	HRGN			hRgn;

	GetClientRect(hwnd, &rc);
	hRgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 3, 3);
	SetWindowRgn(hwnd, hRgn, TRUE);
	if(unicode){
		hCmd = CreateWindowExW(0, L"BUTTON", L"...", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, rc.right - (rc.bottom - rc.top - 2), rc.top + 2, rc.bottom - rc.top - 4, rc.bottom - rc.top - 4, hwnd, NULL, GetModuleHandleW(0), 0);
		SetWindowLongPtrW(hwnd, O_CMDHWND, (LONG_PTR)hCmd);
	}
	else{
		hCmd = CreateWindowEx(0, "BUTTON", "...", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, rc.right - (rc.bottom - rc.top - 2), rc.top + 2, rc.bottom - rc.top - 4, rc.bottom - rc.top - 4, hwnd, NULL, GetModuleHandle(0), 0);
		SetWindowLongPtr(hwnd, O_CMDHWND, (LONG_PTR)hCmd);
	}
	
	return TRUE;
}

static void GCPicker_OnDestroy(HWND hwnd)
{
	BOOL			unicode = IsWindowUnicode(hwnd);

	if(unicode){
		free((void *)GetWindowLongPtrW(hwnd, O_CSTRUCT));
		RemovePropW(hwnd, GCP_ID_W);
	}
	else{
		free((void *)GetWindowLongPtr(hwnd, O_CSTRUCT));
		RemoveProp(hwnd, GCP_ID);
	}
}

static void GCPicker_OnPaint(HWND hwnd)
{
	PAINTSTRUCT		ps;
	RECT			rc;
	PGCOLORTYPE		pC;
	BOOL			unicode = IsWindowUnicode(hwnd);
	HBRUSH			hBrush = NULL;
	int				state;
	HPEN			hPen;

	BeginPaint(hwnd, &ps);
	state = SaveDC(ps.hdc);
	GetWindowRect(hwnd, &rc);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);
	if(unicode){
		pC = (PGCOLORTYPE)GetWindowLongPtrW(hwnd, O_CSTRUCT);
	}
	else{
		pC = (PGCOLORTYPE)GetWindowLongPtr(hwnd, O_CSTRUCT);
	}
	hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	SelectObject(ps.hdc, hPen);
	SelectObject(ps.hdc, GetSysColorBrush(COLOR_BTNFACE));
	RoundRect(ps.hdc, rc.left, rc.top, rc.right - 1, rc.bottom - 1, 3, 3);
	rc.left += 2;
	rc.top += 2;
	rc.right -=3;
	rc.bottom -=3;
	if(IsWindowEnabled(hwnd)){
		if(pC->sysIndex != -1){
			FillRect(ps.hdc, &rc, GetSysColorBrush(pC->sysIndex));
		}
		else{
			hBrush = CreateSolidBrush(pC->crValue);
			FillRect(ps.hdc, &rc, hBrush);
			DeleteObject(hBrush);
		}
	}
	else{
		FillRect(ps.hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
	}
	RestoreDC(ps.hdc, state);
	EndPaint(hwnd, &ps);
}

static HWND InnerCreateWindow(int x, int y, int cx, int cy, HWND hParentWnd, int id, PGCOLORTYPE pgcType, BOOL unicode){
	HWND			hwnd;
	PGCOLORTYPE		pC;

	if(unicode){
		if(!RegisterGCPickerWindowW()){
			return NULL;
		}
		hwnd = CreateWindowExW(0, GCPICKER_CLASS_W, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, x, y, cx, cy, hParentWnd, (HMENU)id, GetModuleHandleW(0), 0);
		if(!hwnd)
			return NULL;
		SetPropW(hwnd, GCP_ID_W, (HANDLE)id);
	}
	else{
		if(!RegisterGCPickerWindow()){
			return NULL;
		}
		hwnd = CreateWindowEx(0, GCPICKER_CLASS, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, x, y, cx, cy, hParentWnd, (HMENU)id, GetModuleHandle(0), 0);
		if(!hwnd)
			return NULL;
		SetProp(hwnd, GCP_ID, (HANDLE)id);
	}

	pC = calloc(1, sizeof(GCOLORTYPE));
	memcpy(pC, pgcType, sizeof(GCOLORTYPE));
	if(unicode){
		SetWindowLongPtrW(hwnd, O_CSTRUCT, (LONG_PTR)pC);
		SetWindowLongPtrW(hwnd, O_PHWND, (LONG_PTR)hParentWnd);
	}
	else{
		SetWindowLongPtr(hwnd, O_CSTRUCT, (LONG_PTR)pC);
		SetWindowLongPtr(hwnd, O_PHWND, (LONG_PTR)hParentWnd);
	}
	return hwnd;
}
