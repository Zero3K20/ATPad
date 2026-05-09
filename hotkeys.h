#ifndef __HOTKEYS_H__
#define __HOTKEYS_H__
/** Prototypes ********************************************************/
int GetModifiers(wchar_t * lpID);
int GetOtherKey(wchar_t * lpKey);
ACCEL * GetAccString(ACCEL * pAcc, int size, int id, wchar_t * lpAcc);
#endif	// HOTKEYS_H__
