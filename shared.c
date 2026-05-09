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
#include <commctrl.h>
#include <richedit.h>

#include "stringconstants.h"
#include "globalvars.h"
#include "numericconstants.h"
#include "update.h"
#include "menu.h"

// #include "debug.h"
static HBITMAP CreateWrapBitmap(GCOLORTYPE gct);
static BOOL IsBookmarkLine(long line, P_TPEDIT pE);


void BitOff(int * data, int position){
	*data &= ~ (1 << position);
}

void BitOn(int * data, int position){
	*data |= (1 << position);
}

BOOL IsBitOn(int data, int position){
	if((data & (1 << position)) == (1 << position))
		return TRUE;
	return FALSE;
}

void ShowPopUp(HWND hwnd, HMENU hMenu){
	POINT pt;

	GetCursorPos(&pt);
	//set window foreground for further PostMessageW call 
	SetForegroundWindow(hwnd);
	//show popup menu
	TrackPopupMenu(hMenu, TPM_RIGHTALIGN, pt.x, pt.y, 0, hwnd, NULL);
	//remove popup menu when user clicks outside it
	PostMessageW(hwnd, WM_NULL, 0, 0);
}

void SetMenuText(int id, const wchar_t * lcpSection, const wchar_t * lcpFile, wchar_t * lpDefault, wchar_t * lpBuffer){

	wchar_t 		szId[12];

	_itow(id, szId, 10);
	GetPrivateProfileStringW(lcpSection, szId, lpDefault, lpBuffer, 256, lcpFile);
}

PMItem GetMItem(PMItem	lpMI, const int size, int id){
	for(int i = 0; i < size; i++){
		if(lpMI->id == id){
			return lpMI;
		}
		lpMI++;
	}
	return NULL;
}

void GetMIText(MItem * lpMI, const int size, int id, wchar_t * lpText){

	for(int i = 0; i < size; i++){
		if(lpMI->id == id){
			wcscpy(lpText, lpMI->szText);
			break;
		}
		lpMI++;
	}
}

void SetMIText(MItem * lpMI, const int size, int id, const wchar_t * lpText){

	for(int i = 0; i < size; i++){
		if(lpMI->id == id){
			wcscpy(lpMI->szText, lpText);
			break;
		}
		lpMI++;
	}
}

void GetTooltip(wchar_t * lpTip, PMItem pItems, int size, UINT id){
	//erase string
	*lpTip = '\0';
	for(int i = 0; i < size; i++, pItems++){
		if(pItems->id == id){
			wcscpy(lpTip, pItems->szText);
			break;
		}
	}
}

void StartUpdateProcess(HWND hwnd){
	HANDLE		hThread;
	DWORD		dwID;
	
	//start thread for checking updates
	hThread = CreateThread(NULL, 0, CheckForUpdateFunc, hwnd, 0, &dwID);
	if(hThread){
		CloseHandle(hThread);
	}
}

void AddWrapBitmap(BOOL fReplace, GCOLORTYPE gct, HIMAGELIST iml){
	HBITMAP			hBmp;

	//create and add word-wrap bitmap
	hBmp = CreateWrapBitmap(gct);
	if(fReplace){
		int		count = ImageList_GetImageCount(iml);
		if(count > 0){
			count--;
			ImageList_Replace(iml, count, hBmp, NULL);
		}
		else{
			ImageList_AddMasked(iml, hBmp, CLR_MASK);
		}
	}
	else{
		ImageList_AddMasked(iml, hBmp, CLR_MASK);
	}
	DeleteObject(hBmp);
	DeleteObject((void *)CLR_MASK);
}

static HBITMAP CreateWrapBitmap(GCOLORTYPE gct){
	HDC				hdc, hdcTemp;
	HBITMAP			hBmp, hBmpOld;
	HBRUSH			hBrush;
	RECT			rc;
	HPEN			hPen, hPenOld;

	//create word-wrap bitmap
	hdc = GetDC(g_hMain);
	hdcTemp = CreateCompatibleDC(hdc);
	hBmp = CreateCompatibleBitmap(hdc, 16, 16);
	ReleaseDC(g_hMain, hdc);
	hBrush = CreateSolidBrush(CLR_MASK);
	SetRect(&rc, 0, 0, 16, 16);
	hBmpOld = SelectObject(hdcTemp, hBmp);
	FillRect(hdcTemp, &rc, hBrush);
	if(gct.sysIndex != -1)
		hPen = CreatePen(PS_SOLID, 1, GetSysColor(gct.sysIndex));
	else
		hPen = CreatePen(PS_SOLID, 1, gct.crValue);
	hPenOld = SelectObject(hdcTemp, hPen);
	MoveToEx(hdcTemp, 2, 6, NULL);
	LineTo(hdcTemp, 9, 6);
	MoveToEx(hdcTemp, 2, 8, NULL);
	LineTo(hdcTemp, 7, 8);
	MoveToEx(hdcTemp, 2, 10, NULL);
	LineTo(hdcTemp, 7, 10);
	MoveToEx(hdcTemp, 2, 12, NULL);
	LineTo(hdcTemp, 9, 12);
	MoveToEx(hdcTemp, 7, 3, NULL);
	LineTo(hdcTemp, 14, 3);
	MoveToEx(hdcTemp, 14, 4, NULL);
	LineTo(hdcTemp, 14, 8);
	MoveToEx(hdcTemp, 13, 8, NULL);
	LineTo(hdcTemp, 9, 8);
	MoveToEx(hdcTemp, 11, 6, NULL);
	LineTo(hdcTemp, 11, 10);
	MoveToEx(hdcTemp, 10, 7, NULL);
	LineTo(hdcTemp, 10, 9);
	MoveToEx(hdcTemp, 9, 8, NULL);
	LineTo(hdcTemp, 9, 8);
	hBmp = SelectObject(hdcTemp, hBmpOld);
	SelectObject(hdcTemp, hPenOld);
	DeleteObject(hPen);
	DeleteDC(hdcTemp);
	return hBmp;
}

void DrawComboItem(const DRAWITEMSTRUCT * lpDrawItem){
	int			state;
	wchar_t		szBuffer[512];
	RECT		rc;

	//draw item in custom drawn combobox
	CopyRect(&rc, &lpDrawItem->rcItem);
	state = SaveDC(lpDrawItem->hDC);
	SetBkMode(lpDrawItem->hDC, TRANSPARENT);
	if((lpDrawItem->itemState & ODS_SELECTED) == ODS_SELECTED){
		FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		if((lpDrawItem->itemState & ODS_DISABLED) == ODS_DISABLED){
			SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_GRAYTEXT));
		}
		else{
			SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
	}
	else{
		FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, GetSysColorBrush(COLOR_WINDOW));
		if((lpDrawItem->itemState & ODS_DISABLED) == ODS_DISABLED){
			SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_GRAYTEXT));
		}
		else{
			SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
		}
	}
	SendMessageW(lpDrawItem->hwndItem, CB_GETLBTEXT, lpDrawItem->itemID, (LPARAM)szBuffer);
	DrawTextW(lpDrawItem->hDC, szBuffer, -1, &rc, DT_VCENTER | DT_LEFT | DT_SINGLELINE);
	RestoreDC(lpDrawItem->hDC, state);
}

void SetControlText(HWND hwnd, int id, wchar_t * lpDefault, wchar_t * lpLangFile){
	wchar_t			szBuffer[512], szKey[16];

	//set control text according to language file value
	_itow(id, szKey, 10);
	GetPrivateProfileStringW(S_CONTROLS, szKey, lpDefault, szBuffer, 512, lpLangFile);
	SetDlgItemTextW(hwnd, id, szBuffer);
}

void DrawLineNumbersRegular(HWND hwnd, HDC hdc, TPSETTINGS settings, GCOLORTYPE ctBM, TPFORMAT lnformat, HFONT linesFont, int textHeight, int lineTextHeight, P_TPEDIT pE){

	RECT		rc, rcLine, rcMargin, rcBookmark, rcLeft, rcRight;
	HDC			hdcTemp;
	HBITMAP		hbm, hbmOld;
	HFONT		hOldF;
	COLORREF	cOldC;
	wchar_t		szNumber[32];
	POINT		pt;
	HBRUSH		hBrush, hBrBm;
	long		first, last, temp, pos, charIndex, line, count;
	HICON		hBmIcon;

	//get bookmark icon
	hBmIcon = ImageList_GetIcon(g_ImlTab, 3, ILD_TRANSPARENT);
	//get total lines count
	count = RichEdit_GetLineCount(hwnd);
	SetBkMode(hdc, TRANSPARENT);
	//get formatting rectangle
	SendMessageW(hwnd, EM_GETRECT, 0, (LPARAM)&rc);
	//set left side rectangle
	if(settings.showLines)
		SetRect(&rcMargin, 0, rc.top - 2, settings.marginWidth + L_BOOKMARK, rc.bottom);
	else
		SetRect(&rcMargin, 0, rc.top - 2, L_BOOKMARK, rc.bottom);
	//prepare compatible DC and bitmap
	hdcTemp = CreateCompatibleDC(hdc);
	SetBkMode(hdcTemp, TRANSPARENT);
	hbm = CreateCompatibleBitmap(hdc, rcMargin.right - rcMargin.left, rcMargin.bottom - rcMargin.top);
	hbmOld = SelectObject(hdcTemp, hbm);
	if(settings.showLines){
		//prepare lines numbers font and colors
		hOldF = SelectObject(hdcTemp, linesFont);
		if(lnformat.cText.sysIndex != -1)
			cOldC = SetTextColor(hdcTemp, GetSysColor(lnformat.cText.sysIndex));
		else
			cOldC = SetTextColor(hdcTemp, lnformat.cText.crValue);
		if(lnformat.cBack.sysIndex != -1)
			hBrush = GetSysColorBrush(lnformat.cBack.sysIndex);
		else
			hBrush = CreateSolidBrush(lnformat.cBack.crValue);
	}
	//prepare selection margin color
	if(ctBM.sysIndex != -1)
		hBrBm = GetSysColorBrush(ctBM.sysIndex);
	else
		hBrBm = CreateSolidBrush(ctBM.crValue);
	//set and fill rectangles
	if(settings.showLines){
		SetRect(&rcLeft, 0, rcMargin.top, settings.marginWidth, rcMargin.bottom);
		FillRect(hdcTemp, &rcLeft, hBrush);
		SetRect(&rcRight, rcLeft.right, rcMargin.top, rcLeft.right + L_BOOKMARK, rcMargin.bottom);
	}
	else{
		SetRect(&rcRight, 0, rcMargin.top, L_BOOKMARK, rcMargin.bottom);
	}
	FillRect(hdcTemp, &rcRight, hBrBm);
	//find first and last visible lines
	int tp = rc.top;	
	pos = SendMessageW(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	first = SendMessageW(hwnd, EM_EXLINEFROMCHAR, 0, pos);
	rc.top = rc.bottom;
	pos = SendMessageW(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	last = SendMessageW(hwnd, EM_EXLINEFROMCHAR, 0, pos);
	while(first <= last){
		//find current line number and increase (numbers start from 0)
		charIndex = SendMessageW(hwnd, EM_LINEINDEX, first, 0);
		line = SendMessageW(hwnd, EM_EXLINEFROMCHAR, 0, charIndex);
		line++;
		//find first character position
		SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, charIndex);
		temp = pt.y + textHeight;
		//set needed rectangles
		if(settings.showLines){
			SetRect(&rcLine, 2, pt.y, settings.marginWidth - 2, temp);
			SetRect(&rcBookmark, rcRight.left, pt.y, rcRight.right, temp);
		}
		else{
			SetRect(&rcBookmark, 0, pt.y, L_BOOKMARK, temp);
		}
		first++;
		temp -= pt.y;
		temp -= lineTextHeight;
		temp /= 2;
		temp += pt.y;
		temp += lineTextHeight;
		tp += textHeight;
		if(temp <= rc.bottom && rcLine.top >= rcMargin.top){
			if(pE && IsBookmarkLine(first - 1, pE)){
				//draw bookmark
				int x, y, w, h;
				h = 16;
				w = h;
				x = rcBookmark.left + ((rcBookmark.right - rcBookmark.left) - w) / 2;
				y = rcBookmark.top + ((rcBookmark.bottom - rcBookmark.top) - h) / 2;
				DrawIconEx(hdcTemp, x, y, hBmIcon, w, h, 0, 0, DI_NORMAL);
			}
			if(settings.showLines){
				//draw line number
				wsprintfW(szNumber, L"%i", first);
				DrawTextW(hdcTemp, szNumber, -1, &rcLine, settings.alignNumbers | DT_SINGLELINE | DT_VCENTER);
			}
		}
	}
	if(count - last == 2){
		//the case when Return key pressed on the last line - need special attention
		pt.y += textHeight;
		temp = pt.y + textHeight;
		if(settings.showLines){
			SetRect(&rcLine, 2, pt.y, settings.marginWidth - 2, temp);
			SetRect(&rcBookmark, rcRight.left, pt.y, rcRight.right, temp);
		}
		else{
			SetRect(&rcBookmark, 0, pt.y, L_BOOKMARK, temp);
		}
		first++;
		temp -= pt.y;
		temp -= lineTextHeight;
		temp /= 2;
		temp += pt.y;
		temp += lineTextHeight;

		if(temp <= rc.bottom && rcLine.top >= rcMargin.top){
			if(pE && IsBookmarkLine(first - 1, pE)){
				int x, y, w, h;
				h = 16;
				w = h;
				x = rcBookmark.left + ((rcBookmark.right - rcBookmark.left) - w) / 2;
				y = rcBookmark.top + ((rcBookmark.bottom - rcBookmark.top) - h) / 2;
				DrawIconEx(hdcTemp, x, y, hBmIcon, w, h, 0, 0, DI_NORMAL);
			}
			if(settings.showLines){
				wsprintfW(szNumber, L"%i", first);
				DrawTextW(hdcTemp, szNumber, -1, &rcLine, settings.alignNumbers | DT_SINGLELINE | DT_VCENTER);
			}
		}
	}
	//copy bitmap to original DC
	BitBlt(hdc, rcMargin.left, rcMargin.top, rcMargin.right - rcMargin.left, rcMargin.bottom - rcMargin.top, hdcTemp, 0, 0, SRCCOPY);
	//clean up
	if(ctBM.sysIndex != -1)
		DeleteObject(hBrBm);
	if(settings.showLines){
		if(lnformat.cBack.sysIndex == -1)
			DeleteObject(hBrush);
		SelectObject(hdcTemp, hOldF);
		SetTextColor(hdcTemp, cOldC);
	}
	SelectObject(hdcTemp, hbmOld);
	DeleteObject(hbm);
	DeleteDC(hdcTemp);
	DestroyIcon(hBmIcon);
}

void DrawLineNumbersWrap(HWND hwnd, HDC hdc, TPSETTINGS settings, GCOLORTYPE ctBM, TPFORMAT lnformat, HFONT linesFont, int textHeight, int lineTextHeight, P_TPEDIT pE, HIMAGELIST hIml){

	RECT		rc, rcLine, rcMargin, rcLeft, rcRight, rcBookmark;
	HDC			hdcTemp;
	HBITMAP		hbm, hbmOld;
	HFONT		hOldF;
	COLORREF	cOldC;
	wchar_t		szNumber[32];
	POINT		pt;
	HBRUSH		hBrush, hBrBm;
	long		first, last, temp, pos, charIndex, line, count;
	TEXTRANGEW	trg;
	wchar_t		szChar[2];
	BOOL		drawWrap = FALSE;
	long		crCount = 0, addition = 0, start;
	HANDLE		pTemp;
	HICON		hIcon = NULL, hBmIcon = NULL;

	//get bookmark icon
	hBmIcon = ImageList_GetIcon(hIml, 3, ILD_TRANSPARENT);
	//get wrap icon
	hIcon = ImageList_GetIcon(hIml, ImageList_GetImageCount(hIml) - 1, ILD_TRANSPARENT);
	//get total lines count
	count = RichEdit_GetLineCount(hwnd);
	SetBkMode(hdc, TRANSPARENT);
	//get control formatting rectangle
	SendMessageW(hwnd, EM_GETRECT, 0, (LPARAM)&rc);
	//set left margin rectangle
	if(settings.showLines)
		SetRect(&rcMargin, 0, rc.top - 2, settings.marginWidth + L_BOOKMARK, rc.bottom);
	else
		SetRect(&rcMargin, 0, rc.top - 2, L_BOOKMARK, rc.bottom);
	//create DC for further drawing
	hdcTemp = CreateCompatibleDC(hdc);
	SetBkMode(hdcTemp, TRANSPARENT);
	//create bitmap for drawing
	hbm = CreateCompatibleBitmap(hdc, rcMargin.right - rcMargin.left, rcMargin.bottom - rcMargin.top);
	hbmOld = SelectObject(hdcTemp, hbm);

	if(settings.showLines){
		hOldF = SelectObject(hdcTemp, linesFont);
		//define line numbers area colors
		if(lnformat.cText.sysIndex != -1)
			cOldC = SetTextColor(hdcTemp, GetSysColor(lnformat.cText.sysIndex));
		else
			cOldC = SetTextColor(hdcTemp, lnformat.cText.crValue);
		if(lnformat.cBack.sysIndex != -1)
			hBrush = GetSysColorBrush(lnformat.cBack.sysIndex);
		else
			hBrush = CreateSolidBrush(lnformat.cBack.crValue);
	}
	if(ctBM.sysIndex != -1)
		hBrBm = GetSysColorBrush(ctBM.sysIndex);
	else
		hBrBm = CreateSolidBrush(ctBM.crValue);

	if(settings.showLines){
		SetRect(&rcLeft, 0, rcMargin.top, settings.marginWidth, rcMargin.bottom);
		//fill line numbers area with background color
		FillRect(hdcTemp, &rcLeft, hBrush);
		SetRect(&rcRight, rcLeft.right, rcMargin.top, rcLeft.right + L_BOOKMARK, rcMargin.bottom);
	}
	else{
		SetRect(&rcRight, 0, rcMargin.top, L_BOOKMARK, rcMargin.bottom);
	}
	FillRect(hdcTemp, &rcRight, hBrBm);

	//get first visible character index
	pos = SendMessageW(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	//begin to search line brakes from the beginning of control to first visible character
	trg.chrg.cpMin = 0;
	trg.chrg.cpMax = pos;
	trg.lpstrText = (wchar_t *)HeapAlloc(g_hHeap, 0, (pos + 1) * sizeof(wchar_t));
	pTemp = (HANDLE)trg.lpstrText;
	SendMessageW(hwnd, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
	for(int i = pos; i >=0; i--){
		if(*trg.lpstrText == 13 || *trg.lpstrText == 10){	
			crCount++;
		}
		trg.lpstrText++;
	}
	HeapFree(g_hHeap, 0, pTemp);
	//get first visible line index
	first = SendMessageW(hwnd, EM_EXLINEFROMCHAR, 0, pos);
	//find last character index and last visible line index
	rc.top = rc.bottom;
	pos = SendMessageW(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	last = SendMessageW(hwnd, EM_EXLINEFROMCHAR, 0, pos);
	//begin searching for CR characters
	start = first;
	do{
		//get character index of line
		charIndex = SendMessageW(hwnd, EM_LINEINDEX, start, 0);
		if(charIndex > 0){
			//check whether previous character is line brake
			trg.chrg.cpMin = charIndex - 1;
			trg.chrg.cpMax = charIndex;
			trg.lpstrText = szChar;
			SendMessageW(hwnd, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
			if(*trg.lpstrText != 13 && *trg.lpstrText != 10){
				addition++;
			}
			else{
				break;
			}
		}
		else{
			break;
		}
		start++;
	}while(start <= last && *trg.lpstrText != 13  && *trg.lpstrText != 10);

	line = crCount;
	if(addition)
		line++;
	
	//begin line numbering loop
	while(first <= last){
		drawWrap = FALSE;
		//get character index of line
		charIndex = SendMessageW(hwnd, EM_LINEINDEX, first, 0);
		if(charIndex > 0){
			//check whether previous character is line brake
			trg.chrg.cpMin = charIndex - 1;
			trg.chrg.cpMax = charIndex;
			trg.lpstrText = szChar;
			SendMessageW(hwnd, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
			if(*trg.lpstrText != 13 && *trg.lpstrText != 10){
				//should draw wrap icon
				drawWrap = TRUE;
			}
			else{
				//just increase line number
				line++;
			}
		}
		else{
			//just increase line number
			line++;
		}

		//get character coordinates
		SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, charIndex);
		temp = pt.y + textHeight;
		if(settings.showLines){
			SetRect(&rcLine, 2, pt.y, settings.marginWidth - 2, temp);
			SetRect(&rcBookmark, rcRight.left, pt.y, rcRight.right, temp);
		}
		else{
			SetRect(&rcBookmark, 0, pt.y, L_BOOKMARK, temp);
		}
		first++;

		temp -= pt.y;
		temp -= lineTextHeight;
		temp /= 2;
		temp += pt.y;
		temp += lineTextHeight;

		if(temp <= rc.bottom && rcLine.top >= rcMargin.top){
			if(pE && IsBookmarkLine(first - 1, pE)){
				int x, y, w, h;
				h = 16;
				w = h;
				x = rcBookmark.left + ((rcBookmark.right - rcBookmark.left) - w) / 2;
				y = rcBookmark.top + ((rcBookmark.bottom - rcBookmark.top) - h) / 2;
				DrawIconEx(hdcTemp, x, y, hBmIcon, w, h, 0, 0, DI_NORMAL);
			}
			if(settings.showLines){
				if(!drawWrap){
					wsprintfW(szNumber, L"%i", line);
					DrawTextW(hdcTemp, szNumber, -1, &rcLine, settings.alignNumbers | DT_SINGLELINE | DT_VCENTER);
				}
				else{
					int x, y, w, h;
					h = 16;
					w = h;
					if(settings.alignNumbers == DT_CENTER)
						x = ((rcLine.right - rcLine.left) - w) / 2;
					else if(settings.alignNumbers == DT_LEFT)
						x = rcLine.left;
					else if(settings.alignNumbers == DT_RIGHT)
						x = rcLine.right - w;
					y = rcLine.top + ((rcLine.bottom - rcLine.top) - h) / 2;
					DrawIconEx(hdcTemp, x, y, hIcon, w, h, 0, 0, DI_NORMAL);
				}
			}
		}
	}
	//in case of Enter after last line
	if(count - last == 2){
		pt.y += textHeight;
		temp = pt.y + textHeight;
		SetRect(&rcLine, 2, pt.y, settings.marginWidth - 2, temp);
		first++;
		temp -= pt.y;
		temp -= lineTextHeight;
		temp /= 2;
		temp += pt.y;
		temp += lineTextHeight;

		if(temp <= rc.bottom && rcLine.top >= rcMargin.top){
			if(settings.showLines){
				wsprintfW(szNumber, L"%i", line + 1);
				DrawTextW(hdcTemp, szNumber, -1, &rcLine, settings.alignNumbers | DT_SINGLELINE | DT_VCENTER);
			}
		}
	}
	//copy line numbering area image onto static control
	BitBlt(hdc, rcMargin.left, rcMargin.top, rcMargin.right - rcMargin.left, rcMargin.bottom - rcMargin.top, hdcTemp, 0, 0, SRCCOPY);
	//free resources
	
	if(ctBM.sysIndex != -1)
		DeleteObject(hBrBm);
	if(settings.showLines){
		if(lnformat.cBack.sysIndex == -1)
			DeleteObject(hBrush);
		SelectObject(hdcTemp, hOldF);
		SetTextColor(hdcTemp, cOldC);
	}
	SelectObject(hdcTemp, hbmOld);
	DeleteObject(hbm);
	DeleteDC(hdcTemp);
	if(hIcon)
		DestroyIcon(hIcon);
	if(hBmIcon)
		DestroyIcon(hBmIcon);
}

void DrawCRLFWhiteSpace(HWND hwnd, BOOL fPreview, TPSETTINGS settings, GCOLORTYPE tabscolor, GCOLORTYPE spacecolor, int textHeight, HBITMAP bCR, HBITMAP bLF){
	HDC			hdc, hdcTempCR, hdcTempLF;
	HBITMAP		hbmOldCR, hbmOldLF;
	RECT		rc, rcClip;
	long		pos, count, last, cnt;
	POINT		pt;
	TEXTRANGEW	trg;
	HANDLE		pTemp;
	BITMAP		bmCR, bmLF;
	HPEN		hPen, hPenOld = NULL;
	HPEN		hPSpace;
	HRGN		hRgn;
	wchar_t		* pChar;

	SendMessageW(hwnd, EM_GETRECT, 0, (LPARAM)&rcClip);
	hdc = GetDC(hwnd);
	hdcTempCR = CreateCompatibleDC(hdc);
	hdcTempLF = CreateCompatibleDC(hdc);
	GetObject(bCR, sizeof(BITMAP), &bmCR);
	GetObject(bLF, sizeof(BITMAP), &bmLF);
	hbmOldCR = SelectObject(hdcTempCR, bCR);
	hbmOldLF = SelectObject(hdcTempLF, bLF);
	if(settings.showWS){
		if(settings.showTabs){
			if(tabscolor.sysIndex == -1){
				hPen = CreatePen(PS_SOLID, 1, tabscolor.crValue);
			}
			else{
				hPen = CreatePen(PS_SOLID, 1, GetSysColor(tabscolor.sysIndex));
			}
			hPenOld = SelectObject(hdc, hPen);
		}
		if(settings.showSpaces){
			if(spacecolor.sysIndex == -1){
				hPSpace = CreatePen(PS_SOLID, 1, spacecolor.crValue);
			}
			else{
				hPSpace = CreatePen(PS_SOLID, 1, GetSysColor(spacecolor.sysIndex));
			}
			if(hPenOld == NULL)
				hPenOld = SelectObject(hdc, hPSpace);
		}
	}
	rcClip.top += 2;
	hRgn = CreateRectRgnIndirect(&rcClip);
	SelectClipRgn(hdc, hRgn);
	if(!fPreview){
		//edit control of main window
		SendMessageW(hwnd, EM_GETRECT, 0, (LPARAM)&rc);
		pos = SendMessageW(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&rc);
		trg.chrg.cpMin = pos;
		SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)pos);
		rc.left = rc.right;
		rc.top = rc.bottom;
		pos = SendMessageW(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&rc);
		trg.chrg.cpMax = pos;
		last = SendMessageW(hwnd, EM_EXLINEFROMCHAR, 0, pos);
		count = RichEdit_GetLineCount(hwnd);
		if(count - last == 2){
			trg.chrg.cpMax++;
		}
		cnt = trg.chrg.cpMax - trg.chrg.cpMin;
		trg.lpstrText = (wchar_t *)HeapAlloc(g_hHeap, 0, (cnt + 1) * sizeof(wchar_t));
		pTemp = (HANDLE)trg.lpstrText;
		SendMessageW(hwnd, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
	}
	else{
		//edit control of settings dialog
		trg.chrg.cpMin = 0;
		trg.chrg.cpMax = wcslen(PREVIEW_STRING);
		trg.lpstrText = PREVIEW_STRING;
	}
	for(int i = trg.chrg.cpMin; i <= trg.chrg.cpMax; i++){
		if(*trg.lpstrText == 13 && settings.showWS && settings.showCRLF){
			//draw CR character
			pChar = trg.lpstrText;
			SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, i);
			pt.y += (textHeight - bmCR.bmHeight) / 2;
			pt.x += 1;
			if(*(--pChar) == 10){
				SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, i - 1);
				pt.x += bmLF.bmWidth + 1;
				pt.y += (textHeight - bmCR.bmHeight) / 2;
			}
			BitBlt(hdc, pt.x, pt.y, bmCR.bmWidth, bmCR.bmHeight, hdcTempCR, 0, 0, SRCCOPY);
		}
		else if(*trg.lpstrText == 10 && settings.showWS && settings.showCRLF){
			//draw LF character
			pChar = trg.lpstrText;
			SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, i);
			pt.y += (textHeight - bmLF.bmHeight) / 2;
			pt.x += 1;
			if(*(--pChar) == 13){
				SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, i - 1);
				pt.x += bmCR.bmWidth + 1;
				pt.y += (textHeight - bmLF.bmHeight) / 2;
			}
			BitBlt(hdc, pt.x, pt.y, bmLF.bmWidth, bmLF.bmHeight, hdcTempLF, 0, 0, SRCCOPY);
		}
		else if(*trg.lpstrText == 9 && settings.showWS && settings.showTabs){
			//draw tab character
			POINT	pt1;
			SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, i);
			pt.y += (textHeight - 1) / 2;
			if(i < trg.chrg.cpMax){
				SelectObject(hdc, hPen);
				SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt1, i + 1);
				MoveToEx(hdc, pt.x + 1, pt.y, NULL);
				LineTo(hdc, pt1.x - 1, pt.y);
				MoveToEx(hdc, pt1.x - 4, pt.y - 2, NULL);
				LineTo(hdc, pt1.x - 4, pt.y + 3);
				MoveToEx(hdc, pt1.x - 3, pt.y - 1, NULL);
				LineTo(hdc, pt1.x - 3, pt.y + 2);
			}
		}
		else if(*trg.lpstrText == 32 && settings.showWS && settings.showSpaces){
			//draw space character
			POINT	pt1;
			SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, i);
			pt.y += (textHeight - 1) / 2;
			if(i < trg.chrg.cpMax){
				SelectObject(hdc, hPSpace);
				SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)&pt1, i + 1);
				MoveToEx(hdc, pt.x + (pt1.x - pt.x) / 2 - 1, pt.y - 1, NULL);
				LineTo(hdc, pt.x + (pt1.x - pt.x) / 2 + 1, pt.y);
				MoveToEx(hdc, pt.x + (pt1.x - pt.x) / 2, pt.y , NULL);
				LineTo(hdc, pt.x + (pt1.x - pt.x) / 2 - 2, pt.y);
			}
		}
		trg.lpstrText++;
	}
	if(!fPreview){
		HeapFree(g_hHeap, 0, pTemp);
	}
	if(settings.showWS){
		SelectObject(hdc, hPenOld);
		if(settings.showTabs)
			DeleteObject(hPen);
		if(settings.showSpaces)
			DeleteObject(hPSpace);
	}
	SelectObject(hdcTempCR, hbmOldCR);
	SelectObject(hdcTempLF, hbmOldLF);
	DeleteDC(hdcTempCR);
	DeleteDC(hdcTempLF);
	SelectClipRgn(hdc, NULL);
	DeleteObject(hRgn);
	ReleaseDC(hwnd, hdc);
}

void GetTextHeight(HWND hwnd, int * pheight, HFONT font){
	HDC			hdc, hdcTemp;
	HFONT		hfOld;
	SIZE		sz;

	//get text height for selected font
	hdc = GetDC(hwnd);
	hdcTemp = CreateCompatibleDC(hdc);
	hfOld = SelectObject(hdcTemp, font);
	GetTextExtentPoint32W(hdcTemp, L"WWW", 3, &sz);
	*pheight = sz.cy;
	SelectObject(hdcTemp, hfOld);
	DeleteDC(hdcTemp);
	ReleaseDC(hwnd, hdc);
}

void GetLineTextHeight(HWND hwnd, int * pheight, HFONT font){
	HDC			hdc, hdcTemp;
	HFONT		hfOld;
	SIZE		sz;

	//get number height for selected font
	hdc = GetDC(hwnd);
	hdcTemp = CreateCompatibleDC(hdc);
	hfOld = SelectObject(hdcTemp, font);
	GetTextExtentPoint32W(hdcTemp, L"999", 3, &sz);
	*pheight = sz.cy;
	SelectObject(hdcTemp, hfOld);
	DeleteDC(hdcTemp);
	ReleaseDC(hwnd, hdc);
}

void CreateCRLF(HWND hwnd, HBITMAP * hCR, HBITMAP * hLF, GCOLORTYPE cBack, TPFORMAT tBack){
	HDC				hdcMain, hdcCR, hdcLF;
	HBITMAP			hbmCR, hbmLF, hbmOldCR, hbmOldLF;
	RECT			rcCR, rcLF;
	HBRUSH			hBrBack;
	BOOL			fDeleteBack = FALSE;
	HPEN			hPen, hPenOldCR, hPenOldLF;

	//create CRLF bitmaps
	if(*hCR)
		DeleteObject(*hCR);
	if(*hLF)
		DeleteObject(*hLF);
	hdcMain = GetDC(hwnd);
	hdcCR = CreateCompatibleDC(hdcMain);
	hdcLF = CreateCompatibleDC(hdcMain);
	hbmCR = CreateCompatibleBitmap(hdcMain, 10, 8);
	hbmLF = CreateCompatibleBitmap(hdcMain, 9, 8);
	SetRect(&rcCR, 0, 0, 10, 8);
	SetRect(&rcLF, 0, 0, 9, 8);
	SetBkMode(hdcCR, TRANSPARENT);
	SetBkMode(hdcLF, TRANSPARENT);
	hbmOldCR = SelectObject(hdcCR, hbmCR);
	hbmOldLF = SelectObject(hdcLF, hbmLF);
	if(cBack.sysIndex == -1){
		hBrBack = CreateSolidBrush(cBack.crValue);
		fDeleteBack = TRUE;
	}
	else{
		hBrBack = GetSysColorBrush(cBack.sysIndex);
	}
	if(tBack.cBack.sysIndex == -1){
		hPen = CreatePen(PS_SOLID, 1, tBack.cBack.crValue);
	}
	else{
		hPen = CreatePen(PS_SOLID, 1, GetSysColor(tBack.cBack.sysIndex));
	}
	hPenOldCR = SelectObject(hdcCR, hPen);
	hPenOldLF = SelectObject(hdcLF, hPen);
	FillRect(hdcCR, &rcCR, hBrBack);
	FillRect(hdcLF, &rcLF, hBrBack);
	//draw CR
	MoveToEx(hdcCR, 3, 1, NULL);
	LineTo(hdcCR, 1, 1);
	MoveToEx(hdcCR, 1, 2, NULL);
	LineTo(hdcCR, 1, 6);
	MoveToEx(hdcCR, 2, 6, NULL);
	LineTo(hdcCR, 4, 6);
	MoveToEx(hdcCR, 7, 1, NULL);
	LineTo(hdcCR, 5, 1);
	MoveToEx(hdcCR, 5, 1, NULL);
	LineTo(hdcCR, 5, 7);
	MoveToEx(hdcCR, 8, 2, NULL);
	LineTo(hdcCR, 8, 4);
	MoveToEx(hdcCR, 7, 4, NULL);
	LineTo(hdcCR, 5, 4);
	MoveToEx(hdcCR, 8, 5, NULL);
	LineTo(hdcCR, 8, 7);
	//draw LF
	MoveToEx(hdcLF, 1, 1, NULL);
	LineTo(hdcLF, 1, 7);
	MoveToEx(hdcLF, 1, 6, NULL);
	LineTo(hdcLF, 4, 6);
	MoveToEx(hdcLF, 5, 1, NULL);
	LineTo(hdcLF, 5, 7);
	MoveToEx(hdcLF, 5, 1, NULL);
	LineTo(hdcLF, 8, 1);
	MoveToEx(hdcLF, 5, 4, NULL);
	LineTo(hdcLF, 7, 4);
	SelectObject(hdcCR, hbmOldCR);
	*hCR = hbmCR;
	SelectObject(hdcLF, hbmOldLF);
	*hLF = hbmLF;

	if(fDeleteBack){
		DeleteObject(hBrBack);
	}
	SelectObject(hdcCR, hPenOldCR);
	SelectObject(hdcCR, hPenOldLF);
	DeleteObject(hPen);
	DeleteDC(hdcCR);
	DeleteDC(hdcLF);
	ReleaseDC(hwnd, hdcMain);
}

static BOOL IsBookmarkLine(long line, P_TPEDIT pE){
	P_TPBMRK	pB;

	//check whether specified line is bookmarked
	pB = pE->bookmarks;
	while(pB){
		if(pB->line == line)
			return TRUE;
		pB = pB->next;
	}
	return FALSE;
}

void GetCurrentPositionRegular(HWND hEdit, CHARRANGE chrg, long * px, long * py){
	long 			x, y, pos, last, count, charCount;
	RECT			rc;
	GETTEXTLENGTHEX	gtx;

	gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
	gtx.codepage = 1200;
	charCount = SendMessageW(hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	count = RichEdit_GetLineCount(hEdit);
	SendMessageW(hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	rc.top = rc.bottom;
	pos = SendMessageW(hEdit, EM_CHARFROMPOS, 0, (LPARAM)&rc);

	last = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, pos);
	x = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin);
	y = chrg.cpMin;
	y -= SendMessageW(hEdit, EM_LINEINDEX, x, 0);
	y++;
	x++;
	if(count - last == 2 && chrg.cpMin == charCount){
		x++;
		y = 1;
	}
	*px = x;
	*py = y;
}

void GetCurrentPositionWrap(HWND hEdit, CHARRANGE chrg, long * px, long * py){
	long 			x, y, pos, last, count, charCount;
	TEXTRANGEW		trg;
	long			crCount = 0, crChar = 0;
	RECT			rc;
	GETTEXTLENGTHEX	gtx;
	HANDLE			* pTemp;

	//get total number of chars in control
	gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
	gtx.codepage = 1200;
	charCount = SendMessageW(hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	count = RichEdit_GetLineCount(hEdit);
	
	//find last visible line start position
	SendMessageW(hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	rc.top = rc.bottom;
	pos = SendMessageW(hEdit, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	last = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, pos);

	//get pseudo line index
	x = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin);

	trg.chrg.cpMin = 0;
	trg.chrg.cpMax = chrg.cpMin;
	trg.lpstrText = (wchar_t *)HeapAlloc(g_hHeap, 0, (trg.chrg.cpMax + 1) * sizeof(wchar_t));
	pTemp = (HANDLE)trg.lpstrText;
	SendMessageW(hEdit, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
	for(int i = 0; i <= trg.chrg.cpMax; i++){
		if(*trg.lpstrText++ == 13){
			//increase CR count
			crCount++;
			//store CR position + 1: start of next line
			crChar = i + 1;
		}
	}
	HeapFree(g_hHeap, 0, pTemp);

	if(crCount == 0){
		//store character index as column
		y = chrg.cpMin + 1;
	}
	else{
		//get difference between start position of selection and start position of line
		y = chrg.cpMin - crChar + 1;
	}
	x = crCount + 1;

	//in case of Enter after last line
	if(count - last == 2 && chrg.cpMin == charCount){
		y = 1;
	}
	*px = x;
	*py = y;
}

// int ReplaceEditText(HWND hEdit, int params){
	// FINDTEXTEXW		ftx;
	// int				result;

	// if(params == -1)
		// params = g_FParam;

	// ZeroMemory(&ftx, sizeof(ftx));
	// SendMessageW(hEdit, EM_EXGETSEL, 0, (LPARAM)&ftx.chrg);

	// wchar_t			szBuffer[ftx.chrg.cpMax - ftx.chrg.cpMin + 1];
	// SendMessageW(hEdit, EM_GETSELTEXT, 0, (LPARAM)szBuffer);
	// if(wcscmp(szBuffer, g_SearchString) == 0){
		// SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)g_ReplaceString);
	// }
	// if((g_FParam & FR_DOWN) == FR_DOWN && *g_ReplaceString)
		// ftx.chrg.cpMin = ftx.chrg.cpMax;
	// ftx.chrg.cpMax = -1;
	// ftx.lpstrText = g_SearchString;
	// result = SendMessageW(hEdit, EM_FINDTEXTEXW, params, (LPARAM)&ftx);
	// if(result != -1){
		// result = SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&ftx.chrgText);
	// }
	// return result;
// }
