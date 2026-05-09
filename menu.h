#ifndef __MENU_H__
#define __MENU_H__

#define	MT_REGULARITEM		0
#define	MT_COLORITEM		1

typedef struct {
	int			id;
	int			xPos;
	int			yPos;
	int			xCheck;
	int			yCheck;
	int			type;
	BOOL		firstLevel;
	wchar_t		szText[128];
	wchar_t		szReserved[MAX_PATH];
}MItem, *PMItem;

void PrepareMenuGradientColors(COLORREF color);
void SetMenuOwnerDraw(HMENU hMenu);
void FreeSingleMenu(HMENU hMenu, int pos);
void FreeMenus(HMENU hMenu);
void SetMenuItemProperties(PMItem pmi, HMENU hMenu, int pos);
void MeasureMItem(HFONT hFont, MEASUREITEMSTRUCT * lpMI);
void DrawMItem(const DRAWITEMSTRUCT * lpDI, HBITMAP hBmp, HBITMAP hBmpGray, COLORREF crMask, BOOL vsEnabled);
// void CreateMenuColors(int winVer);

#endif	// MENU_H__
