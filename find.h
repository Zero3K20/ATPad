#ifndef __FIND_H__
#define __FIND_H__

BOOL CALLBACK Find_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int SearchText(HWND hEdit);
void AddToHistory(HWND hwnd, int id);
void LoadFromHistory(HWND hwnd, int id);
void CheckForSelection(HWND hwnd, int type);
void FindOnClick(void);

#endif	// FIND_H__
