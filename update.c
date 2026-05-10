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

#define WIN32_LEAN_AND_MEAN

// #ifndef _WIN32_WINNT
// #define _WIN32_WINNT 0x0501
// #endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <winbase.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <wininet.h>
#include <ctype.h>
#include "update.h"

DWORD WINAPI CheckForUpdateFunc(LPVOID lpParam){
	HINTERNET 		hINet, hConnection, hData;
	char 			szBuffer[UPDM_VERSION_SIZE], szAddress[UPDM_ADDRESS_SIZE], szFile[UPDM_FILE_SIZE], szVersion[UPDM_VERSION_SIZE], szErrBuffer[4096];
	DWORD 			dwRead, dwErr, dwSize = 4096;
	HWND			hwnd;

	hwnd = (HWND)lpParam;

	SendMessage(hwnd, UPDM_GETSTRINGS, (WPARAM)szAddress, (LPARAM)szFile);
	SendMessage(hwnd, UPDM_GETVERSION, (WPARAM)szVersion, 0);

	hINet = InternetOpen("InetURL/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(!hINet){
		InternetGetLastResponseInfo(&dwErr, szErrBuffer, &dwSize);
		PostMessage(hwnd, UPDM_INETERROR, (WPARAM)szErrBuffer, 0);
		return 1;
	}
	hConnection = InternetConnect(hINet, szAddress, 80, " ", " ", INTERNET_SERVICE_HTTP, 0, 0);
	if(!hConnection){
		InternetCloseHandle(hINet);
		InternetGetLastResponseInfo(&dwErr, szErrBuffer, &dwSize);
		PostMessage(hwnd, UPDM_INETERROR, (WPARAM)szErrBuffer, 0);
		return 1;
	}
	hData = HttpOpenRequest(hConnection, "GET", szFile, NULL, NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION, 0);
	if(!hData){
		InternetCloseHandle(hConnection);
		InternetCloseHandle(hINet);
		InternetGetLastResponseInfo(&dwErr, szErrBuffer, &dwSize);
		PostMessage(hwnd, UPDM_INETERROR, (WPARAM)szErrBuffer, 0);
		return 1;
	}
	HttpSendRequest(hData, NULL, 0, NULL, 0);
	if(InternetReadFile(hData, szBuffer, UPDM_VERSION_SIZE - 1, &dwRead)){
		if(dwRead > 0){
			char	*p1, *p2;
			szBuffer[dwRead] = 0;
			p1 = strchr(szBuffer, '.');
			p2 = strrchr(szBuffer, '.');
			if(p1 && p2 && p1 < p2 && p1 > szBuffer){
				if(isdigit(szBuffer[0]) && isdigit(szBuffer[dwRead - 1])){
					if(strcmp(szVersion, szBuffer) < 0){
						PostMessage(hwnd, UPDM_UPDATEFOUND, (WPARAM)szBuffer, 0);
					}
					else{
						PostMessage(hwnd, UPDM_SAMEVERSION, 0, 0);
					}
				}
				else{
					PostMessage(hwnd, UPDM_INETERROR, (WPARAM)"Error reading file or file does not match.", 0);;
				}
			}
			else{
				PostMessage(hwnd, UPDM_INETERROR, (WPARAM)"Error reading file or file does not match.", 0);;
			}
		}
		else{
			InternetGetLastResponseInfo(&dwErr, szErrBuffer, &dwSize);
			PostMessage(hwnd, UPDM_INETERROR, (WPARAM)szErrBuffer, 0);
		}
	}
	else{
		InternetGetLastResponseInfo(&dwErr, szErrBuffer, &dwSize);
		PostMessage(hwnd, UPDM_INETERROR, (WPARAM)szErrBuffer, 0);
	}
	InternetCloseHandle(hData);
	InternetCloseHandle(hConnection);
	InternetCloseHandle(hINet);
	return 0;
}
