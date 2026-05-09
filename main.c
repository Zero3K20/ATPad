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

#ifndef TOOL_BAR_CUSTOM_DRAW
#define	TOOL_BAR_CUSTOM_DRAW
#endif

#define WIN32_LEAN_AND_MEAN  /* speed up compilations */
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdlib.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <richedit.h>
#include <commdlg.h>
#include <dlgs.h>
#include <objbase.h>
#include <Iphlpapi.h>
#include <stdio.h>
#include <wctype.h>

#include "main.h"
#include "toolbar.h"
#include "menu.h"
#include "stringconstants.h"
#include "globalvars.h"
#include "settings.h"
#include "numericconstants.h"
#include "gradients.h"
#include "registryclean.h"
#include "traynotify.h"
#include "find.h"
#include "goto.h"
#include "print.h"
#include "shared.h"
#include "about.h"
#include "send.h"
#include "update.h"
#include "horsplit.h"
#include "hotkeys.h"
#include "hotdlg.h"

#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))
#define INRANGE(a, b, c)	((a >= b && a <= c) ? ((c > b) ? (0) : (2)) : ((a < b) ? -1 : 1))

/** Prototypes **************************************************************/

static LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
static void Main_OnCommand(HWND, int, HWND, UINT);
static void Main_OnDestroy(HWND);
static BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void Main_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void Main_OnClose(HWND hwnd);
static void Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
static void Main_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
static void Main_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
static void Main_OnDropFiles(HWND hwnd, HDROP hdrop);
static void Main_OnSysColorChange(HWND hwnd);
static void Main_OnMove(HWND hwnd, int x, int y);
static void Main_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu);
static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static int WinVer(void);
static void GetSubPath(wchar_t * lpResult, const wchar_t * lpFile);
static int ParseTPCommandLine(wchar_t * pINIPath, wchar_t * pDataPath, wchar_t * pProgPath, wchar_t * pLoadPath);
static BOOL IsLastBackslash(wchar_t * src);
static void PrepareMenu(HWND hwnd);
static void CreateMenuFont(void);
static void PrepareMenuRecursive(HMENU hMenu, wchar_t * lpLangFile);
static P_TPEDIT AddTab(wchar_t * lpText, wchar_t * lpFile);
static void CreateImageList(HIMAGELIST *lpIml, int idBmp, COLORREF crMask, int x, int y, int imgCount);
static void GetStrings(wchar_t * lpLangFile);
static void GetSettings(void);
static void Cleanup(HWND hwnd);
static void ResizeAllEdits(void);
static void CreateEdit(P_TPEDIT pE);
static void CheckChangesInSettings(void);
static BOOL FontChanged(LPLOGFONTW lf1, LPLOGFONTW lf2);
static void ApplyNewFont(LPLOGFONTW lf, BOOL fForText);
static void ApplyNewTabStops(void);
static void ApplyNewBackColor(COLORREF clr);
static void ApplyNewTextColor(COLORREF clr);
static HWND GetActiveEdit(void);
static HWND GetActiveStatic(void);
static P_TPEDIT GetActiveHandle(void);
static void ApplyNewShowLineNumbers(void);
static void ApplyNewLeftMargin(void);
static void ApplyNewWordWrap(void);
static void CheckViewItems(void);
static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
static void ShowCurrentPositionRegular(HWND hEdit, CHARRANGE chrg);
static void ShowCurrentPositionWrap(HWND hEdit, CHARRANGE chrg);
static void DrawTabItem(const DRAWITEMSTRUCT * lpDrawItem);
static int GetMenuPosition(HMENU hMenu, int id);
static void AddWindowMenu(HMENU hMenu, int index, wchar_t * lpText, BOOL fCheck);
static void ClearWindowMenu(void);
static void RecreateWindowMenu(void);
static void CheckWindowMenu(HMENU hMenu, int id);
static BOOL OSFileDialog(wchar_t * lpCaption, wchar_t * lpNameFull, wchar_t * lpNameShort, BOOL fOpen, BOOL fUntitled);
static void GetShortName(int index, wchar_t * lpName, BOOL fWithStar);
static void GetLongName(int index, wchar_t * lpName);
static void CloseTab(P_TPEDIT pE);
static DWORD CALLBACK ReadStreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb);
static DWORD CALLBACK WriteStreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb);
static BOOL LoadFile(wchar_t * lpPath, P_TPEDIT pE);
static LRESULT CALLBACK OFNParentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
static BOOL SaveFile(wchar_t * lpLongName, P_TPEDIT pE);
static void GetFileNameFromPath(wchar_t * lpPath, wchar_t * lpName);
static void SaveAll(void);
static BOOL SaveAllOnExit(BOOL fCloseTab);
static void PrintCurrentPosition(int x, int y);
static void ResizeStatic(P_TPEDIT pE, int cy);
static void BuildLanguageMenu(void);
static void InsertLanguageMenu(HMENU hMenu, WIN32_FIND_DATAW * lpfd, int lindex);
static BOOL IsLangID(int id);
static void GetNewLanguageFile(int id);
static void ResetUntitledString(void);
static void StoreBookmarks(wchar_t * lpSection, P_TPEDIT pE);
static void LoadRecentFiles(void);
static void AddToRecentFiles(P_TPEDIT pE, BOOL fAddToMenu);
static BOOL IsRecentInList(wchar_t * lpPath);
static void GetRecentPath(int id, wchar_t * lpPath);
static int ShiftRecentFilesUp(void);
static P_TPEDIT LoadDocument(HWND hMain, wchar_t * lpNameFull, wchar_t * lpNameShort, P_TPEDIT pE, BOOL fAddTab);
static void CheckForOuterChanges(void);
static void GetProgramSettings(void);
static BOOL LoadInSameTab(wchar_t * lpNameFull, wchar_t * lpNameShort);
static BOOL IsVisualStyleEnabled(void);
static BOOL IsDLLVersionForStyle(void);
static void CloseApplication(int mode);
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
static P_TPEDIT GetHandleByTab(HWND hTab, int index);
static void ClearRecentFiles(void);
static void LoadLastSession(HWND hMain);
static void RemoveFromLastSession(wchar_t * lpPath);
static void AddToLastSession(wchar_t * lpPath, P_TPEDIT pE);
static void RecreateLastSession(void);
static void GetTempSaveName(wchar_t * lpLongName, wchar_t * lpTempSave, BOOL fGetExtension);
static HICON GetSmallIcon(int index);
static void GetAppSmallIcon(void);
static void SetSmallIcon(P_TPEDIT pE, wchar_t * lpPath);
static BOOL IsFileDropped(wchar_t * lpPath);
static void CreateRebarBackground(HWND hwnd);
static void DrawToolbarButton(HDC hdc, LPRECT lprc, int state, int id);
static LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Child_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void Child_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void Child_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate);
static void Child_OnDestroy(HWND hwnd);
static void Child_OnClose(HWND hwnd);
static void Child_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static HWND CreateNewChild(wchar_t * lpNameFull, wchar_t * lpNameShort, P_TPEDIT pE);
static P_TPEDIT GetHandleByChild(HWND hChild);
static P_TPEDIT GetHandleByTabIndex(int tabIndex);
static void ApplyUntitledCaptions(void);
static BOOL AskOnClose(P_TPEDIT pE);
static VOID CALLBACK RefreshTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
static void WriteLastPosition(HWND hwnd);
static TPPOSITION ReadLastPosition(void);
static BOOL TextWithStar(HWND hwnd);
static BOOL EndsWith(wchar_t * lpString, wchar_t * lpChar);
static void ChangeChildText(HWND hChild, BOOL fStar);
static void CleanBookmarks(P_TPEDIT pE);
static void ToggleBookmark(P_TPEDIT pE);
static void GoToBookmark(P_TPEDIT pE, BOOL fNext);
static void RemoveBookmark(P_TPEDIT pE, long line);
static void SetBookmarkInRange(P_TPEDIT pE);
static void RecheckBookmarks(P_TPEDIT pE);
static void LoadDateTimeMasks(wchar_t * lpLangFile);
static void GetVersionNumber(void);
static void ShowNewVersionBaloon(wchar_t * szNewVersion);
static long GetRealLine(P_TPEDIT pE);
static P_TPBMRK AddRecentBookmark(P_TPEDIT pE, long line, P_TPBMRK prev);
static void LoadRecentBookmarks(wchar_t * lpSection, P_TPEDIT pE);
static void SetTrayTip(void);
static void GetMACAddress(void);
static void ClearLastSession(void);
static void ClearRecents(void);
static BOOL InOtherIndices(int number);
static BOOL InOtherLastIndices(int number);
static void LoadOthersLastSession(void);
static int __cdecl Compare(const void * i1, const void * i2);
static int __cdecl AccCompare(const void * i1, const void * i2);
static wchar_t * GetATFileType(P_TPEDIT pE);
static void ConvertCase(HWND hEdit, BOOL fToUpper);
static HMENU ClearRefreshMenu(void);
static void BuildRefreshMenu(HMENU hMenu);
static void AddBookmarksToRecentLast(wchar_t * lpSection, wchar_t * lpKeyFixed, wchar_t * lpKeyRemovable, wchar_t * lpValue, wchar_t * lpPath);
static void AddValueToRecentLast(wchar_t * lpSection, wchar_t * lpKey, wchar_t * lpValueFixed, wchar_t * lpValueRemovable, wchar_t * lpPath);
static void ConstructINIKey(wchar_t * lpPath, wchar_t * lpLongName);
static void GetLanguageID(void);
static BOOL IsAccPrepared(void);
static void CopyDefAccelerators(void);
static void CopyDefAcceleratorsToTemp(void);
static void CopyChangedAccelerators(void);
static BOOL ChangedAccelerators(void);
static void LoadATPAccelerators(void);
static void WriteAccelerators(void);
static void CopyMenusArray(void);
static void ReMaxChild(void);
static BOOL CALLBACK DTFormat_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void DTFormat_OnClose(HWND hwnd);
static void DTFormat_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL DTFormat_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void SetShowDateTime(HWND hwnd);
static void InsertDateTime(int type);

typedef struct _TPREAD {
	HANDLE			hHandle;
	int				type;
}TPREAD, * P_TPREAD;

/** Global variables ********************************************************/
static P_TPEDIT			m_TempPE;
static BYTE				m_BOMUnicode[] = {0xff, 0xfe};
static BYTE				m_BOMBE[] = {0xfe, 0xff};
static BYTE				m_BOMUTF8[] = {0xef, 0xbb, 0xbf};
static int				m_WinVer;
static int				m_RecentFiles = 0, m_OtherRecentFiles = 0, * m_Langs, m_CountLangs;
static int				m_RecentIndices[RECENT_FILES_MAX], *m_OtherIndices, *m_LastOtherIndices, m_LastOtherFiles;
static HANDLE			m_Mutex;
static HWND				m_hTbrMain, m_hTabMain, m_hStatusMain, m_hTTToolbar, m_hTTTab, m_hRebar;
static HWND				m_hSelCombo;
static HIMAGELIST		m_ImlNormal, m_ImlGray;
static int				m_idTimer = 0, m_CurrType;
static BOOL				m_CheckOuter = FALSE, m_InTray = FALSE, m_RemovableDrive, m_LanguageSet = TRUE;
static HMENU			m_hPopup = NULL;
static NOTIFYICONDATAW	m_nData;
static HICON			m_hSmallIcon;
static HBITMAP			m_hRBack;
static int				m_WinState = 0;
static SIZE				m_TempSize;
static POINT			m_TempPoint;
static char				m_Version[12];
static wchar_t			m_MACAddress[32] = L"00-00-00-00-00-00";
static wchar_t			m_DriveLetter[1];
static int				m_ShowPosition = 1;
static ACCEL			*m_pAcc;
static int				m_AccCount;
static HACCEL			m_hAcc;
static PMItem			m_pMenus;
//temporary
static COLORREF			m_clrHot = RGB(255, 230, 181), m_clrSel = RGB(255, 174, 106), m_clrFrame = RGB(75, 75, 111);

static MItem			m_MainMenus[] = {
						{IDM_FILE, -1, 0, -1, -1, MT_REGULARITEM, TRUE, L"File", L""},
						{IDM_EDIT, -1, 0, -1, -1, MT_REGULARITEM, TRUE, L"Edit", L""},
						{IDM_VIEW, -1, 0, -1, -1, MT_REGULARITEM, TRUE, L"View", L""},
						{IDM_OPTIONS, -1, 0, -1, -1, MT_REGULARITEM, TRUE, L"Options", L""},
						{IDM_WINDOW, -1, 0, -1, -1, MT_REGULARITEM, TRUE, L"Window", L""},
						{IDM_HELP_UP, -1, 0, -1, -1, MT_REGULARITEM, TRUE, L"Help", L""},
						{IDM_NEW, 0, 0, -1, -1, MT_REGULARITEM, FALSE, L"New", L""}, 
						{IDM_OPEN, 11, 0, -1, -1, MT_REGULARITEM, FALSE, L"Open", L""}, 
						{IDM_SAVE, 1, 0, -1, -1, MT_REGULARITEM, FALSE, L"Save", L""},
						{IDM_SAVE_AS, 2, 0, -1, -1, MT_REGULARITEM, FALSE, L"Save As", L""},
						{IDM_SAVE_ALL, 3, 0, -1, -1, MT_REGULARITEM, FALSE, L"Save All", L""},
						{IDM_RECENT, -1, 0, -1, -1, MT_REGULARITEM, FALSE, L"Recent Files", L""},
						{IDM_REFRESH, 20, 0, -1, -1, MT_REGULARITEM, FALSE, L"Refresh", L""},
						{IDM_PAGE_SETUP, 27, 0, -1, -1, MT_REGULARITEM, FALSE, L"Page Setup", L""},
						{IDM_PRINT, 12, 0, -1, -1, MT_REGULARITEM, FALSE, L"Print", L""},
						{IDM_UNDO, 18, 0, -1, -1, MT_REGULARITEM, FALSE, L"Undo", L""}, 
						{IDM_REDO, 19, 0, -1, -1, MT_REGULARITEM, FALSE, L"Redo", L""},
						{IDM_CUT, 4, 0, -1, -1, MT_REGULARITEM, FALSE, L"Cut", L""}, 
						{IDM_COPY, 5, 0, -1, -1, MT_REGULARITEM, FALSE, L"Copy", L""}, 
						{IDM_PASTE, 6, -1, -1, -1, MT_REGULARITEM, FALSE, L"Paste", L""},
						{IDM_FIND, 13, 0, -1, -1, MT_REGULARITEM, FALSE, L"Find", L""}, 
						{IDM_FIND_NEXT, 14, 0, -1, -1, MT_REGULARITEM, FALSE, L"Find Next", L""},
						{IDM_REPLACE, 15, 0, -1, -1, MT_REGULARITEM, FALSE, L"Replace", L""}, 
						{IDM_GOTO, 17, 0, -1, -1, MT_REGULARITEM, FALSE, L"Go To", L""}, 
						{IDM_INSERT, 42, 0, -1, -1, MT_REGULARITEM, FALSE, L"Insert", L""}, 
						{IDM_INS_DATE_TIME, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Date And Time", L""}, 
						{IDM_INS_DATE, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Date", L""}, 
						{IDM_INS_TIME, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Time", L""}, 
						{IDM_UNDO_ALL, 37, 0, -1, -1, MT_REGULARITEM, FALSE, L"Undo All", L""}, 
						{IDM_UNDO_ALL_FILES, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"All Open Files", L""},
						{IDM_UNDO_CURR_FILE, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Current File", L""},
						{IDM_REDO_ALL, 39, 0, -1, -1, MT_REGULARITEM, FALSE, L"Redo All", L""}, 
						{IDM_REDO_ALL_FILES, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"All Open Files", L""},
						{IDM_REDO_CURR_FILE, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Current File", L""},
						{IDM_F_R_FILES, -1, -1, 40, 0, MT_REGULARITEM, FALSE, L"Find/Replace Results", L""},
						{IDM_SNIPPETS, -1, -1, 40, 0, MT_REGULARITEM, FALSE, L"Snippets", L""},
						{IDM_HOT_KEYS, 38, 0, -1, -1, MT_REGULARITEM, FALSE, L"Hot Keys", L""}, 
						{IDM_DF_SETTINGS, 41, 0, -1, -1, MT_REGULARITEM, FALSE, L"Date/Time Settings", L""}, 
						{IDM_SELECT_ALL, 16, -1, -1, -1, MT_REGULARITEM, FALSE, L"Select All", L""},
						{IDM_SETTINGS, 7, 0, -1, -1, MT_REGULARITEM, FALSE, L"Settings", L""}, 
						{IDM_LANG, 10, 0, -1, -1, MT_REGULARITEM, FALSE, L"Language", L""},
						{IDM_HELP, 8, 0, -1, -1, MT_REGULARITEM, FALSE, L"Help", L""}, 
						{IDM_ABOUT, 9, 0, -1, -1, MT_REGULARITEM, FALSE, L"About", L""}, 
						{IDM_EMPTY, 21, 0, -1, -1, MT_REGULARITEM, FALSE, L"(Empty)", L""}, 
						{IDM_SEND_ATTACH, 22, 0, -1, -1, MT_REGULARITEM, FALSE, L"Send As Attachment", L""},
						{IDM_CLOSE_ALL, 23, 0, -1, -1, MT_REGULARITEM, FALSE, L"Close All", L""},
						{IDM_PROG_OPEN, 32, 0, -1, -1, MT_REGULARITEM, FALSE, L"Open ATPad", L""}, 
						{IDM_EXIT_TRAY, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Exit", L""},
						{IDM_CASCADE, 26, 0, -1, -1, MT_REGULARITEM, FALSE, L"Cascade", L""},
						{IDM_TILE_HOR, 24, 0, -1, -1, MT_REGULARITEM, FALSE, L"Tile horizontally", L""},
						{IDM_TILE_VERT, 25, 0, -1, -1, MT_REGULARITEM, FALSE, L"Tile vertically", L""},
						{IDM_BM_TOGGLE, 28, 0, -1, -1, MT_REGULARITEM, FALSE, L"Toggle bookmark", L""},
						{IDM_BM_NEXT, 29, 0, -1, -1, MT_REGULARITEM, FALSE, L"Next bookmark", L""},
						{IDM_BM_PREV, 30, 0, -1, -1, MT_REGULARITEM, FALSE, L"Previous bookmark", L""},
						{IDM_BM_REMOVE, 31, 0, -1, -1, MT_REGULARITEM, FALSE, L"Remove all bookmarks", L""},
						{IDM_TO_UPPER, 33, 0, -1, -1, MT_REGULARITEM, FALSE, L"Convert To Uppercase", L""},
						{IDM_TO_LOWER, 34, 0, -1, -1, MT_REGULARITEM, FALSE, L"Convert To Lowercase", L""},
						{IDM_REFRESH_EVERY, 35, 0, -1, -1, MT_REGULARITEM, FALSE, L"Refresh Every:", L""},
						{IDM_EXIT, -1, -1, -1, -1, MT_REGULARITEM, FALSE, L"Exit", L""}};

static TBBUTTON		 	m_TBBtnArray[] = {{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{0,IDM_NEW,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{11,IDM_OPEN,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{1,IDM_SAVE,0,TBSTYLE_BUTTON,0,0,0,0},
						{2,IDM_SAVE_AS,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{3,IDM_SAVE_ALL,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{4,IDM_CUT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{5,IDM_COPY,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{6,IDM_PASTE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{18,IDM_UNDO,0,TBSTYLE_BUTTON,0,0,0,0},
						{19,IDM_REDO,0,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{28,IDM_BM_TOGGLE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{29,IDM_BM_NEXT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{30,IDM_BM_PREV,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{31,IDM_BM_REMOVE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{22,IDM_SEND_ATTACH,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{23,IDM_CLOSE_ALL,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{20,IDM_REFRESH,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{27,IDM_PAGE_SETUP,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{12,IDM_PRINT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{7,IDM_SETTINGS,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
						{0,0,TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0},
						{8,IDM_HELP,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0}
						};
static ACCEL			m_AccDef[] = {{FCONTROL | FVIRTKEY,VK_S,IDM_SAVE},
						{FCONTROL | FVIRTKEY,VK_N,IDM_NEW},
						{FCONTROL | FVIRTKEY,VK_O,IDM_OPEN},
						{FCONTROL | FVIRTKEY,VK_P,IDM_PRINT},
						{FALT | FVIRTKEY,VK_F4,IDM_EXIT},
						{FCONTROL | FVIRTKEY,VK_Z,IDM_UNDO},
						{FCONTROL | FVIRTKEY,VK_Y,IDM_REDO},
						{FCONTROL | FVIRTKEY,VK_X,IDM_CUT},
						{FCONTROL | FVIRTKEY,VK_C,IDM_COPY},
						{FCONTROL | FVIRTKEY,VK_V,IDM_PASTE},
						{FCONTROL | FVIRTKEY,VK_A,IDM_SELECT_ALL},
						{FCONTROL | FVIRTKEY,VK_F,IDM_FIND},
						{FCONTROL | FVIRTKEY,VK_H,IDM_REPLACE},
						{FVIRTKEY,VK_F3,IDM_FIND_NEXT},
						{FCONTROL | FVIRTKEY,VK_G,IDM_GOTO},
						{FCONTROL | FSHIFT | FVIRTKEY,VK_A,IDM_SAVE_AS},
						{FCONTROL | FSHIFT | FVIRTKEY,VK_S,IDM_SAVE_ALL},
						{FSHIFT | FVIRTKEY,VK_F4,IDM_BM_TOGGLE},
						{FVIRTKEY,VK_F4,IDM_BM_NEXT},
						{FCONTROL | FSHIFT | FVIRTKEY,VK_U,IDM_TO_UPPER},
						{FCONTROL | FSHIFT | FVIRTKEY,VK_L,IDM_TO_LOWER},
						{FVIRTKEY,VK_F5,IDM_REFRESH}
						};
static wchar_t			*m_RefDef[] = {L"Turn off", L"15 sec", L"30 sec", L"1 min", L"3 min", L"5 min", L"10 min", L"15 min", L"20 min", L"30 min"};
static int				m_RefFreq[] = {0, 15, 30, 60, 180, 300, 600, 900, 1200, 1800};

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX 	icc;
    WNDCLASSEXW	 			wc;
    MSG 					msg;
	int						args, result;
	wchar_t					szINIPath[MAX_PATH], szDataPath[MAX_PATH], szProgPath[MAX_PATH], szLoadPath[MAX_PATH];
	TPPOSITION				tpp;
	RECT					rcsb;

	//parse command line arguments and make decision
	args = ParseTPCommandLine(szINIPath, szDataPath, szProgPath, szLoadPath);
	//create mutex for further checking
	m_Mutex = CreateMutexW(NULL, TRUE, PROG_MUTEX);
	result = GetLastError();

	*g_Paths.sINI = '\0';
	if((args & ARG_INI_PATH) == ARG_INI_PATH){
		//INI file path has been set via command line
		wcscpy(g_Paths.sINI, szINIPath);
		if(!_wcsistr(g_Paths.sINI, INI_FILE)){
			if(IsLastBackslash(g_Paths.sINI))
				wcscat(g_Paths.sINI, INI_FILE);
			else
				wcscat(g_Paths.sINI, INI_FILE_SUBPATH);
		}
	}
	else{
		//get path to INI file in program directory
		GetSubPath(g_Paths.sINI, INI_FILE_SUBPATH);
	}

	//get snippets file path
	GetSubPath(g_Paths.sSnippetsPath, SNP_FILE_SUBPATH);

	//get general program settings
	GetProgramSettings();

	if(result == ERROR_ALREADY_EXISTS){
		//running second instance of program
		if((args & ARG_EXIT) == ARG_EXIT){
			//program exit required by command line arguments
			if((args & ARG_SILENT) == ARG_SILENT){
				//save all documents and exit
				CloseApplication(ARG_SILENT);
			}
			else if((args & ARG_NO_SAVE) == ARG_NO_SAVE){
				//exit without saving
				CloseApplication(ARG_NO_SAVE);
			}
			else{
				//exit with standard saving prompt
				CloseApplication(ARG_EXIT);
			}
			return 0;
		}
		if(g_Settings.singleInstance){
			HWND		hwnd = NULL;
			//find previous instance window
			EnumWindows(EnumWindowsProc, (LPARAM)&hwnd);
			if(hwnd){
				//something has been dragged onto program icon
				if(args == ARG_LOAD_ON_START){
					//open dragged document in previous instance
					int		atom = GlobalAddAtomW(szLoadPath);
					PostMessageW(hwnd, TBNPM_OPENFROMCLINE, 0, (LPARAM)atom);
				}
				else{
					//restore window
					if((GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_MINIMIZE) == WS_MINIMIZE){
						SendMessageW(hwnd, WM_SHELLNOTIFY, IDI_TRAY, WM_LBUTTONDBLCLK);
					}
				}
			}
			//exit
			return 0;
		}
	}

	if((args & ARG_PROG_PATH) == ARG_PROG_PATH){
		//full program path has been sent via command line (may be 3-d party launcher) - needed for shortcuts creation
		wcscpy(g_Paths.sProgFullPath, szProgPath);
	}
	else{
		//get real full program path
		GetModuleFileNameW(NULL, g_Paths.sProgFullPath, MAX_PATH);
	}

    g_hInstance = hInstance;

    /* Initialize common controls. Also needed for MANIFEST's */
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES /*|ICC_COOL_CLASSES|ICC_DATE_CLASSES|ICC_PAGESCROLLER_CLASS|ICC_USEREX_CLASSES*/;
    InitCommonControlsEx(&icc);

    LoadLibraryW(L"riched20.dll");  // Rich Edit v2.0, v3.0

	//initialize COM
	CoInitialize(NULL);
	//get and store application small icon
	GetAppSmallIcon();
	//get process heap
	g_hHeap = GetProcessHeap();

    //register the main window class
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
    wc.lpszClassName = PROG_CLASS;
    wc.lpfnWndProc = MainWndProc;
    wc.style = CS_VREDRAW|CS_HREDRAW;
    wc.hInstance = g_hInstance;
    wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
	wc.hIconSm = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MNU_MAIN);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    if (!RegisterClassExW(&wc)){
        return 1;
	}
	//register MDI child window class
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = ChildWndProc;
	//no background - reduce flickering
	wc.hbrBackground = NULL;
	wc.lpszClassName = MDI_CHILD_CLASS;
	if (!RegisterClassExW(&wc)){
        return 1;
	}
	
	//copy menus array in order to preserve default strings
	CopyMenusArray();
	
	//load possible accelerators
	if(IsAccPrepared()){
		LoadATPAccelerators();
	}
	else{
		CopyDefAccelerators();
		WriteAccelerators();
	}

	//get last saved position
	tpp = ReadLastPosition();
    //create the main window
    g_hMain = CreateWindowExW(WS_EX_ACCEPTFILES, PROG_CLASS,
        PROGRAM_NAME,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        (int)tpp.left,
        (int)tpp.top,
        tpp.width,
        tpp.height,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );
	
    if (!g_hMain) return 1;
	
	//set close parameter
    SetPropW(g_hMain, P_SAVE_ON_EXIT, (HANDLE)1);
	//show main window
    ShowWindow(g_hMain, nCmdShow);
	//if saved position was maximized - maximize window
	if(tpp.maximized){
		SendMessageW(g_hMain, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
    UpdateWindow(g_hMain);

	//load accelerators
	m_hAcc = CreateAcceleratorTableW(m_pAcc, m_AccCount);
	// m_hAcc = LoadAcceleratorsW(g_hInstance, MAKEINTRESOURCEW(IDT_ACC));

	//create splitter window
	GetClientRect(m_hStatusMain, &rcsb);
	CreateHSplit(g_hMain, rcsb.bottom - rcsb.top, 160);

	//show assist panel windows
	if(IsBitOn(g_Settings.res1, SB_SHOW_FR))
		SendMessageW(g_hMain, WM_COMMAND, MAKEWPARAM(IDM_F_R_FILES, 0), 0);
	if(IsBitOn(g_Settings.res1, SB_SHOW_SNIPPETS))
		SendMessageW(g_hMain, WM_COMMAND, MAKEWPARAM(IDM_SNIPPETS, 0), 0);
	
	if(args == ARG_LOAD_ON_START){
		//load document if needed
		wchar_t		szPathShort[MAX_PATH];
		P_TPEDIT 	pE = GetActiveHandle();
		GetFileNameFromPath(szLoadPath, szPathShort);
		wcscpy(pE->szLongName, szLoadPath);
		wcscpy(pE->szShortName, szPathShort);
		LoadDocument(g_hMain, szLoadPath, szPathShort, pE, FALSE);
		RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
		GetAppSmallIcon();
	}

	//create notify icon
	AddNotifyIcon(g_hMain, g_hInstance, IDR_ICO_MAIN, &m_nData, PROGRAM_NAME);

	//check for new version is there is so setting
	if(g_Settings.checkNVOnStart){
		g_CheckingFromButton = FALSE;
		StartUpdateProcess(g_hMain);
	}

    //pump messages until we are done
    while (GetMessageW(&msg, NULL, 0, 0))
    {
		//check whether message is for find dialog
		if(!IsWindow(g_hFind) || !IsDialogMessageW(g_hFind, &msg)){
			//translate accelerators
			if(!TranslateAcceleratorW(g_hMain, m_hAcc, &msg)){
				//translate MDI accelerators
		        if(!TranslateMDISysAccel(g_hClient, &msg)){
					TranslateMessage(&msg);
			        DispatchMessageW(&msg);
				}
			}
		}
    }
	
	DestroyAcceleratorTable(m_hAcc);
	Cleanup(g_hMain);
	//uninitialize COM
	CoUninitialize();
	//exit
    return msg.wParam;
}

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR			lpnm;

    switch (msg)
    {
	HANDLE_MSG (hwnd, WM_COMMAND, Main_OnCommand);
   	HANDLE_MSG (hwnd, WM_DESTROY, Main_OnDestroy);
	HANDLE_MSG (hwnd, WM_CREATE, Main_OnCreate);
	HANDLE_MSG (hwnd, WM_SIZE, Main_OnSize);
	HANDLE_MSG (hwnd, WM_CLOSE, Main_OnClose);
	HANDLE_MSG (hwnd, WM_DRAWITEM, Main_OnDrawItem);
	HANDLE_MSG (hwnd, WM_MEASUREITEM, Main_OnMeasureItem);
	HANDLE_MSG (hwnd, WM_ACTIVATE, Main_OnActivate);
	HANDLE_MSG (hwnd, WM_DROPFILES, Main_OnDropFiles);
	HANDLE_MSG (hwnd, WM_SYSCOLORCHANGE, Main_OnSysColorChange);
	HANDLE_MSG (hwnd, WM_MOVE, Main_OnMove);
	HANDLE_MSG (hwnd, WM_INITMENUPOPUP, Main_OnInitMenuPopup);

	case UPDM_INETERROR:
		if(g_CheckingFromButton){
			MessageBox(hwnd, (char *)wParam, 0, 0);
		}
		return TRUE;
	case UPDM_UPDATEFOUND:{
		wchar_t		temp[12];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char *)wParam, 12, temp, 12);
		ShowNewVersionBaloon(temp);
		return TRUE;
	}
	case UPDM_SAMEVERSION:
		if(g_CheckingFromButton)
			MessageBoxW(hwnd, g_Strings.sSameVersion, g_Strings.sCheckUpdate, MB_OK | MB_ICONINFORMATION);
		return TRUE;
	case UPDM_GETVERSION:
		strcpy((char *)wParam, m_Version);
		return TRUE;
	case UPDM_GETSTRINGS:
		strcpy((char *)wParam, UPDATE_CHECK_URL);
		strcpy((char *)lParam, "/version.txt");
		return TRUE;
	case TBNPM_GET_MENU_TXT:
		GetMIText(m_MainMenus, NELEMS(m_MainMenus), wParam, (wchar_t *)lParam);
		return TRUE;
	case TBNPM_GET_ACC:{
		ACCEL				*pAcc;
		P_TP_TEMPACCEL		pTmp = (P_TP_TEMPACCEL)lParam;
		pAcc = GetAccString(m_pAcc, m_AccCount, pTmp->acc.cmd, pTmp->text);
		if(pAcc){
			pTmp->acc.fVirt = pAcc->fVirt;
			pTmp->acc.key = pAcc->key;
			return TRUE;
		}
		else{
			return FALSE;
		}
	}
	case TBNPM_TOGGLESP:
		m_ShowPosition = !m_ShowPosition;
		return TRUE;
	case TBNPM_ACT_BY_EDIT:
		//activate MDI child by edit handle
		if(GetParent((HWND)wParam) != (HWND)SendMessageW(g_hClient, WM_MDIGETACTIVE, 0, 0)){
			SendMessageW(g_hClient, WM_MDIACTIVATE, (WPARAM)GetParent((HWND)wParam), 0);
		}
	case TBNPM_GET_PE_BY_TAB:
		//return handle by tab index
		return (LRESULT)GetHandleByTabIndex(wParam);
	case TBNPM_GET_TABS_CNT:
		//return tabs count
		return (LRESULT)TabCtrl_GetItemCount(m_hTabMain);
	case TBNPM_CLEARRECENT:
		//clear recent files list
		ClearRecentFiles();
		return TRUE;
	case TBNPM_GET_ACT_HANDLE:
		//return active handle
		return (LRESULT)GetActiveHandle();
	case TBNPM_GETACTIVEEDIT:
		//return active edit handle
		return (LRESULT)GetActiveEdit();
	case TBNPM_CLOSEPARAM:
		//set close parameter
		SetPropW(hwnd, P_SAVE_ON_EXIT, (HANDLE)wParam);
		return TRUE;
	case TBNPM_OPENFROMCLINE:{
		//open file dragged onto program icon
		wchar_t		szLoadPath[MAX_PATH], szPathShort[MAX_PATH];
		
		if(m_InTray){
			SendMessageW(hwnd, WM_SHELLNOTIFY, IDI_TRAY, WM_LBUTTONDBLCLK);
		}
		//file path was saved as global atom
		GlobalGetAtomNameW((ATOM)lParam, szLoadPath, MAX_PATH);
		DeleteAtom((ATOM)lParam);
		GetFileNameFromPath(szLoadPath, szPathShort);
		LoadDocument(hwnd, szLoadPath, szPathShort, NULL, TRUE);
		RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
		return TRUE;
	}
	case TBNPM_SAVEONCLOSE:{
		//save all on close
		P_TPEDIT	pE;
		int			count;

		count = TabCtrl_GetItemCount(m_hTabMain);
		for(int i = 0; i < count; i++){
			pE = GetHandleByTab(m_hTabMain, i);
			if(pE && pE->status == ST_FILE && pE->changed){
				SaveFile(pE->szLongName, pE);
			}
		}
		return TRUE;
	}
	case TBNPM_HSP_SIZED:{
		RECT		rc;
		GetClientRect(hwnd, &rc);
		SendMessageW(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
		return TRUE;
	}
	case WM_SHELLNOTIFY:
		if(wParam == IDI_TRAY){
			//message from tray icon
			if(lParam == WM_LBUTTONDBLCLK){
				//double click on icon - show main window and remove icon
				m_InTray = FALSE;
				Shell_NotifyIconW(NIM_DELETE, &m_nData);
				if(m_WinState == 1){
					ShowWindow(hwnd, SW_MAXIMIZE);
				}
				else{
					ShowWindow(hwnd, SW_RESTORE);
				}
				SetForegroundWindow(hwnd);
			}
			else if (lParam == WM_RBUTTONDOWN){
				//show popup menu
				ShowPopUp(hwnd, GetSubMenu(m_hPopup, 0));
			}
		}
		return TRUE;
	case WM_NOTIFY:
		lpnm = (LPNMHDR)lParam;
		switch(lpnm->code){
		case NM_CUSTOMDRAW:{
			//draw custom toolbar buttons if visual style is enabled
			if(g_VSEnabled){
				LPNMTBCUSTOMDRAW	lpcd = (LPNMTBCUSTOMDRAW)lParam;
				if((lpcd->nmcd.dwDrawStage & CDDS_ITEMPREPAINT) == CDDS_ITEMPREPAINT){
					if((lpcd->nmcd.uItemState & CDIS_HOT) == CDIS_HOT){
						DrawToolbarButton(lpcd->nmcd.hdc, &lpcd->nmcd.rc, lpcd->nmcd.uItemState, lpcd->nmcd.dwItemSpec);
						return CDRF_SKIPDEFAULT;
					}
				}
				//always return CDRF_NOTIFYITEMDRAW - otherwise no drawing will be done
				return CDRF_NOTIFYITEMDRAW;
			}
			else{
				return CDRF_DODEFAULT;
			}
		}
		// case NM_RCLICK:{
			// if(lpnm->hwndFrom == m_hTabMain){
				// //right click on tab
				// TCHITTESTINFO	tch;
				// int				index;
				// P_TPEDIT	pE;
					
				// //get tab index by cursor pos and hit test
				// GetCursorPos(&tch.pt);
				// MapWindowPoints(HWND_DESKTOP, m_hTabMain, &tch.pt, 1);
				// index = TabCtrl_HitTest(m_hTabMain, &tch);
				// //get current handle
				// pE = GetHandleByTabIndex(index);
				// if(pE->status == ST_FILE){
					// ;
				// }
			// }
			// return TRUE;
		// }
		case NM_CLICK:{
			if(lpnm->hwndFrom == m_hTabMain){
				TCHITTESTINFO	tch;
				int				index;
				RECT 			rc;
				
				//click on tab - get current tab by cursor position and hit test
				GetCursorPos(&tch.pt);
				MapWindowPoints(HWND_DESKTOP, m_hTabMain, &tch.pt, 1);
				index = TabCtrl_HitTest(m_hTabMain, &tch);
				TabCtrl_GetItemRect(m_hTabMain, index, &rc);
				SetRect(&rc, rc.right - 20, rc.top + 4, rc.right - 4, rc.top + 20);

				if(PtInRect(&rc, tch.pt)){
					//cursor position is withing "close" button
					P_TPEDIT	pE;
					
					pE = GetHandleByTabIndex(index);
					if(pE){
						//close current child
						if(AskOnClose(pE)){
							pE->removeLastSession = TRUE;
							SendMessageW(g_hClient, WM_MDIDESTROY, (WPARAM)(HWND)SendMessageW(g_hClient, WM_MDIGETACTIVE, 0, 0), 0);
						}
					}
				}
			}
			return TRUE;
		}
		case TCN_SELCHANGE:{
			P_TPEDIT	pE;
			int			tabIndex = TabCtrl_GetCurSel(m_hTabMain);
			
			//tab selection changed - activate appropriate MDI child
			pE = GetHandleByTabIndex(tabIndex);
			SendMessageW(g_hClient, WM_MDIACTIVATE, (WPARAM)pE->hChild, 0);
			
			if(g_Settings.trackChanges){
				//check for possible outer changes
				CheckForOuterChanges();
			}
			return TRUE;
		}
		case TTN_NEEDTEXTW:{
			if(lpnm->hwndFrom == m_hTTToolbar){
				//show toolbar tooltips
				wchar_t			szTooltip[128];
				TOOLTIPTEXTW 	* ttp;
				
				GetTooltip(szTooltip, m_MainMenus, NELEMS(m_MainMenus), lpnm->idFrom);
				ttp = (TOOLTIPTEXTW *)lParam;
				wcscpy(ttp->szText, szTooltip);
			}
			else if(lpnm->hwndFrom == m_hTTTab){
				LPNMTTDISPINFOW	lpnmtdi = (LPNMTTDISPINFOW) lParam;
				wchar_t			szBuffer[MAX_PATH];
				TCHITTESTINFO	tch;
				int				index;

				//show tab tooltip (by cursor position and hit test)
				GetCursorPos(&tch.pt);
				MapWindowPoints(HWND_DESKTOP, m_hTabMain, &tch.pt, 1);
				index = TabCtrl_HitTest(m_hTabMain, &tch);
				if(index >= 0){
					lpnmtdi->lpszText = szBuffer;
					GetLongName(index, szBuffer);
				}
			}
			return TRUE;
		}
		}
    default:
		//always call DefFrameProcW (we are in MDI application)
        return DefFrameProcW(hwnd, g_hClient, msg, wParam, lParam);
   }
}

static void Main_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
	P_TPEDIT		pE;
	HMENU			h1 = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_EDIT));
	HMENU 			h2 = GetSubMenu(h1, GetMenuPosition(h1, IDM_REFRESH_EVERY));

	PrepareMenuGradientColors(GetSysColor(COLOR_BTNFACE));
	if(hMenu == h2){
		pE = GetActiveHandle();
		if(pE){
			if(pE->status == ST_FILE){
				if(pE->timerOn){
					//select update interval
					CheckMenuRadioItem(h2, REFRESH_MENU_MIN, REFRESH_MENU_MIN + 1800, REFRESH_MENU_MIN + pE->interval, MF_BYCOMMAND);
				}
				else{
					//timer off - uncheck appropriate menu item
					CheckMenuRadioItem(h2, REFRESH_MENU_MIN, REFRESH_MENU_MIN + 1800, REFRESH_MENU_MIN, MF_BYCOMMAND);
				}
			}
		}
	}
}

static void Main_OnMove(HWND hwnd, int x, int y)
{
	RECT		rc;

	if((GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_MINIMIZE) != WS_MINIMIZE && IsWindowVisible(hwnd)){
		//save window position for possible save
		GetWindowRect(hwnd, &rc);
		m_TempPoint.x = rc.left;
		m_TempPoint.y = rc.top;
	}
}

static void Main_OnSysColorChange(HWND hwnd)
{
	REBARBANDINFOW	band;

	//redraw all on system colors change
	g_VSEnabled = IsVisualStyleEnabled();
	CreateRebarBackground(hwnd);
	RedrawWindow(g_hMain, NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
	band.cbSize = sizeof(band);
	band.fMask = RBBIM_BACKGROUND | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_STYLE;
	SendMessage(m_hRebar, RB_GETBANDINFO, 0, (LPARAM)&band);
	band.hbmBack = m_hRBack;
	SendMessage(m_hRebar, RB_SETBANDINFO, 0, (LPARAM)&band);
	RedrawWindow(m_hRebar, NULL, NULL, RDW_INVALIDATE);
}

static void Main_OnDropFiles(HWND hwnd, HDROP hdrop)
{
	int				count;
	wchar_t			szNameFull[MAX_PATH], szNameShort[MAX_PATH];

	//get count of files dropped
	count = DragQueryFileW(hdrop, 0xffffffff, NULL, 0);
	for(int i = 0; i < count; i++){
		//get file path
		DragQueryFileW(hdrop, i, szNameFull, MAX_PATH);
		//check whether really file is dropped (not directory etc)
		if(IsFileDropped(szNameFull)){
			wcscpy(szNameShort, szNameFull);
			GetFileNameFromPath(szNameFull, szNameShort);
			if(g_Settings.openNew){
				//load file in new tab
				LoadDocument(hwnd, szNameFull, szNameShort, NULL, TRUE);
			}
			else{
				if(i > 0){
					//load next files in new tabs
					LoadDocument(hwnd, szNameFull, szNameShort, NULL, TRUE);
				}
				else{
					//load first file in the same tab
					if(!LoadInSameTab(szNameFull, szNameShort)){
						break;
					}
				}
			}
			if(g_Settings.lastSession){
				//save file in last session files
				AddToLastSession(szNameFull, GetActiveHandle());
			}
		}
	}
}

static void Main_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	HWND		hEdit;

	if(state != WA_INACTIVE){
		hEdit = GetActiveEdit();
		if(hEdit){
			SetFocus(hEdit);
			if(g_Settings.trackChanges && m_CheckOuter && state != WA_INACTIVE){
				//check for outer changes
				CheckForOuterChanges();
			}
		}
	}
}

static void Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	if(lpDrawItem->CtlType == ODT_MENU)
		//draw menu item
		DrawMItem(lpDrawItem, g_hBmpNormal, g_hBmpGray, CLR_MASK, g_VSEnabled);
	else if(lpDrawItem->CtlType == ODT_TAB)
		//draw tab item
		DrawTabItem(lpDrawItem);
}

static void Main_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	//measure menu item
	MeasureMItem(g_hMenuFont, lpMeasureItem);
}

static void Main_OnClose(HWND hwnd)
{
	//check View menu items state
	if((GetMenuState(g_hMenu, IDM_F_R_FILES, MF_BYCOMMAND) & MF_CHECKED) == MF_CHECKED)
		BitOn(&g_Settings.res1, SB_SHOW_FR);
	else
		BitOff(&g_Settings.res1, SB_SHOW_FR);
	if((GetMenuState(g_hMenu, IDM_SNIPPETS, MF_BYCOMMAND) & MF_CHECKED) == MF_CHECKED)
		BitOn(&g_Settings.res1, SB_SHOW_SNIPPETS);
	else
		BitOff(&g_Settings.res1, SB_SHOW_SNIPPETS);
	WritePrivateProfileStructW(S_PREFERENCES, K_SETTINGS, &g_Settings, sizeof(g_Settings), g_Paths.sINI);

	if((BOOL)GetPropW(hwnd, P_SAVE_ON_EXIT)){
		//recreate last session list
		if(g_Settings.lastSession)
			RecreateLastSession();
		//if save-on-close parameter is TRUE - save all documents
		if(SaveAllOnExit(TRUE)){
			//close program if user has not hit "Cancel"
			WriteLastPosition(hwnd);
			DestroyWindow(hwnd);
		}
	}
	else{
		//otherwise just clean up and exit
		//recreate last session list
		if(g_Settings.lastSession)
			RecreateLastSession();
		WriteLastPosition(hwnd);
		DestroyWindow(hwnd);
	}
}

static void Main_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	RECT			rc, rcsb, rcsp, rcm;
	REBARBANDINFOW	band;
	int				y, h;

	if(state != SIZE_MINIMIZED){
		//store window state and position for possible save
		if(state == SIZE_MAXIMIZED){
			m_WinState = 1;
		}
		else{
			m_WinState = 0;
			GetWindowRect(hwnd, &rc);
			m_TempSize.cx = rc.right - rc.left;
			m_TempSize.cy = rc.bottom - rc.top;
		}
		GetClientRect(hwnd, &rcm);
		//move and repaint rebar control
		GetClientRect(m_hTbrMain, &rc);
		band.cbSize = sizeof(band);
		band.fMask = RBBIM_BACKGROUND | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_STYLE;
		SendMessage(m_hRebar, RB_GETBANDINFO, 0, (LPARAM)&band);
		band.cx = cx;
		SendMessage(m_hRebar, RB_SETBANDINFO, 0, (LPARAM)&band);
		//move toolbar
		MoveWindow(m_hTbrMain, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		//move status bar
		GetClientRect(m_hStatusMain, &rcsb);
		MoveWindow(m_hStatusMain, 0, cy - (rcsb.bottom - rcsb.top), cx, rcsb.bottom - rcsb.top, TRUE);
		//move tab control
		MoveWindow(m_hTabMain, 0, rc.bottom - rc.top, cx, 22 + 4, TRUE);
		//resize and move MDI client window
		y = rc.bottom - rc.top + 22 + 4;
		h = cy - y - (rcsb.bottom - rcsb.top);
		if(IsWindowVisible(g_hSplit)){
			SendMessageW(g_hSplit, TBNPM_MOVE_HSP, cy - (rcsb.bottom - rcsb.top), rcm.right - rcm.left);
			GetWindowRect(g_hSplit, &rcsp);
			h -= rcsp.bottom - rcsp.top;
		}
		MoveWindow(g_hClient, 0, y, cx, h, TRUE);
	}
	else{
		//on minimize - hide window and show tray icon
		if(g_Settings.minTotray){
			if(!m_InTray){
				m_InTray = TRUE;
				SetTrayTip();
				Shell_NotifyIconW(NIM_ADD, &m_nData);
				ShowWindow(hwnd, SW_HIDE);
			}
		}
	}
}

static BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	REBARINFO			rbi;
	REBARBANDINFOW		band;
	RECT				rc;
	CLIENTCREATESTRUCT	clStruct;
	wchar_t				szTemp[MAX_PATH];

	//get drive type
	m_RemovableDrive = (GetDriveTypeW(NULL) == DRIVE_REMOVABLE ? TRUE : FALSE);
	//get drive letter
	GetModuleFileNameW(NULL, szTemp, MAX_PATH);
	m_DriveLetter[0] = szTemp[0];
	//set default search direction
	g_FParam = FR_DOWN;
	//get Windows version
	m_WinVer = WinVer();
	//get computer MAC address
	GetMACAddress();
	//check whether visual style is enabled
	g_VSEnabled = IsVisualStyleEnabled();
	//initialize printing
	InitPrintMembers();
	//get language files directory
	GetSubPath(g_Paths.sLangDir, L"\\lang\\");
	//get program settings
	GetSettings();
	//get possible last sessionf from another MACs
	LoadOthersLastSession();
	//check whether UI language is set
	if(!m_LanguageSet){
		//if not - store id from choosen language file and store settings
		GetLanguageID();
		WritePrivateProfileStructW(S_PREFERENCES, K_SETTINGS, &g_Settings, sizeof(g_Settings), g_Paths.sINI);
	}
	//create standard menu font
	CreateMenuFont();
	//get bitmaps
	g_hBmpNormal = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_IML_NORMAL));
	g_hBmpGray = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_IML_GRAY));
	g_hBmpTab = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_TAB_IML));
	//create CRLF bitmaps
	CreateCRLF(hwnd, &g_hCR, &g_hLF, g_BreaksColor, g_TextAreaFormat);
	//prepare menu
	PrepareMenu(hwnd);
	//build language menu
	BuildLanguageMenu();
	//load recent files
	LoadRecentFiles();
	//create MDI client
	clStruct.hWindowMenu = NULL;
	clStruct.idFirstChild = 1;
	g_hClient = CreateWindowExW(0, MDI_CLIENT_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, &clStruct);
	//create toolbar
	m_hTbrMain = CreateWindowExW(0, TOOLBARCLASSNAMEW, NULL, WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_TRANSPARENT, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	m_hTTToolbar = (HWND)SendMessageW(m_hTbrMain, TB_GETTOOLTIPS, 0, 0);
	CreateImageListsW(&m_ImlNormal, &m_ImlGray, g_hInstance, IDB_IML_NORMAL, IDB_IML_GRAY, CLR_MASK, 16, 16, 0);
	DoToolbarW(m_hTbrMain, m_ImlNormal, m_ImlGray, NELEMS(m_TBBtnArray), m_TBBtnArray);
	//create rebar and rebar background image
	CreateRebarBackground(hwnd);
	GetClientRect(m_hTbrMain, &rc);
	m_hRebar = CreateWindowExW(WS_EX_TOOLWINDOW, REBARCLASSNAMEW, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER, 0, 0, 0, 0, hwnd, NULL, g_hInstance, NULL);
	ZeroMemory(&rbi, sizeof(rbi));
	SendMessage(m_hRebar, RB_SETBANDINFO, 0, (LPARAM)&rbi);
	ZeroMemory(&band, sizeof(band));
	band.cbSize = sizeof(band);
	band.fMask = RBBIM_BACKGROUND | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_STYLE;
	band.hbmBack = m_hRBack;
	band.cx = 0;
	band.cxMinChild = 0;
	band.cyMinChild = rc.bottom - rc.top;
	band.hwndChild = m_hTbrMain;
	SendMessage(m_hRebar, RB_INSERTBAND, -1, (LPARAM)&band);
	//create status bar
	m_hStatusMain = CreateWindowExW(0, STATUSCLASSNAMEW, NULL, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, 0, 0, CW_USEDEFAULT, 22, hwnd, NULL, g_hInstance, NULL);
	int	parts[] = {480, 640, -1}; 
	SendMessageW(m_hStatusMain, SB_SETPARTS, 3, (LPARAM)parts);
	SendMessageW(m_hStatusMain, SB_SIMPLE, FALSE, 0);
	SendMessageW(m_hStatusMain, WM_SETFONT, (WPARAM)g_hMenuFont, MAKELPARAM(TRUE, 0));
	//create tab control
	CreateImageList(&g_ImlTab, IDB_TAB_IML, CLR_MASK, 16, 16, 0);
	AddWrapBitmap(FALSE, g_LineNumbersFormat.cText, g_ImlTab);
	m_hTabMain = CreateWindowExW(0, WC_TABCONTROLW, NULL, WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH | TCS_TOOLTIPS | TCS_HOTTRACK | TCS_OWNERDRAWFIXED, 100, 100, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInstance, NULL);
	m_hTTTab = TabCtrl_GetToolTips(m_hTabMain);
	TabCtrl_SetUnicodeFormat(m_hTabMain, TRUE);
	TabCtrl_SetImageList(m_hTabMain, g_ImlTab);
	SendMessageW(m_hTabMain, WM_SETFONT, (WPARAM)g_hMenuFont, MAKELPARAM(TRUE, 0));
	//set tab width
	TabCtrl_SetItemSize(m_hTabMain, g_Settings.tabWidth, 22);
	if(!g_Settings.lastSession){
		//add first tab
		AddTab(g_Strings.sUntitled, NULL);
	}
	else{
		//load last session
		LoadLastSession(hwnd);
	}
	//create main timer
	m_idTimer = SetTimer(hwnd, 1, 100, TimerProc);
	//get program version
	GetVersionNumber();
	
	return TRUE;
}

static void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	wchar_t				szNameFull[MAX_PATH], szNameShort[MAX_PATH];
	wchar_t				szMessage[MAX_PATH + 128];
	P_TPEDIT			pE;
	WIN32_FIND_DATAW	fd;
	HANDLE				hFile;
	int					result;
	BOOL				bContinue = FALSE;

    switch (id)
    {
	case IDM_INS_DATE_TIME:
		InsertDateTime(0);
		break;
	case IDM_INS_DATE:
		InsertDateTime(1);
		break;
	case IDM_INS_TIME:
		InsertDateTime(2);
		break;
	case IDM_DF_SETTINGS:
		result = DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_DF_FORMATS), hwnd, DTFormat_DlgProc, 0);
		if(result == IDOK){
			g_DTFormats = g_TempDTFormats;
			WritePrivateProfileStringW(S_DT_FORMATS, K_D_FORMAT, g_DTFormats.DateFormat, g_Paths.sINI);
			WritePrivateProfileStringW(S_DT_FORMATS, K_T_FORMAT, g_DTFormats.TimeFormat, g_Paths.sINI);
			if(g_DTFormats.TimeFirst)
				WritePrivateProfileStringW(S_DT_FORMATS, K_T_FIRST, L"1", g_Paths.sINI);
			else
				WritePrivateProfileStringW(S_DT_FORMATS, K_T_FIRST, L"0", g_Paths.sINI);
			if(g_DTFormats.CurrSettings)
				WritePrivateProfileStringW(S_DT_FORMATS, K_T_CURR_SETT, L"1", g_Paths.sINI);
			else
				WritePrivateProfileStringW(S_DT_FORMATS, K_T_CURR_SETT, L"0", g_Paths.sINI);
		}
		break;
	case IDM_F_R_FILES:
	case IDM_SNIPPETS:{
		int		imgID;

		if((GetMenuState(g_hMenu, id, MF_BYCOMMAND) & MF_CHECKED) == MF_CHECKED){
			if(SendMessageW(g_hSplit, TBNPM_DEL_ASS_TAB, id, 0) == 0){
				ShowWindow(g_hSplit, SW_HIDE);
				SendMessageW(GetParent(g_hSplit), TBNPM_HSP_SIZED, 0, 0);
			}
		}
		else{
			if(!IsWindowVisible(g_hSplit)){
				ShowWindow(g_hSplit, SW_SHOW);
				SendMessageW(GetParent(g_hSplit), TBNPM_HSP_SIZED, 0, 0);
			}
			switch(id){
			case IDM_F_R_FILES:
				imgID = 6;
				break;
			case IDM_SNIPPETS:
				imgID = 5;
				break;
			}
			SendMessageW(g_hSplit, TBNPM_INSERT_ASS_TAB, id, imgID);
		}
		break;
	}
	case IDM_HOT_KEYS:
		result = DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_HOT_KEYS), hwnd, Hotdlg_DlgProc, 0);
		if(result == IDC_CMD_ADD){
			qsort(g_TempAcc, g_TempAccCount, sizeof(ACCEL), AccCompare);
			if(ChangedAccelerators()){
				bContinue = TRUE;
			}
		}
		else if(result == IDC_CMD_DEFAULT){
			CopyDefAcceleratorsToTemp();
			if(ChangedAccelerators()){
				bContinue = TRUE;
			}
		}
		if(bContinue){
			CopyChangedAccelerators();
			WriteAccelerators();
			PrepareMenu(hwnd);
			//redraw menu bar
			DrawMenuBar(hwnd);
			ReMaxChild();
			//rebuild language menu
			BuildLanguageMenu();
			//rebuild 'Window' menu (possible 'Untitled')
			RecreateWindowMenu();
			//reload recent files
			LoadRecentFiles();
			//load accelerators
			if(m_hAcc)
				DestroyAcceleratorTable(m_hAcc);
			m_hAcc = CreateAcceleratorTableW(m_pAcc, m_AccCount);
		}
		if(result == IDC_CMD_ADD || result == IDC_CMD_DEFAULT){
			free(g_TempAcc);
			g_TempAcc = 0;
			g_TempAccCount = 0;
		}
		break;
	// case IDM_FIND_IN_FILES:{
		// //show find dialog
		// int result = DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_FIND_IN_FILES), hwnd, FIF_DlgProc, 2);
		// if(result == IDOK){
			// if(!IsWindowVisible(g_hSplit)){
				// ShowWindow(g_hSplit, SW_SHOW);
				// SendMessageW(hwnd, TBNPM_HSP_SIZED, 0, 0);
			// }
			// SendMessageW(g_hSplit, TBNPM_SELECT_ASS_TAB, IDM_F_R_FILES, 0);
		// }
		// break;
	// }
	case IDM_HELP:		//show help
		GetSubPath(szNameFull, HELP_SUBPATH);
		ShellExecuteW(hwnd, L"open", szNameFull, NULL, NULL, SW_SHOWNORMAL);
		break;
	case IDM_BM_REMOVE:
		pE = GetActiveHandle();
		CleanBookmarks(pE);
		InvalidateRect(pE->hStatic, NULL, FALSE);
		break;
	case IDM_BM_TOGGLE:
		pE = GetActiveHandle();
		ToggleBookmark(pE);
		InvalidateRect(pE->hStatic, NULL, FALSE);
		break;
	case IDM_BM_NEXT:
		pE = GetActiveHandle();
		GoToBookmark(pE, TRUE);
		break;
	case IDM_BM_PREV:
		pE = GetActiveHandle();
		GoToBookmark(pE, FALSE);
		break;
	case IDM_CASCADE:
		//cascade childs
		SendMessageW(g_hClient, WM_MDICASCADE, 0, 0);
		break;
	case IDM_TILE_HOR:
		//tile childs horizontally
		SendMessageW(g_hClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);
		break;
	case IDM_TILE_VERT:
		//tile childs vertically
		SendMessageW(g_hClient, WM_MDITILE, MDITILE_VERTICAL, 0);
		break;
	case IDM_CLOSE_ALL:
		//close all childs
		SaveAllOnExit(TRUE);
		//clear status bar
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)NULL);
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)NULL);
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 2 | SBT_POPOUT, (LPARAM)NULL);
		break;
	case IDM_SEND_ATTACH:
		//send document as attachment
		pE = GetActiveHandle();
		if(pE){
			wchar_t		szTemp[MAX_PATH], szSaveTemp[MAX_PATH];
			if(pE->status == ST_FILE){
				//if document is file
				if(!pE->changed){
					//if no changes have been made
					SendAsAttachment(hwnd, pE->szLongName, EMAIL_SUBJECT, szTemp);
					if(wcslen(szTemp) > 0){
						DeleteFileW(szTemp);
					}
				}
				else{
					//changes have been made to file - prompt to save
					wcscpy(szMessage, g_Strings.sSaveChanges);
					wcscat(szMessage, L" ");
					wcscat(szMessage, pE->szLongName);
					wcscat(szMessage, L"?");
					int result = MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
					switch(result){
					case IDYES:
						//save file and send
						SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDM_SAVE, 0), 0);
						SendAsAttachment(hwnd, pE->szLongName, EMAIL_SUBJECT, szTemp);
						if(wcslen(szTemp) > 0){
							DeleteFileW(szTemp);
						}
						break;
					case IDNO:
						//save file under temporary name, send and delete temporary file
						GetTempSaveName(pE->szLongName, szSaveTemp, TRUE);
						m_CurrType = pE->type;
						SaveFile(szSaveTemp, pE);
						SendAsAttachment(hwnd, szSaveTemp, EMAIL_SUBJECT, szTemp);
						if(wcslen(szTemp) > 0){
							DeleteFileW(szTemp);
						}
						DeleteFileW(szSaveTemp);
						break;
					case IDCANCEL:
						break;
					}
				}
			}
			else{
				//document is 'Untitled'
				if(pE->changed){
					//if changes have been made - prompt to save
					wcscpy(szMessage, g_Strings.sSaveChanges);
					wcscat(szMessage, L" ");
					wcscat(szMessage, pE->szLongName);
					wcscat(szMessage, L"?");
					int result = MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_YESNO | MB_ICONEXCLAMATION);
					switch(result){
					case IDYES:{
						//save and send
						SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDM_SAVE, 0), 0);
						SendAsAttachment(hwnd, pE->szLongName, EMAIL_SUBJECT, szTemp);
						if(wcslen(szTemp) > 0){
							DeleteFileW(szTemp);
						}
						break;
					}
					case IDNO:
						//save file under temporary name, send and delete temporary file
						GetTempSaveName(pE->szLongName, szSaveTemp, FALSE);
						m_CurrType = pE->type;
						SaveFile(szSaveTemp, pE);
						SendAsAttachment(hwnd, szSaveTemp, EMAIL_SUBJECT, szTemp);
						if(wcslen(szTemp) > 0){
							DeleteFileW(szTemp);
						}
						DeleteFileW(szSaveTemp);
						break;
					}
				}
			}
		}
		
		break;
	case IDM_ABOUT:		
		//show about dialog
		CreateAboutDialog(hwnd, g_hInstance, m_nData.hIcon);
		break;
	case IDM_PRINT:
		//print document
		PrintEdit(GetActiveHandle());
		break;
	case IDM_PAGE_SETUP:
		//show print setup dialog
		g_pSetup.lStructSize = sizeof(g_pSetup);
		g_pSetup.hwndOwner = hwnd;
		g_pSetup.hInstance = g_hInstance;
		g_pSetup.Flags = PSD_MARGINS | PSD_MINMARGINS | PSD_INTHOUSANDTHSOFINCHES;
		if(PageSetupDlgW(&g_pSetup)){
			WritePrivateProfileStructW(S_PRINT, K_PRINT_SETUP, &g_pSetup, sizeof(g_pSetup), g_Paths.sINI);
			SetPaper();
		}
		break;
	case IDM_GOTO:
		//go to specified line in document
		DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_GOTO), hwnd, Goto_DlgProc, 0);
		SetFocus(GetActiveEdit());
		break;
	case IDM_FIND_NEXT:
		FindOnClick();
		// //find next text occurence in document
		// if(SearchText() == -1){
			// wcscpy(szMessage, g_Strings.sNoOccurrences);
			// wcscat(szMessage, L"\n");
			// wcscat(szMessage, g_SearchString);
			// MessageBoxW(hwnd, szMessage, PROGRAM_NAME, MB_OK | MB_ICONEXCLAMATION);
		// }
		break;
	case IDM_FIND:
		//show find dialog
		CreateDialogParamW(g_hInstance, MAKEINTRESOURCEW(DLG_FIND_REPLACE), hwnd, Find_DlgProc, 0);
		break;
	case IDM_REPLACE:
		//show replace dialog
		CreateDialogParamW(g_hInstance, MAKEINTRESOURCEW(DLG_FIND_REPLACE), hwnd, Find_DlgProc, 1);
		break;
	case IDM_PROG_OPEN:
		//restore main window from tray
		m_InTray = FALSE;
		Shell_NotifyIconW(NIM_DELETE, &m_nData);
		if(m_WinState == 1){
			ShowWindow(hwnd, SW_MAXIMIZE);
		}
		else{
			ShowWindow(hwnd, SW_RESTORE);
		}
		break;
	case IDM_REFRESH:{
		//reload current document
		CHARRANGE		chrg;
		BOOL			prevCheck, isBookmark = FALSE;
		P_TPBMRK		pTemp;

		//store checking flag and prevent checking during reloading
		prevCheck = m_CheckOuter;
		m_CheckOuter = FALSE;
		pE = GetActiveHandle();
		if(g_Settings.preservePosition == POS_SAVED){
			//store text position
			SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
		}
		if(pE->bookmarks){
			//store bookmarks
			isBookmark = TRUE;
			pTemp = pE->bookmarks;
			pE->bookmarks = NULL;
		}
		if(pE->changed){
			//if changes have been made - warning about possible data losss
			wcscpy(szMessage, g_Strings.sDiscardChanges);
			wcscat(szMessage, L" ");
			wcscat(szMessage, pE->szShortName);
			wcscat(szMessage, L"?");
			if(MessageBoxW(hwnd, szMessage, PROGRAM_NAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
				LoadDocument(hwnd, pE->szLongName, pE->szShortName, pE, FALSE);
		}
		else{
			LoadDocument(hwnd, pE->szLongName, pE->szShortName, pE, FALSE);
		}
		if(g_Settings.preservePosition == POS_SAVED){
			//restore text position
			SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
		}
		else if(g_Settings.preservePosition == POS_END){
			//jump to the end of document
			int		count = RichEdit_GetLineCount(pE->hEdit);
			chrg.cpMin = SendMessageW(pE->hEdit, EM_LINEINDEX, count - 1, 0);
			chrg.cpMax = chrg.cpMin;
			SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
		}
		if(isBookmark){
			//restore boolmarks
			pE->bookmarks = pTemp;
			RecheckBookmarks(pE);
			InvalidateRect(pE->hEdit, NULL, FALSE);
		}
		//restore checking flag
		m_CheckOuter = prevCheck;
		break;
	}
	case IDM_OPEN:
		//open document
		if(OSFileDialog(g_Strings.sOpenFileCaption, szNameFull, szNameShort, TRUE, TRUE)){
			if(g_Settings.openNew){
				//open in new tab
				LoadDocument(hwnd, szNameFull, szNameShort, NULL, TRUE);
			}
			else{
				//open in same tab
				LoadInSameTab(szNameFull, szNameShort);
			}
			if(g_Settings.lastSession){
				//add document to last session
				AddToLastSession(szNameFull, GetActiveHandle());
			}
		}
		break;
	case IDM_SAVE_AS:{
		//'Save As...'
		P_TPEDIT		pE = GetActiveHandle();
		HMENU			hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));
		wcscpy(szNameFull, pE->szLongName);
		if(OSFileDialog(g_Strings.sSaveFileCaption, szNameFull, szNameShort, FALSE, pE->status == ST_NEW ? TRUE : FALSE)){
			//save document
			if(SaveFile(szNameFull, pE)){
				SendMessageW(pE->hEdit, EM_SETMODIFY, FALSE, 0);
				pE->changed = FALSE;
				pE->status = ST_FILE;
				pE->type = m_CurrType;
				wcscpy(pE->szLongName, szNameFull);
				wcscpy(pE->szShortName, szNameShort);
				ZeroMemory(&fd, sizeof(fd));
				hFile = FindFirstFileW(szNameFull, &fd);
				if(hFile != INVALID_HANDLE_VALUE){
					//store last file write time
					memcpy(&pE->ft, &fd.ftLastWriteTime, sizeof(FILETIME));
					FindClose(hFile);
				}
				//set new small icon if type of file has been changed
				SetSmallIcon(pE, szNameFull);
				//change child caption
				SetWindowTextW(pE->hChild, pE->szShortName);
				//change and check window menu
				RecreateWindowMenu();
				CheckWindowMenu(hMenu, WIN_MENU_MIN + TabCtrl_GetCurSel(m_hTabMain));
				//change tab
				RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
				//redraw status bar
				SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)pE->szLongName);
				SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)GetATFileType(pE));
			}
		}
		break;
	}
	case IDM_SAVE_ALL:
		//save all documents
		SaveAll();
		break;
	case IDM_SAVE:{
		//save document
		P_TPEDIT		pE = GetActiveHandle();
		if(pE->changed){
			if(pE->status == ST_NEW){
				//in case of 'Untitled' - Save As...
				SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDM_SAVE_AS, 0), 0);
			}
			else{
				//preserve file type (ANSI, Unicode etc)
				m_CurrType = pE->type;
				//save file
				if(SaveFile(pE->szLongName, pE)){
					SendMessageW(pE->hEdit, EM_SETMODIFY, FALSE, 0);
					pE->changed = FALSE;
					pE->status = ST_FILE;
					ZeroMemory(&fd, sizeof(fd));
					hFile = FindFirstFileW(pE->szLongName, &fd);
					if(hFile != INVALID_HANDLE_VALUE){
						memcpy(&pE->ft, &fd.ftLastWriteTime, sizeof(FILETIME));
						FindClose(hFile);
					}
					//set small associated icon
					SetSmallIcon(pE, pE->szLongName);
					//change tab
					RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
				}
			}
		}
		break;
	}
	case IDM_SETTINGS:
		//show settings dialog
		if(!g_hSettings){
			if(DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(DLG_SETTINGS), hwnd, Settings_DlgProc, 0) == IDOK){
				CheckChangesInSettings();
			}
		}
		SetFocus(GetActiveEdit());
		break;
	case IDM_NEW:
		//add new tab and open empty document
		AddTab(g_Strings.sUntitled, NULL);
		break;
	case IDM_EXIT:
	case IDM_EXIT_TRAY:
		//close application
		SendMessageW(hwnd, WM_CLOSE, 0, 0);
		break;
	case IDM_TO_UPPER:
		ConvertCase(GetActiveEdit(), TRUE);
		break;
	case IDM_TO_LOWER:
		ConvertCase(GetActiveEdit(), FALSE);
		break;
	case IDM_UNDO:
		SendMessageW(GetActiveEdit(), EM_UNDO, 0, 0);
		break;
	case IDM_REDO:
		SendMessageW(GetActiveEdit(), EM_REDO, 0, 0);
		break;
	case IDM_CUT:
		SendMessageW(GetActiveEdit(), WM_CUT, 0, 0);
		break;
	case IDM_COPY:
		SendMessageW(GetActiveEdit(), WM_COPY, 0, 0);
		break;
	case IDM_PASTE:
		SendMessageW(GetActiveEdit(), WM_PASTE, 0, 0);
		break;
	case IDM_SELECT_ALL:{
		CHARRANGE	chr;
		chr.cpMin = 0;
		chr.cpMax = -1;
		SendMessageW(GetActiveEdit(), EM_EXSETSEL, 0, (LPARAM)&chr);
		break;
	}
	case IDM_REDO_CURR_FILE:{
		P_TPEDIT	pE = GetActiveHandle();
		if(pE && pE->hEdit){
			while(SendMessageW(pE->hEdit, EM_CANREDO, 0, 0)){
				SendMessageW(pE->hEdit, EM_REDO, 0, 0);
			}
		}
		break;		
	}
	case IDM_UNDO_CURR_FILE:{
		P_TPEDIT	pE = GetActiveHandle();
		if(pE && pE->hEdit){
			while(SendMessageW(pE->hEdit, EM_CANUNDO, 0, 0)){
				SendMessageW(pE->hEdit, EM_UNDO, 0, 0);
			}
		}
		break;		
	}
	case IDM_REDO_ALL_FILES:{
		//redo all changes in all edit boxes
		int			count = TabCtrl_GetItemCount(m_hTabMain);
		P_TPEDIT	pE;
		for(int i = 0; i < count; i++){
			pE = GetHandleByTabIndex(i);
			if(pE && pE->hEdit){
				while(SendMessageW(pE->hEdit, EM_CANREDO, 0, 0)){
					SendMessageW(pE->hEdit, EM_REDO, 0, 0);
				}
			}
		}
		break;
	}
	case IDM_UNDO_ALL_FILES:{
		//undo all changes in all edit boxes
		int			count = TabCtrl_GetItemCount(m_hTabMain);
		P_TPEDIT	pE;
		for(int i = 0; i < count; i++){
			pE = GetHandleByTabIndex(i);
			if(pE && pE->hEdit){
				while(SendMessageW(pE->hEdit, EM_CANUNDO, 0, 0)){
					SendMessageW(pE->hEdit, EM_UNDO, 0, 0);
				}
			}
		}
		break;
	}
	default:
		if(id >= WIN_MENU_MIN && id <= WIN_MENU_MAX){
			//menu item from 'Window' menu has been selected
			HMENU		hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));

			if((GetMenuState(hMenu, id, MF_BYCOMMAND) && MF_CHECKED) != MF_CHECKED){
				//if item is unchecked
				P_TPEDIT	pE;
				int			tabIndex = id - WIN_MENU_MIN;
				//activate appropriate child
				pE = GetHandleByTabIndex(tabIndex);
				SendMessageW(g_hClient, WM_MDIACTIVATE, (WPARAM)pE->hChild, 0);
			}
		}
		else if(id > RECENT_MENU_MIN && id <= RECENT_MENU_MIN + RECENT_FILES_MAX){	
			//menu item from 'Recent files' has been selected
			//get file path
			GetRecentPath(id, szNameFull);
			ZeroMemory(&fd, sizeof(fd));
			if(GetFileAttributesW(szNameFull) != 0xffffffff){
				//if file exists
				GetFileNameFromPath(szNameFull, szNameShort);
				//load document
				pE = LoadDocument(hwnd, szNameFull, szNameShort, NULL, TRUE);
				//load possible bookmarks
				LoadRecentBookmarks(S_RECENT_BOOKMARKS, pE);
				if(pE->bookmarks){
					InvalidateRect(pE->hEdit, NULL, FALSE);
				}
				if(g_Settings.lastSession){
					//add to last session
					AddToLastSession(szNameFull, pE);
				}
			}
			else{
				//file does not exists
				wcscpy(szMessage, szNameFull);
				wcscat(szMessage, L"\n");
				wcscat(szMessage, g_Strings.sFileNotExist);
				MessageBoxW(hwnd, szMessage, PROGRAM_NAME, MB_OK | MB_ICONEXCLAMATION);
			}
		}
		else{
			if(codeNotify == 0 && hwndCtl == 0 && (id >= REFRESH_MENU_MIN && id <= REFRESH_MENU_MAX)){
				//tab reload pop-up menu
				P_TPEDIT	pE = GetActiveHandle();
				if(id == REFRESH_MENU_MIN){
					pE->timerOn = FALSE;
					KillTimer(pE->hChild, 1);
				}
				else{
					//set timer interval
					if(id < REFRESH_MENU_MAX){
						pE->interval = id - REFRESH_MENU_MIN;
						if(pE->timerOn){
							KillTimer(pE->hChild, 1);
						}
						else{
							pE->timerOn = TRUE;
						}
						SetTimer(pE->hChild, 1, pE->interval * 1000, RefreshTimerProc);
					}
					else{
						//TODO - add dialog for interval
						pE->interval = -1;
					}
				}
			}
			else if(IsLangID(id)){
				//menu item from 'Languages' has been selected
				HMENU		hMenu = GetSubMenu(GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_OPTIONS)), GetMenuPosition(g_hMenu, IDM_LANG));
				if((GetMenuState(hMenu, id, MF_BYCOMMAND) && MF_CHECKED) != MF_CHECKED){
					//get new language file and store its ID
					GetNewLanguageFile(id - LANG_MENU_MIN);
					g_Settings.langID = id - LANG_MENU_MIN;
					WritePrivateProfileStructW(S_PREFERENCES, K_SETTINGS, &g_Settings, sizeof(g_Settings), g_Paths.sINI);
					WritePrivateProfileStringW(S_PREFERENCES, K_LANGUAGE_FILE, g_Paths.sLangFile, g_Paths.sINI);
					//rebuild menu
					PrepareMenu(g_hMain);
					//redraw menu bar
					DrawMenuBar(g_hMain);
					//rebuild language menu
					BuildLanguageMenu();
					//change 'Untitled' string
					ResetUntitledString();
					//rebuild 'Window' menu (possible 'Untitled')
					RecreateWindowMenu();
					//reload recent files
					LoadRecentFiles();
					//change all 'Untitled' captions
					ApplyUntitledCaptions();
					ReMaxChild();
					pE = GetActiveHandle();
					if(pE){
						SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)pE->szLongName);
						SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)GetATFileType(pE));
						SendMessageW(pE->hEdit, WM_SETFOCUS, 0, 0);
					}
					//aplly new language settings for results pane
					ApplyPaneLanguage();
				}
			}
			else{
				//always DefFrameProcW
				DefFrameProcW(hwnd, g_hClient, WM_COMMAND, MAKEWPARAM(id, codeNotify), (LPARAM)hwndCtl);
			}
		}
    }
}

static void Main_OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}

static BOOL SaveAllOnExit(BOOL fCloseTab){
	int			count = TabCtrl_GetItemCount(m_hTabMain);
	TCITEMW		ti;
	P_TPEDIT	pE;
	NMHDR		nmhdr;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	
	//loop through all tabs and checks whether changes has been made
	for(int i = count - 1; i >= 0; i--){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE){
			if(SendMessageW(pE->hEdit, EM_GETMODIFY, 0, 0)){
				wchar_t		szMessage[MAX_PATH * 2];
				int			result;
				//document changed - activate appropriate tab
				nmhdr.code = TCN_SELCHANGE;
				nmhdr.hwndFrom = m_hTabMain;
				nmhdr.idFrom = 0;
				TabCtrl_SetCurSel(m_hTabMain, i);
				SendMessageW(g_hMain, WM_NOTIFY, 0, (LPARAM)&nmhdr);
				//prompt to save changes
				wcscpy(szMessage, g_Strings.sSaveChanges);
				wcscat(szMessage, L" ");
				wcscat(szMessage, pE->szLongName);
				wcscat(szMessage, L"?");
				result = MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
				switch(result){
				case IDYES:
				case IDNO:
					if(result == IDYES){
						//save document
						SendMessageW(g_hMain, WM_COMMAND, MAKEWPARAM(IDM_SAVE, 0), 0);
					}
					break;
				case IDCANCEL:
					//in case of Cancel - abort operation
					return FALSE;
				}
			}
			if(fCloseTab){
				//destroy child window
				SendMessageW(g_hClient, WM_MDIDESTROY, (WPARAM)pE->hChild, 0);
			}
		}
	}
	return TRUE;
}

static void CloseTab(P_TPEDIT pE){

	int			count;
	P_TPEDIT	tptemp;
	TCITEMW		ti;
	HMENU		hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));

	//close tab
	if(pE->status == ST_FILE){
		//add file to recent list
		AddToRecentFiles(pE, TRUE);
		if(pE->removeLastSession){
			//add to last session
			RemoveFromLastSession(pE->szLongName);
		}
	}
	//clean up
	if(pE->timerOn)
		KillTimer(pE->hChild, 1);
	if(pE->hIcon && pE->status == ST_FILE)
		DestroyIcon(pE->hIcon);
	if(pE->bookmarks)
		CleanBookmarks(pE);
	TabCtrl_DeleteItem(m_hTabMain, pE->tabIndex);
	HeapFree(g_hHeap, 0, pE);
	count = TabCtrl_GetItemCount(m_hTabMain);
	if(count){
		//change remained tabs properties (tabIndex of pE structure)
		ZeroMemory(&ti, sizeof(ti));
		ti.mask = TCIF_PARAM;
		for(int i = 0; i < count; i++){
			SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
			tptemp = (P_TPEDIT)ti.lParam;
			tptemp->tabIndex = i;
		}
		//rebuild 'Window' menu
		RecreateWindowMenu();
		CheckWindowMenu(hMenu, WIN_MENU_MIN + TabCtrl_GetCurSel(m_hTabMain));
	}
	else{
		//just clear 'Window' menu
		ClearWindowMenu();
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)NULL);
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)NULL);
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 2 | SBT_POPOUT, (LPARAM)NULL);
	}
}

static BOOL SaveFile(wchar_t * lpLongName, P_TPEDIT pE){

	HANDLE		hFile;
	EDITSTREAM	eStream;
	int			flags = SF_TEXT;
	DWORD		written;

	hFile = CreateFileW(lpLongName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE){
		eStream.dwCookie = (DWORD)hFile;
		eStream.pfnCallback = WriteStreamCallback;
		eStream.dwError = 0;
		if(m_CurrType == CODE_UNICODE){
			WriteFile(hFile, m_BOMUnicode, 2, &written, NULL);
			flags |= SF_UNICODE;
		}
		else if(m_CurrType == CODE_UNICODE_BE){
			WriteFile(hFile, m_BOMBE, 2, &written, NULL);
			flags |= SF_UNICODE;
		}
		else if(m_CurrType == CODE_UTF8){
			WriteFile(hFile, m_BOMUTF8, 3, &written, NULL);
			flags = (CP_UTF8 << 16) | SF_USECODEPAGE | SF_TEXT;
		}
		RichEdit_StreamOut(pE->hEdit, flags, &eStream);
		CloseHandle(hFile);
	}
	else
		return FALSE;
	return TRUE;
}

static void ResizeAllEdits(void){
	int			count;
	RECT		rc;
	TCITEMW		ti;
	P_TPEDIT	pE;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		if(ti.lParam){
			pE = (P_TPEDIT)ti.lParam;
			if(pE && pE->hChild && pE->hEdit && pE->hStatic){
				GetClientRect(pE->hChild, &rc);
				MoveWindow(pE->hEdit, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				ResizeStatic(pE, rc.bottom - rc.top);
			}
		}
	}
}

static void ResizeStatic(P_TPEDIT pE, int cy){
	if(g_Settings.showLines)
		MoveWindow(pE->hStatic, 0, 0, g_Settings.marginWidth + L_BOOKMARK, cy, TRUE);
	else
		MoveWindow(pE->hStatic, 0, 0, L_BOOKMARK, cy, TRUE);
}

static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_SETFOCUS:{
			CHARRANGE	chrg;
			CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
			SendMessageW(hwnd, EM_EXGETSEL, 0, (LPARAM)&chrg);
			if(!g_Settings.wordWrap)
				ShowCurrentPositionRegular(hwnd, chrg);
			else
				ShowCurrentPositionWrap(hwnd, chrg);
			return FALSE;
		}
		case WM_PAINT:{
			HideCaret(hwnd);
			CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
			// if(g_Settings.showLines){
				// DrawLineNumbers(hwnd);
			// }
			RedrawWindow(GetActiveStatic(), NULL, NULL, RDW_INVALIDATE);
			if(g_Settings.showWS && (g_Settings.showCRLF || g_Settings.showTabs || g_Settings.showSpaces))
				DrawCRLFWhiteSpace(hwnd, FALSE, g_Settings, g_TabsColor, g_SpaceColor, g_TextHeight, g_hCR, g_hLF);
			ShowCaret(hwnd);
			return FALSE;
		}
		case WM_DRAWITEM:{
			LPDRAWITEMSTRUCT	lpdi = (LPDRAWITEMSTRUCT)lParam;
			if(!g_Settings.wordWrap)
				DrawLineNumbersRegular(hwnd, lpdi->hDC, g_Settings, g_BMColor, g_LineNumbersFormat, g_hLinesFont, g_TextHeight, g_LineNumberHeight, GetHandleByChild(GetParent(hwnd)));
			else
				DrawLineNumbersWrap(hwnd, lpdi->hDC, g_Settings, g_BMColor, g_LineNumbersFormat, g_hLinesFont, g_TextHeight, g_LineNumberHeight, GetHandleByChild(GetParent(hwnd)), g_ImlTab);
			
			return FALSE;
		}
		default:
			return CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
	}
}

static int WinVer(void){
	OSVERSIONINFOW	osv;

	osv.dwOSVersionInfoSize = sizeof(osv);
	GetVersionExW(&osv);
	if(osv.dwMajorVersion > 5 ){
		return 6;
	}
	else if(osv.dwMajorVersion == 5 && osv.dwMinorVersion >=1){
		return 6;
	}
	else if( osv.dwMajorVersion == 5){
		return 5;
	}
	return 4;
}

static BOOL IsVisualStyleEnabled(void){
	HANDLE				hLib;
	FARPROC				hProc1, hProc2;
	BOOL				result;

	if(m_WinVer < 6 || !IsDLLVersionForStyle())
		return FALSE;
	hLib = LoadLibrary("UxTheme.dll");
	if(!hLib)
		return FALSE;
	hProc1 = GetProcAddress(hLib, "IsThemeActive");
	hProc2 = GetProcAddress(hLib, "IsAppThemed");
	if(hProc1){
		__asm{
			call hProc1
			mov result, eax
		}
	}
	if(hProc2){
		__asm{
			call hProc2
			or result, eax
		}
	}
	FreeLibrary(hLib);
	return result;
}

static BOOL IsDLLVersionForStyle(void){
	DLLVERSIONINFO		dvi;
	HANDLE				hLib;
	FARPROC				hProc;
	BOOL				bReturn = FALSE;

	dvi.cbSize = sizeof(dvi);
	hLib = LoadLibrary("comctl32.dll");
	if(hLib){
		hProc = GetProcAddress(hLib, "DllGetVersion");
		if(hProc){
			__asm{
				lea eax, dvi
				push eax
				call hProc
			}
			if(dvi.dwMajorVersion > 5)
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

static void GetSubPath(wchar_t * lpResult, const wchar_t * lpFile){
	GetModuleFileNameW(NULL, lpResult, MAX_PATH);
	PathRemoveFileSpecW(lpResult);
	wcscat(lpResult, lpFile);
}

static int ParseTPCommandLine(wchar_t * pINIPath, wchar_t * pDataPath, wchar_t * pProgPath, wchar_t * pLoadPath){
	LPWSTR 			* szArglist;
   	int 			nArgs;
	wchar_t			* szArg;
	int				c, result = 0;

	pINIPath[0] = '\0';
	pDataPath[0] = '\0';
	pLoadPath[0] = '\0';
	//parse command line into WCHAR array
   	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if(szArglist != 0){
		if(nArgs > 1){			//the first argument is a program name
			for(int i = 1; i < nArgs; i++){
				if((_wcsicmp(szArglist[i], L"-conf") == 0) || (_wcsicmp(szArglist[i], L"-config") == 0)){
					if(nArgs >= 3){
						if(wcslen(szArglist[i + 1]) > 0){
							wcscpy(pINIPath, szArglist[i + 1]);
							PathUnquoteSpacesW(pINIPath);
							result |= ARG_INI_PATH;
						}
					}
					if(nArgs >= 4){
						if(wcslen(szArglist[i + 2]) > 0){
							wcscpy(pDataPath, szArglist[i + 2]);
							PathUnquoteSpacesW(pDataPath);
							result |= ARG_DATA_PATH;
						}
					}
					if(nArgs == 5){
						if(wcslen(szArglist[i + 3]) > 0){
							wcscpy(pProgPath, szArglist[i + 3]);
							PathUnquoteSpacesW(pProgPath);
							result |= ARG_PROG_PATH;
						}
					}
					break;
				}
				szArg = szArglist[i];
				if(*szArg == '-'){			//arguments should start from '-'
					while((c = *szArg++) != '\0'){
						switch(c){
							case 'x':				//close program
								result |= ARG_EXIT;
								break;
							case 's':				//silent close
								result |= ARG_SILENT;
								break;
							case 'n':				//close without saving
								result |= ARG_NO_SAVE;
								break;
						}
					}
				}
				else{
					result = ARG_LOAD_ON_START;
					if(nArgs == 1){
						wcscpy(pLoadPath, szArglist[1]);
					}
					else{
						for(int i = 1; i < nArgs; i++){
							wcscat(pLoadPath, szArglist[i]);
							if(i < nArgs - 1){
								wcscat(pLoadPath, L" ");
							}
						}
					}
					break;
				}
			}
		}
		LocalFree(szArglist);
	}
	return result;
}

static BOOL IsLastBackslash(wchar_t * src){
	wchar_t 	* temp;

	temp = src;
	while(*temp++)
		;
	*temp--;
	*temp--;
	if(*temp == '\\')
		return TRUE;
	else
		return FALSE;
}

static void AddToLastSession(wchar_t * lpPath, P_TPEDIT pE){
	wchar_t				szKeys[512], *pw, szFile[MAX_PATH * 2], szKey[12];
	wchar_t				szPattern[MAX_PATH * 2], szPRemovable[MAX_PATH * 2];
	int					index = 0;

	wcscpy(szPattern, m_MACAddress);
	wcscat(szPattern, DELIMETER);
	wcscat(szPattern, lpPath);
	wcscpy(szPRemovable, REMOVABLE_MAC);
	wcscat(szPRemovable, DELIMETER);
	wcscat(szPRemovable, lpPath);
	GetPrivateProfileStringW(S_LASTSESSION, NULL, NULL, szKeys, 512, g_Paths.sINI);
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_LASTSESSION, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				
				index = _wtoi(pw);
				if((wcscmp(szPattern, szFile) == 0) || (m_RemovableDrive && wcscmp(szPRemovable, szFile))){
					//the same file already exists, just save possible bookmarks
					StoreBookmarks(S_LASTS_BOOKMARKS, pE);
					return;
				}
			}
			while(*pw++)
				;
		}
	}
	index++;
	qsort(m_LastOtherIndices, m_LastOtherFiles, sizeof(int), Compare);
	while(InOtherLastIndices(index))
		index++;
	_itow(index, szKey, 10); 
	AddValueToRecentLast(S_LASTSESSION, szKey, szPattern, szPRemovable, lpPath);
	//store bookmarks
	StoreBookmarks(S_LASTS_BOOKMARKS, pE);
}

static void RemoveFromLastSession(wchar_t * lpPath){
	wchar_t				szKeys[512], *pw, szFile[MAX_PATH * 2], szPattern[MAX_PATH * 2];
	
	GetPrivateProfileStringW(S_LASTSESSION, NULL, NULL, szKeys, 512, g_Paths.sINI);
	pw = szKeys;
	if(*pw){
		ConstructINIKey(szPattern, lpPath);
		// wcscpy(szPattern, m_MACAddress);
		// wcscat(szPattern, DELIMETER);
		// wcscat(szPattern, lpPath);
		// wcscpy(szPRemovable, REMOVABLE_MAC);
		// wcscat(szPRemovable, DELIMETER);
		// wcscat(szPRemovable, lpPath);
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_LASTSESSION, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				if((wcscmp(szPattern, szFile) == 0) || (m_RemovableDrive && wcscmp(szPattern, szFile) == 0)){
					WritePrivateProfileStringW(S_LASTSESSION, pw, NULL, g_Paths.sINI);
					//remove bookmarks
					WritePrivateProfileStringW(S_LASTS_BOOKMARKS, szPattern, NULL, g_Paths.sINI);
					return;
				}
			}
			while(*pw++)
				;
		}
	}
}

static void LoadOthersLastSession(void){
	wchar_t				szKeys[512], *pw, szFile[MAX_PATH * 2], *pName, szMAC[32];
	register int		i;

	GetPrivateProfileStringW(S_LASTSESSION, NULL, NULL, szKeys, 512, g_Paths.sINI);
	m_LastOtherFiles = 0;
	if(wcslen(szKeys) == 0){
		return;
	}
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_LASTSESSION, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				pName = szFile;
				for(i = 0; *pName != CHAR_DELIMETER; i++){
					szMAC[i] = *pName++;
				}
				pName++;
				szMAC[i] = '\0';
				if(m_RemovableDrive && wcscmp(szMAC, REMOVABLE_MAC) == 0){
					//do nothing when working from removable drive and stored pattern is removable
					;
				}
				else{
					if(wcscmp(szMAC, m_MACAddress) != 0){
						if(m_LastOtherFiles == 0)
							m_LastOtherIndices = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, sizeof(int));
						else
							m_LastOtherIndices = HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, m_LastOtherIndices, (m_LastOtherFiles + 1) * sizeof(int));
						*(m_LastOtherIndices + m_LastOtherFiles) = _wtoi(pw);
						m_LastOtherFiles++;
					}
				}
			}
			while(*pw++)
				;
		}
	}
}

static void LoadLastSession(HWND hMain){
	wchar_t				szKeys[512], *pw, szFile[MAX_PATH], szNameShort[MAX_PATH], *pName, szMAC[32];
	P_TPEDIT			pE;
	register int		i;

	GetPrivateProfileStringW(S_LASTSESSION, NULL, NULL, szKeys, 512, g_Paths.sINI);
	if(wcslen(szKeys) == 0){
		AddTab(g_Strings.sUntitled, NULL);
		return;
	}
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_LASTSESSION, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				pName = szFile;
				for(i = 0; *pName != CHAR_DELIMETER; i++){
					szMAC[i] = *pName++;
				}
				pName++;
				szMAC[i] = '\0';
				if(wcscmp(szMAC, m_MACAddress) == 0){
					if(GetFileAttributesW(pName) == 0xffffffff){
						//remove file from last session list
						WritePrivateProfileStringW(S_LASTSESSION, pw, NULL, g_Paths.sINI);
					}
					else{
						GetFileNameFromPath(pName, szNameShort);
						pE = LoadDocument(hMain, pName, szNameShort, NULL, TRUE);
						LoadRecentBookmarks(S_LASTS_BOOKMARKS, pE);
						if(pE->bookmarks){
							InvalidateRect(pE->hEdit, NULL, FALSE);
						}
					}
				}
				else if(m_RemovableDrive && wcscmp(szMAC, REMOVABLE_MAC) == 0){
					if(GetFileAttributesW(pName) == 0xffffffff){
						//remove file from last session list
						WritePrivateProfileStringW(S_LASTSESSION, pw, NULL, g_Paths.sINI);
					}
					else{
						*pName == m_DriveLetter[0];
						GetFileNameFromPath(pName, szNameShort);
						pE = LoadDocument(hMain, pName, szNameShort, NULL, TRUE);
						LoadRecentBookmarks(S_LASTS_BOOKMARKS, pE);
						if(pE->bookmarks){
							InvalidateRect(pE->hEdit, NULL, FALSE);
						}
					}
				}
			}
			while(*pw++)
				;
		}
		//recreate last session files
		RecreateLastSession();
	}
}

static void RecreateLastSession(void){
	wchar_t				szKey[12], szPattern[MAX_PATH * 2];
	int					count, index = 1;
	P_TPEDIT			pE;

	//clear last session files and bookmarks
	ClearLastSession();
	count = TabCtrl_GetItemCount(m_hTabMain);
	qsort(m_LastOtherIndices, m_LastOtherFiles, sizeof(int), Compare);
	for(int i = 0; i < count; i++){
		pE = GetHandleByTab(m_hTabMain, i);
		if(pE && pE->status == ST_FILE){
			while(InOtherLastIndices(index))
				index++;
			_itow(index, szKey, 10);
			ConstructINIKey(szPattern, pE->szLongName);
			// if(!m_RemovableDrive){
				// wcscpy(szPattern, m_MACAddress);
				// wcscat(szPattern, DELIMETER);
				// wcscat(szPattern, pE->szLongName);
			// }
			// else{
				// if(m_DriveLetter[0] == pE->szLongName[0]){
					// wcscpy(szPattern, REMOVABLE_MAC);
					// wcscat(szPattern, DELIMETER);
					// wcscat(szPattern, pE->szLongName);
				// }
				// else{
					// wcscpy(szPattern, m_MACAddress);
					// wcscat(szPattern, DELIMETER);
					// wcscat(szPattern, pE->szLongName);
				// }
			// }
			WritePrivateProfileStringW(S_LASTSESSION, szKey, szPattern, g_Paths.sINI);
			StoreBookmarks(S_LASTS_BOOKMARKS, pE);
			index++;
		}
	}
}

static void LoadRecentFiles(void){
	wchar_t			szKeys[512], *pw, szFile[MAX_PATH * 2], szPath[MAX_PATH * 2], *pName, szMAC[32];
	HMENU			h1, h2;
	int				temp = 0, number;
	int				count;
	register int	i;
	MENUITEMINFOW	mi;
	MItem			mit;

	ZeroMemory(&mit, sizeof(mit));
	mit.xPos = -1;
	mit.yPos = -1;
	mit.xCheck = -1;
	mit.yCheck = -1;

	GetPrivateProfileStringW(S_RECENT_FILES, NULL, NULL, szKeys, 512, g_Paths.sINI);
	if(wcslen(szKeys) == 0)
		return;
	h1 = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_FILE));
	h2 = GetSubMenu(h1, GetMenuPosition(h1, IDM_RECENT));
	count = GetMenuItemCount(h2);
	for(int i = count - 1; i >= 0; i--){
		FreeSingleMenu(h2, i);
		DeleteMenu(h2, i, MF_BYPOSITION);
	}
	m_RecentFiles = 0;
	m_OtherRecentFiles = 0;
	for(i = 0; i < RECENT_FILES_MAX; i++)
		m_RecentIndices[i] = 0;
	if(m_OtherIndices){
		HeapFree(g_hHeap, 0, m_OtherIndices);
		m_OtherIndices = NULL;
	}
	//define count of recent files
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_RECENT_FILES, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				pName = szFile;
				for(i = 0; *pName != CHAR_DELIMETER; i++){
					szMAC[i] = *pName++;
				}
				szMAC[i] = '\0';
				if(wcscmp(szMAC, m_MACAddress) == 0){
					m_RecentFiles++;
					m_RecentIndices[m_RecentFiles - 1] = _wtoi(pw);
				}
				else if(m_RemovableDrive && wcscmp(szMAC, REMOVABLE_MAC) == 0){
					m_RecentFiles++;
					m_RecentIndices[m_RecentFiles - 1] = _wtoi(pw);
				}
				else{
					m_OtherRecentFiles++;
					if(m_OtherRecentFiles == 1){
						m_OtherIndices = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, sizeof(int));
					}
					else{
						m_OtherIndices = HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, m_OtherIndices, m_OtherRecentFiles * sizeof(int));
					}
					*(m_OtherIndices + (m_OtherRecentFiles - 1)) = _wtoi(pw);
				}
			}
			while(*pw++)
				;
		}
	}
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_STRING | MIIM_ID;
	temp = m_RecentFiles;
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_RECENT_FILES, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				pName = szFile;
				for(i = 0; *pName != CHAR_DELIMETER; i++){
					szMAC[i] = *pName++;
				}
				szMAC[i] = '\0';
				pName++;
				if(wcscmp(szMAC, m_MACAddress) == 0 || (m_RemovableDrive && wcscmp(szMAC, REMOVABLE_MAC) == 0)){
					if(m_RemovableDrive){
						if(m_DriveLetter[0] != *pName && wcscmp(szMAC, REMOVABLE_MAC) == 0){
							//store the file with real usb drive letter
							*pName = m_DriveLetter[0];
							wcscpy(szPath, REMOVABLE_MAC);
							wcscat(szPath, DELIMETER);
							wcscat(szPath, pName);
							WritePrivateProfileStringW(S_RECENT_FILES, pw, pName, g_Paths.sINI);
						}
					}
					if(GetFileAttributesW(pName) == 0xffffffff){
						//file does not exist - remove data from ini file, decrease recent files count and shift recent indices array
						WritePrivateProfileStringW(S_RECENT_FILES, pw, NULL, g_Paths.sINI);
						m_RecentFiles--;
						number = _wtoi(pw);
						for(i = 0; i < RECENT_FILES_MAX; i++){
							if(m_RecentIndices[i] == number){
								if(i == RECENT_FILES_MAX - 1){
									//last member
									m_RecentIndices[i] = 0;
								}
								else{
									//shift indices up
									for(int j = i; j < RECENT_FILES_MAX - 1; j++){
										m_RecentIndices[j] = m_RecentIndices[j + 1];
										m_RecentIndices[j + 1] = 0;
									}
								}
								break;
							}
						}
					}
					else{
						_itow(temp, szPath, 10);
						if(temp < 10)
							wcscat(szPath, L"    ");
						else
							wcscat(szPath, L"   ");
						
						wcscat(szPath, pName);
						mi.wID = RECENT_MENU_MIN + temp;
						mi.dwTypeData = szPath;
						mit.id = mi.wID;
						wcscpy(mit.szText, szPath);
						InsertMenuItemW(h2, 0, TRUE, &mi);
						SetMenuItemProperties(&mit, h2, GetMenuPosition(h2, mit.id));
						temp--;
					}
				}
			}
			while(*pw++)
				;
		}
	}
}

static void ClearRecents(void){
	wchar_t				szKeys[512], *pw, szFile[MAX_PATH * 2], *pName, szMAC[32];
	register int		i;

	GetPrivateProfileStringW(S_RECENT_FILES, NULL, NULL, szKeys, 512, g_Paths.sINI);
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_RECENT_FILES, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				pName = szFile;
				for(i = 0; *pName != CHAR_DELIMETER; i++){
					szMAC[i] = *pName++;
				}
				pName++;
				szMAC[i] = '\0';
				if(wcscmp(szMAC, m_MACAddress) == 0 || (m_RemovableDrive && wcscmp(szMAC, REMOVABLE_MAC) == 0)){
					WritePrivateProfileStringW(S_RECENT_FILES, pw, NULL, g_Paths.sINI);
					WritePrivateProfileStructW(S_RECENT_BOOKMARKS, szFile, NULL, 0, g_Paths.sINI);
				}
			}
			while(*pw++)
				;
		}
	}
	m_RecentFiles = 0;
	for(int i = 0; i < RECENT_FILES_MAX; i++)
		m_RecentIndices[i] = 0;
}

static void ClearRecentFiles(void){
	HMENU			h1, h2;
	int				count;
	MItem			mit;

	ClearRecents();
	h1 = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_FILE));
	h2 = GetSubMenu(h1, GetMenuPosition(h1, IDM_RECENT));
	count = GetMenuItemCount(h2);
	for(int i = count - 1; i >= 0; i--){
		FreeSingleMenu(h2, i);
		DeleteMenu(h2, i, MF_BYPOSITION);
	}
	ZeroMemory(&mit, sizeof(mit));
	mit.xCheck = -1;
	mit.yCheck = -1;
	mit.xPos = -1;
	mit.yPos = -1;
	mit.id = IDM_EMPTY;
	wcscpy(mit.szText, L"Empty");
	AppendMenuW(h2, MF_OWNERDRAW, IDM_EMPTY, mit.szText);
	SetMenuItemProperties(&mit, h2, 0);
	EnableMenuItem(h2, 0, MF_BYPOSITION | MF_GRAYED);
}

static void StoreBookmarks(wchar_t * lpSection, P_TPEDIT pE){
	P_TPBMRK		pB;
	wchar_t			szPath[MAX_PATH * 2], szPRemovable[MAX_PATH * 2];
	wchar_t			szNumber[32], szBookmarks[BOOKMARKS_MAX * 32];

	*szBookmarks = '\0';
	pB = pE->bookmarks;
	wcscpy(szPath, m_MACAddress);
	wcscat(szPath, DELIMETER);
	wcscat(szPath, pE->szLongName);
	wcscpy(szPRemovable, REMOVABLE_MAC);
	wcscat(szPRemovable, DELIMETER);
	wcscat(szPRemovable, pE->szLongName);
	if(pB){
		while(pB){
			_ltow(pB->line, szNumber, 10);
			wcscat(szBookmarks, szNumber);
			wcscat(szBookmarks, DELIMETER);
			pB = pB->next;
		}
		AddBookmarksToRecentLast(lpSection, szPath, szPRemovable, szBookmarks, pE->szLongName);
	}
	else{
		AddBookmarksToRecentLast(lpSection, szPath, szPRemovable, NULL, pE->szLongName);
	}
}

static void AddValueToRecentLast(wchar_t * lpSection, wchar_t * lpKey, wchar_t * lpValueFixed, wchar_t * lpValueRemovable, wchar_t * lpPath){
	if(!m_RemovableDrive){
		WritePrivateProfileStringW(lpSection, lpKey, lpValueFixed, g_Paths.sINI);
	}
	else{
		if(m_DriveLetter[0] == lpPath[0]){
			WritePrivateProfileStringW(lpSection, lpKey, lpValueRemovable, g_Paths.sINI);
		}
		else{
			WritePrivateProfileStringW(lpSection, lpKey, lpValueFixed, g_Paths.sINI);
		}
	}
}

static void AddBookmarksToRecentLast(wchar_t * lpSection, wchar_t * lpKeyFixed, wchar_t * lpKeyRemovable, wchar_t * lpValue, wchar_t * lpPath){
	if(!m_RemovableDrive){
		WritePrivateProfileStringW(lpSection, lpKeyFixed, lpValue, g_Paths.sINI);
	}
	else{
		if(m_DriveLetter[0] == lpPath[0]){
			WritePrivateProfileStringW(lpSection, lpKeyRemovable, lpValue, g_Paths.sINI);
		}
		else{
			WritePrivateProfileStringW(lpSection, lpKeyFixed, lpValue, g_Paths.sINI);
		}
	}
}

static void AddToRecentFiles(P_TPEDIT pE, BOOL fAddToMenu){
	wchar_t			szKey[12], szDel[12], szPath[MAX_PATH * 2];
	BOOL			fDelete = FALSE;
	int				number = 0, curr;
	
	if(!IsRecentInList(pE->szLongName)){
		if(m_RecentFiles < RECENT_FILES_MAX){
			for(int i = 0; i < RECENT_FILES_MAX; i++){
				if(m_RecentIndices[i] != 0)
					number = m_RecentIndices[i];
				else{
					curr = i;
					break;
				}
			}
			number++;
			qsort(m_OtherIndices, m_OtherRecentFiles, sizeof(int), Compare);
			while(InOtherIndices(number))
				number++;
			m_RecentFiles++;
			m_RecentIndices[curr] = number;
		}
		else{
			fDelete = TRUE;
		}
		if(fDelete){
			//get recent path #1
			_itow(m_RecentIndices[0], szDel, 10);
			GetPrivateProfileStringW(S_RECENT_FILES, szDel, NULL, szPath, MAX_PATH, g_Paths.sINI);
			//shift recent paths up
			number = ShiftRecentFilesUp();
			//delete the first recent bookmarks
			if(wcslen(szPath) > 0)
				WritePrivateProfileStringW(S_RECENT_BOOKMARKS, szPath, NULL, g_Paths.sINI);
		}
		_itow(number, szKey, 10);
		ConstructINIKey(szPath, pE->szLongName);
		// if(!m_RemovableDrive){
			// wcscpy(szPath, m_MACAddress);
			// wcscat(szPath, DELIMETER);
			// wcscat(szPath, pE->szLongName);
		// }
		// else{
			// if(m_DriveLetter[0] == pE->szLongName[0]){
				// wcscpy(szPath, REMOVABLE_MAC);
				// wcscat(szPath, DELIMETER);
				// wcscat(szPath, pE->szLongName);
			// }
			// else{
				// wcscpy(szPath, m_MACAddress);
				// wcscat(szPath, DELIMETER);
				// wcscat(szPath, pE->szLongName);
			// }
		// }
		WritePrivateProfileStringW(S_RECENT_FILES, szKey, szPath, g_Paths.sINI);
		if(fAddToMenu)
			LoadRecentFiles();
	}
	if(pE->bookmarks){
		StoreBookmarks(S_RECENT_BOOKMARKS, pE);
	}
}

static int ShiftRecentFilesUp(void){
	wchar_t			szKey1[12], szKey2[12], szTemp[MAX_PATH];
	register int	i;

	for(i = 0; i < RECENT_FILES_MAX - 1; i++){
		_itow(m_RecentIndices[i], szKey1, 10);
		_itow(m_RecentIndices[i + 1], szKey2, 10);
		GetPrivateProfileStringW(S_RECENT_FILES, szKey2, NULL, szTemp, MAX_PATH, g_Paths.sINI);
		WritePrivateProfileStringW(S_RECENT_FILES, szKey1, szTemp, g_Paths.sINI);
	}
	return m_RecentIndices[i];
}

static void GetRecentPath(int id, wchar_t * lpPath){
	HMENU			h1, h2;
	wchar_t			szTemp[MAX_PATH], *pw;

	h1 = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_FILE));
	h2 = GetSubMenu(h1, GetMenuPosition(h1, IDM_RECENT));
	GetMenuStringW(h2, id, szTemp, MAX_PATH, MF_BYCOMMAND);
	pw = szTemp;
	while(*pw++ != ' ')
		;
	while(*pw == ' ')
		pw++;
	wcscpy(lpPath, pw);
}

static BOOL IsRecentInList(wchar_t * lpPath){
	wchar_t			*szEntries, *pw, szPattern[MAX_PATH * 2];
	int				entriesSize;
	DWORD			result;

	entriesSize = (m_RecentFiles + m_OtherRecentFiles) * MAX_PATH * 2;
	if(entriesSize <= 0)
		return FALSE;
	szEntries = malloc(sizeof(wchar_t) * entriesSize);
	if(!szEntries)
		return FALSE;
	result = GetPrivateProfileSectionW(S_RECENT_FILES, szEntries, entriesSize, g_Paths.sINI);
	if(result == 0)
	{
		free(szEntries);
		return FALSE;
	}
	if(!m_RemovableDrive){
		wcscpy(szPattern, m_MACAddress);
		wcscat(szPattern, DELIMETER);
		wcscat(szPattern, lpPath);
	}
	else{
		if(m_DriveLetter[0] == lpPath[0]){
			wcscpy(szPattern, REMOVABLE_MAC);
			wcscat(szPattern, DELIMETER);
			wcscat(szPattern, lpPath);
		}
		else{
			wcscpy(szPattern, m_MACAddress);
			wcscat(szPattern, DELIMETER);
			wcscat(szPattern, lpPath);
		}
	}
	pw = szEntries;
	while(*pw || *(pw - 1)){
		if(*pw && *pw != 31888){
			while(*pw++ != '=')
				;
			if(wcscmp(pw, szPattern) == 0){
				free(szEntries);
				return TRUE;
			}
		}
		while(*pw++)
			;
	}

	free(szEntries);
	return FALSE;
}

static void PrepareMenu(HWND hwnd){
	wchar_t			szLang[MAX_PATH];
	HMENU			hRefresh;

	// CreateMenuColors(m_WinVer);
	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);
	GetStrings(szLang);
	if(m_hPopup){
		FreeMenus(m_hPopup);
		DestroyMenu(m_hPopup);
	}
	m_hPopup = LoadMenuW(g_hInstance, MAKEINTRESOURCEW(IDR_MNU_TRAY));
	if(g_hMenu){
		FreeMenus(g_hMenu);
		DestroyMenu(g_hMenu);
	}
	g_hMenu = LoadMenuW(g_hInstance, MAKEINTRESOURCEW(IDR_MNU_MAIN));
	SetMenu(hwnd, g_hMenu);
	hRefresh = ClearRefreshMenu();

	PrepareMenuRecursive(g_hMenu, szLang);
	PrepareMenuRecursive(m_hPopup, szLang);

	BuildRefreshMenu(hRefresh);
}

static void PrepareMenuRecursive(HMENU hMenu, wchar_t * lpLangFile){
	int				count;
	MENUITEMINFOW	mi;
	wchar_t			szText[128], szDefault[128], szAcc[128];
	PMItem			pmi;

	count = GetMenuItemCount(hMenu);
	for(int i = 0; i < count; i++){
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_FTYPE;
		GetMenuItemInfoW(hMenu, i, TRUE, &mi);
		if((mi.fType & MFT_SEPARATOR) != MFT_SEPARATOR){
			if(mi.hSubMenu){
				PrepareMenuRecursive(mi.hSubMenu, lpLangFile);
			}
			
			GetMIText(m_pMenus, NELEMS(m_MainMenus), mi.wID, szDefault);
			SetMenuText(mi.wID, S_MENU, lpLangFile, szDefault, szText);
			if(GetAccString(m_pAcc, m_AccCount, mi.wID, szAcc) != NULL){
				wcscat(szText, szAcc);
			}
			SetMIText(m_MainMenus, NELEMS(m_MainMenus), mi.wID, szText);
			pmi = GetMItem(m_MainMenus, NELEMS(m_MainMenus), mi.wID);
			if(pmi){
				SetMenuItemProperties(pmi, hMenu, i);
			}
		}
		if((mi.fType & MFT_OWNERDRAW) != MFT_OWNERDRAW){
			mi.fType |= MFT_OWNERDRAW;
			SetMenuItemInfoW(hMenu, i, TRUE, &mi);
		}
	}
}

static void CreateMenuFont(void){

	NONCLIENTMETRICSW	nc;

	ZeroMemory(&nc, sizeof(nc));
	nc.cbSize = sizeof(nc);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(nc), &nc, 0);
	g_hMenuFont = CreateFontIndirectW(&nc.lfMenuFont);
}

static void WriteAccelerators(void){
	wchar_t			szKey[24];
	ACCEL			*p = m_pAcc;

	//remove previous values
	WritePrivateProfileSectionW(S_ACCELERATORS, NULL, g_Paths.sINI);
	for(int i = 0; i < m_AccCount; i++, p++){
		_itow(p->cmd, szKey, 10);
		WritePrivateProfileStructW(S_ACCELERATORS, szKey, p, sizeof(ACCEL), g_Paths.sINI);
	}
}

static void LoadATPAccelerators(void){

	wchar_t			szKeys[1024], *pw;
	ACCEL			acc, *pAcc;

	if(m_pAcc)
		free(m_pAcc);
	m_AccCount = 0;
	GetPrivateProfileStringW(S_ACCELERATORS, NULL, NULL, szKeys, 1024, g_Paths.sINI);
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStructW(S_ACCELERATORS, pw, &acc, sizeof(ACCEL), g_Paths.sINI);
				m_AccCount++;
				if(m_AccCount == 1){
					m_pAcc = malloc(sizeof(ACCEL));
					memcpy(m_pAcc, &acc, sizeof(ACCEL));
				}
				else{
					m_pAcc = realloc(m_pAcc, sizeof(ACCEL) * m_AccCount);
					pAcc = m_pAcc;
					pAcc += (m_AccCount - 1);
					memcpy(pAcc, &acc, sizeof(ACCEL));
				}
			}
			while(*pw++)
				;
		}
		qsort(m_pAcc, m_AccCount, sizeof(ACCEL), AccCompare);
	}
}

static BOOL ChangedAccelerators(void){
	LPACCEL		p1, p2;

	if(m_AccCount != g_TempAccCount)
		return TRUE;
	p1 = m_pAcc;
	p2 = g_TempAcc;
	for(int i = 0; i < m_AccCount; i++, p1++, p2++){
		if(p1->key != p2->key || p1->fVirt != p2->fVirt || p1->cmd != p2->cmd)
			return TRUE;
	}
	return FALSE;
}

static void CopyChangedAccelerators(void){
	m_AccCount = g_TempAccCount;
	if(m_pAcc)
		free(m_pAcc);
	m_pAcc = (LPACCEL)malloc(m_AccCount * sizeof(ACCEL));
	memcpy(m_pAcc, g_TempAcc, m_AccCount * sizeof(ACCEL));
	qsort(m_pAcc, m_AccCount, sizeof(ACCEL), AccCompare);
}

static void CopyDefAccelerators(void){
	m_AccCount = NELEMS(m_AccDef);
	if(m_pAcc)
		free(m_pAcc);
	m_pAcc = (ACCEL *)malloc(m_AccCount * sizeof(ACCEL));
	memcpy(m_pAcc, m_AccDef, m_AccCount * sizeof(ACCEL));
	qsort(m_pAcc, m_AccCount, sizeof(ACCEL), AccCompare);
}

static void CopyDefAcceleratorsToTemp(void){
	g_TempAccCount = NELEMS(m_AccDef);
	if(g_TempAcc)
		free(g_TempAcc);
	g_TempAcc = (ACCEL *)malloc(g_TempAccCount * sizeof(ACCEL));
	memcpy(g_TempAcc, m_AccDef, g_TempAccCount * sizeof(ACCEL));
	qsort(g_TempAcc, g_TempAccCount, sizeof(ACCEL), AccCompare);
}

static void CopyMenusArray(void){
	int			size = NELEMS(m_MainMenus) * sizeof(MItem);

	m_pMenus = (PMItem)malloc(size);
	memcpy(m_pMenus, m_MainMenus, size);
}

static void Cleanup(HWND hwnd){
	if(m_pMenus)
		free(m_pMenus);
	if(m_pAcc)
		free(m_pAcc);
	if(g_pDFormats)
		free(g_pDFormats);
	if(g_pTFormats)
		free(g_pTFormats);
	SendMessageW(g_hSplit, TBNPM_CLEAR_LIST, 0, 0);
	if(m_OtherIndices){
		HeapFree(g_hHeap, 0, m_OtherIndices);
		m_OtherIndices = NULL;
	}
	if(m_LastOtherIndices){
		HeapFree(g_hHeap, 0, m_LastOtherIndices);
		m_LastOtherIndices = NULL;
	}
	if(m_Langs){
		HeapFree(g_hHeap, 0, m_Langs);
		m_Langs = NULL;
		m_CountLangs = 0;
	}
	if(g_hCR)
		DeleteObject(g_hCR);
	if(g_hLF)
		DeleteObject(g_hLF);
	if(m_hRBack){
		DeleteObject(m_hRBack);
	}
	if(m_hSmallIcon)
		DestroyIcon(m_hSmallIcon);
	RemovePropW(hwnd, P_SAVE_ON_EXIT);
	if(m_idTimer)
		KillTimer(hwnd, m_idTimer);
	if(m_InTray){
		m_InTray = FALSE;
		Shell_NotifyIconW(NIM_DELETE, &m_nData);
	}
	if(m_hPopup){
		FreeMenus(m_hPopup);
		DestroyMenu(m_hPopup);
		m_hPopup = 0;
	}
	if(g_hMenu){
		FreeMenus(g_hMenu);
	}
	if(g_hMenuFont)
		DeleteObject(g_hMenuFont);
	if(g_hTextFont)
		DeleteObject(g_hTextFont);
	if(g_hLinesFont)
		DeleteObject(g_hLinesFont);
	if(g_hBmpNormal)
		DeleteObject(g_hBmpNormal);
	if(g_hBmpGray)
		DeleteObject(g_hBmpGray);
	if(g_hBmpTab)
		DeleteObject(g_hBmpTab);
	if(m_ImlNormal)
		ImageList_Destroy(m_ImlNormal);
	if(m_ImlGray)
		ImageList_Destroy(m_ImlGray);
	if(g_ImlTab)
		ImageList_Destroy(g_ImlTab);
	//clean registry
	CleanRegMUICache(PROG_EXE_NAME);
	CleanRegMRU(PROG_EXE_NAME);
	CleanRegOpenWithList(PROG_EXE_NAME);
	CleanRegOpenSaveMRU(PROG_EXE_NAME);
}

static HWND CreateNewChild(wchar_t * lpNameFull, wchar_t * lpNameShort, P_TPEDIT pE){
	MDICREATESTRUCTW	mdc;
	HWND				hChild;

	mdc.szClass = MDI_CHILD_CLASS;
	mdc.szTitle = lpNameShort;
	mdc.hOwner = g_hInstance;
	mdc.x = CW_USEDEFAULT;
	mdc.y = CW_USEDEFAULT;
	mdc.cx = 500;
	mdc.cy = 400;
	mdc.style = WS_MAXIMIZE;
	mdc.lParam = (long int)pE;
	m_TempPE = pE;
	hChild = (HWND)SendMessageW(g_hClient, WM_MDICREATE, 0, (LPARAM)&mdc);
	m_TempPE = NULL;
	return hChild;
}

static P_TPEDIT AddTab(wchar_t * lpText, wchar_t * lpFile){
	TCITEMW		ti;
	wchar_t		szBuffer[128];
	int			index;
	P_TPEDIT	pE;
	HMENU		hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));

	index = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_IMAGE | TCIF_TEXT | TCIF_PARAM;
	ti.pszText = szBuffer;
	ti.iImage = 0;
	wcscpy(szBuffer, lpText);
	pE = (P_TPEDIT)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, sizeof(TPEDIT));
	wcscpy(pE->szShortName, lpText);
	if(lpFile){
		wcscpy(pE->szLongName, lpFile);
		pE->status = ST_FILE;
	}
	else{
		wcscpy(pE->szLongName, lpText);
		pE->status = ST_NEW;
		pE->hIcon = m_hSmallIcon;
	}
	pE->interval = 180; //3 min
	ti.lParam = (LONG)pE;
	pE->tabIndex = SendMessageW(m_hTabMain, TCM_INSERTITEMW, index, (LPARAM)&ti);
	CreateNewChild(lpFile, lpText, pE);
	TabCtrl_SetCurSel(m_hTabMain, index);
	RedrawWindow(pE->hEdit, NULL, NULL, RDW_INVALIDATE);
	AddWindowMenu(hMenu, index, pE->szLongName, TRUE);
	RedrawWindow(g_hMain, NULL, NULL, RDW_INVALIDATE);
	return pE;
}

static void CreateImageList(HIMAGELIST *lpIml, int idBmp, COLORREF crMask, int x, int y, int imgCount){
	HBITMAP			hBmp;

	*lpIml = ImageList_Create(x, y, ILC_MASK | ILC_COLOR24, imgCount, 0);
	hBmp = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(idBmp));
	ImageList_AddMasked(*lpIml, hBmp, crMask);
	DeleteObject(hBmp);
	DeleteObject((void*)crMask);
}

static void GetStrings(wchar_t * lpLangFile){
	GetPrivateProfileStringW(S_STRINGS, L"untitled", L"Untitled", g_Strings.sUntitled, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"sample", L"Sample text line", g_Strings.sSample, 64, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"font_text", L"Choose text font", g_Strings.sFontTextCaption, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"font_lines", L"Choose line numbers font", g_Strings.sFontLinesCaption, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"save_changes", L"Do you want to save the changes to", g_Strings.sSaveChanges, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"line", L"Ln", g_Strings.sLine, 64, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"column", L"Col", g_Strings.sColumn, 64, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"open_file", L"Open File", g_Strings.sOpenFileCaption, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"save_file", L"Save File As:", g_Strings.sSaveFileCaption, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"file_not_exist", L"does not exist", g_Strings.sFileNotExist, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"discard_changes", L"Discard all changes and reload", g_Strings.sDiscardChanges, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"outside_changes1", L"has been changed outside the program.", g_Strings.sOutChanges1, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"outside_changes2", L"Do you want to reload it?", g_Strings.sOutChanges2, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"outside_changes3", L"(all unsaved information will be lost)", g_Strings.sOutChanges3, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"refresh_every", L"Refresh every:", g_Strings.sRefreshEvery, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"check_for_update", L"Checking for update", g_Strings.sCheckUpdate, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"new_version_1", L"The new version is available", g_Strings.sNewVersion1, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"new_version_2", L"Click 'OK' in order to proceed to download page.", g_Strings.sNewVersion2, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"same_version", L"You are using the latest version of program.", g_Strings.sSameVersion, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"total_documents", L"Total documents:", g_Strings.sDocsTotal, 64, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"current_document", L"Current document:", g_Strings.sDocsCurrent, 64, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"pref_browser", L"Choose preferred browser", g_Strings.sPrefBrowserCaption, 64, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"total_matches", L"Total matches found:", g_Strings.sTotalFound, 128, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"key_exists", L"This hot key already exists. Please, choose another one.", g_Strings.sKeyExists, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"snippet_exists", L"The snipped with the same name already exists. Please, choose another name.", g_Strings.sSnippetExists, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"del_snip_quest", L"Do you want to delete selected snippet?", g_Strings.sDelSnipQuestion, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"auto_url_change", L"Changing of automatically hyperlinks detection will be applied on next program's start", g_Strings.sAutoURLChange, 256, lpLangFile);
	GetPrivateProfileStringW(S_STRINGS, L"no_more_occurrences", L"No more occurrences for:", g_Strings.sNoOccurrences, 256, lpLangFile);
	GetPrivateProfileStringW(S_DT_FORMATS, K_D_FORMAT, DTS_DATE_FORMAT, g_DTFormats.DateFormat, 128, g_Paths.sINI);
	GetPrivateProfileStringW(S_DT_FORMATS, K_T_FORMAT, DTS_TIME_FORMAT, g_DTFormats.TimeFormat, 64, g_Paths.sINI);
	g_DTFormats.TimeFirst = GetPrivateProfileIntW(S_DT_FORMATS, K_T_FIRST, 0, g_Paths.sINI); 
	g_DTFormats.CurrSettings = GetPrivateProfileIntW(S_DT_FORMATS, K_T_CURR_SETT, 1, g_Paths.sINI); 
	LoadDateTimeMasks(lpLangFile);
}

static void LoadDateTimeMasks(wchar_t * lpLangFile){
	wchar_t			szBuffer[1024 * 10], *pw;

	if(g_pDFormats)
		free(g_pDFormats);
	if(g_pTFormats)
		free(g_pTFormats);
	g_pDFormats = (wchar_t *)calloc(1024 * 10, sizeof(wchar_t));
	g_pTFormats = (wchar_t *)calloc(1024 * 10, sizeof(wchar_t));
	*szBuffer = '\0';
	GetPrivateProfileSectionW(S_DF_CHARS, szBuffer, 1024 * 10, lpLangFile);
	pw = szBuffer;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				wcscat(g_pDFormats, pw);
				wcscat(g_pDFormats, L"\n");
			}
			while(*pw++)
				;
		}
	}
	else{
		wcscpy(g_pDFormats, DATE_FORMAT_MASKS);
	}
	*szBuffer = '\0';
	GetPrivateProfileSectionW(S_TF_CHARS_H12, szBuffer, 1024 * 10, lpLangFile);
	pw = szBuffer;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				wcscat(g_pTFormats, pw);
				wcscat(g_pTFormats, L"\n");
			}
			while(*pw++)
				;
		}
	}
	else{
		wcscpy(g_pTFormats, TIME_FMT_MASKS_H12);
	}
	*szBuffer = '\0';
	GetPrivateProfileSectionW(S_TF_CHARS_H24, szBuffer, 1024 * 10, lpLangFile);
	pw = szBuffer;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				wcscat(g_pTFormats, pw);
				wcscat(g_pTFormats, L"\n");
			}
			while(*pw++)
				;
		}
	}
	else{
		wcscpy(g_pTFormats, TIME_FMT_MASKS_H24);
	}
	*szBuffer = '\0';
	GetPrivateProfileSectionW(S_TF_CHARS, szBuffer, 1024 * 10, lpLangFile);
	pw = szBuffer;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				wcscat(g_pTFormats, pw);
				wcscat(g_pTFormats, L"\n");
			}
			while(*pw++)
				;
		}
	}
	else{
		wcscpy(g_pTFormats, TIME_FMT_MASKS);
	}
	pw = g_pDFormats;
	while(*pw++){
		if(*pw == '=')
			*pw = '\t';
	}
	pw = g_pTFormats;
	while(*pw++){
		if(*pw == '=')
			*pw = '\t';
	}
}

static HMENU ClearRefreshMenu(void){
	HMENU			h1, h2;
	int				count;

	h1 = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_EDIT));
	h2 = GetSubMenu(h1, GetMenuPosition(h1, IDM_REFRESH_EVERY));
	count = GetMenuItemCount(h2);
	FreeMenus(h2);
	for(int i = count - 1; i >=0; i--)
		DeleteMenu(h2, i, MF_BYPOSITION);
	return h2;
}

static void BuildRefreshMenu(HMENU hMenu){
	wchar_t			szKeys[512], *pw;
	wchar_t			szLang[MAX_PATH];
	MItem			mit;
	int				pos = 0;

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);

	GetPrivateProfileStringW(S_REFRESHFREQ, NULL, NULL, szKeys, 512, szLang);
	
	ZeroMemory(&mit, sizeof(mit));
	mit.xCheck = 40;
	mit.xPos = -1;
	mit.yPos = -1;
	
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_REFRESHFREQ, pw, m_RefDef[pos], mit.szText, 32, szLang);
				mit.id = REFRESH_MENU_MIN + _wtol(pw);
				AppendMenuW(hMenu, MF_STRING, mit.id, mit.szText);
				SetMenuItemProperties(&mit, hMenu, pos++);
			}
			while(*pw++)
				;
		}
	}
	else{
		for(; pos < NELEMS(m_RefDef); pos++){
			wcscpy(mit.szText, m_RefDef[pos]);
			mit.id = REFRESH_MENU_MIN + m_RefFreq[pos];
			AppendMenuW(hMenu, MF_STRING, mit.id, mit.szText);
			SetMenuItemProperties(&mit, hMenu, pos);
		}
	}
}

static TPPOSITION ReadLastPosition(void){
	TPPOSITION		tpp;
	RECT			rcDesktop;
	int				sx, sy;

	if(GetSystemMetrics(SM_CMONITORS) == 1){
		GetClientRect(GetDesktopWindow(), &rcDesktop);
		sx = rcDesktop.right - rcDesktop.left;
		sy = rcDesktop.bottom - rcDesktop.top;
	}
	else{
		sx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		sy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	if(!GetPrivateProfileStructW(S_PREFERENCES, K_POSITION, &tpp, sizeof(tpp), g_Paths.sINI)){
		tpp.left = 0;
		tpp.top = 0;
		tpp.width = 760;
		tpp.height = 520;
		tpp.maximized = FALSE;
	}
	else{
		tpp.left = ((double)sx) * tpp.left;
		tpp.top = ((double)sy) * tpp.top;
	}
	return tpp;
}

static void GetProgramSettings(void){
	if(!GetPrivateProfileStructW(S_PREFERENCES, K_SETTINGS, &g_Settings, sizeof(g_Settings), g_Paths.sINI)){
		//check settings from version 1.0
		if(!GetPrivateProfileStructW(S_PREFERENCES, K_SETTINGS, &g_Settings, sizeof(g_Settings) - sizeof(int) * 2, g_Paths.sINI)){
			g_Settings.showLines = TRUE;
			g_Settings.marginWidth = L_MARGIN_MIN;
			g_Settings.alignNumbers = AL_RIGHT;
			g_Settings.tabWidth = T_WIDTH_DEF;
			g_Settings.langID = 0x409;
			m_LanguageSet = FALSE;
			g_Settings.singleInstance = TRUE;
			g_Settings.minTotray = TRUE;
			g_Settings.openNew = TRUE;
			g_Settings.trackChanges = FALSE;
			g_Settings.preservePosition = POS_END;
			g_Settings.showWS = FALSE;
			g_Settings.showCRLF = FALSE;
			g_Settings.showTabs = FALSE;
			g_Settings.showSpaces = FALSE;
			g_Settings.tabStop = T_SIZE_DEF;
			g_Settings.checkNVOnStart = FALSE;
		}
	}
	if(!GetPrivateProfileStringW(S_PREFERENCES, K_LANGUAGE_FILE, NULL, g_Paths.sLangFile, 128, g_Paths.sINI)){
		wcscpy(g_Paths.sLangFile, L"english.lng");
		WritePrivateProfileStringW(S_PREFERENCES, K_LANGUAGE_FILE, g_Paths.sLangFile, g_Paths.sINI);
	}
	// if(!GetPrivateProfileStructW(S_PREFERENCES, K_SEARCH_PARAMS, &g_SearchParams, sizeof(g_SearchParams), g_Paths.sINI)){
		g_SearchParams.updown = FR_DOWN;
	// }
}

static void GetSettings(void){
	HDC			hdc;

	hdc = GetDC(g_hMain);
	if(!GetPrivateProfileStructW(S_PREFERENCES, K_TEXT_FORMAT, &g_TextAreaFormat, sizeof(TPFORMAT), g_Paths.sINI)){
		g_TextAreaFormat.cBack.sysIndex = COLOR_WINDOW;
		g_TextAreaFormat.cBack.crValue = GetSysColor(COLOR_WINDOW);
		g_TextAreaFormat.cText.sysIndex = COLOR_WINDOWTEXT;
		g_TextAreaFormat.cText.crValue = GetSysColor(COLOR_WINDOWTEXT);
		ZeroMemory(&g_TextAreaFormat.lf, sizeof(LOGFONTW));
		g_TextAreaFormat.fontHeight = 10;
		g_TextAreaFormat.lf.lfWeight = FW_NORMAL;
		wcscpy(g_TextAreaFormat.lf.lfFaceName, L"Lucida Console");
	}
	g_TextAreaFormat.lf.lfHeight = -MulDiv(g_TextAreaFormat.fontHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	if(!GetPrivateProfileStructW(S_PREFERENCES, K_LINES_FORMAT, &g_LineNumbersFormat, sizeof(TPFORMAT), g_Paths.sINI)){
		g_LineNumbersFormat.cBack.sysIndex = -1;
		g_LineNumbersFormat.cBack.crValue = 0xc0c0c0;	//silver
		g_LineNumbersFormat.cText.sysIndex = COLOR_WINDOWTEXT;
		g_LineNumbersFormat.cText.crValue = GetSysColor(COLOR_WINDOWTEXT);
		ZeroMemory(&g_LineNumbersFormat.lf, sizeof(LOGFONTW));
		g_LineNumbersFormat.fontHeight = 8;
		g_LineNumbersFormat.lf.lfWeight = FW_NORMAL;
		wcscpy(g_LineNumbersFormat.lf.lfFaceName, L"Lucida Console");
	}
	g_LineNumbersFormat.lf.lfHeight = -MulDiv(g_LineNumbersFormat.fontHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	if(!GetPrivateProfileStructW(S_PREFERENCES, K_BREAKS_COLOR, &g_BreaksColor, sizeof(GCOLORTYPE), g_Paths.sINI)){
		g_BreaksColor.sysIndex = -1;
		g_BreaksColor.crValue = COLOR_B;
	}
	if(!GetPrivateProfileStructW(S_PREFERENCES, K_TABS_COLOR, &g_TabsColor, sizeof(GCOLORTYPE), g_Paths.sINI)){
		g_TabsColor.sysIndex = -1;
		g_TabsColor.crValue = COLOR_TABS;
	}
	if(!GetPrivateProfileStructW(S_PREFERENCES, K_SPACE_COLOR, &g_SpaceColor, sizeof(GCOLORTYPE), g_Paths.sINI)){
		g_SpaceColor.sysIndex = -1;
		g_SpaceColor.crValue = COLOR_SPACE;
	}
	if(!GetPrivateProfileStructW(S_PREFERENCES, K_BM_COLOR, &g_BMColor, sizeof(GCOLORTYPE), g_Paths.sINI)){
		g_BMColor.sysIndex = -1;
		g_BMColor.crValue = 0xd3d3d3;	//light gray
	}
	ReleaseDC(g_hMain, hdc);
	g_hTextFont = CreateFontIndirectW(&g_TextAreaFormat.lf);
	g_hLinesFont = CreateFontIndirectW(&g_LineNumbersFormat.lf);
	GetTextHeight(g_hMain, &g_TextHeight, g_hTextFont);
	GetLineTextHeight(g_hMain, &g_LineNumberHeight, g_hLinesFont);
}

static void CreateEdit(P_TPEDIT pE){
	HWND		hEdit, hStatic;
	int			staticStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD | SS_OWNERDRAW;
	RECT		rc;
	CHARFORMAT2	cr;
	int			tabStop, staticWidth;

	hEdit = CreateWindowExW(WS_EX_STATICEDGE, RICHEDIT_CLASSW, NULL, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_SELECTIONBAR | ES_NOHIDESEL | ES_SAVESEL | ES_WANTRETURN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pE->hChild, NULL, g_hInstance, NULL);
	if(IsBitOn(g_Settings.res1, SB_AUTO_URL))
		RichEdit_AutoURLDetect(hEdit, TRUE);

	if(g_Settings.showLines)
		staticWidth = g_Settings.marginWidth + L_BOOKMARK;
	else
		staticWidth = L_BOOKMARK;
	hStatic = CreateWindowExW(0, L"STATIC", NULL, staticStyle, 0, 0, staticWidth, 200, hEdit, NULL, g_hInstance, NULL);
	SetWindowLongPtrW(hEdit, GWLP_USERDATA, (LONG_PTR)SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)EditProc));
	// SetWindowLongPtrW(hStatic, GWLP_USERDATA, (LONG_PTR)SetWindowLongPtrW(hStatic, GWLP_WNDPROC, (LONG_PTR)StaticProc));
	SendMessageW(hEdit, EM_SETTEXTMODE, TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE, 0);
	SendMessageW(hEdit, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_LINK | ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_PROTECTED);
	SendMessageW(hEdit, EM_SETEDITSTYLE, SES_XLTCRCRLFTOCR, SES_XLTCRCRLFTOCR);
	SendMessageW(hEdit, EM_LIMITTEXT, -1, 0);
	SendMessageW(hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	rc.top += 2;
	if(g_Settings.showLines)
		rc.left += g_Settings.marginWidth + L_BOOKMARK;
	else
		rc.left += L_BOOKMARK;
	SendMessageW(hEdit, EM_SETRECTNP, 0, (LPARAM)&rc);
	SendMessageW(hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
	SendMessageW(hEdit, EM_SETMODIFY, FALSE, 0);
	SendMessageW(hEdit, WM_SETFONT, (WPARAM)g_hTextFont, TRUE);
	if(g_TextAreaFormat.cBack.sysIndex != -1)
		SendMessageW(hEdit, EM_SETBKGNDCOLOR, 0, GetSysColor(g_TextAreaFormat.cBack.sysIndex));
	else
		SendMessageW(hEdit, EM_SETBKGNDCOLOR, 0, g_TextAreaFormat.cBack.crValue);
	ZeroMemory(&cr, sizeof(cr));
	cr.cbSize = sizeof(cr);
	cr.dwMask = CFM_COLOR | CFM_EFFECTS;
	cr.dwEffects = CFE_PROTECTED;
	if(g_TextAreaFormat.cText.sysIndex != -1)
		cr.crTextColor = GetSysColor(g_TextAreaFormat.cText.sysIndex);
	else
		cr.crTextColor = g_TextAreaFormat.cText.crValue;
	SendMessageW(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cr);

	if(g_Settings.wordWrap)
		SendMessageW(hEdit, EM_SETTARGETDEVICE, 0, 0);

	tabStop = g_Settings.tabStop * 4;
	SendMessageW(hEdit, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);

	pE->hEdit = hEdit;
	pE->hStatic = hStatic;
}

static BOOL FontChanged(LPLOGFONTW lf1, LPLOGFONTW lf2){

	if(lf1->lfHeight != lf2->lfHeight || 
		lf1->lfItalic != lf2->lfItalic || 
		lf1->lfWeight != lf2->lfWeight || 
		lf1->lfWidth != lf2->lfWidth || 
		lf1->lfCharSet != lf2->lfCharSet || 
		wcscmp(lf1->lfFaceName, lf2->lfFaceName)){

		return TRUE;
	}
	return FALSE;
}

static void ClearLastSession(void){
	wchar_t				szKeys[512], *pw, szFile[MAX_PATH * 2], *pName, szMAC[32];
	register int		i;

	GetPrivateProfileStringW(S_LASTSESSION, NULL, NULL, szKeys, 512, g_Paths.sINI);
	pw = szKeys;
	if(*pw){
		while(*pw || *(pw - 1)){
			if(*pw && *pw != 31888){
				GetPrivateProfileStringW(S_LASTSESSION, pw, NULL, szFile, MAX_PATH, g_Paths.sINI);
				pName = szFile;
				for(i = 0; *pName != CHAR_DELIMETER; i++){
					szMAC[i] = *pName++;
				}
				pName++;
				szMAC[i] = '\0';
				if(wcscmp(szMAC, m_MACAddress) == 0 || (m_RemovableDrive && wcscmp(szMAC, REMOVABLE_MAC) == 0)){
					WritePrivateProfileStringW(S_LASTSESSION, pw, NULL, g_Paths.sINI);
					WritePrivateProfileStructW(S_LASTS_BOOKMARKS, szFile, NULL, 0, g_Paths.sINI);
				}
			}
			while(*pw++)
				;
		}
	}
}

static void CheckChangesInSettings(void){
	BOOL 	fTextFontChanged = FALSE, fLinesFontChanged = FALSE;
	BOOL 	fTextColorChanged = FALSE, fLinesColorChanged = FALSE;
	BOOL 	fTextBGChanged = FALSE, fLinesBGChanged = FALSE;
	BOOL	fSettingsChanged = FALSE, fRedrawNeeded = FALSE;
	BOOL	fBreakColorChanged = FALSE, fWSColorChanged = FALSE;
	BOOL	fSelectionBarColorChanged = FALSE;

	if(g_Settings.res1 != g_TempSettings.res1){
		if(IsBitOn(g_Settings.res1, SB_AUTO_URL) != IsBitOn(g_TempSettings.res1, SB_AUTO_URL)){
			MessageBoxW(g_hMain, g_Strings.sAutoURLChange, PROGRAM_NAME, MB_OK | MB_ICONINFORMATION);
		}
		g_Settings.res1 = g_TempSettings.res1;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.lastSession != g_TempSettings.lastSession){
		g_Settings.lastSession = g_TempSettings.lastSession;
		fSettingsChanged = TRUE;
		if(!g_Settings.lastSession){
			//clear last session files and bookmarks
			ClearLastSession();
		}
		else{
			RecreateLastSession();
		}
	}
	if(wcscmp(g_sDefBrowser, g_sTempDefBrowser) != 0){
		wcscpy(g_sDefBrowser, g_sTempDefBrowser);
		fSettingsChanged = TRUE;
	}
	if(g_Settings.checkNVOnStart != g_TempSettings.checkNVOnStart){
		g_Settings.checkNVOnStart = g_TempSettings.checkNVOnStart;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.preservePosition != g_TempSettings.preservePosition){
		g_Settings.preservePosition = g_TempSettings.preservePosition;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.trackChanges != g_TempSettings.trackChanges){
		g_Settings.trackChanges = g_TempSettings.trackChanges;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.openNew != g_TempSettings.openNew){
		g_Settings.openNew = g_TempSettings.openNew;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.singleInstance != g_TempSettings.singleInstance){
		g_Settings.singleInstance = g_TempSettings.singleInstance;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.minTotray != g_TempSettings.minTotray){
		g_Settings.minTotray = g_TempSettings.minTotray;
		fSettingsChanged = TRUE;
	}
	if(g_Settings.tabWidth != g_TempSettings.tabWidth){
		g_Settings.tabWidth = g_TempSettings.tabWidth;
		TabCtrl_SetItemSize(m_hTabMain, g_Settings.tabWidth, 22);
		fSettingsChanged = TRUE;
	}
	if(g_Settings.tabStop != g_TempSettings.tabStop){
		g_Settings.tabStop = g_TempSettings.tabStop;
		ApplyNewTabStops();
		fSettingsChanged = TRUE;
	}
	if(g_Settings.wordWrap != g_TempSettings.wordWrap){
		g_Settings.wordWrap = g_TempSettings.wordWrap;
		ApplyNewWordWrap();
		fSettingsChanged = TRUE;
	}
	if(g_Settings.showLines != g_TempSettings.showLines){
		g_Settings.showLines = g_TempSettings.showLines;
		ApplyNewShowLineNumbers();
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(g_Settings.showWS != g_TempSettings.showWS){
		g_Settings.showWS = g_TempSettings.showWS;
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(g_Settings.showCRLF != g_TempSettings.showCRLF){
		g_Settings.showCRLF = g_TempSettings.showCRLF;
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(g_Settings.showTabs != g_TempSettings.showTabs){
		g_Settings.showTabs = g_TempSettings.showTabs;
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(g_Settings.showSpaces != g_TempSettings.showSpaces){
		g_Settings.showSpaces = g_TempSettings.showSpaces;
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(g_Settings.marginWidth != g_TempSettings.marginWidth){
		g_Settings.marginWidth = g_TempSettings.marginWidth;
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		LockWindowUpdate(g_hMain);
		ApplyNewLeftMargin();
		LockWindowUpdate(NULL);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(g_Settings.alignNumbers != g_TempSettings.alignNumbers){
		g_Settings.alignNumbers = g_TempSettings.alignNumbers;
		fSettingsChanged = TRUE;
		fRedrawNeeded = TRUE;
	}
	if(fSettingsChanged){
		WritePrivateProfileStructW(S_PREFERENCES, K_SETTINGS, &g_Settings, sizeof(g_Settings), g_Paths.sINI);
	}
	if(FontChanged(&g_TextAreaFormat.lf, &g_TempTextAreaFormat.lf)){
		memcpy(&g_TextAreaFormat.lf, &g_TempTextAreaFormat.lf, sizeof(LOGFONTW));
		g_TextAreaFormat.fontHeight = g_TempTextAreaFormat.fontHeight;
		fTextFontChanged = TRUE;
	}
	if(FontChanged(&g_LineNumbersFormat.lf, &g_TempLineNumbersFormat.lf)){
		memcpy(&g_LineNumbersFormat.lf, &g_TempLineNumbersFormat.lf, sizeof(LOGFONTW));
		g_LineNumbersFormat.fontHeight = g_TempLineNumbersFormat.fontHeight;
		fLinesFontChanged = TRUE;
	}
	if(g_BreaksColor.sysIndex != g_TempBreaksColor.sysIndex || 
		g_BreaksColor.crValue != g_TempBreaksColor.crValue){

		memcpy(&g_BreaksColor, &g_TempBreaksColor, sizeof(GCOLORTYPE));
		CreateCRLF(g_hMain, &g_hCR, &g_hLF, g_BreaksColor, g_TextAreaFormat);
		fBreakColorChanged = TRUE;
	}
	if(g_TabsColor.sysIndex != g_TempTabsColor.sysIndex || 
		g_TabsColor.crValue != g_TempTabsColor.crValue){

		memcpy(&g_TabsColor, &g_TempTabsColor, sizeof(GCOLORTYPE));
		fWSColorChanged = TRUE;
	}
	if(g_SpaceColor.sysIndex != g_TempSpaceColor.sysIndex || 
		g_SpaceColor.crValue != g_TempSpaceColor.crValue){

		memcpy(&g_SpaceColor, &g_TempSpaceColor, sizeof(GCOLORTYPE));
		fWSColorChanged = TRUE;
	}
	if(g_TextAreaFormat.cText.sysIndex != g_TempTextAreaFormat.cText.sysIndex || 
		g_TextAreaFormat.cText.crValue != g_TempTextAreaFormat.cText.crValue){

		memcpy(&g_TextAreaFormat.cText, &g_TempTextAreaFormat.cText, sizeof(GCOLORTYPE));
		CreateCRLF(g_hMain, &g_hCR, &g_hLF, g_BreaksColor, g_TextAreaFormat);
		fTextColorChanged = TRUE;
	}
	if(g_LineNumbersFormat.cText.sysIndex != g_TempLineNumbersFormat.cText.sysIndex || 
		g_LineNumbersFormat.cText.crValue != g_TempLineNumbersFormat.cText.crValue){

		memcpy(&g_LineNumbersFormat.cText, &g_TempLineNumbersFormat.cText, sizeof(GCOLORTYPE));
		fLinesColorChanged = TRUE;
	}
	if(g_TextAreaFormat.cBack.sysIndex != g_TempTextAreaFormat.cBack.sysIndex || 
		g_TextAreaFormat.cBack.crValue != g_TempTextAreaFormat.cBack.crValue){

		memcpy(&g_TextAreaFormat.cBack, &g_TempTextAreaFormat.cBack, sizeof(GCOLORTYPE));
		CreateCRLF(g_hMain, &g_hCR, &g_hLF, g_BreaksColor, g_TextAreaFormat);
		fTextBGChanged = TRUE;
	}
	if(g_LineNumbersFormat.cBack.sysIndex != g_TempLineNumbersFormat.cBack.sysIndex || 
		g_LineNumbersFormat.cBack.crValue != g_TempLineNumbersFormat.cBack.crValue){

		memcpy(&g_LineNumbersFormat.cBack, &g_TempLineNumbersFormat.cBack, sizeof(GCOLORTYPE));
		fLinesBGChanged = TRUE;
	}

	if(g_BMColor.sysIndex != g_TempBMColor.sysIndex || 
		g_BMColor.crValue != g_TempBMColor.crValue){

		memcpy(&g_BMColor, &g_TempBMColor, sizeof(GCOLORTYPE));
		fSelectionBarColorChanged = TRUE;
	}

	if(fTextFontChanged)
		ApplyNewFont(&g_TextAreaFormat.lf, TRUE);
	if(fLinesFontChanged)
		ApplyNewFont(&g_LineNumbersFormat.lf, FALSE);
	if(fTextBGChanged){
		if(g_TextAreaFormat.cBack.sysIndex != -1)
			ApplyNewBackColor(GetSysColor(g_TextAreaFormat.cBack.sysIndex));
		else
			ApplyNewBackColor(g_TextAreaFormat.cBack.crValue);
	}
	if(fTextColorChanged){
		if(g_TextAreaFormat.cText.sysIndex != -1)
			ApplyNewTextColor(GetSysColor(g_TextAreaFormat.cText.sysIndex));
		else
			ApplyNewTextColor(g_TextAreaFormat.cText.crValue);
	}

	if(fTextFontChanged || fTextBGChanged || fTextColorChanged){
		WritePrivateProfileStructW(S_PREFERENCES, K_TEXT_FORMAT, &g_TextAreaFormat, sizeof(g_TextAreaFormat), g_Paths.sINI);
	}

	if(fBreakColorChanged){
		WritePrivateProfileStructW(S_PREFERENCES, K_BREAKS_COLOR, &g_BreaksColor, sizeof(g_BreaksColor), g_Paths.sINI);
	}
	
	if(fWSColorChanged){
		WritePrivateProfileStructW(S_PREFERENCES, K_TABS_COLOR, &g_TabsColor, sizeof(g_TabsColor), g_Paths.sINI);
	}

	if(fSelectionBarColorChanged){
		WritePrivateProfileStructW(S_PREFERENCES, K_BM_COLOR, &g_BMColor, sizeof(g_BMColor), g_Paths.sINI);
		InvalidateRect(GetActiveEdit(), NULL, FALSE);
	}

	if(fLinesColorChanged){
		AddWrapBitmap(TRUE, g_LineNumbersFormat.cText, g_ImlTab);
	}

	if(fLinesFontChanged || fLinesColorChanged || fLinesBGChanged || fRedrawNeeded || fBreakColorChanged || fWSColorChanged){
		WritePrivateProfileStructW(S_PREFERENCES, K_LINES_FORMAT, &g_LineNumbersFormat, sizeof(g_LineNumbersFormat), g_Paths.sINI);
		ResizeAllEdits();
		InvalidateRect(GetActiveEdit(), NULL, FALSE);
	}
}

static void SaveAll(void){
	int			count = TabCtrl_GetItemCount(m_hTabMain);
	int			current = TabCtrl_GetCurSel(m_hTabMain);

	for(int i = 0; i < count; i++){
		TabCtrl_SetCurSel(m_hTabMain, i);
		SendMessageW(g_hMain, WM_COMMAND, MAKEWPARAM(IDM_SAVE, 0), 0);
	}
	TabCtrl_SetCurSel(m_hTabMain, current);
}

static BOOL AskOnClose(P_TPEDIT pE){
	wchar_t		szMessage[MAX_PATH * 2];
	int			result;

	if(SendMessageW(pE->hEdit, EM_GETMODIFY, 0, 0)){
		wcscpy(szMessage, g_Strings.sSaveChanges);
		wcscat(szMessage, L" ");
		wcscat(szMessage, pE->szLongName);
		wcscat(szMessage, L"?");
		result = MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
		switch(result){
		case IDYES:
		case IDNO:
			if(result == IDYES){
				SendMessageW(g_hMain, WM_COMMAND, MAKEWPARAM(IDM_SAVE, 0), 0);
			}
			return TRUE;
		}
	}
	else{
		return TRUE;
	}
	return FALSE;
}

static P_TPEDIT GetActiveHandle(void){
	HWND		hChild;

	hChild = (HWND)SendMessageW(g_hClient, WM_MDIGETACTIVE, 0, 0);
	return GetHandleByChild(hChild);
	// TCITEMW		ti;
	// int			index;

	// ZeroMemory(&ti, sizeof(ti));
	// ti.mask = TCIF_PARAM;
	// index = TabCtrl_GetCurSel(m_hTabMain);
	// SendMessageW(m_hTabMain, TCM_GETITEMW, index, (LPARAM)&ti);
	// return (P_TPEDIT)ti.lParam;
}

static P_TPEDIT GetHandleByChild(HWND hChild){
	TCITEMW		ti;
	int			count = TabCtrl_GetItemCount(m_hTabMain);
	P_TPEDIT	pE;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hChild == hChild){
			return pE;
		}
	}
	return NULL;
}

static P_TPEDIT GetHandleByTabIndex(int tabIndex){
	TCITEMW		ti;
	int			count = TabCtrl_GetItemCount(m_hTabMain);
	P_TPEDIT	pE;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->tabIndex == tabIndex){
			return pE;
		}
	}
	return NULL;
}

static P_TPEDIT GetHandleByTab(HWND hTab, int index){
	TCITEMW		ti;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	SendMessageW(hTab, TCM_GETITEMW, index, (LPARAM)&ti);
	return (P_TPEDIT)ti.lParam;
}

static HWND GetActiveEdit(void){
	P_TPEDIT	pE;
	
	pE = GetActiveHandle();
	if(pE){
		return pE->hEdit;
	}
	return NULL;
}

static HWND GetActiveStatic(void){
	P_TPEDIT	pE;
	
	pE = GetActiveHandle();
	if(pE){
		return pE->hStatic;
	}
	return NULL;
}

static void GetNewLanguageFile(int id){
	WIN32_FIND_DATAW	fd;
	HANDLE				hFile;
	wchar_t				szFile[MAX_PATH], szTemp[MAX_PATH];
	BOOL				fResult = TRUE;

	wcscpy(szFile, g_Paths.sLangDir);
	wcscat(szFile, L"*.lng");
	ZeroMemory(&fd, sizeof(fd));
	hFile = FindFirstFileW(szFile, &fd);
	if(hFile != INVALID_HANDLE_VALUE){
		wcscpy(szTemp, g_Paths.sLangDir);
		wcscat(szTemp, fd.cFileName);
		if(GetPrivateProfileIntW(L"language", L"id", 0, szTemp) == id){
			wcscpy(g_Paths.sLangFile, fd.cFileName);
			FindClose(hFile);
			return;
		}
		while(fResult){
			fResult = FindNextFileW(hFile, &fd);
			if(fResult){
				wcscpy(szTemp, g_Paths.sLangDir);
				wcscat(szTemp, fd.cFileName);
				if(GetPrivateProfileIntW(L"language", L"id", 0, szTemp) == id){
					wcscpy(g_Paths.sLangFile, fd.cFileName);
					break;
				}
			}
		}
		FindClose(hFile);
	}
}

static void BuildLanguageMenu(void){
	HMENU				h1, h2;
	WIN32_FIND_DATAW	fd;
	HANDLE				hFile;
	wchar_t				szFile[MAX_PATH];
	int					count, lindex = 0;
	BOOL				fResult = TRUE;

	if(m_Langs){
		HeapFree(g_hHeap, 0, m_Langs);
		m_Langs = NULL;
		m_CountLangs = 0;
	}
	wcscpy(szFile, g_Paths.sLangDir);
	wcscat(szFile, L"*.lng");
	h1 = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_OPTIONS));
	h2 = GetSubMenu(h1, GetMenuPosition(h1, IDM_LANG));
	count = GetMenuItemCount(h2);
	for(int i = count - 1; i >= 0; i--){
		RemoveMenu(h2, i, MF_BYPOSITION);
	}
	ZeroMemory(&fd, sizeof(fd));
	hFile = FindFirstFileW(szFile, &fd);
	if(hFile != INVALID_HANDLE_VALUE){
		InsertLanguageMenu(h2, &fd, lindex++);
		while(fResult){
			fResult = FindNextFileW(hFile, &fd);
			if(fResult){
				InsertLanguageMenu(h2, &fd, lindex++);
			}
		}
		FindClose(hFile);
		m_CountLangs = lindex;
	}
}

static BOOL IsLangID(int id){

	for(int i = 0; i < m_CountLangs; i++){
		if(*(m_Langs + i) == id){
			return TRUE;
		}
	}
	return FALSE;
}

static void InsertLanguageMenu(HMENU hMenu, WIN32_FIND_DATAW * lpfd, int lindex){
	wchar_t		szTemp[MAX_PATH];
	int			id;
	MItem		mit;

	ZeroMemory(&mit, sizeof(mit));
	mit.xCheck = 40;
	mit.yCheck = 0;
	mit.xPos = -1;
	mit.yPos = -1;

	if(!m_Langs)
		m_Langs = (int *)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, sizeof(int));
	else
		m_Langs = HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, m_Langs, (lindex + 1) * sizeof(int));

	wcscpy(szTemp, g_Paths.sLangDir);
	wcscat(szTemp, lpfd->cFileName);
	GetPrivateProfileStringW(L"language", L"name", NULL, mit.szText, 128, szTemp);
	id = GetPrivateProfileIntW(L"language", L"id", 0, szTemp);
	mit.id = id + LANG_MENU_MIN;
	AppendMenuW(hMenu, MF_STRING, mit.id, mit.szText);
	SetMenuItemProperties(&mit, hMenu, GetMenuPosition(hMenu, mit.id));
	*(m_Langs + lindex) = mit.id;
	if(g_Settings.langID == id)
		CheckMenuItem(hMenu, mit.id, MF_CHECKED | MF_BYCOMMAND);
}

static void ApplyNewFont(LPLOGFONTW lf, BOOL fForText){
	int			count, tabStop;
	TCITEMW		ti;
	P_TPEDIT	pE;

	if(fForText){
		if(g_hTextFont)
			DeleteObject(g_hTextFont);
		g_hTextFont = CreateFontIndirectW(lf);
		GetTextHeight(g_hMain, &g_TextHeight, g_hTextFont);
		count = TabCtrl_GetItemCount(m_hTabMain);
		ZeroMemory(&ti, sizeof(ti));
		ti.mask = TCIF_PARAM;
		for(int i = 0; i < count; i++){
			SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
			pE = (P_TPEDIT)ti.lParam;
			if(pE && pE->hEdit){
				BOOL fRedraw = IsWindowVisible(pE->hEdit);
				tabStop = g_Settings.tabStop * 4;
				SendMessageW(pE->hEdit, WM_SETFONT, (WPARAM)g_hTextFont, FALSE);
				SendMessageW(pE->hEdit, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);
				if(fRedraw)
					InvalidateRect(pE->hEdit, NULL, FALSE);
			}
		}
		if(g_TextAreaFormat.cText.sysIndex != -1)
			ApplyNewTextColor(GetSysColor(g_TextAreaFormat.cText.sysIndex));
		else
			ApplyNewTextColor(g_TextAreaFormat.cText.crValue);
	}
	else{
		if(g_hLinesFont)
			DeleteObject(g_hLinesFont);
		g_hLinesFont = CreateFontIndirectW(lf);
		GetLineTextHeight(g_hMain, &g_LineNumberHeight, g_hLinesFont);
	}
}

static void ApplyNewTabStops(void){
	int			count, tabStop;
	TCITEMW		ti;
	P_TPEDIT	pE;

	GetTextHeight(g_hMain, &g_TextHeight, g_hTextFont);
	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hEdit){
			BOOL fRedraw = IsWindowVisible(pE->hEdit);
			tabStop = g_Settings.tabStop * 4;
			SendMessageW(pE->hEdit, EM_SETTABSTOPS, 1, (LPARAM)&tabStop);
			if(fRedraw)
				InvalidateRect(pE->hEdit, NULL, FALSE);
		}
	}
}

static void ApplyNewBackColor(COLORREF clr){
	int			count;
	TCITEMW		ti;
	P_TPEDIT	pE;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hEdit){
			SendMessageW(pE->hEdit, EM_SETBKGNDCOLOR, FALSE, clr);
		}
	}
}

static void ApplyNewTextColor(COLORREF clr){
	int			count;
	TCITEMW		ti;
	P_TPEDIT	pE;
	CHARFORMAT2	cr;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	ZeroMemory(&cr, sizeof(cr));
	cr.cbSize = sizeof(cr);
	cr.dwMask = CFM_COLOR;
	cr.crTextColor = clr;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hEdit){
			SendMessageW(pE->hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cr);
		}
	}
}

static void ApplyNewWordWrap(void){
	int			count;
	TCITEMW		ti;
	P_TPEDIT	pE;
	CHARRANGE	chrg;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hEdit){
			if(g_Settings.wordWrap)
				SendMessageW(pE->hEdit, EM_SETTARGETDEVICE, 0, 0);
			else
				SendMessageW(pE->hEdit, EM_SETTARGETDEVICE, 0, 1);
			if(IsWindowVisible(pE->hEdit)){
				RedrawWindow(pE->hEdit, NULL, NULL, RDW_INVALIDATE);
				SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
				if(!g_Settings.wordWrap)
					ShowCurrentPositionRegular(pE->hEdit, chrg);
				else
					ShowCurrentPositionWrap(pE->hEdit, chrg);
			}
		}
	}
}

static void ApplyNewShowLineNumbers(void){
	int			count;
	TCITEMW		ti;
	P_TPEDIT	pE;
	RECT		rc, rcStatic;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hEdit){
			GetWindowRect(pE->hStatic, &rcStatic);
			SendMessageW(pE->hEdit, EM_GETRECT, 0, (LPARAM)&rc);
			if(g_Settings.showLines){
				rc.left += g_Settings.marginWidth;
				SetWindowPos(pE->hStatic, 0, 0, 0, g_Settings.marginWidth + L_BOOKMARK, rcStatic.bottom - rcStatic.top, SWP_NOZORDER);
			}
			else{
				rc.left -= g_Settings.marginWidth;
				SetWindowPos(pE->hStatic, 0, 0, 0, L_BOOKMARK, rcStatic.bottom - rcStatic.top, SWP_NOZORDER);
			}
			SendMessageW(pE->hEdit, EM_SETRECTNP, 0, (LPARAM)&rc);
		}
	}
}

static void ApplyNewLeftMargin(void){
	int			count;
	TCITEMW		ti;
	P_TPEDIT	pE;
	RECT		rc;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->hEdit){
			SendMessageW(pE->hEdit, EM_GETRECT, 0, (LPARAM)&rc);
			if(g_Settings.showLines){
				rc.left = g_Settings.marginWidth + L_BOOKMARK;
				SetWindowPos(pE->hStatic, 0, 0, 0, g_Settings.marginWidth + L_BOOKMARK, rc.bottom - rc.top, SWP_NOZORDER);
			}
			else{
				rc.left = 1 + L_BOOKMARK;
				SetWindowPos(pE->hStatic, 0, 0, 0, 1 + L_BOOKMARK, rc.bottom - rc.top, SWP_NOZORDER);
			}
			SendMessageW(pE->hEdit, EM_SETRECTNP, 0, (LPARAM)&rc);
		}
	}
}

static VOID CALLBACK RefreshTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime){
	P_TPEDIT		pE;
	CHARRANGE		chrg;
	BOOL			isBookmark = FALSE;
	P_TPBMRK		pTemp;

	pE = GetHandleByChild(hwnd);
	if(pE->bookmarks){
		isBookmark = TRUE;
		pTemp = pE->bookmarks;
		pE->bookmarks = NULL;
	}
	if(g_Settings.preservePosition == POS_SAVED){
		SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
	}
	LoadDocument(hwnd, pE->szLongName, pE->szShortName, pE, FALSE);
	if(g_Settings.preservePosition == POS_SAVED){
		SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
	}
	else if(g_Settings.preservePosition == POS_END){
		int		count = RichEdit_GetLineCount(pE->hEdit);
		chrg.cpMin = SendMessageW(pE->hEdit, EM_LINEINDEX, count - 1, 0);
		chrg.cpMax = chrg.cpMin;
		SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
	}
	if(isBookmark){
		pE->bookmarks = pTemp;
		RecheckBookmarks(pE);
		InvalidateRect(pE->hEdit, NULL, FALSE);
	}
}

static void CheckViewItems(void){
	HWND			hTab;
	int				count, flag = MF_UNCHECKED;
	TCITEMW			tci;
	
	ZeroMemory(&tci, sizeof(tci));
	tci.mask = TCIF_PARAM;
	hTab = (HWND)SendMessageW(g_hSplit, TBNPM_GET_ASS_TAB, 0, 0);
	count = TabCtrl_GetItemCount(hTab);
	for(int i = 0; i < count; i++){
		SendMessageW(hTab, TCM_GETITEMW, i, (LPARAM)&tci);
		if(tci.lParam == IDM_F_R_FILES){
			flag = MF_CHECKED;
			break;
		}
	}
	CheckMenuItem(g_hMenu, IDM_F_R_FILES, MF_BYCOMMAND | flag);
	flag = MF_UNCHECKED;
	for(int i = 0; i < count; i++){
		SendMessageW(hTab, TCM_GETITEMW, i, (LPARAM)&tci);
		if(tci.lParam == IDM_SNIPPETS){
			flag = MF_CHECKED;
			break;
		}
	}
	CheckMenuItem(g_hMenu, IDM_SNIPPETS, MF_BYCOMMAND | flag);
	flag = MF_UNCHECKED;
}

static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime){
	P_TPEDIT		pE;
	BOOL			fPrevChanged;
	CHARRANGE		chrg;

	if(IsWindowVisible(g_hSplit)){
		CheckViewItems();
	}
	else{
		CheckMenuItem(g_hMenu, IDM_F_R_FILES, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(g_hMenu, IDM_SNIPPETS, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if(TabCtrl_GetItemCount(m_hTabMain)){
		EnableMenuItem(g_hMenu, IDM_CASCADE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_TILE_HOR, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_TILE_VERT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_SAVE_AS, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE_AS, TBSTATE_ENABLED);
		EnableMenuItem(g_hMenu, IDM_SAVE_ALL, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE_ALL, TBSTATE_ENABLED);
		EnableMenuItem(g_hMenu, IDM_CLOSE_ALL, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_CLOSE_ALL, TBSTATE_ENABLED);
		EnableMenuItem(g_hMenu, IDM_PRINT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_GOTO, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_SELECT_ALL, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_BM_TOGGLE, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_TOGGLE, TBSTATE_ENABLED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_CASCADE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_TILE_HOR, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_TILE_VERT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE, 0);
		EnableMenuItem(g_hMenu, IDM_SEND_ATTACH, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SEND_ATTACH, 0);
		EnableMenuItem(g_hMenu, IDM_REFRESH, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_REFRESH, 0);
		EnableMenuItem(g_hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_COPY, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_CUT, 0);
		EnableMenuItem(g_hMenu, IDM_TO_UPPER, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_TO_LOWER, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_UNDO, 0);
		EnableMenuItem(g_hMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_REDO, 0);
		EnableMenuItem(g_hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_PASTE, 0);
		EnableMenuItem(g_hMenu, IDM_FIND, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_FIND_NEXT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_REPLACE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_SAVE_AS, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE_AS, 0);
		EnableMenuItem(g_hMenu, IDM_SAVE_ALL, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE_ALL, 0);
		EnableMenuItem(g_hMenu, IDM_CLOSE_ALL, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_CLOSE_ALL, 0);
		EnableMenuItem(g_hMenu, IDM_PRINT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_GOTO, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_SELECT_ALL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_BM_TOGGLE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_BM_NEXT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_BM_PREV, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_BM_REMOVE, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_TOGGLE, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_NEXT, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_PREV, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_REMOVE, 0);
		return;
	}
	pE = GetActiveHandle();
	if(pE->bookmarks){
		EnableMenuItem(g_hMenu, IDM_BM_NEXT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_BM_PREV, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_BM_REMOVE, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_NEXT, TBSTATE_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_PREV, TBSTATE_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_REMOVE, TBSTATE_ENABLED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_BM_NEXT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_BM_PREV, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_BM_REMOVE, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_NEXT, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_PREV, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_BM_REMOVE, 0);
	}
	fPrevChanged = pE->changed;
	if(SendMessageW(pE->hEdit, EM_GETMODIFY, 0, 0)){
		EnableMenuItem(g_hMenu, IDM_SAVE, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE, TBSTATE_ENABLED);
		pE->changed = TRUE;
		if(!TextWithStar(pE->hChild))
			ChangeChildText(pE->hChild, TRUE);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SAVE, 0);
		pE->changed = FALSE;
		if(TextWithStar(pE->hChild))
			ChangeChildText(pE->hChild, FALSE);
	}
	if(fPrevChanged != pE->changed){
		RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
	}
	if(pE->status == ST_NEW){
		EnableMenuItem(g_hMenu, IDM_REFRESH_EVERY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_REFRESH, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_REFRESH, 0);
		if(pE->changed){
			EnableMenuItem(g_hMenu, IDM_SEND_ATTACH, MF_BYCOMMAND | MF_ENABLED);
			SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SEND_ATTACH, TBSTATE_ENABLED);
		}
		else{
			EnableMenuItem(g_hMenu, IDM_SEND_ATTACH, MF_BYCOMMAND | MF_GRAYED);
			SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SEND_ATTACH, 0);
		}
	}
	else{
		EnableMenuItem(g_hMenu, IDM_REFRESH_EVERY, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_REFRESH, TBSTATE_ENABLED);
		EnableMenuItem(g_hMenu, IDM_SEND_ATTACH, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_SEND_ATTACH, TBSTATE_ENABLED);
	}
	SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
	if(chrg.cpMin != chrg.cpMax){
		EnableMenuItem(g_hMenu, IDM_COPY, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_CUT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_TO_UPPER, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_TO_LOWER, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_COPY, TBSTATE_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_CUT, TBSTATE_ENABLED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_TO_UPPER, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_TO_LOWER, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_COPY, 0);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_CUT, 0);
	}
	if(SendMessageW(pE->hEdit, EM_CANUNDO, 0, 0)){
		EnableMenuItem(g_hMenu, IDM_UNDO, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_UNDO, TBSTATE_ENABLED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_UNDO, 0);
	}
	if(SendMessageW(pE->hEdit, EM_CANREDO, 0, 0)){
		EnableMenuItem(g_hMenu, IDM_REDO, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_REDO, TBSTATE_ENABLED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_REDO, 0);
	}
	if(SendMessageW(pE->hEdit, EM_CANPASTE, 0, 0)){
		EnableMenuItem(g_hMenu, IDM_PASTE, MF_BYCOMMAND | MF_ENABLED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_PASTE, TBSTATE_ENABLED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
		SendMessageW(m_hTbrMain, TB_SETSTATE, IDM_PASTE, 0);
	}
	if(g_hFind){
		EnableMenuItem(g_hMenu, IDM_FIND, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_FIND_NEXT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(g_hMenu, IDM_REPLACE, MF_BYCOMMAND | MF_GRAYED);
	}
	else{
		EnableMenuItem(g_hMenu, IDM_FIND, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_FIND_NEXT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(g_hMenu, IDM_REPLACE, MF_BYCOMMAND | MF_ENABLED);
	}
}

static void CreateRebarBackground(HWND hwnd){
	HDC				hdcMain, hdc;
	HBITMAP			hbm, hbmOld;
	RECT			rc;
	int				y;

	GetClientRect(m_hTbrMain, &rc);
	y = rc.bottom - rc.top;
	hdcMain = GetDC(hwnd);
	hdc = CreateCompatibleDC(hdcMain);
	hbm = CreateCompatibleBitmap(hdcMain, 1, y);
	SetRect(&rc, 0, 0, 1, y);
	SetBkMode(hdc, TRANSPARENT);
	hbmOld = SelectObject(hdc, hbm);
	
	if(g_VSEnabled){
		COLORREF		clr1, clr2;
		clr1 = clr2 = GetSysColor(COLOR_BTNFACE);
		clr1 = ColorAdjustLuma(clr1, 195, FALSE);
		clr2 = ColorAdjustLuma(clr2, -195, FALSE);
		Fill2ColorsRectangle(hdc, &rc, clr1, clr2, GRADIENT_FILL_RECT_V);
	}
	else{
		FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
	}
	SelectObject(hdc, hbmOld);
	if(m_hRBack){
		DeleteObject(m_hRBack);
	}
	m_hRBack = hbm;
	DeleteDC(hdc);
	ReleaseDC(hwnd, hdcMain);
}

static void DrawTabItem(const DRAWITEMSTRUCT * lpDrawItem){
	int				state = 0;
	RECT			rc;
	TCHITTESTINFO	tch;
	int				index;
	COLORREF		clr1, clr2, clrTextOld = 0;
	wchar_t			szBuffer[128];

	CopyRect(&rc, &lpDrawItem->rcItem);
	SetBkMode(lpDrawItem->hDC, TRANSPARENT);

	clr1 = clr2 = GetSysColor(COLOR_BTNFACE);
	
	GetCursorPos(&tch.pt);
	MapWindowPoints(HWND_DESKTOP, m_hTabMain, &tch.pt, 1);
	index = TabCtrl_HitTest(m_hTabMain, &tch);

	if((lpDrawItem->itemState & ODS_SELECTED) == ODS_SELECTED)
		state = ODS_SELECTED;

	if(state == 0){
		rc.bottom += 2;
		if(index == lpDrawItem->itemID){
			clrTextOld = SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_BTNTEXT));
			if(g_VSEnabled){
				clr1 = ColorAdjustLuma(clr1, 195, FALSE);
				clr2 = ColorAdjustLuma(clr2, -195, FALSE);
				Fill2ColorsRectangle(lpDrawItem->hDC, &rc, clr1, clr2, GRADIENT_FILL_RECT_V);
			}
			else{
				FillRect(lpDrawItem->hDC, &rc, GetSysColorBrush(COLOR_BTNFACE));
			}
			ImageList_Draw(g_ImlTab, 1, lpDrawItem->hDC, rc.right - 20, rc.top + 4, ILD_TRANSPARENT);
		}
		else{
			clrTextOld = SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_GRAYTEXT));
			if(g_VSEnabled){
				clr1 = ColorAdjustLuma(clr1, 35, FALSE);
				clr2 = ColorAdjustLuma(clr2, -95, FALSE);
				Fill2ColorsRectangle(lpDrawItem->hDC, &rc, clr1, clr2, GRADIENT_FILL_RECT_V);
			}
			else{
				FillRect(lpDrawItem->hDC, &rc, GetSysColorBrush(COLOR_BTNFACE));
			}
			ImageList_Draw(g_ImlTab, 2, lpDrawItem->hDC, rc.right - 20, rc.top + 4, ILD_TRANSPARENT);
		}
	}
	else if(state == ODS_SELECTED){
		if(g_VSEnabled){
			clr1 = ColorAdjustLuma(clr1, 95, FALSE);
			clr2 = ColorAdjustLuma(clr2, -95, FALSE);
			Fill2ColorsRectangle(lpDrawItem->hDC, &rc, clr1, clr2, GRADIENT_FILL_RECT_V);
		}
		else{
			rc.top +=1;
			DrawFrameControl(lpDrawItem->hDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH); 
		}
		clrTextOld = SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_BTNTEXT));
		ImageList_Draw(g_ImlTab, 0, lpDrawItem->hDC, rc.right - 20, rc.top + 4, ILD_TRANSPARENT);
	}	
	DrawIconEx(lpDrawItem->hDC, rc.left + 4, rc.top + 4, GetSmallIcon(lpDrawItem->itemID), 16, 16, 0, 0, DI_NORMAL);

	SetRect(&rc, rc.left + 24, rc.top, rc.right - 24, rc.bottom);
	GetShortName(lpDrawItem->itemID, szBuffer, TRUE);
	DrawTextW(lpDrawItem->hDC, szBuffer, -1, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

	if(clrTextOld)
		SetTextColor(lpDrawItem->hDC, clrTextOld);
}

static HICON GetSmallIcon(int index){
	TCITEMW		ti;
	P_TPEDIT	pE;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	SendMessageW(m_hTabMain, TCM_GETITEMW, index, (LPARAM)&ti);
	if(ti.lParam){
		pE = (P_TPEDIT)ti.lParam;
		return pE->hIcon;
	}
	return NULL;
}

static void GetShortName(int index, wchar_t * lpName, BOOL fWithStar){
	TCITEMW		ti;
	P_TPEDIT	pE;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	SendMessageW(m_hTabMain, TCM_GETITEMW, index, (LPARAM)&ti);
	if(ti.lParam){
		*lpName = '\0';
		pE = (P_TPEDIT)ti.lParam;
		if(fWithStar){
			if(pE->changed){
				wcscat(lpName, L"*");
			}
		}
		wcscat(lpName, pE->szShortName);
	}
}

static void GetLongName(int index, wchar_t * lpName){
	TCITEMW		ti;
	P_TPEDIT	pE;

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	SendMessageW(m_hTabMain, TCM_GETITEMW, index, (LPARAM)&ti);
	if(ti.lParam){
		pE = (P_TPEDIT)ti.lParam;
		wcscpy(lpName, pE->szLongName);
	}
}

static void ResetUntitledString(void){
	int				count;
	TCITEMW			ti;
	P_TPEDIT		pE;

	count = TabCtrl_GetItemCount(m_hTabMain);
	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE){
			if(pE->status == ST_NEW){
				wcscpy(pE->szShortName, g_Strings.sUntitled);
				wcscpy(pE->szLongName, g_Strings.sUntitled);
			}
		}
	}
	RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
}

static void ShowCurrentPositionRegular(HWND hEdit, CHARRANGE chrg){
	long 			x, y; //, pos, last, count, charCount;
	// RECT			rc;
	// GETTEXTLENGTHEX	gtx;

	// gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
	// gtx.codepage = 1200;
	// charCount = SendMessageW(hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	// count = RichEdit_GetLineCount(hEdit);
	// SendMessageW(hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	// rc.top = rc.bottom;
	// pos = SendMessageW(hEdit, EM_CHARFROMPOS, 0, (LPARAM)&rc);

	// last = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, pos);
	// x = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin);
	// y = chrg.cpMin;
	// y -= SendMessageW(hEdit, EM_LINEINDEX, x, 0);
	// y++;
	// x++;
	// if(count - last == 2 && chrg.cpMin == charCount){
		// x++;
		// y = 1;
	// }
	GetCurrentPositionRegular(hEdit, chrg, &x, &y);
	PrintCurrentPosition(x, y);
}

static void ShowCurrentPositionWrap(HWND hEdit, CHARRANGE chrg){
	long 			x, y; //, pos, last, count, charCount;
	// TEXTRANGEW		trg;
	// long			crCount = 0, crChar = 0;
	// RECT			rc;
	// GETTEXTLENGTHEX	gtx;
	// HANDLE			* pTemp;

	// //get total number of chars in control
	// gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
	// gtx.codepage = 1200;
	// charCount = SendMessageW(hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	// count = RichEdit_GetLineCount(hEdit);
	
	// //find last visible line start position
	// SendMessageW(hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	// rc.top = rc.bottom;
	// pos = SendMessageW(hEdit, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	// last = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, pos);

	// //get pseudo line index
	// x = SendMessageW(hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin);

	// trg.chrg.cpMin = 0;
	// trg.chrg.cpMax = chrg.cpMin;
	// trg.lpstrText = (wchar_t *)HeapAlloc(g_hHeap, 0, (trg.chrg.cpMax + 1) * sizeof(wchar_t));
	// pTemp = (HANDLE)trg.lpstrText;
	// SendMessageW(hEdit, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
	// for(int i = 0; i <= trg.chrg.cpMax; i++){
		// if(*trg.lpstrText++ == 13){
			// //increase CR count
			// crCount++;
			// //store CR position + 1: start of next line
			// crChar = i + 1;
		// }
	// }
	// HeapFree(g_hHeap, 0, pTemp);

	// if(crCount == 0){
		// //store character index as column
		// y = chrg.cpMin + 1;
	// }
	// else{
		// //get difference between start position of selection and start position of line
		// y = chrg.cpMin - crChar + 1;
	// }
	// x = crCount + 1;

	// //in case of Enter after last line
	// if(count - last == 2 && chrg.cpMin == charCount){
		// y = 1;
	// }
	GetCurrentPositionWrap(hEdit, chrg, &x, &y);
	PrintCurrentPosition(x, y);
}

static void PrintCurrentPosition(int x, int y){
	wchar_t			row[128], col[128], szBuffer[256];

	_itow(x, row, 10);
	_itow(y, col, 10);
	wcscpy(szBuffer, g_Strings.sLine);
	wcscat(szBuffer, L" ");
	wcscat(szBuffer, row);
	wcscat(szBuffer, L", ");
	wcscat(szBuffer, g_Strings.sColumn);
	wcscat(szBuffer, L" ");
	wcscat(szBuffer, col);
	SendMessageW(m_hStatusMain, SB_SETTEXTW, 2 | SBT_POPOUT, (LPARAM)szBuffer);
}

static int GetMenuPosition(HMENU hMenu, int id){
	int				count = GetMenuItemCount(hMenu);
	MENUITEMINFOW	mi;

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_ID;
	for(int i = 0; i < count; i++){
		GetMenuItemInfoW(hMenu, i, TRUE, &mi);
		if(mi.wID == id){
			return i;
		}
	}
	return -1;
}

static void AddWindowMenu(HMENU hMenu, int index, wchar_t * lpText, BOOL fCheck){
	MItem		mit;

	ZeroMemory(&mit, sizeof(mit));
	mit.xPos = -1;
	mit.yPos = -1;
	mit.xCheck = 40;
	mit.id = WIN_MENU_MIN + index;
	wcscpy(mit.szText, lpText);
	if(GetMenuItemCount(hMenu) == 3){
		AppendMenuW(hMenu, MF_SEPARATOR | MF_OWNERDRAW, 0, NULL);
	}
	AppendMenuW(hMenu, MF_STRING, mit.id, lpText);
	SetMenuItemProperties(&mit, hMenu, GetMenuPosition(hMenu, mit.id));
	if(fCheck)
		CheckWindowMenu(hMenu, mit.id);
}

static void ClearWindowMenu(void){
	HMENU		hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));
	int			count = GetMenuItemCount(hMenu);

	for(int i = count - 1; i >= 3; i--){
		FreeSingleMenu(hMenu, i);
		DeleteMenu(hMenu, i, MF_BYPOSITION);
	}
}

static void RecreateWindowMenu(void){
	HMENU		hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));
	int			count = TabCtrl_GetItemCount(m_hTabMain);
	wchar_t		szBuffer[128];

	ClearWindowMenu();

	for(int i = 0; i < count; i++){
		GetLongName(i, szBuffer);
		AddWindowMenu(hMenu, i, szBuffer, FALSE);
	}
	CheckWindowMenu(hMenu, WIN_MENU_MIN + TabCtrl_GetCurSel(m_hTabMain));
}

static void CheckWindowMenu(HMENU hMenu, int id){
	int				count;
	
	count = GetMenuItemCount(hMenu);
	for(int i = 4; i <= count; i++){
		if(GetMenuItemID(hMenu, i) == id)
			CheckMenuItem(hMenu, i, MF_BYPOSITION | MF_CHECKED);
		else
			CheckMenuItem(hMenu, i, MF_BYPOSITION | MF_UNCHECKED);
	}
}

static BOOL OSFileDialog(wchar_t * lpCaption, wchar_t * lpNameFull, wchar_t * lpNameShort, BOOL fOpen, BOOL fUntitled){
	OPENFILENAMEW		ofn;

	if(fOpen){
		*lpNameFull = '\0';
	}
	*lpNameShort = '\0';
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hMain;
	ofn.hInstance = g_hInstance;
	ofn.lpstrTitle = lpCaption;
	if(!fOpen){
		if(!fUntitled){
			wchar_t		szTemp[MAX_PATH];
			wcscpy(szTemp, lpNameFull);
			GetFileNameFromPath(szTemp, lpNameFull);
		}
	}
	ofn.lpstrFile = lpNameFull;
	ofn.lpstrFileTitle = lpNameShort;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	if(fOpen){
		ofn.lpstrFilter = OPEN_FILTER;
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	}
	else{
		ofn.lpstrDefExt = L"txt";
		ofn.lCustData = GetActiveHandle()->type;
		m_CurrType = ofn.lCustData;
		ofn.lpstrFilter = SAVE_FILTER;
		ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_OVERWRITEPROMPT;
		ofn.lpfnHook = OFNHookProc;
	}
	if(fOpen)
		return GetOpenFileNameW(&ofn);
	else
		return GetSaveFileNameW(&ofn);
}

static void GetAppSmallIcon(void){
	wchar_t				szPath[MAX_PATH];
	SHFILEINFOW			shf;

	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	ZeroMemory(&shf, sizeof(shf));
	SHGetFileInfoW(szPath, 0, &shf, sizeof(shf), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
	m_hSmallIcon = shf.hIcon;
}

static BOOL LoadFile(wchar_t * lpPath, P_TPEDIT pE){
	EDITSTREAM			eStream;
	HANDLE				hFile;
	int					read, flags = 0;
	BYTE				bytes[4];
	WIN32_FIND_DATAW	fd;
	TPREAD				tpr;

	hFile = CreateFileW(lpPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE){
		ReadFile(hFile, bytes, 3, (PULONG)&read, NULL);
		if(bytes[0] == 0xff && bytes[1] == 0xfe){
			flags = SF_TEXT | SF_UNICODE;
			SetFilePointer(hFile, -1, 0, FILE_CURRENT);
			pE->type = CODE_UNICODE;
		}
		else if(bytes[0] == 0xfe && bytes[1] == 0xff){
			flags = SF_TEXT | SF_UNICODE;
			SetFilePointer(hFile, -1, 0, FILE_CURRENT);
			pE->type = CODE_UNICODE_BE;
		}
		else if(bytes[0] == 0xef && bytes[1] == 0xbb && bytes[2] == 0xbf){
			flags = (CP_UTF8 << 16) | SF_USECODEPAGE | SF_TEXT;
			pE->type = CODE_UTF8;
		}
		else{
			flags = SF_TEXT;
			SetFilePointer(hFile, -3, 0, FILE_CURRENT);
			pE->type = CODE_TEXT;
		}
		tpr.hHandle = hFile;
		tpr.type = pE->type;
		eStream.dwCookie = (DWORD)&tpr;
		eStream.pfnCallback = ReadStreamCallback;
		eStream.dwError = 0;
		RichEdit_StreamIn(pE->hEdit, flags, &eStream);
		SendMessageW(pE->hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
		SendMessageW(pE->hEdit, EM_SETMODIFY, FALSE, 0);
		pE->changed = FALSE;
		pE->status = ST_FILE;
		CloseHandle(hFile);
		SetSmallIcon(pE, lpPath);
		ZeroMemory(&fd, sizeof(fd));
		hFile = FindFirstFileW(lpPath, &fd);
		if(hFile != INVALID_HANDLE_VALUE){
			memcpy(&pE->ft, &fd.ftLastWriteTime, sizeof(FILETIME));
			FindClose(hFile);
		}
	}
	else{
		return FALSE;
	}
	return TRUE;
}

static void SetSmallIcon(P_TPEDIT pE, wchar_t * lpPath){

	SHFILEINFOW			shf;

	if(pE->hIcon && pE->status == ST_FILE)
		DestroyIcon(pE->hIcon);
	ZeroMemory(&shf, sizeof(shf));
	SHGetFileInfoW(lpPath, 0, &shf, sizeof(shf), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
	pE->hIcon = shf.hIcon;
	SendMessageW(pE->hChild, WM_SETICON, ICON_SMALL, (LPARAM)pE->hIcon);
	DrawMenuBar(g_hMain);
}

static void ReMaxChild(void){
	HWND		hChild;
	BOOL		max;

	hChild = (HWND)SendMessageW(g_hClient, WM_MDIGETACTIVE, 0, (LPARAM)&max);
	if(max){
		//if child is maximized - restore and maximize in order to change main window caption
		SendMessageW(g_hClient, WM_MDIRESTORE, (WPARAM)hChild, 0);
		SendMessageW(g_hClient, WM_MDIMAXIMIZE, (WPARAM)hChild, 0);
	}
}

static BOOL IsFileDropped(wchar_t * lpPath){
	SHFILEINFOW			shf;

	ZeroMemory(&shf, sizeof(shf));
	SHGetFileInfoW(lpPath, 0, &shf, sizeof(shf), SHGFI_TYPENAME);
	if(_wcsicmp(shf.szTypeName, L"File Folder") == 0)
		return FALSE;
	return TRUE;
}

static LRESULT CALLBACK OFNParentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	if(msg == WM_COMMAND){
		if(HIWORD(wParam) == CBN_SELENDOK){
			if((HWND)lParam == m_hSelCombo){
				m_CurrType = SendMessageW(m_hSelCombo, CB_GETCURSEL, 0, 0);
			}
		}
	}
	return CallWindowProcW((WNDPROC)GetWindowLongPtrW(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
}

static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam){

	HWND				hParent, hStatic;
	WINDOWPLACEMENT		wp;
	RECT				rc, rc1, rc2;
	int					of, h, exStyle, style, top, width, height;
	HFONT				hFont;
	OPENFILENAMEW		* pof;

	if(uiMsg == WM_INITDIALOG){
		hParent = GetParent(hdlg);
		ZeroMemory(&wp, sizeof(wp));
		wp.length = sizeof(wp);
		GetWindowPlacement(hParent, &wp);
		GetWindowRect(GetDlgItem(hParent, cmb1), &rc);
		GetWindowRect(GetDlgItem(hParent, edt1), &rc1);
		of = rc.top - rc1.bottom;
		h = rc.bottom - rc.top;
		GetClientRect(hParent, &rc2);
		rc2.bottom += h;
		exStyle = GetWindowLongPtrW(hParent, GWL_EXSTYLE);
		style = GetWindowLongPtrW(hParent, GWL_STYLE);
		AdjustWindowRectEx(&rc2, style, FALSE, exStyle);
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top;
		wp.rcNormalPosition.bottom += rc2.bottom - rc2.top;
		MapWindowPoints(HWND_DESKTOP, hParent, (LPPOINT)&rc, 2);
		top = rc.bottom + of;
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		m_hSelCombo = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", NULL, CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_TABSTOP, rc.left, top, width, 400, hParent, NULL, g_hInstance, NULL);
		GetWindowRect(GetDlgItem(hParent, stc2), &rc);
		MapWindowPoints(HWND_DESKTOP, hParent, (LPPOINT)&rc, 2);
		width = rc.right - rc.left;
		hStatic = CreateWindowExW(0, L"STATIC", L"Encoding:", SS_CENTERIMAGE | WS_CHILD | WS_VISIBLE, rc.left, top, width, height, hParent, NULL, g_hInstance, NULL);
		hFont = (HFONT)SendMessageW(hParent, WM_GETFONT, 0, 0);
		SendMessageW(m_hSelCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageW(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
		SetWindowPlacement(hParent, &wp);
		SendMessageW(m_hSelCombo, CB_ADDSTRING, 0, (LPARAM)ENC1);
		SendMessageW(m_hSelCombo, CB_ADDSTRING, 0, (LPARAM)ENC2);
		SendMessageW(m_hSelCombo, CB_ADDSTRING, 0, (LPARAM)ENC3);
		SendMessageW(m_hSelCombo, CB_ADDSTRING, 0, (LPARAM)ENC4);
		pof = (OPENFILENAMEW *)lParam;
		SendMessageW(m_hSelCombo, CB_SETCURSEL, (WPARAM)pof->lCustData, 0);
		SetWindowLongPtrW(hParent, GWLP_USERDATA, (LONG_PTR)SetWindowLongPtrW(hParent, GWLP_WNDPROC, (LONG_PTR)OFNParentProc));
	}
	return FALSE;
}

static void GetFileNameFromPath(wchar_t * lpPath, wchar_t * lpName){
	wcscpy(lpName, lpPath);
	PathStripPathW(lpName);
	// while(*lpPath++)
		// ;
	// while(*lpPath != '\\')
		// *lpPath--;
	// lpPath++;
	// wcscpy(lpName, lpPath);
}

static P_TPEDIT LoadDocument(HWND hMain, wchar_t * lpNameFull, wchar_t * lpNameShort, P_TPEDIT pE, BOOL fAddTab){
	P_TPEDIT		tpe;

	m_CheckOuter = FALSE;
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	// LockWindowUpdate(hMain);
	if(fAddTab){
		tpe = AddTab(lpNameShort, lpNameFull);
		SendMessageW(tpe->hEdit, WM_SETREDRAW, FALSE, 0);
		LoadFile(lpNameFull, tpe);
		SendMessageW(tpe->hEdit, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(tpe->hEdit, NULL, FALSE);
		if(GetActiveEdit() == tpe->hEdit){
			SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)tpe->szLongName);
			SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)GetATFileType(tpe));
		}
	}
	else{
		//clear possible search results
		SendMessageW(g_hSplit, TBNPM_REMOVE_RESULTS, (WPARAM)pE->hEdit, 0);
		SendMessageW(pE->hEdit, WM_SETREDRAW, FALSE, 0);
		LoadFile(lpNameFull, pE);
		SendMessageW(pE->hEdit, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(pE->hEdit, NULL, FALSE);
		wcscpy(pE->szLongName, lpNameFull);
		wcscpy(pE->szShortName, lpNameShort);
		SetWindowTextW(pE->hChild, pE->szShortName);
		if(GetActiveEdit() == pE->hEdit){
			SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)pE->szLongName);
			SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)GetATFileType(pE));
		}
	}
	RedrawWindow(m_hTabMain, NULL, NULL, RDW_INVALIDATE);
	// LockWindowUpdate(NULL);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	m_CheckOuter = TRUE;
	if(fAddTab)
		return tpe;
	else
		return pE;
}

static wchar_t * GetATFileType(P_TPEDIT pE){
	//return document encoding string
	switch(pE->type){
	case CODE_TEXT:
		return ENC1;
	case CODE_UNICODE:
		return ENC2;
	case CODE_UNICODE_BE:
		return ENC3;
	case CODE_UTF8:
		return ENC4;
	default:
		return NULL;
	}
}

static void CheckForOuterChanges(void){
	//check for changes may be done out of program
	P_TPEDIT			pE;
	HANDLE				hFile;
	WIN32_FIND_DATAW	fd;
	BOOL				prevCheck, isBookmark = FALSE;
	CHARRANGE			chrg;
	P_TPBMRK			pTemp;

	pE = GetActiveHandle();
	if(pE->status == ST_FILE){
		ZeroMemory(&fd, sizeof(fd));
		//get file last write time
		hFile = FindFirstFileW(pE->szLongName, &fd);
		if(hFile != INVALID_HANDLE_VALUE){
			FindClose(hFile);
			if(pE->ft.dwHighDateTime != fd.ftLastWriteTime.dwHighDateTime || pE->ft.dwLowDateTime != fd.ftLastWriteTime.dwLowDateTime){
				//if there is a diffence in times - prompt to reload
				wchar_t		szMessage[MAX_PATH * 3];
				wcscpy(szMessage, pE->szShortName);
				wcscat(szMessage, L" ");
				wcscat(szMessage, g_Strings.sOutChanges1);
				wcscat(szMessage, L"\n");
				wcscat(szMessage, g_Strings.sOutChanges2);
				wcscat(szMessage, L"\n");
				wcscat(szMessage, g_Strings.sOutChanges3);
				//prevent checking for outer changes
				prevCheck = m_CheckOuter;
				m_CheckOuter = FALSE;
				if(MessageBoxW(g_hMain, szMessage, PROGRAM_NAME, MB_YESNO | MB_ICONQUESTION) == IDYES){
					if(g_Settings.preservePosition == POS_SAVED){
						//save text position
						SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
					}
					if(pE->bookmarks){
						//preserve bookmarks
						isBookmark = TRUE;
						pTemp = pE->bookmarks;
						pE->bookmarks = NULL;
					}
					LoadDocument(g_hMain, pE->szLongName, pE->szShortName, pE, FALSE);
					if(g_Settings.preservePosition == POS_SAVED){
						//restore text position
						SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
					}
					else if(g_Settings.preservePosition == POS_END){
						//jump to the end of document
						int		count = RichEdit_GetLineCount(pE->hEdit);
						chrg.cpMin = SendMessageW(pE->hEdit, EM_LINEINDEX, count - 1, 0);
						chrg.cpMax = chrg.cpMin;
						SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
					}
					if(isBookmark){
						//restore bookmarks
						pE->bookmarks = pTemp;
						RecheckBookmarks(pE);
						InvalidateRect(pE->hEdit, NULL, FALSE);
					}
				}
				//restore outer checking flag
				m_CheckOuter = prevCheck;
			}
		}
	}
}

static BOOL LoadInSameTab(wchar_t * lpNameFull, wchar_t * lpNameShort){
	//load document in current tab instead of creating new one
	BOOL		result = FALSE;
	P_TPEDIT	pE;

	//check for changes in previously open document
	result = AskOnClose(GetActiveHandle());
	if(result){
		pE = GetActiveHandle();
		//clean bookmarks
		CleanBookmarks(pE);
		//load document
		LoadDocument(g_hMain, lpNameFull, lpNameShort, pE, FALSE);
		//change child window caption
		SetWindowTextW(pE->hChild, lpNameShort);
	}

	return result;
}

static void RecheckBookmarks(P_TPEDIT pE){
	//check whether there are bookmarks with line number more than line count of rich edit and remove them
	P_TPBMRK		pB;
	long			count, *pLines, *pTemp, *pAdd;
	
	count = RichEdit_GetLineCount(pE->hEdit);
	pB = pE->bookmarks;
	pLines = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, BOOKMARKS_MAX * sizeof(long));
	pTemp = pAdd = pLines;
	while(pB){
		if(pB->line >= count){
			*pAdd = pB->line + 1;
			pAdd++;
		}
		pB = pB->next;
	}
	while(*pTemp){
		//send decreased line (previously increased by 1)
		RemoveBookmark(pE, *pTemp - 1);
		pTemp++;
	}
	HeapFree(g_hHeap, 0, pLines);
}

static void CloseApplication(int mode){
	//close application from command line switches
	HWND		hwnd = NULL;

	//get previous instance main window
	EnumWindows(EnumWindowsProc, (LPARAM)&hwnd);
	if(hwnd){
		if(mode == ARG_EXIT){
			//simple exit
			SendMessageW(hwnd, WM_CLOSE, 0, 0);
		}
		else{
			if(mode == ARG_SILENT){
				//exit with silent saving
				m_CheckOuter = FALSE;
				SendMessageW(hwnd, TBNPM_CLOSEPARAM, 0, 0);
				SendMessageW(hwnd, TBNPM_SAVEONCLOSE, 0, 0);
				SendMessageW(hwnd, WM_CLOSE, 0, 0);
			}
			else if(mode == ARG_NO_SAVE){
				//exit without saving
				SetPropW(hwnd, P_SAVE_ON_EXIT, (HANDLE)0);
				SendMessageW(hwnd, WM_CLOSE, 0, 0);
			}
		}
	}
}

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam){
	//call this procedure when running new instance of ATPad with parameters
	wchar_t			szClass[256];

	GetClassNameW(hwnd, szClass, 256);
	if(wcscmp(szClass, PROG_CLASS) == 0){
		//return handle to main window of previous instance
		*(HWND *)lParam = hwnd;
		return FALSE;
	}
	return TRUE;
}

static void GetTempSaveName(wchar_t * lpLongName, wchar_t * lpTempSave, BOOL fGetExtension){
	//prepare temporary file name
	wchar_t			szTempName[128], * pName;

	GetTempPathW(MAX_PATH, lpTempSave);
	_ltow(GetTickCount(), szTempName, 10);
	if(fGetExtension){
		pName = PathFindExtensionW(lpLongName);
		wcscat(szTempName, pName);
	}
	else{
		wcscat(szTempName, L".txt");
	}
	wcscat(lpTempSave, szTempName);
}

static void DrawToolbarButton(HDC hdc, LPRECT lprc, int state, int id){
	//draw custom toolbar button
	TBBUTTONINFOW		tbi;
	int					x, y, cx, cy;
	COLORREF			clr1, clr2;
	HBRUSH				hBrush;

	ZeroMemory(&tbi, sizeof(tbi));
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_IMAGE | TBIF_COMMAND;
	tbi.idCommand = id;
	SendMessageW(m_hTbrMain, TB_GETBUTTONINFOW, (WPARAM)id, (LPARAM)&tbi);
	hBrush = CreateSolidBrush(m_clrFrame);

	if((state & CDIS_SELECTED) == 0){
		clr1 = clr2 = m_clrHot;
		clr1 = ColorAdjustLuma(clr1, 195, FALSE);
		clr2 = ColorAdjustLuma(clr2, -195, FALSE);
		Fill2ColorsRectangle(hdc, lprc, clr1, clr2, GRADIENT_FILL_RECT_V);
	}
	else{
		clr1 = clr2 = m_clrSel;
		clr1 = ColorAdjustLuma(clr1, 195, FALSE);
		clr2 = ColorAdjustLuma(clr2, -195, FALSE);
		Fill2ColorsRectangle(hdc, lprc, clr2, clr1, GRADIENT_FILL_RECT_V);
	}

	FrameRect(hdc, lprc, hBrush);
	ImageList_GetIconSize(m_ImlNormal, &cx, &cy);
	x = lprc->left + ((lprc->right - lprc->left) - cx) / 2;
	y = lprc->top + ((lprc->bottom - lprc->top) - cy) / 2;
	ImageList_Draw(m_ImlNormal, tbi.iImage, hdc, x, y, ILD_TRANSPARENT);
	DeleteObject(hBrush);
}

static LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	LPNMHDR			lpnm;
	P_TPEDIT		pE;

	switch(msg){
	HANDLE_MSG (hwnd, WM_CREATE, Child_OnCreate);
	HANDLE_MSG (hwnd, WM_SIZE, Child_OnSize);
	HANDLE_MSG (hwnd, WM_MDIACTIVATE, Child_OnMDIActivate);
	HANDLE_MSG (hwnd, WM_DESTROY, Child_OnDestroy);
	HANDLE_MSG (hwnd, WM_CLOSE, Child_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, Child_OnCommand);

	case WM_NOTIFY:{
		lpnm = (LPNMHDR)lParam;
		ENPROTECTED	*lppr;
		
		switch(lpnm->code){
			case EN_PROTECTED:
				//fires on each change attempt - this allows us to set up some important properties before the contents of rich edit is changed
				lppr = (ENPROTECTED *)lParam;
				pE = GetHandleByChild(hwnd);
				memcpy(&pE->range, &lppr->chrg, sizeof(CHARRANGE));
				pE->lines = RichEdit_GetLineCount(lppr->nmhdr.hwndFrom);
				if(pE->bookmarks)
					SetBookmarkInRange(pE);
				//return FALSE - allow changes to be processed
				return FALSE;
			case EN_SELCHANGE:{
				SELCHANGE	* psch = (SELCHANGE *)lParam;
				if(m_ShowPosition){
					if(!g_Settings.wordWrap)
						ShowCurrentPositionRegular(lpnm->hwndFrom, psch->chrg);
					else
						ShowCurrentPositionWrap(lpnm->hwndFrom, psch->chrg);
				}
				return TRUE;
			}
			case EN_MSGFILTER:{
				HMENU		hMenu = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_EDIT));
				HMENU 		h2 = GetSubMenu(hMenu, GetMenuPosition(hMenu, IDM_REFRESH_EVERY));
				MSGFILTER	* lpmsg = (MSGFILTER *)lParam;
				POINT		pt;
				
				if(lpmsg->msg == WM_RBUTTONUP){
					pE = GetHandleByChild(hwnd);
					if(pE->status == ST_FILE){
						if(pE->timerOn){
							//select update interval
							CheckMenuRadioItem(h2, REFRESH_MENU_MIN, REFRESH_MENU_MIN + 1800, REFRESH_MENU_MIN + pE->interval, MF_BYCOMMAND);
						}
						else{
							//timer off - uncheck appropriate menu item
							CheckMenuRadioItem(h2, REFRESH_MENU_MIN, REFRESH_MENU_MIN + 1800, REFRESH_MENU_MIN, MF_BYCOMMAND);
						}
					}
					pt.x = LOWORD(lpmsg->lParam);
					pt.y = HIWORD(lpmsg->lParam);
					MapWindowPoints(pE->hEdit, HWND_DESKTOP, &pt, 1);
					TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, g_hMain, NULL);
					return TRUE;
				}
				else
					return FALSE;
				}
			case EN_LINK:{
				ENLINK * lpel = (ENLINK *)lParam;
				if(lpel->msg == WM_LBUTTONUP){
					wchar_t 	szBuffer[256];
					TEXTRANGEW	tr;
					
					pE = GetHandleByChild(hwnd);
					tr.chrg.cpMax = lpel->chrg.cpMax;
					tr.chrg.cpMin = lpel->chrg.cpMin;
					tr.lpstrText = szBuffer;
					SendMessageW(pE->hEdit, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					if(_wcsistr(szBuffer, L"http:") == szBuffer || _wcsistr(szBuffer, L"ftp:") == szBuffer || _wcsistr(szBuffer, L"https:") == szBuffer || _wcsistr(szBuffer, L"gopher:") == szBuffer || _wcsistr(szBuffer, L"www.") == szBuffer){
						if(wcslen(g_sDefBrowser) == 0)
							ShellExecuteW(hwnd, L"open", szBuffer, NULL, NULL, SW_SHOWDEFAULT);
						else{
							if((int)ShellExecuteW(hwnd, NULL, g_sDefBrowser, szBuffer, NULL, SW_SHOWDEFAULT) <= 32){
								ShellExecuteW(hwnd, L"open", szBuffer, NULL, NULL, SW_SHOWDEFAULT);
							}
						}
					}
					else{
						ShellExecuteW(hwnd, L"open", szBuffer, NULL, NULL, SW_SHOWDEFAULT);
					}
				}
				return FALSE;
			}
		}
	}
	default:
		return DefMDIChildProcW(hwnd, msg, wParam, lParam);
	}
}

static void Child_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	long			lines;
	long			* pLines, * pTemp, * pAdd;
	P_TPEDIT		pE;
	P_TPBMRK		pB;
	GETTEXTLENGTHEX	gtx;
	CHARRANGE		chrg;

	switch(codeNotify){
	case EN_UPDATE:
		pE = GetHandleByChild(hwnd);
		lines = RichEdit_GetLineCount(pE->hEdit);
		if(pE->bookmarks){
			gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
			gtx.codepage = 1200;
			if(lines == 1 && SendMessageW(pE->hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0) == 0){
				//no text in rich edit - clear all bookmarks
				CleanBookmarks(pE);
			}
			else if(lines > pE->lines){
				pB = pE->bookmarks;
				pLines = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, 256 * sizeof(long));
				pTemp = pAdd = pLines;
				while(pB){
					if(pB->inRange == 1 || pB->inRange == 2){
						//bookmark is below the selection or the caret is at the beginning of bookmark line - increase it's line
						pB->line += lines - pE->lines;
					}
					else if(pB->inRange == 0){
						//bookmark is in selection - delete it (add line + 1 because of further checking for NULL)
						*pAdd = pB->line + 1;
						pAdd++;
					}
					pB = pB->next;
				}
				while(*pTemp){
					//send decreased line (previously increased by 1)
					RemoveBookmark(pE, *pTemp - 1);
					pTemp++;
				}
				HeapFree(g_hHeap, 0, pLines);
			}
			else if(lines < pE->lines){
				SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
				// selLine = SendMessageW(pE->hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin); 
				pB = pE->bookmarks;
				pLines = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, 256 * sizeof(long));
				pTemp = pAdd = pLines;
				while(pB){
					if(pB->inRange == 1){
						//bookmark is below the selection - decrease it's line
						pB->line -= pE->lines - lines;
					}
					else if(pB->inRange == 0 || pB->inRange == 2){
						//bookmark is in selection or the caret is at the beginning of bookmark line - delete it (add line + 1 because of further checking for NULL)
						*pAdd = pB->line + 1;
						pAdd++;
					}
					pB = pB->next;
				}
				while(*pTemp){
					//send decreased line (previously increased by 1)
					RemoveBookmark(pE, *pTemp - 1);
					pTemp++;
				}
				HeapFree(g_hHeap, 0, pLines);
			}
			pE->lines = lines;
			SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&pE->range);
			SetBookmarkInRange(pE);
		}
		else{
			pE->lines = lines;
		}
		break;
	}
}

static void Child_OnClose(HWND hwnd)
{
	if(AskOnClose(GetHandleByChild(hwnd))){
		//destroy window if user don't chose 'Cancel'
		SendMessageW(g_hClient, WM_MDIDESTROY, (WPARAM)hwnd, 0);
	}
}

static void Child_OnDestroy(HWND hwnd)
{
	P_TPEDIT		pE;

	pE = GetHandleByChild(hwnd);
	//clear possible search results
	SendMessageW(g_hSplit, TBNPM_REMOVE_RESULTS, (WPARAM)pE->hEdit, 0);
	//close appropriate tab
	CloseTab(pE);
}

static void Child_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate)
{
	P_TPEDIT		pE;
	HMENU			hWinM = GetSubMenu(g_hMenu, GetMenuPosition(g_hMenu, IDM_WINDOW));

	if(hwndActivate == hwnd){
		//if child window is getting focus
		pE = GetHandleByChild(hwnd);
		//select appropriate tab
		TabCtrl_SetCurSel(m_hTabMain, pE->tabIndex);
		//set focus to appropriate rich edit
		SetFocus(pE->hEdit);
		//check appropriate window menu
		CheckWindowMenu(hWinM, WIN_MENU_MIN + pE->tabIndex);
		//set status bar text
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 0 | SBT_POPOUT, (LPARAM)pE->szLongName);
		SendMessageW(m_hStatusMain, SB_SETTEXTW, 1 | SBT_POPOUT, (LPARAM)GetATFileType(pE));
	}
}

static void Child_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	P_TPEDIT		pE;

	//call default resizing procedure
	DefMDIChildProcW(hwnd, WM_SIZE, MAKEWPARAM(state, 0), MAKELPARAM(cx, cy));
	pE = GetHandleByChild(hwnd);
	//resize rich edit
	MoveWindow(pE->hEdit, 0, 0, cx, cy, TRUE);
	//resize lines numbering status
	ResizeStatic(pE, cy);
}

static BOOL Child_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	m_TempPE->hChild = hwnd;
	//create rich edit
	CreateEdit(m_TempPE);
	return TRUE;
}

static void ApplyUntitledCaptions(void){
	//change caption of 'Untitled' windows when UI language is changed
	P_TPEDIT		pE;
	TCITEMW			ti;
	int				count = TabCtrl_GetItemCount(m_hTabMain);

	ZeroMemory(&ti, sizeof(ti));
	ti.mask = TCIF_PARAM;
	for(int i = 0; i < count; i++){
		SendMessageW(m_hTabMain, TCM_GETITEMW, i, (LPARAM)&ti);
		pE = (P_TPEDIT)ti.lParam;
		if(pE && pE->status == ST_NEW){
			SetWindowTextW(pE->hChild, g_Strings.sUntitled);
		}
	}
}

static void WriteLastPosition(HWND hwnd){
	//save last position of main window
	RECT		rcDesktop, rc;
	TPPOSITION	tpp;
	int			sx, sy;

	if(GetSystemMetrics(SM_CMONITORS) == 1){
		//single monitor
		GetClientRect(GetDesktopWindow(), &rcDesktop);
		sx = rcDesktop.right - rcDesktop.left;
		sy = rcDesktop.bottom - rcDesktop.top;
	}
	else{
		//multiple monitors
		sx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		sy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	GetWindowRect(hwnd, &rc);
	if(!IsWindowVisible(hwnd) && m_InTray){
		//window is hidden and tray icon is visible - get previously saved metrics
		tpp.left = ((double)m_TempPoint.x / (double)(sx));
		tpp.top = ((double)m_TempPoint.y / (double)(sy));
	}
	else{
		//get real metrics
		tpp.left = ((double)rc.left / (double)(sx));
		tpp.top = ((double)rc.top / (double)(sy));
	}
	if((GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_MAXIMIZE) == WS_MAXIMIZE || m_WinState == 1){
		//window is maximaized - set maximized flag and store metrics for restore
		tpp.maximized = TRUE;
		tpp.width = 760;
		tpp.height = 520;
	}
	else{
		//clear maximized flag
		tpp.maximized = FALSE;
		if(!IsWindowVisible(hwnd) && m_InTray){
			//window is hidden and tray icon is visible - get previously saved metrics
			tpp.width = m_TempSize.cx;
			tpp.height = m_TempSize.cy;
		}
		else{
			//get real metrics
			tpp.width = rc.right - rc.left;
			tpp.height = rc.bottom - rc.top;
		}
	}
	//store position
	WritePrivateProfileStructW(S_PREFERENCES, K_POSITION, &tpp, sizeof(tpp), g_Paths.sINI);
}

static void ChangeChildText(HWND hChild, BOOL fStar){
	//add or remove '*' character to the child window caption
	wchar_t		szText[128];

	GetWindowTextW(hChild, szText, 127);
	if(fStar){
		wcscat(szText, L"*");
	}
	else{
		szText[wcslen(szText) - 1] = '\0';
	}
	SetWindowTextW(hChild, szText);
}

static BOOL TextWithStar(HWND hwnd){
	//check whether text is ended with '*' character
	wchar_t		szText[127];

	GetWindowTextW(hwnd, szText, 127);
	return EndsWith(szText, L"*");
}

static BOOL EndsWith(wchar_t * lpString, wchar_t * lpChar){
	//check whether string is ended with specified character
	while(*lpString++)
		;
	lpString--;
	lpString--;
	if(*lpString == *lpChar)
		return TRUE;
	else
		return FALSE;
}

static void CleanBookmarks(P_TPEDIT pE){
	//clean all bookmarks and free memory
	P_TPBMRK	pB, pNext;

	pNext = pE->bookmarks;
	while(pNext){
		pB = pNext;
		pNext = pNext->next;
		HeapFree(g_hHeap, 0, pB);
	}
	pE->bookmarks = NULL;
}

static void GoToBookmark(P_TPEDIT pE, BOOL fNext){
	CHARRANGE	chrg;
	long		line;
	P_TPBMRK	pB, pLast;

	//get real line number
	line = GetRealLine(pE);
	pB = pE->bookmarks;
	if(fNext){
		//jump to next bookmark if there is any, otherwise to the first
		while(pB->next && pB->line <= line){
			pB = pB->next;
		}
		if(pB->next == NULL){
			if(pB->line <= line)
				line = pE->bookmarks->line;
			else
				line = pB->line;
		}
		else{
			line = pB->line;
		}
	}
	else{
		//jump to previous bookmark, if there is any, otherwise to the last
		while(pB->next)
			pB = pB->next;
		pLast = pB;
		while(pB->prev && pB->line >= line){
			pB = pB->prev;
		}
		if(pB->prev == NULL){
			if(pB->line >= line)
				line = pLast->line;
			else
				line = pB->line;
		}
		else{
			line = pB->line;
		}
	}
	chrg.cpMin = SendMessageW(pE->hEdit, EM_LINEINDEX, line, 0);
	chrg.cpMax = chrg.cpMin;
	SendMessageW(pE->hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
}

static long GetRealLine(P_TPEDIT pE){
	long			line, pos, last, count, charCount;
	CHARRANGE		chrg;
	GETTEXTLENGTHEX	gtx;
	RECT			rc;

	//get exactly count of chars in rich edit
	gtx.flags = GTL_NUMCHARS | GTL_PRECISE;
	gtx.codepage = 1200;
	charCount = SendMessageW(pE->hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	//get lines count
	count = RichEdit_GetLineCount(pE->hEdit);
	//get formatting rectangle
	SendMessageW(pE->hEdit, EM_GETRECT, 0, (LPARAM)&rc);
	//find last visible character and last visible line
	rc.top = rc.bottom;
	pos = SendMessageW(pE->hEdit, EM_CHARFROMPOS, 0, (LPARAM)&rc);
	last = SendMessageW(pE->hEdit, EM_EXLINEFROMCHAR, 0, pos);
	//get current selection
	SendMessageW(pE->hEdit, EM_EXGETSEL, 0, (LPARAM)&chrg);
	//get first line from selection
	line = SendMessageW(pE->hEdit, EM_EXLINEFROMCHAR, 0, chrg.cpMin);
	if(count - last == 2 && chrg.cpMin == charCount){
		//this is the case when Return key is pressed at the end of last line (rich edit does not counts this character, so we need to increase lines count manually)
		line++;
	}
	return line;
}

static P_TPBMRK AddRecentBookmark(P_TPEDIT pE, long line, P_TPBMRK prev){
	P_TPBMRK		pTemp;

	//add bookmark loaded from list of recent files
	pTemp = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, sizeof(TPBMRK));
	pTemp->line = line;
	if(prev){
		prev->next = pTemp;
		pTemp->prev = prev;
	}
	else{
		pE->bookmarks = pTemp;
	}
	return pTemp;
}

static void ToggleBookmark(P_TPEDIT pE){
	P_TPBMRK		pB = NULL, pNext, pTemp;
	long			line;

	line = GetRealLine(pE);

	pNext = pE->bookmarks;
	while(pNext){
		pB = pNext;
		pNext = pNext->next;
		if(pB->line == line){
			pTemp = pB;
			if(pB == pE->bookmarks){
				pE->bookmarks = pNext;
				if(pNext){
					pNext->prev = NULL;
				}
			}
			else{
				pB->prev->next = pNext;
				if(pNext){
					pNext->prev = pB->prev;
				}
			}
			HeapFree(g_hHeap, 0, pTemp);
			return;
		}
	}
	pTemp = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, sizeof(TPBMRK));
	pTemp->line = line;
	if(pB){
		//should keep the list sorted
		pB = pE->bookmarks;
		while(pB->next && pB->line < line){
			pB = pB->next;
		}
		if(pB->next == NULL){
			if(pB->line < line){
				pTemp->prev = pB;
				pB->next = pTemp;
			}
			else{
				pNext = pB;
				if(pB->prev){
					pTemp->prev = pB->prev;
					pB->prev->next = pTemp;
					pNext->prev = pTemp;
					pTemp->next = pNext;
				}
				else{
					//pB == pE->bookmarks
					pTemp->next = pB;
					pB->prev = pTemp;
					pE->bookmarks = pTemp;
				}
			}
		}
		else{
			//pB->line > line
			pNext = pB;
			if(pB->prev){
				pTemp->prev = pB->prev;
				pB->prev->next = pTemp;
				pNext->prev = pTemp;
				pTemp->next = pNext;
			}
			else{
				//pB == pE->bookmarks
				pTemp->next = pB;
				pB->prev = pTemp;
				pE->bookmarks = pTemp;
			}
		}
	}
	else{
		pE->bookmarks = pTemp;
		//no previous
	}
}

static void RemoveBookmark(P_TPEDIT pE, long line){
	P_TPBMRK		pB, pPrev, pNext;

	pB = pE->bookmarks;
	while(pB){
		if(pB->line == line){
			if(pB->prev == NULL){
				//case pB = pE->bookmarks;
				pNext = pE->bookmarks->next;
				HeapFree(g_hHeap, 0, pE->bookmarks);
				pE->bookmarks = pNext;
			}
			else{
				//not the first
				pNext = pB->next;
				pPrev = pB->prev;
				HeapFree(g_hHeap, 0, pB);
				pPrev->next = pNext;
				if(pNext)
					//not the last
					pNext->prev == pPrev;
			}
			break;
		}
		pB = pB->next;
	}
}

static void SetBookmarkInRange(P_TPEDIT pE){
	P_TPBMRK		pB;

	//check whether each bookmark falls in specified range
	pB = pE->bookmarks;
	while(pB){
		pB->inRange = INRANGE(SendMessageW(pE->hEdit, EM_LINEINDEX, pB->line, 0), pE->range.cpMin, pE->range.cpMax);
		pB = pB->next;
	}
}

static void GetVersionNumber(void){
	//get program version string
	char szPath[MAX_PATH], szBuffer[MAX_PATH];
	char FORMAT_STRING[] = "\\StringFileInfo\\%04x%04x\\%s";
	DWORD dwSize, dwBytes = 0;
	HGLOBAL hMem;
	UINT cbLang;
	LPVOID lpt;
	UINT cbBuffSize;
	WORD * langInfo;

	m_Version[0] = '\0';
	GetModuleFileName(g_hInstance, szPath, MAX_PATH);
	dwBytes = GetFileVersionInfoSize(szPath, &dwSize);
	if(dwBytes){
		hMem = GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, dwBytes);
		if(GetFileVersionInfo(szPath, 0, dwBytes, hMem)){
			if(VerQueryValue(hMem, "\\VarFileInfo\\Translation", (LPVOID*)&langInfo, &cbLang)){
				wsprintf(szBuffer, FORMAT_STRING, langInfo[0], langInfo[1], "FileVersion");
				if(VerQueryValue(hMem, szBuffer, &lpt, &cbBuffSize)){
					strcpy(m_Version, lpt);
				}
			}
		}
	}
}

static void ShowNewVersionBaloon(wchar_t * szNewVersion){

	wchar_t		szBuffer[256];

	wcscpy(szBuffer, g_Strings.sNewVersion1);
	wcscat(szBuffer, L" - ");
	wcscat(szBuffer, szNewVersion);
	wcscat(szBuffer, L".");
	if(g_CheckingFromButton){
		//show message only if we check for new versio manually
		wcscat(szBuffer, L"\n");
		wcscat(szBuffer, g_Strings.sNewVersion2);
		if(MessageBoxW(g_hMain, szBuffer, g_Strings.sCheckUpdate, MB_OKCANCEL | MB_ICONINFORMATION) == IDOK){
			if(wcslen(g_sDefBrowser) == 0)
				ShellExecuteW(g_hMain, L"open", DOWNLOAD_PAGE, NULL, NULL, SW_SHOWDEFAULT);
			else{
				if((int)ShellExecuteW(g_hMain, NULL, g_sDefBrowser, DOWNLOAD_PAGE, NULL, SW_SHOWDEFAULT) <= 32){
					ShellExecuteW(g_hMain, L"open", DOWNLOAD_PAGE, NULL, NULL, SW_SHOWDEFAULT);
				}
			}
		}
	}
	// else{
		// m_TrackBaloonClick = TRUE;
		// m_nData.uFlags = NIF_INFO;
		// m_nData.uTimeout = 15000;
		// m_nData.dwInfoFlags = NIIF_INFO;
		// wcscpy(m_nData.szInfoTitle, g_Strings.sCheckUpdate);
		// wcscpy(m_nData.szInfo, szBuffer);
		// Shell_NotifyIconW(NIM_MODIFY, &m_nData);
	// }
}

static void LoadRecentBookmarks(wchar_t * lpSection, P_TPEDIT pE){
	P_TPBMRK		pTemp = NULL;
	long			line;
	wchar_t			szPath[MAX_PATH * 2], szBookmarks[BOOKMARKS_MAX * 32], *p1, *p2;

	ConstructINIKey(szPath, pE->szLongName);
	//get bbokmarks string delimited by '^' character
	GetPrivateProfileStringW(lpSection, szPath, NULL, szBookmarks, BOOKMARKS_MAX * 32, g_Paths.sINI);
	if(wcslen(szBookmarks) > 0){
		p1 = p2 = szBookmarks;
		while(*p1){
			if(*p1 == CHAR_DELIMETER){
				*p1 = '\0';
				line = _wtol(p2);
				//process each bookmark line and return pointer to its structure
				pTemp = AddRecentBookmark(pE, line, pTemp);
				p1++;
				p2 = p1;
			}
			p1++;
		}
		//store lines count of rich edit
		pE->lines = RichEdit_GetLineCount(pE->hEdit);
		//recheck boockmarks - there may be less lines, than needed (for example, deleted by other program)
		RecheckBookmarks(pE);
	}
}

static void ConstructINIKey(wchar_t * lpPath, wchar_t * lpLongName){
	//construct key for INI file depending on program location
	if(!m_RemovableDrive){
		//program is running from fixed drive - build key with real MAC address
		wcscpy(lpPath, m_MACAddress);
		wcscat(lpPath, DELIMETER);
		wcscat(lpPath, lpLongName);
	}
	else{
		//program is running from removable drive
		if(m_DriveLetter[0] == lpLongName[0]){
			//the path is from removable drive as well - construct key with "dummy" MAC address
			wcscpy(lpPath, REMOVABLE_MAC);
			wcscat(lpPath, DELIMETER);
			wcscat(lpPath, lpLongName);
		}
		else{
			//the path is from fixed drive - construct key with real MAC address
			wcscpy(lpPath, m_MACAddress);
			wcscat(lpPath, DELIMETER);
			wcscat(lpPath, lpLongName);
		}
	}
}

static void SetTrayTip(void){
	P_TPEDIT		pE;
	wchar_t			szTip[128], szBuffer[128];
	
	//construct tray tooltip
	pE = GetActiveHandle();
	wcscpy(szTip, PROGRAM_NAME);
	wcscat(szTip, L" ");
	MultiByteToWideChar(CP_ACP, 0, m_Version, -1, szBuffer, 128);
	wcscat(szTip, szBuffer);
	wcscat(szTip, L"\n");
	wcscat(szTip, g_Strings.sDocsTotal);
	wcscat(szTip, L" ");
	_itow(TabCtrl_GetItemCount(m_hTabMain), szBuffer, 10);
	wcscat(szTip, szBuffer);
	wcscat(szTip, L"\n");
	wcscat(szTip, g_Strings.sDocsCurrent);
	wcscat(szTip, L" ");
	if(pE){
		wcscat(szTip, pE->szShortName);
	}
	wcscpy(m_nData.szTip, szTip);
}

static void GetMACAddress(void){
	PIP_ADAPTER_INFO 	pAdapterInfo;
	ULONG				ulBuffLen;
	char				szMAC[32];

	//set default MAC address - for case there is no MAC adapter ?
	wcscpy(m_MACAddress, L"00-00-00-00-00-00");
	//initialize structure
	pAdapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO));
	ulBuffLen = sizeof(IP_ADAPTER_INFO);
	//define needed size
	if(GetAdaptersInfo(pAdapterInfo, &ulBuffLen) == ERROR_BUFFER_OVERFLOW){
		//allocate buffer
		free(pAdapterInfo);
		pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulBuffLen);
	}
	//get MAC adapters info
	if(GetAdaptersInfo(pAdapterInfo, &ulBuffLen) == NO_ERROR){
		if(pAdapterInfo){
			//get address of first adapter
			sprintf(szMAC, "%02X-%02X-%02X-%02X-%02X-%02X", pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2], pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
			//conver string to unicode
			MultiByteToWideChar(CP_ACP, 0, szMAC, -1, m_MACAddress, 32);
		}
	}
	free(pAdapterInfo);
}

static BOOL InOtherIndices(int number){
	int		*temp = m_OtherIndices;

	//check whether given number is in indices of recent files which don't belong to current computer
	for(int i = 0; i < m_OtherRecentFiles; i++){
		if(*temp++ == number)
			return TRUE;
	}
	return FALSE;
}

static BOOL InOtherLastIndices(int number){
	int		*temp = m_LastOtherIndices;
	
	//check whether given number is in indices of last session files which don't belong to current computer
	for(int i = 0; i < m_LastOtherFiles; i++){
		if(*temp++ == number)
			return TRUE;
	}
	return FALSE;
}

static void ConvertCase(HWND hEdit, BOOL fToUpper){
	TEXTRANGEW		trg;
	wchar_t			*pTemp;

	//get current selection
	SendMessageW(hEdit, EM_EXGETSEL, 0, (LPARAM)&trg.chrg);
	//allocate buffer
	trg.lpstrText = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, (trg.chrg.cpMax - trg.chrg.cpMin + 1) * sizeof(wchar_t));
	pTemp = trg.lpstrText;
	//get selected text
	SendMessageW(hEdit, EM_GETTEXTRANGE, 0, (LPARAM)&trg);
	//convert text case
	for(int i = trg.chrg.cpMin; i <= trg.chrg.cpMax; i++){
		if(fToUpper)
			*pTemp = towupper(*pTemp++);
		else
			*pTemp = towlower(*pTemp++);
	}
	//replace selection
	SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)trg.lpstrText);
	SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&trg.chrg);
	//free buffer
	HeapFree(g_hHeap, 0, trg.lpstrText);
}

static void GetLanguageID(void){
	wchar_t			szPath[MAX_PATH];

	wcscpy(szPath, g_Paths.sLangDir);
	wcscat(szPath, g_Paths.sLangFile);
	//get language id
	if(PathFileExistsW(szPath)){
		g_Settings.langID = GetPrivateProfileIntW(S_LANG, L"id", 0x409, szPath);
	}
}

static BOOL IsAccPrepared(void){
	wchar_t			szSection[1024 * 10];

	*szSection = '\0';
	GetPrivateProfileSectionW(S_ACCELERATORS, szSection, 1024 * 10, g_Paths.sINI);
	if(wcslen(szSection) == 0)
		return FALSE;
	else
		return TRUE;
}

static BOOL CALLBACK DTFormat_DlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG (hwnd, WM_CLOSE, DTFormat_OnClose);
	HANDLE_MSG (hwnd, WM_COMMAND, DTFormat_OnCommand);
	HANDLE_MSG (hwnd, WM_INITDIALOG, DTFormat_OnInitDialog);

	default: return FALSE;
	}
}

static void DTFormat_OnClose(HWND hwnd)
{
	EndDialog(hwnd, IDCANCEL);
}

static void DTFormat_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	wchar_t			szCaption[256], szLang[MAX_PATH];

	switch(id){
	case IDCANCEL:
		EndDialog(hwnd, IDCANCEL);
		break;
	case IDOK:
		EndDialog(hwnd, IDOK);
		break;
	case IDC_CMD_SHOW_DSMP:
		wcscpy(szLang, g_Paths.sLangDir);
		wcscat(szLang, g_Paths.sLangFile);
		GetPrivateProfileStringW(S_STRINGS, L"date_mask_caption", L"Date format masks", szCaption, 256, szLang);
		MessageBoxW(hwnd, g_pDFormats, szCaption, MB_OK);
		break;
	case IDC_CMD_SHOW_TSMP:
		wcscpy(szLang, g_Paths.sLangDir);
		wcscat(szLang, g_Paths.sLangFile);
		GetPrivateProfileStringW(S_STRINGS, L"time_mask_caption", L"Time format masks", szCaption, 256, szLang);
		MessageBoxW(hwnd, g_pTFormats, szCaption, MB_OK);
		break;
	case IDC_EDT_DATE_FMT:
		if(codeNotify == EN_CHANGE){
			GetDlgItemTextW(hwnd, IDC_EDT_DATE_FMT, g_TempDTFormats.DateFormat, 128);
			SetShowDateTime(hwnd);
		}
		break;
	case IDC_EDT_TIME_FMT:
		if(codeNotify == EN_CHANGE){
			GetDlgItemTextW(hwnd, IDC_EDT_TIME_FMT, g_TempDTFormats.TimeFormat, 64);
			SetShowDateTime(hwnd);
		}
		break;
	case IDC_OPT_TIME_FIRST:
		if(codeNotify == BN_CLICKED){
			if(IsDlgButtonChecked(hwnd, IDC_OPT_TIME_FIRST) == BST_CHECKED){
				g_TempDTFormats.TimeFirst = 1;
				SetShowDateTime(hwnd);
			}
		}
		break;
	case IDC_OPT_DATE_FIRST:
		if(codeNotify == BN_CLICKED){
			if(IsDlgButtonChecked(hwnd, IDC_OPT_DATE_FIRST) == BST_CHECKED){
				g_TempDTFormats.TimeFirst = 0;
				SetShowDateTime(hwnd);
			}
		}
		break;
	case IDC_OPT_USE_SYS_LANG:
		if(codeNotify == BN_CLICKED){
			if(IsDlgButtonChecked(hwnd, IDC_OPT_USE_SYS_LANG) == BST_CHECKED){
				g_TempDTFormats.CurrSettings = 0;
				SetShowDateTime(hwnd);
			}
		}
		break;
	case IDC_OPT_USE_USER_LANG:
		if(codeNotify == BN_CLICKED){
			if(IsDlgButtonChecked(hwnd, IDC_OPT_USE_USER_LANG) == BST_CHECKED){
				g_TempDTFormats.CurrSettings = 1;
				SetShowDateTime(hwnd);
			}
		}
		break;
	}
}

static BOOL DTFormat_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	wchar_t		szCaption[256], szLang[MAX_PATH];

	g_TempDTFormats = g_DTFormats;

	wcscpy(szLang, g_Paths.sLangDir);
	wcscat(szLang, g_Paths.sLangFile);

	GetPrivateProfileStringW(S_CONTROLS, L"1010", L"Date/time settings", szCaption, 256, szLang);
	SetWindowTextW(hwnd, szCaption);
	SetControlText(hwnd, IDC_GRP_DT_FORMAT, L"Formats", szLang);
	SetControlText(hwnd, IDC_ST_DATE_FMT, L"Date", szLang);
	SetControlText(hwnd, IDC_ST_TIME_FMT, L"Time", szLang);
	SetControlText(hwnd, IDC_ST_PVW, L"Preview", szLang);
	SetControlText(hwnd, IDC_GRP_DT_ORDER, L"Order", szLang);
	SetControlText(hwnd, IDC_OPT_DATE_FIRST, L"Insert time after date", szLang);
	SetControlText(hwnd, IDC_OPT_TIME_FIRST, L"Insert date after time", szLang);
	SetControlText(hwnd, IDC_OPT_USE_SYS_LANG, L"Use default language settings", szLang);
	SetControlText(hwnd, IDC_OPT_USE_USER_LANG, L"Use current language settings", szLang);
	SetControlText(hwnd, IDCANCEL, L"Cancel", szLang);
	SetControlText(hwnd, IDOK, L"OK", szLang);
	SetDlgItemTextW(hwnd, IDC_EDT_DATE_FMT, g_TempDTFormats.DateFormat);
	SetDlgItemTextW(hwnd, IDC_EDT_TIME_FMT, g_TempDTFormats.TimeFormat);
	CheckDlgButton(hwnd, IDC_OPT_TIME_FIRST, g_TempDTFormats.TimeFirst ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_OPT_DATE_FIRST, !g_TempDTFormats.TimeFirst ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_OPT_USE_SYS_LANG, !g_TempDTFormats.CurrSettings ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_OPT_USE_USER_LANG, g_TempDTFormats.CurrSettings ? BST_CHECKED : BST_UNCHECKED);
	SetShowDateTime(hwnd);
	return FALSE;
}

static void InsertDateTime(int type){
	SYSTEMTIME		st;
	wchar_t			szDate[128], szTime[64], szBuffer[256];
	HWND			hEdit;
	int				langID;

	if(g_TempDTFormats.CurrSettings)
		langID = g_Settings.langID;
	else
		langID = LOCALE_USER_DEFAULT;

	hEdit = GetActiveEdit();
	ZeroMemory(&st, sizeof(st));
	GetLocalTime(&st);
	GetDateFormatW(langID, 0, &st, g_DTFormats.DateFormat, szDate, 128);
	GetTimeFormatW(langID, 0, &st, g_DTFormats.TimeFormat, szTime, 64);
	switch(type){
	case 0:		//date and time
		if(g_TempDTFormats.TimeFirst){
			wcscpy(szBuffer, szTime);
			wcscat(szBuffer, szDate);
		}
		else{
			wcscpy(szBuffer, szDate);
			wcscat(szBuffer, szTime);
		}
		break;
	case 1:		//date
		wcscpy(szBuffer, szDate);
		break;
	case 2:		//time
		wcscpy(szBuffer, szTime);
		break;
	}
	
	SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuffer);
	SetFocus(hEdit);
}

static void SetShowDateTime(HWND hwnd){

	SYSTEMTIME		st;
	wchar_t			szDate[128], szTime[64], szBuffer[256];
	int				langID;

	ZeroMemory(&st, sizeof(st));
	GetLocalTime(&st);
	if(g_TempDTFormats.CurrSettings)
		langID = g_Settings.langID;
	else
		langID = LOCALE_USER_DEFAULT;
	GetDateFormatW(langID, 0, &st, g_TempDTFormats.DateFormat, szDate, 128);
	GetTimeFormatW(langID, 0, &st, g_TempDTFormats.TimeFormat, szTime, 64);
	if(g_TempDTFormats.TimeFirst){
		wcscpy(szBuffer, szTime);
		wcscat(szBuffer, szDate);
	}
	else{
		wcscpy(szBuffer, szDate);
		wcscat(szBuffer, szTime);
	}
	SetDlgItemTextW(hwnd, IDC_ST_DATE_PVW, szBuffer);
}

static int __cdecl Compare(const void * i1, const void * i2){
	//compare function for qsort
	return(*(int *)i1 - *(int *)i2);
}

static int __cdecl AccCompare(const void * i1, const void * i2){
	ACCEL		*a1, *a2;

	a1 = (ACCEL *)i1;
	a2 = (ACCEL *)i2;
	return(a1->cmd - a2->cmd);
}

static DWORD CALLBACK ReadStreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb){

	int				limit;
	P_TPREAD		tr;

	tr = (P_TPREAD)dwCookie;
	ReadFile(tr->hHandle, pbBuff, cb, (PULONG)pcb, NULL);
	if(tr->type == CODE_UNICODE_BE){
		//we should reverse bytes in case of big endian
		if(*pcb){
			__asm{
				mov ecx, pcb
				mov ecx, dword ptr [ecx]
				mov limit, ecx
				mov edx, pbBuff
				xor ecx, ecx
				_loop:
					mov al, byte ptr [edx]
					mov ah, byte ptr [edx+1]
					mov [edx], ah
					mov [edx+1], al
					add edx, 2
					add ecx, 2
					cmp ecx, limit
				jl _loop
			}
		}
	}
	return 0;
}

static DWORD CALLBACK WriteStreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb){

	int				limit;

	if(m_CurrType == CODE_UNICODE_BE){
		//we should reverse bytes in case of big endian
		if(cb){
			__asm{
				mov ecx, cb
				mov limit, ecx
				mov edx, pbBuff
				xor ecx, ecx
				_loop:
					mov al, byte ptr [edx]
					mov ah, byte ptr [edx+1]
					mov [edx], ah
					mov [edx+1], al
					add edx, 2
					add ecx, 2
					cmp ecx, limit
				jl _loop
			}
		}
	}
	WriteFile((HANDLE)dwCookie, pbBuff, cb, (PULONG)pcb, NULL);
	return 0;
}
