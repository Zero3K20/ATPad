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
#include <mapi.h>
#include <strsafe.h>

static void ReplaceExtention(char * lpSource, char * lpDest);

static ULONG (PASCAL *SendMAPIMail)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);

BOOL SendAsAttachment(HWND hParent, wchar_t * lpAttachment, wchar_t * lpSubject, wchar_t * lpTemp, size_t cchTemp){

	HINSTANCE 		hMAPI;
	char			szFileName[MAX_PATH], szPath[MAX_PATH], szSubject[MAX_PATH], szTempANSI[MAX_PATH];
	MapiFileDesc 	fileDesc;
	MapiMessage 	message;
	int				result;
	BOOL			replaced = FALSE;
	wchar_t			szTempPath[MAX_PATH - 14], szTempFileName[128];

	*lpTemp = '\0';

	hMAPI = LoadLibrary("MAPI32.DLL");
	if(!hMAPI)
		return FALSE;

	(FARPROC)SendMAPIMail = GetProcAddress(hMAPI, "MAPISendMail");
	if(!SendMAPIMail){
		FreeLibrary(hMAPI);
		return FALSE;
	}

	WideCharToMultiByte(CP_ACP, 0, lpAttachment, -1, szFileName, MAX_PATH, "a", &replaced);
	if(replaced){
		GetTempPathW(MAX_PATH - 14, szTempPath);
		GetTempFileNameW(szTempPath, L"tpt", 1, szTempFileName);
		WideCharToMultiByte(CP_ACP, 0, szTempFileName, -1, szTempANSI, MAX_PATH, NULL, NULL);
		ReplaceExtention(szFileName, szTempANSI);
		MultiByteToWideChar(CP_ACP, 0, szTempANSI, -1, szTempFileName, MAX_PATH);
		StringCchCopyA(szFileName, ARRAYSIZE(szFileName), szTempANSI);
		CopyFileW(lpAttachment, szTempFileName, FALSE);
		StringCchCopyW(lpTemp, cchTemp, szTempFileName);
	}
	StringCchCopyA(szPath, ARRAYSIZE(szPath), szFileName);
	if(lpSubject){
		WideCharToMultiByte(CP_ACP, 0, lpSubject, -1, szSubject, MAX_PATH, NULL, NULL);
	}
	else{
		*szSubject = '\0';
	}

	ZeroMemory(&fileDesc, sizeof(fileDesc));
	fileDesc.nPosition = -1;
	fileDesc.lpszPathName = szPath;
	fileDesc.lpszFileName = szFileName;

	ZeroMemory(&message, sizeof(message));
	message.lpszSubject = szSubject;
	message.nFileCount = 1;
	message.lpFiles = &fileDesc;

	result = SendMAPIMail(0, (ULONG_PTR)hParent, &message, MAPI_LOGON_UI | MAPI_DIALOG, 0);

	FreeLibrary(hMAPI);

	if (result != SUCCESS_SUCCESS && result != MAPI_USER_ABORT && result != MAPI_E_LOGIN_FAILURE)
		return FALSE;

	return TRUE;
}

static void ReplaceExtention(char * lpSource, char * lpDest){
	while(*lpSource++)
		;
	while(*lpDest++)
		;
	while(*(--lpSource) != '.')
		;
	while(*(--lpDest) != '.')
		;
	while((*lpDest++ = *lpSource++))
		;
}
