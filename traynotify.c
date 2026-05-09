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
// #include <windowsx.h>
// #include <commctrl.h>
// #include <tchar.h>
// #include <stdlib.h>
#include <shellapi.h>
#include <shlwapi.h>
// #include <richedit.h>
// #include <commdlg.h>
// #include <dlgs.h>
#include "traynotify.h"

static BOOL IsNewDLLVersion(char * szLib);

static BOOL IsNewDLLVersion(char * szLib){
	DLLVERSIONINFO		dvi;
	HANDLE				hLib;
	FARPROC				hProc;
	BOOL				bReturn = FALSE;

	dvi.cbSize = sizeof(dvi);
	hLib = LoadLibrary(szLib);
	if(hLib){
		hProc = GetProcAddress(hLib, "DllGetVersion");
		if(hProc){
			__asm{
				lea eax, dvi
				push eax
				call hProc
			}
			if(dvi.dwMajorVersion > 4)
				bReturn = TRUE;
			else
				bReturn = FALSE;
		}
		else{
			bReturn = FALSE;
		}
		FreeLibrary(hLib);
	}
	return bReturn;
}

void AddNotifyIcon(HWND hwnd, HINSTANCE hInstance, int iconID, NOTIFYICONDATAW * lpnid, wchar_t * lpTooltip){
	if(IsNewDLLVersion("shell32.dll"))
		lpnid->cbSize = sizeof(NOTIFYICONDATAW);
	else
		lpnid->cbSize = NOTIFYICONDATAW_V1_SIZE;

	lpnid->hWnd = hwnd;
	lpnid->uID = IDI_TRAY;
	lpnid->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	lpnid->uCallbackMessage = WM_SHELLNOTIFY;
	lpnid->hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(iconID));
	wcscpy(lpnid->szTip, lpTooltip);
}
