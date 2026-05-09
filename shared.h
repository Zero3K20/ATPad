#ifndef __SHARED_H__
#define __SHARED_H__

#include <commctrl.h>
#include "menu.h"

// #include "debug.h"
void BitOff(int * data, int position);
void BitOn(int * data, int position);
BOOL IsBitOn(int data, int position);
void ShowPopUp(HWND hwnd, HMENU hMenu);
void SetMenuText(int id, const wchar_t * lcpSection, const wchar_t * lcpFile, wchar_t * lpDefault, wchar_t * lpBuffer);
void SetMIText(MItem * lpMI, const int size, int id, const wchar_t * lpText);
void GetMIText(MItem * lpMI, const int size, int id, wchar_t * lpText);
void GetTooltip(wchar_t * lpTip, PMItem pItems, int size, UINT id);
void SetControlText(HWND hwnd, int id, wchar_t * lpDefault, wchar_t * lpLangFile);
void DrawComboItem(const DRAWITEMSTRUCT * lpDrawItem);
void AddWrapBitmap(BOOL fReplace, GCOLORTYPE gct, HIMAGELIST iml);
void DrawLineNumbersRegular(HWND hwnd, HDC hdc, TPSETTINGS settings, GCOLORTYPE ctBM, TPFORMAT lnformat, HFONT linesFont, int textHeight, int lineTextHeight, P_TPEDIT pE);
void DrawLineNumbersWrap(HWND hwnd, HDC hdc, TPSETTINGS settings, GCOLORTYPE ctBM, TPFORMAT lnformat, HFONT linesFont, int textHeight, int lineTextHeight, P_TPEDIT pE, HIMAGELIST hIml);
void DrawCRLFWhiteSpace(HWND hwnd, BOOL fPreview, TPSETTINGS settings, GCOLORTYPE wscolor, GCOLORTYPE spacecolor, int textHeight, HBITMAP bCR, HBITMAP bLF);
void GetTextHeight(HWND hwnd, int * pheight, HFONT font);
void GetLineTextHeight(HWND hwnd, int * pheight, HFONT font);
void CreateCRLF(HWND hwnd, HBITMAP * hCR, HBITMAP * hLF, GCOLORTYPE cBack, TPFORMAT tBack);
void StartUpdateProcess(HWND hwnd);
void GetCurrentPositionRegular(HWND hEdit, CHARRANGE chrg, long * px, long * py);
void GetCurrentPositionWrap(HWND hEdit, CHARRANGE chrg, long * px, long * py);
// int ReplaceEditText(HWND hEdit, int params);
PMItem GetMItem(PMItem	lpMI, const int size, int id);

#endif	// SHARED_H__
