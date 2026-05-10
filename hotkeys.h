#ifndef __HOTKEYS_H__
#define __HOTKEYS_H__

#include <stddef.h>

/** Prototypes ********************************************************/
int GetModifiers(wchar_t * lpID, size_t cchID);
int GetOtherKey(wchar_t * lpKey, size_t cchKey);
ACCEL * GetAccString(ACCEL * pAcc, int size, int id, wchar_t * lpAcc, size_t cchAcc);
#endif	// HOTKEYS_H__
