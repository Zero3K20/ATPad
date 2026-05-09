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
#include <glistbox.h>
#include "gcolors.h"

#define	GCOLOR_CLASS_W			L"GCOLOR_CLASS_Wnd"
#define	GCOLOR_CLASS			"GCOLOR_CLASS_Wnd"
#define	T_CAPTION_1_W			L"Custom"
#define	T_CAPTION_2_W			L"Web"
#define	T_CAPTION_3_W			L"System"
#define	T_CAPTION_1				"Custom"
#define	T_CAPTION_2				"Web"
#define	T_CAPTION_3				"System"
#define	LST_CLR_PROC_PROP_W		L"_GColors_LstProc_Name"
#define	LST_CLR_PROC_PROP		"_GColors_LstProc_Name"

#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))

static void CreateBasicRects(LPRECT lpClient);
static void DrawBasicRects(HDC hdc);
static LRESULT CALLBACK BasicProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK Colors_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Colors_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void Colors_OnDestroy(HWND hwnd);
static void Colors_OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
static BOOL RegisterGColorsWindowW(void);
static BOOL RegisterGColorsWindow(void);
static int IsSystemColor(PGCOLORTYPE pgcType, BOOL unicode);
static int IsWebColor(PGCOLORTYPE pgcType, BOOL unicode);
static LRESULT CALLBACK LstProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void ListOnReturn(HWND hwnd, BOOL unicode);
static void BasicOnReturn(void);
static void DrawBasicFocus(int x, int y);
static HWND InnerCreateWindow(int x, int y, HWND hParentWnd, PGCOLORTYPE pgcType, BOOL unicode);
static HWND CreateWebList(int x, int y, int w, int h, HWND hParent, BOOL unicode);
static HWND CreateSystemList(int x, int y, int w, int h, HWND hParent, BOOL unicode);
static HWND CreateBasicWindow(int x, int y, int w, int h, HWND hParent, BOOL unicode);

// basic colors
static int arrBasics[8][6] = {0xFFFFFF,0xE0E0E0,0xC0C0C0,0x808080,0x404040,0x0,		
							0xC0C0FF,0x8080FF,0xFF,0xC0,0x80,0x40,					
							0xC0E0FF,0x80C0FF,0x80FF,0x40C0,0x4080,0x404080,			
							0xC0FFFF,0x80FFFF,0xFFFF,0xC0C0,0x8080,0x4040,			
							0xC0FFC0,0x80FF80,0xFF00,0xC000,0x8000,0x4000,			
							0xFFFFC0,0xFFFF80,0xFFFF00,0xC0C000,0x808000,0x404000,	
							0xFFC0C0,0xFF8080,0xFF0000,0xC00000,0x800000,0x400000,	
							0xFFC0FF,0xFF80FF,0xFF00FF,0xC000C0,0x800080,0x400040};
// basic colors rectangles
static RECT arrRectBasics[8][6];

// web colors
static int arrWebs[] = {0x000000,0x696969,0x808080,0x0A9A9A9,0x0C0C0C0,0x0D3D3D3,0x0DCDCDC,0x0F5F5F5,0x0FFFFFF,
						0x8F8FBC,0x5C5CCD,0x2A2AA5,0x2222B2,0x8080F0,0x000080,0x00008B,0x0000FF,0x0FAFAFF,
						0x0E1E4FF,0x7280FA,0x4763FF,0x7A96E9,0x507FFF,0x0045FF,0x7AA0FF,0x2D52A0,0x0EEF5FF,
						0x1E69D2,0x13458B,0x60A4F4,0x0B9DAFF,0x3F85CD,0x0E6F0FA,0x0C4E4FF,0x008CFF,0x87B8DE,
						0x8CB4D2,0x0D7EBFA,0x0ADDEFF,0x0CDEBFF,0x0D5EFFF,0x0B5E4FF,0x00A5FF,0x0B3DEF5,0x0E6F5FD,
						0x0F0FAFF,0x0B86B8,0x20A5DA,0x0DCF8FF,0x00D7FF,0x8CE6F0,0x0CDFAFF,0x0AAE8EE,0x6BB7BD,
						0x0DCF5F5,0x0D2FAFA,0x008080,0x00FFFF,0x0E0FFFF,0x0F0FFFF,0x238E6B,0x32CD9A,0x2F6B55,
						0x2FFFAD,0x00FF7F,0x00FC7C,0x8FBC8F,0x228B22,0x32CD32,0x90EE90,
						0x98FB98,0x006400,0x008000,0x00FF00,0x0F0FFF0,0x578B2E,0x71B33C,0x7FFF00,
						0x0FAFFF5,0x9AFA00,0x0AACD66,0x0D4FF7F,0x0D0E040,0x0AAB220,0x0CCD148,
						0x4F4F2F,0x0EEEEAF,0x808000,0x8B8B00,0x0FFFF00,0x0FFFF00,0x0FFFFE0,0x0FFFFF0,0x0D1CE00,
						0x0A09E5F,0x0E6E0B0,0x0E6D8AD,0x0FFBF00,0x0EBCE87,0x0FACE87,0x0B48246,0x0FFF8F0,0x0FF901E,
						0x908070,0x998877,0x0DEC4B0,0x0ED9564,0x0E16941,0x701919,0x0FAE6E6,
						0x800000,0x8B0000,0x0CD0000,0x0FF0000,0x0FFF8F8,0x0CD5A6A,0x8B3D48,0x0EE687B,
						0x0DB7093,0x0E22B8A,0x82004B,0x0CC3299,0x0D30094,0x0D355BA,0x0D8BFD8,0x0DDA0DD,0x0EE82EE,
						0x800080,0x8B008B,0x0FF00FF,0x0FF00FF,0x0D670DA,0x8515C7,0x9314FF,0x0B469FF,0x0F5F0FF,0x9370DB,
						0x3C14DC,0x0CBC0FF,0x0C1B6FF
						};
// web colors names
static wchar_t * arrNamesWeb[] = {L"Black",L"DimGray",L"Gray",L"DarkGray",L"Silver",L"LightGrey",L"Gainsboro",L"WhiteSmoke",L"White",
								L"RosyBrown",L"IndianRed",L"Brown",L"FireBrick",L"LightCoral",L"Maroon",L"DarkRed",L"Red",L"Snow",
								L"MistyRose",L"Salmon",L"Tomato",L"DarkSalmon",L"Coral",L"OrangeRed",L"LightSalmon",L"Sienna",L"Seashell",
								L"Chocolate",L"SaddleBrown",L"SandyBrown",L"PeachPuff",L"Peru",L"Linen",L"Bisque",L"DarkOrange",L"BurlyWood",
								L"Tan",L"AntiqueWhite",L"NavajoWhite",L"BlanchedAlmond",L"PapayaWhip",L"Moccasin",L"Orange",L"Wheat",L"OldLace",
								L"FloralWhite",L"DarkGoldenrod",L"Goldenrod",L"Cornsilk",L"Gold",L"Khaki",L"LemonChiffon",L"PaleGoldenrod",L"DarkKhaki",
								L"Beige",L"LightGoldenrodYellow",L"Olive",L"Yellow",L"LightYellow",L"Ivory",L"OliveDrab",L"YellowGreen",L"DarkOliveGreen",
								L"GreenYellow",L"Chartreuse",L"LawnGreen",L"DarkSeaGreen",L"ForestGreen",L"LimeGreen",L"LightGreen",
								L"PaleGreen",L"DarkGreen",L"Green",L"Lime",L"Honeydew",L"SeaGreen",L"MediumSeaGreen",L"SpringGreen",
								L"MintCream",L"MediumSpringGreen",L"MediumAquamarine",L"Aquamarine",L"Turquoise",L"LightSeaGreen",L"MediumTurquoise",
								L"DarkSlateGray",L"PaleTurquoise",L"Teal",L"DarkCyan",L"Aqua",L"Cyan",L"LightCyan",L"Azure",L"DarkTurquoise",
								L"CadetBlue",L"PowderBlue",L"LightBlue",L"DeepSkyBlue",L"SkyBlue",L"LightSkyBlue",L"SteelBlue",L"AliceBlue",L"DodgerBlue",
								L"SlateGray",L"LightSlateGray",L"LightSteelBlue",L"CornflowerBlue",L"RoyalBlue",L"MidnightBlue",L"Lavender",
								L"Navy",L"DarkBlue",L"MediumBlue",L"Blue",L"GhostWhite",L"SlateBlue",L"DarkSlateBlue",L"MediumSlateBlue",
								L"MediumPurple",L"BlueViolet",L"Indigo",L"DarkOrchid",L"DarkViolet",L"MediumOrchid",L"Thistle",L"Plum",L"Violet",
								L"Purple",L"DarkMagenta",L"Magenta",L"Fuchsia",L"Orchid",L"MediumVioletRed",L"DeepPink",L"HotPink",L"LavenderBlush",L"PaleVioletRed",
								L"Crimson",L"Pink",L"LightPink"
								};

// system colors names
static wchar_t * arrNamesSystem[] = {L"ActiveBorder",
									L"ActiveCaption",
									L"ActiveCaptionText",
									L"ApplicationWorkspace",
									L"Control",
									L"ControlDark",
									L"ControlDarkDark",
									L"ControlLight",
									L"ControlLightLight",
									L"ControlText",
									L"Desktop",
									L"GrayText",
									L"Highlight",
									L"HighlightText",
									L"HotTrack",
									L"InactiveBorder",
									L"InactiveCaption",
									L"InactiveCaptionText",
									L"InfoBack",
									L"InforText",
									L"Menu",
									L"MenuText",
									L"ScrollBar",
									L"Window",
									L"WindowFrame",
									L"WindowText"};

// system colors indexes
static int	arrIndexesSystem[] = {COLOR_ACTIVEBORDER,
									COLOR_ACTIVECAPTION,
									COLOR_CAPTIONTEXT,
									COLOR_APPWORKSPACE,
									COLOR_BTNFACE,
									COLOR_BTNSHADOW,
									COLOR_3DDKSHADOW,
									COLOR_3DLIGHT,
									COLOR_3DHILIGHT,
									COLOR_BTNTEXT,
									COLOR_DESKTOP,
									COLOR_GRAYTEXT,
									COLOR_HIGHLIGHT,
									COLOR_HIGHLIGHTTEXT,
									COLOR_HOTLIGHT,
									COLOR_INACTIVEBORDER,
									COLOR_INACTIVECAPTION,
									COLOR_INACTIVECAPTIONTEXT,
									COLOR_INFOBK,
									COLOR_INFOTEXT,
									COLOR_MENU,
									COLOR_MENUTEXT,
									COLOR_SCROLLBAR,
									COLOR_WINDOW,
									COLOR_WINDOWFRAME,
									COLOR_WINDOWTEXT};

static BOOL			bRegistered = FALSE, bDrawBasic = TRUE;
static HWND			hParent, hColors, hTab, hBasics, hWebs, hSystems;
static HFONT		hFont;
static GCOLORTYPE	savedGC;
static POINT		currBasic;

HWND CreateGColorsWindowW(int x, int y, HWND hParentWnd, PGCOLORTYPE pgcType){
	return InnerCreateWindow(x, y, hParentWnd, pgcType, TRUE);
}

HWND CreateGColorsWindow(int x, int y, HWND hParentWnd, PGCOLORTYPE pgcType){
	return InnerCreateWindow(x, y, hParentWnd, pgcType, FALSE);
}

static HWND InnerCreateWindow(int x, int y, HWND hParentWnd, PGCOLORTYPE pgcType, BOOL unicode){
	int					selected;

	// save or clear color type
	if(pgcType != NULL)
		memcpy(&savedGC, pgcType, sizeof(GCOLORTYPE));
	else
		ZeroMemory(&savedGC, sizeof(GCOLORTYPE));
	// store parent window
	hParent = hParentWnd;
	// if the function is called for the first time
	if(!bRegistered){
		// register gcolor window
		if(unicode){
			bRegistered = RegisterGColorsWindowW();
		}
		else{
			bRegistered = RegisterGColorsWindow();
		}
		if(!bRegistered)
			return NULL;
		// create font, main window and set its font
		if(unicode){
			hFont = CreateFontW(-MulDiv(8, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Sans Serif");
			hColors = CreateWindowExW(0, GCOLOR_CLASS_W, NULL, WS_VISIBLE | WS_POPUP, x, y, 183, 159, hParent, NULL, GetModuleHandleW(0), NULL);
			if(hColors == NULL)
				return NULL;
			SendMessageW(hColors, WM_SETFONT, (WPARAM)hFont, TRUE);
		}
		else{
			hFont = CreateFont(-MulDiv(8, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "MS Sans Serif");
			hColors = CreateWindowEx(0, GCOLOR_CLASS, NULL, WS_VISIBLE | WS_POPUP, x, y, 183, 159, hParent, NULL, GetModuleHandle(0), NULL);
			if(hColors == NULL)
				return NULL;
			SendMessage(hColors, WM_SETFONT, (WPARAM)hFont, TRUE);
		}
		// set basic colors rectangle to the first
		currBasic.x = 0;
		currBasic.y = 0;
	}
	else{
		// if this is not the first call to function, just show gcolor window on its previous position
		SetWindowPos(hColors, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	if((selected = IsSystemColor(pgcType, unicode)) != -1){
		// if received color is system color - show system colors list and select appropriate tab
		ShowWindow(hBasics, SW_SHOW);
		if(unicode)
			SendMessageW(hSystems, LB_SETCURSEL, selected, 0);
		else
			SendMessage(hSystems, LB_SETCURSEL, selected, 0);
		ShowWindow(hBasics, SW_HIDE);
		ShowWindow(hWebs, SW_HIDE);
		ShowWindow(hSystems, SW_SHOW);
		if(unicode)
			SendMessageW(hTab, TCM_SETCURSEL, 2, 0);
		else
			SendMessage(hTab, TCM_SETCURSEL, 2, 0);
		SetFocus(hSystems);
	}
	else if((selected = IsWebColor(pgcType, unicode)) != -1){
		// if received color is web color - show web colors list and select appropriate tab
		ShowWindow(hBasics, SW_SHOW);
		if(unicode)
			SendMessageW(hWebs, LB_SETCURSEL, selected, 0);
		else
			SendMessage(hWebs, LB_SETCURSEL, selected, 0);
		ShowWindow(hBasics, SW_HIDE);
		ShowWindow(hWebs, SW_SHOW);
		ShowWindow(hSystems, SW_HIDE);
		if(unicode)
			SendMessageW(hTab, TCM_SETCURSEL, 1, 0);
		else
			SendMessage(hTab, TCM_SETCURSEL, 1, 0);
		SetFocus(hWebs);
	}
	else{
		// otherwise show basic colors
		ShowWindow(hBasics, SW_SHOW);
		ShowWindow(hWebs, SW_HIDE);
		ShowWindow(hSystems, SW_HIDE);
		if(unicode)
			SendMessageW(hTab, TCM_SETCURSEL, 0, 0);
		else
			SendMessage(hTab, TCM_SETCURSEL, 0, 0);
		SetFocus(hBasics);
	}
	return hColors;
}

/*-@@+@@--------------------------------[Do not edit manually]------------
 Procedure: IsSystemColor
 Created  : Wed Jul 18 11:44:08 2007
 Modified : Wed Jul 18 11:44:08 2007

 Synopsys : Checks whether given color belongs to system colors
 Input    : pgcType - pointer to GCOLORTYPE structure
            unicode - Unicode flag
 Output   : Returns system colors listbox item index if given color belongs 
            to system colors, otherwise returns -1
 Errors   : 
 ------------------------------------------------------------------@@-@@-*/
static int IsSystemColor(PGCOLORTYPE pgcType, BOOL unicode){
	int count = 0;

	if(unicode)
		count = SendMessageW(hSystems, LB_GETCOUNT, 0, 0);
	else
		count = SendMessage(hSystems, LB_GETCOUNT, 0, 0);

	for(int i = 0; i < count; i++){
		PGLIST_IMAGE pData;
		if(unicode)
			pData = (PGLIST_IMAGE)SendMessageW(hSystems, LB_GETITEMDATA, i, 0);
		else
			pData = (PGLIST_IMAGE)SendMessage(hSystems, LB_GETITEMDATA, i, 0);
		if(pData->dwValue == pgcType->crValue && pData->reserved == pgcType->sysIndex)
			return i;
	}
	return -1;
}

/*-@@+@@--------------------------------[Do not edit manually]------------
 Procedure: IsWebColor
 Created  : Wed Jul 18 11:45:01 2007
 Modified : Wed Jul 18 11:45:29 2007

 Synopsys : Checks whether given color belongs to web colors
 Input    : pgcType - pointer to GCOLORTYPE structure
            unicode - Unicode flag
 Output   : Returns web colors listbox item index if given color belongs to 
            web colors, otherwise returns -1
 Errors   : 
 ------------------------------------------------------------------@@-@@-*/
static int IsWebColor(PGCOLORTYPE pgcType, BOOL unicode){
	int count = 0;

	if(unicode)
		count = SendMessageW(hWebs, LB_GETCOUNT, 0, 0);
	else
		count = SendMessage(hWebs, LB_GETCOUNT, 0, 0);
	for(int i = 0; i < count; i++){
		PGLIST_IMAGE pData;
		if(unicode)
			pData = (PGLIST_IMAGE)SendMessageW(hWebs, LB_GETITEMDATA, i, 0);
		else
			pData = (PGLIST_IMAGE)SendMessage(hWebs, LB_GETITEMDATA, i, 0);
		if(pData->dwValue == pgcType->crValue)
			return i;
	}
	return -1;
}

static BOOL RegisterGColorsWindowW(void){

    WNDCLASSEXW 			wcx;

	ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.hInstance = GetModuleHandleW(0);
	wcx.lpfnWndProc = Colors_WndProc;
    wcx.lpszClassName = GCOLOR_CLASS_W;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    if (!RegisterClassExW(&wcx)){
		if(GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
			return TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL RegisterGColorsWindow(void){

    WNDCLASSEX 			wcx;

	ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.hInstance = GetModuleHandle(0);
	wcx.lpfnWndProc = Colors_WndProc;
    wcx.lpszClassName = GCOLOR_CLASS;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    if (!RegisterClassEx(&wcx)){
		if(GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
			return TRUE;
		return FALSE;
	}
	return TRUE;
}

static LRESULT CALLBACK Colors_WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR			lpnmhdr;
	int				currTab;
	POINT			pt;
	BOOL			unicode;

	unicode = IsWindowUnicode(hwnd);
	switch(msg){
	HANDLE_MSG (hwnd, WM_CREATE, Colors_OnCreate);
	HANDLE_MSG (hwnd, WM_DESTROY, Colors_OnDestroy);
	HANDLE_MSG (hwnd, WM_KEYDOWN, Colors_OnKeyDown);
	case GCM_CLOSE:
		bRegistered = FALSE;
		hParent = NULL;
		DestroyWindow(hwnd);
		return 0;
	case WM_ACTIVATE:
		// hide gcolor window when it becames inactive
		if(wParam == WA_INACTIVE)
			ShowWindow(hwnd, SW_HIDE);
		return 0;
	case WM_NOTIFY:
		lpnmhdr = (LPNMHDR)lParam;
		if(lpnmhdr->code == TCN_SELCHANGE){
			// show appropriate colors
			if(unicode)
				currTab = SendMessageW(lpnmhdr->hwndFrom, TCM_GETCURSEL, 0, 0);
			else
				currTab = SendMessage(lpnmhdr->hwndFrom, TCM_GETCURSEL, 0, 0);
			switch(currTab){
			case 0:
				ShowWindow(hBasics, SW_SHOW);
				ShowWindow(hWebs, SW_HIDE);
				ShowWindow(hSystems, SW_HIDE);
				SetFocus(hBasics);
				break;
			case 1:
				ShowWindow(hBasics, SW_HIDE);
				ShowWindow(hWebs, SW_SHOW);
				ShowWindow(hSystems, SW_HIDE);
				SetFocus(hWebs);
				break;
			case 2:
				ShowWindow(hBasics, SW_HIDE);
				ShowWindow(hWebs, SW_HIDE);
				ShowWindow(hSystems, SW_SHOW);
				SetFocus(hSystems);
				break;
			}
			return 0;
		}
		else
			if(unicode)
				return DefWindowProcW(hwnd, msg, wParam, lParam);
			else
				return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	case WM_COMMAND:
		if((HWND)lParam == hBasics && HIWORD(wParam) == STN_CLICKED){
			// select appropriate basic colors rectangle
			GetCursorPos(&pt);
			MapWindowPoints(HWND_DESKTOP, hBasics, &pt, 1);
			for(int i = 0; i < 8; i++){
				for(int j = 0; j < 6; j++){
					if(PtInRect(&arrRectBasics[i][j], pt)){
						if(currBasic.x != i || currBasic.y != j){
							DrawBasicFocus(i, j);
							BasicOnReturn();
							return 0;
						}
					}
				}	
			}		
		}
		if(unicode)
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		else
			return DefWindowProc(hwnd, msg, wParam, lParam);
	default: 
		if(unicode)
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		else
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

static void Colors_OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if(vk == VK_ESCAPE)
		ShowWindow(hwnd, SW_HIDE);
}

static void Colors_OnDestroy(HWND hwnd)
{
	DeleteObject(hFont);
}

static BOOL Colors_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	RECT			rc;
	TCITEMW			tciw;
	TCITEM			tci;
	wchar_t			szBufferU[128];
	char			szBuffer[128];
	BOOL			unicode;

	unicode = IsWindowUnicode(hwnd);

	GetClientRect(hwnd, &rc);
	// create tab window and set its font
	if(unicode){
		hTab = CreateWindowExW(0, L"SysTabControl32", NULL, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, GetModuleHandleW(0), NULL);
		if(hTab == NULL)
			return FALSE;
		SendMessageW(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);
	}
	else{
		hTab = CreateWindowEx(0, "SysTabControl32", NULL, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, GetModuleHandle(0), NULL);
		if(hTab == NULL)
			return FALSE;
		SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);
	}
	// add tabs
	if(unicode){
		ZeroMemory(&tciw, sizeof(tciw));
		tciw.mask = TCIF_TEXT;
		tciw.pszText = szBufferU;
		wcscpy(szBufferU, T_CAPTION_1_W);
		SendMessageW(hTab, TCM_INSERTITEMW, 0, (LPARAM)&tciw);
		wcscpy(szBufferU, T_CAPTION_2_W);
		SendMessageW(hTab, TCM_INSERTITEMW, 1, (LPARAM)&tciw);
		wcscpy(szBufferU, T_CAPTION_3_W);
		SendMessageW(hTab, TCM_INSERTITEMW, 2, (LPARAM)&tciw);
		SendMessageW(hTab, TCM_ADJUSTRECT, FALSE, (LPARAM)&rc);
	}
	else{
		ZeroMemory(&tci, sizeof(tci));
		tci.mask = TCIF_TEXT;
		tci.pszText = szBuffer;
		strcpy(szBuffer, T_CAPTION_1);
		SendMessage(hTab, TCM_INSERTITEM, 0, (LPARAM)&tci);
		strcpy(szBuffer, T_CAPTION_2);
		SendMessage(hTab, TCM_INSERTITEM, 1, (LPARAM)&tci);
		strcpy(szBuffer, T_CAPTION_3);
		SendMessage(hTab, TCM_INSERTITEM, 2, (LPARAM)&tci);
		SendMessage(hTab, TCM_ADJUSTRECT, FALSE, (LPARAM)&rc);
	}
	// create basic colors window
	hBasics = CreateBasicWindow(0, rc.top, rc.right, rc.bottom - rc.top, hwnd, unicode);
	if(hBasics == NULL)
		return FALSE;
	// create web colors window
	hWebs = CreateWebList(0, rc.top, rc.right, rc.bottom - rc.top, hwnd, unicode);
	if(hWebs == NULL)
		return FALSE;
	if(unicode)
		SendMessageW(hWebs, WM_SETFONT, (WPARAM)hFont, TRUE);
	else
		SendMessage(hWebs, WM_SETFONT, (WPARAM)hFont, TRUE);
	// create system colors window
	hSystems = CreateSystemList(0, rc.top, rc.right, rc.bottom - rc.top, hwnd, unicode);
	if(hSystems == NULL)
		return FALSE;
	if(unicode)
		SendMessageW(hSystems, WM_SETFONT, (WPARAM)hFont, TRUE);
	else
		SendMessage(hSystems, WM_SETFONT, (WPARAM)hFont, TRUE);
	return TRUE;
}

static HWND CreateWebList(int x, int y, int w, int h, HWND hParent, BOOL unicode){
	HWND			hWeb;
	GLIST_IMAGE		gi;
	WNDPROC			hProc;
	char			szName[128];

	ZeroMemory(&gi, sizeof(gi));
	gi.nType = GLIST_I_COLOR;
	gi.frame = TRUE;
	gi.szImage.cx = 22;
	gi.szImage.cy = 12;
	gi.itemHeight = 16;
	if(unicode){
		hWeb = CreateGListWindowW(WS_EX_CLIENTEDGE ,WS_VISIBLE | WS_CHILD, x, y, w, h, hParent);
		if(hWeb == NULL)
			return NULL;
		// subclass web colors window
		SetLastError(0);
		hProc = (WNDPROC)SetWindowLongPtrW(hWeb, GWLP_WNDPROC, (LONG_PTR)LstProc);
		if(hProc == 0 && GetLastError() != 0)
			return NULL;
		SetPropW(hWeb, LST_CLR_PROC_PROP_W, hProc);
	}
	else{
		hWeb = CreateGListWindow(WS_EX_CLIENTEDGE ,WS_VISIBLE | WS_CHILD, x, y, w, h, hParent);
		if(hWeb == NULL)
			return NULL;
		// subclass web colors window
		SetLastError(0);
		hProc = (WNDPROC)SetWindowLongPtr(hWeb, GWLP_WNDPROC, (LONG_PTR)LstProc);
		if(hProc == 0 && GetLastError() != 0)
			return NULL;
		SetProp(hWeb, LST_CLR_PROC_PROP, hProc);
	}
	// add colors entries
	for(int i = 0; i < NELEMS(arrWebs); i++){
		gi.dwValue = arrWebs[i];
		if(unicode){
			if(GList_AddEntryW(hWeb, &gi, arrNamesWeb[i]) == -1)
				return NULL;
		}
		else{
			// convert unicode string to ascii
			WideCharToMultiByte(CP_ACP, 0, arrNamesWeb[i], -1, szName, 128, NULL, NULL);
			if(GList_AddEntry(hWeb, &gi, szName) == -1)
				return NULL;
		}
	}
	return hWeb;
}

static HWND CreateSystemList(int x, int y, int w, int h, HWND hParent, BOOL unicode){
	int arrSystem[26];
	HWND			hSys;
	GLIST_IMAGE		gi;
	WNDPROC			hProc;
	char			szName[128];

	arrSystem[0] = GetSysColor(COLOR_ACTIVEBORDER);
	arrSystem[1] = GetSysColor(COLOR_ACTIVECAPTION);
	arrSystem[2] = GetSysColor(COLOR_CAPTIONTEXT);
	arrSystem[3] = GetSysColor(COLOR_APPWORKSPACE);
	arrSystem[4] = GetSysColor(COLOR_BTNFACE);
	arrSystem[5] = GetSysColor(COLOR_BTNSHADOW);
	arrSystem[6] = GetSysColor(COLOR_3DDKSHADOW);
	arrSystem[7] = GetSysColor(COLOR_3DLIGHT);
	arrSystem[8] = GetSysColor(COLOR_3DHILIGHT);
	arrSystem[9] = GetSysColor(COLOR_BTNTEXT);
	arrSystem[10] = GetSysColor(COLOR_DESKTOP);
	arrSystem[11] = GetSysColor(COLOR_GRAYTEXT);
	arrSystem[12] = GetSysColor(COLOR_HIGHLIGHT);
	arrSystem[13] = GetSysColor(COLOR_HIGHLIGHTTEXT);
	arrSystem[14] = GetSysColor(COLOR_HOTLIGHT);
	arrSystem[15] = GetSysColor(COLOR_INACTIVEBORDER);
	arrSystem[16] = GetSysColor(COLOR_INACTIVECAPTION);
	arrSystem[17] = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
	arrSystem[18] = GetSysColor(COLOR_INFOBK);
	arrSystem[19] = GetSysColor(COLOR_INFOTEXT);
	arrSystem[20] = GetSysColor(COLOR_MENU);
	arrSystem[21] = GetSysColor(COLOR_MENUTEXT);
	arrSystem[22] = GetSysColor(COLOR_SCROLLBAR);
	arrSystem[23] = GetSysColor(COLOR_WINDOW);
	arrSystem[24] = GetSysColor(COLOR_WINDOWFRAME);
	arrSystem[25] = GetSysColor(COLOR_WINDOWTEXT);
	
	ZeroMemory(&gi, sizeof(gi));
	gi.nType = GLIST_I_COLOR;
	gi.frame = TRUE;
	gi.szImage.cx = 22;
	gi.szImage.cy = 12;
	gi.itemHeight = 16;
	if(unicode){
		hSys = CreateGListWindowW(WS_EX_CLIENTEDGE ,WS_VISIBLE | WS_CHILD, x, y, w, h, hParent);
		if(hSys == NULL)
			return NULL;
		// subclass system colors window
		SetLastError(0);
		hProc = (WNDPROC)SetWindowLongPtrW(hSys, GWLP_WNDPROC, (LONG_PTR)LstProc);
		if(hProc == 0 && GetLastError() != 0)
			return NULL;
		SetPropW(hSys, LST_CLR_PROC_PROP_W, hProc);
	}
	else{
		hSys = CreateGListWindow(WS_EX_CLIENTEDGE ,WS_VISIBLE | WS_CHILD, x, y, w, h, hParent);
		if(hSys == NULL)
			return NULL;
		// subclass system colors window
		SetLastError(0);
		hProc = (WNDPROC)SetWindowLongPtr(hSys, GWLP_WNDPROC, (LONG_PTR)LstProc);
		if(hProc == 0 && GetLastError() != 0)
			return NULL;
		SetProp(hSys, LST_CLR_PROC_PROP, hProc);
	}
	// add colors entries
	for(int i = 0; i < NELEMS(arrSystem); i++){
		gi.dwValue = arrSystem[i];
		gi.reserved = arrIndexesSystem[i];
		if(unicode){
			if(GList_AddEntryW(hSys, &gi, arrNamesSystem[i]) == -1)
				return NULL;
		}
		else{
			// convert unicode to ascii
			WideCharToMultiByte(CP_ACP, 0, arrNamesSystem[i], -1, szName, 128, NULL, NULL);
			if(GList_AddEntry(hSys, &gi, szName) == -1)
				return NULL;
		}
			
	}
	return hSys;
}

static HWND CreateBasicWindow(int x, int y, int w, int h, HWND hParent, BOOL unicode){

	HWND		hBasic;
	WNDPROC		hProc;

	if(unicode){
		hBasic = CreateWindowExW(0, L"static", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY, x, y, w, h, hParent, NULL, GetModuleHandleW(0), NULL);
		if(hBasic == NULL)
			return NULL;
		// subclass basic colors window
		SetLastError(0);
		hProc = (WNDPROC)SetWindowLongPtrW(hBasic, GWLP_WNDPROC, (LONG_PTR)BasicProc);
		if(hProc == 0 && GetLastError() != 0)
			return NULL;
		SetPropW(hBasic, LST_CLR_PROC_PROP_W, hProc);
	}
	else{
		hBasic = CreateWindowEx(0, "static", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY, x, y, w, h, hParent, NULL, GetModuleHandle(0), NULL);
		if(hBasic == NULL)
			return NULL;
		// subclass basic colors window
		SetLastError(0);
		hProc = (WNDPROC)SetWindowLongPtr(hBasic, GWLP_WNDPROC, (LONG_PTR)BasicProc);
		if(hProc == 0 && GetLastError() != 0)
			return NULL;
		SetProp(hBasic, LST_CLR_PROC_PROP, hProc);
	}
	return hBasic;
}

static void CreateBasicRects(LPRECT lpClient){
	int side;

	// create basic colors rectangle
	side = (lpClient->right - lpClient->left - 9) / 8;
	for(int i =0; i < 8; i++){
		for(int j = 0; j < 6; j++){
			SetRect(&arrRectBasics[i][j],i * (side + 1) + 1, j * (side + 1) + 1, (i + 1) * (side + 1), (j + 1) * (side + 1));
		}
	}
}

static void DrawBasicRects(HDC hdc){
	HBRUSH		hbr;

	// draw basic colors
	for(int i =0; i < 8; i++){
		for(int j = 0; j < 6; j++){
			hbr = CreateSolidBrush(arrBasics[i][j]);
			FillRect(hdc, &arrRectBasics[i][j], hbr);
			DeleteObject(hbr);
		}
	}
}

static LRESULT CALLBACK BasicProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT		ps;
	HBRUSH			hbr;
	RECT			rc;
	HDC				hdc;
	WNDPROC			hProc;
	BOOL			unicode;

	unicode = IsWindowUnicode(hwnd);

	if(unicode)
		hProc = (WNDPROC)GetPropW(hwnd, LST_CLR_PROC_PROP_W);
	else
		hProc = (WNDPROC)GetProp(hwnd, LST_CLR_PROC_PROP);

	switch(msg){
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if(bDrawBasic){
			hbr = CreateSolidBrush(0x808080);
			GetClientRect(hwnd, &rc);
			CreateBasicRects(&rc);
			FillRect(hdc, &rc, hbr);
			DrawBasicRects(hdc);
			DeleteObject(hbr);
		}
		// draw focus rectangle arround the color or remove previous focus rectangle
		DrawFocusRect(hdc, &arrRectBasics[currBasic.x][currBasic.y]);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE)
			ShowWindow(hColors, SW_HIDE);
		else if(wParam == VK_RETURN)
			BasicOnReturn();
		else if(wParam == VK_RIGHT){
			if(currBasic.x < 7)
				DrawBasicFocus(currBasic.x + 1, currBasic.y);
		}
		else if(wParam == VK_LEFT){
			if(currBasic.x > 0)
				DrawBasicFocus(currBasic.x - 1, currBasic.y);
		}
		else if(wParam == VK_DOWN){
			if(currBasic.y < 5)
				DrawBasicFocus(currBasic.x, currBasic.y + 1);
		}
		else if(wParam == VK_UP){
			if(currBasic.y > 0)
				DrawBasicFocus(currBasic.x, currBasic.y - 1);
		}
		else
			if(unicode)
				return CallWindowProcW(hProc, hwnd, msg, wParam, lParam);
			else
				return CallWindowProc(hProc, hwnd, msg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		if(unicode){
			RemovePropW(hwnd, LST_CLR_PROC_PROP_W);
			return CallWindowProcW(hProc, hwnd, msg, wParam, lParam);
		}
		else{
			RemoveProp(hwnd, LST_CLR_PROC_PROP);
			return CallWindowProc(hProc, hwnd, msg, wParam, lParam);
		}
	default:
		if(unicode)
			return CallWindowProcW(hProc, hwnd, msg, wParam, lParam);
		else
			return CallWindowProc(hProc, hwnd, msg, wParam, lParam);
	}
}

static LRESULT CALLBACK LstProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	WNDPROC		hProc;
	BOOL			unicode;

	unicode = IsWindowUnicode(hwnd);
	if(unicode)
		hProc = (WNDPROC)GetPropW(hwnd, LST_CLR_PROC_PROP_W);
	else
		hProc = (WNDPROC)GetProp(hwnd, LST_CLR_PROC_PROP);

	switch(msg){
	case WM_LBUTTONUP:
		ListOnReturn(hwnd, unicode);
		return 0;
	case WM_KEYDOWN:
		if(wParam == VK_RETURN)
			ListOnReturn(hwnd, unicode);
		else if(wParam == VK_ESCAPE)
			ShowWindow(hColors, SW_HIDE);
		else
			if(unicode)
				return CallWindowProcW(hProc, hwnd, msg, wParam, lParam);
			else
				return CallWindowProc(hProc, hwnd, msg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		if(unicode){
			RemovePropW(hwnd, LST_CLR_PROC_PROP_W);
			return CallWindowProcW(hProc, hwnd, msg, wParam, lParam);
		}
		else{
			RemoveProp(hwnd, LST_CLR_PROC_PROP);
			return CallWindowProc(hProc, hwnd, msg, wParam, lParam);
		}
	default:
		if(unicode)
			return CallWindowProcW(hProc, hwnd, msg, wParam, lParam);
		else
			return CallWindowProc(hProc, hwnd, msg, wParam, lParam);
	}
}

static void ListOnReturn(HWND hwnd, BOOL unicode){
	PGLIST_IMAGE		pData;
	int					index;

	if(unicode)
		index = SendMessageW(hwnd, LB_GETCURSEL, 0, 0);
	else
		index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
	// return selected color
	if(index != -1){
		if(unicode)
			pData = (PGLIST_IMAGE)SendMessageW(hwnd, LB_GETITEMDATA, index, 0);
		else
			pData = (PGLIST_IMAGE)SendMessage(hwnd, LB_GETITEMDATA, index, 0);
		savedGC.sysIndex = (hwnd == hSystems) ? pData->reserved : -1;
		savedGC.crValue = pData->dwValue;
		PostMessage(hParent, GCN_COLOR_SELECTED, 0, (LPARAM)&savedGC);
		ShowWindow(hColors, SW_HIDE);
	}
}

static void BasicOnReturn(void){
	// return selected color
	savedGC.sysIndex = -1;
	savedGC.crValue = arrBasics[currBasic.x][currBasic.y];
	PostMessage(hParent, GCN_COLOR_SELECTED, 0, (LPARAM)&savedGC);
	ShowWindow(hColors, SW_HIDE);
}

static void DrawBasicFocus(int x, int y){
	// remove previous focus rectangle arround the basic color and draw new one
	bDrawBasic = FALSE;
	RedrawWindow(hBasics, NULL, NULL, RDW_INVALIDATE);
	currBasic.x = x;
	currBasic.y = y;
	RedrawWindow(hBasics, NULL, NULL, RDW_INVALIDATE);
	bDrawBasic = TRUE;
}


