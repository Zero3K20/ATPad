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

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "hotkeys.h"

typedef struct _KEY_TEXT {
	short key;
	const wchar_t *text;
} KEY_TEXT;

static void AddKeyString(short key, wchar_t * lpKey, size_t cchKey);
static const wchar_t *GetKeyText(short key);

static const KEY_TEXT g_KeyTextMap[] = {
	{ 'A', L"A" }, { 'B', L"B" }, { 'C', L"C" }, { 'D', L"D" }, { 'E', L"E" }, { 'F', L"F" },
	{ 'G', L"G" }, { 'H', L"H" }, { 'I', L"I" }, { 'J', L"J" }, { 'K', L"K" }, { 'L', L"L" },
	{ 'M', L"M" }, { 'N', L"N" }, { 'O', L"O" }, { 'P', L"P" }, { 'Q', L"Q" }, { 'R', L"R" },
	{ 'S', L"S" }, { 'T', L"T" }, { 'U', L"U" }, { 'V', L"V" }, { 'W', L"W" }, { 'X', L"X" },
	{ 'Y', L"Y" }, { 'Z', L"Z" }, { VK_ESCAPE, L"Esc" },
	{ VK_F1, L"F1" }, { VK_F2, L"F2" }, { VK_F3, L"F3" }, { VK_F4, L"F4" }, { VK_F5, L"F5" }, { VK_F6, L"F6" },
	{ VK_F7, L"F7" }, { VK_F8, L"F8" }, { VK_F9, L"F9" }, { VK_F10, L"F10" }, { VK_F11, L"F11" }, { VK_F12, L"F12" },
	{ VK_BACK, L"Backspace" }, { VK_INSERT, L"Ins" }, { VK_HOME, L"Home" }, { VK_PRIOR, L"PgUp" }, { VK_NEXT, L"PgDn" },
	{ VK_END, L"End" }, { VK_DELETE, L"Del" }, { VK_SPACE, L"Space" }, { VK_UP, L"Up" }, { VK_DOWN, L"Down" },
	{ VK_LEFT, L"Left" }, { VK_RIGHT, L"Right" },
	{ '0', L"0" }, { '1', L"1" }, { '2', L"2" }, { '3', L"3" }, { '4', L"4" },
	{ '5', L"5" }, { '6', L"6" }, { '7', L"7" }, { '8', L"8" }, { '9', L"9" },
	{ VK_NUMPAD0, L"Num 0" }, { VK_NUMPAD1, L"Num 1" }, { VK_NUMPAD2, L"Num 2" }, { VK_NUMPAD3, L"Num 3" }, { VK_NUMPAD4, L"Num 4" },
	{ VK_NUMPAD5, L"Num 5" }, { VK_NUMPAD6, L"Num 6" }, { VK_NUMPAD7, L"Num 7" }, { VK_NUMPAD8, L"Num 8" }, { VK_NUMPAD9, L"Num 9" },
	{ VK_PAUSE, L"Pause" }, { VK_ADD, L"+" }, { VK_SUBTRACT, L"-" }, { VK_MULTIPLY, L"*" }, { VK_DIVIDE, L"/" }, { VK_RETURN, L"Enter" }
};

/*-@@+@@------------------------------------------------------------------
 Procedure: GetModifiers
 Created  : Thu Aug  9 18:09:45 2007
 Modified : Thu Aug  9 18:09:45 2007

 Synopsys : Checks whether any of modifier keys (CTRL, ALT, SHIFT, WIN) is
            presssed
 Input    : lpID - TCHAR to return modifiers
 Output   : Combination of modifiers, if any is pressed, otherwise - 0
 Errors   :
 ------------------------------------------------------------------@@-@@-*/
int GetModifiers(wchar_t * lpID, size_t cchID){
	int			result = 0;

	StringCchCopyW(lpID, cchID, L"");
	if((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000){
		result |= FCONTROL;
		StringCchCatW(lpID, cchID, L"Ctrl+");
	}
	if((GetKeyState(VK_MENU) & 0x8000) == 0x8000){
		result |= FALT;
		StringCchCatW(lpID, cchID, L"Alt+");
	}
	if((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000){
		result |= FSHIFT;
		StringCchCatW(lpID, cchID, L"Shift+");
	}
	// if((GetKeyState(VK_LWIN) & 0x8000) == 0x8000 || (GetKeyState(VK_RWIN) & 0x8000) == 0x8000){
		// result |= MOD_WIN;
		// StringCchCatW(lpID, cchID, L"Win+");
	// }
	return result;
}

/*-@@+@@------------------------------------------------------------------
 Procedure: GetOtherKey
 Created  : Thu Aug  9 18:11:20 2007
 Modified : Thu Aug  9 18:11:20 2007

 Synopsys : Returns the first of currently pressed keys (not modifiers)
 Input    : lpKey - TCHAR to return the key string representation
 Output   : Virtual key code or 0
 Errors   :
 ------------------------------------------------------------------@@-@@-*/
int GetOtherKey(wchar_t * lpKey, size_t cchKey){
	StringCchCopyW(lpKey, cchKey, L"");
	for(int i = 0; i < (int)ARRAYSIZE(g_KeyTextMap); i++){
		if((GetKeyState(g_KeyTextMap[i].key) & 0x8000) == 0x8000){
			StringCchCopyW(lpKey, cchKey, g_KeyTextMap[i].text);
			return g_KeyTextMap[i].key;
		}
	}
	return FALSE;
}

ACCEL * GetAccString(ACCEL * pAcc, int size, int id, wchar_t * lpAcc, size_t cchAcc){
	*lpAcc = '\0';
	for(int i = 0; i < size; i++){
		if(pAcc[i].cmd == id){
			StringCchCopyW(lpAcc, cchAcc, L"\t");
			if((pAcc[i].fVirt & FCONTROL) == FCONTROL)
				StringCchCatW(lpAcc, cchAcc, L"Ctrl+");
			if((pAcc[i].fVirt & FALT) == FALT)
				StringCchCatW(lpAcc, cchAcc, L"Alt+");
			if((pAcc[i].fVirt & FSHIFT) == FSHIFT)
				StringCchCatW(lpAcc, cchAcc, L"Shift+");
			AddKeyString(pAcc[i].key, lpAcc, cchAcc);
			return &pAcc[i];
		}
	}
	return NULL;
}

static void AddKeyString(short key, wchar_t * lpKey, size_t cchKey){
	const wchar_t	*keyText;

	keyText = GetKeyText(key);
	if(keyText){
		StringCchCatW(lpKey, cchKey, keyText);
	}
}

static const wchar_t *GetKeyText(short key){
	for(int i = 0; i < (int)ARRAYSIZE(g_KeyTextMap); i++){
		if(g_KeyTextMap[i].key == key){
			return g_KeyTextMap[i].text;
		}
	}
	return NULL;
}
