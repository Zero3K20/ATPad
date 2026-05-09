#ifndef __GCPICKER_H__
#define __GCPICKER_H__

#include <gcolors.h>

HWND CreateGCPickerWindowW(int x, int y, int cx, int cy, HWND hParentWnd, int id, PGCOLORTYPE pgcType);
HWND CreateGCPickerWindow(int x, int y, int cx, int cy, HWND hParentWnd, int id, PGCOLORTYPE pgcType);

#endif	// GCPICKER_H__
