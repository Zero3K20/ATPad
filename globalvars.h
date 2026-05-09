#ifndef	__GLOBALVARS_H_
#define	__GLOBALVARS_H_

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <gcolors.h>
#include <gcpicker.h>

#define	CLR_MASK	RGB(255, 0, 255)

typedef struct _TPFORMAT {
	int			fontHeight;
	LOGFONTW	lf;
	GCOLORTYPE	cBack;
	GCOLORTYPE	cText;
}TPFORMAT, *P_TPFORMAT;

typedef struct _TPSTRINGS {
	wchar_t		sUntitled[128];
	wchar_t		sSample[64];
	wchar_t		sFontTextCaption[128];
	wchar_t		sFontLinesCaption[128];
	wchar_t		sSaveChanges[256];
	wchar_t		sLine[64];
	wchar_t		sColumn[64];
	wchar_t		sOpenFileCaption[128];
	wchar_t		sSaveFileCaption[128];
	wchar_t		sFileNotExist[256];
	wchar_t		sDiscardChanges[256];
	wchar_t		sOutChanges1[256];
	wchar_t		sOutChanges2[256];
	wchar_t		sOutChanges3[256];
	wchar_t		sRefreshEvery[128];
	wchar_t		sCheckUpdate[128];
	wchar_t		sNewVersion1[128];
	wchar_t		sNewVersion2[128];
	wchar_t		sSameVersion[128];
	wchar_t		sDocsTotal[64];
	wchar_t		sDocsCurrent[64];
	wchar_t		sPrefBrowserCaption[64];
	wchar_t		sTotalFound[128];
	wchar_t		sKeyExists[256];
	wchar_t		sSnippetExists[256];
	wchar_t		sDelSnipQuestion[256];
	wchar_t		sAutoURLChange[256];
	wchar_t		sNoOccurrences[256];
}TPSTRINGS, *P_TPSTRINGS;

typedef struct _TPSETTINGS {
	BOOL		showLines;
	int			marginWidth;
	int			alignNumbers;
	int			tabWidth;
	BOOL		wordWrap;
	int			langID;
	BOOL		singleInstance;
	BOOL		minTotray;
	BOOL		openNew;
	BOOL		lastSession;
	BOOL		trackChanges;
	int			preservePosition;
	BOOL		showWS;
	BOOL		showCRLF;
	BOOL		showTabs;
	BOOL		showSpaces;
	BOOL		checkNVOnStart;
	int			tabStop;
	int			res1;
	int			res2;
}TPSETTINGS, *P_TPSETTINGS;

typedef struct _TPPOSITION {
	double		left;
	double		top;
	int			width;
	int			height;
	BOOL		maximized;
}TPPOSITION, * P_TPPOSITION;

typedef struct _TPBMRK * P_TPBMRK;
typedef struct _TPBMRK {
	long			line;
	P_TPBMRK		next;
	P_TPBMRK		prev;
	BOOL			inRange;
}TPBMRK;

typedef struct _TPEDIT {
	HWND		hEdit;
	HWND		hStatic;
	HWND		hChild;
	int			tabIndex;
	int			type;
	wchar_t		szShortName[128];
	wchar_t		szLongName[MAX_PATH];
	FILETIME	ft;
	BOOL		changed;
	int			status;
	HICON		hIcon;
	long		interval;
	BOOL		timerOn;
	P_TPBMRK	bookmarks;
	CHARRANGE	range;
	long		lines;
	BOOL		removeLastSession;
}TPEDIT, *P_TPEDIT;

typedef struct _TPPATHS {
	wchar_t		sLangDir[MAX_PATH];
	wchar_t		sLangFile[128];
	wchar_t		sINI[MAX_PATH];
	wchar_t		sProgFullPath[MAX_PATH];
	wchar_t		sSnippetsPath[MAX_PATH];
}TPPATHS, *P_TPPATHS;

typedef struct _FRC_ITEM {
	long		row;
	long		col;
	long		min;
	long		max;
	HWND		hEdit;
}FRC_ITEM, *P_FRC_ITEM;

typedef struct _TP_TEMPACCEL {
	ACCEL		acc;
	wchar_t		text[128];
}TP_TEMPACCEL, *P_TP_TEMPACCEL;

typedef struct _DTFORMATS {
	wchar_t		DateFormat[128];
	wchar_t		TimeFormat[64];
	int			TimeFirst;
	int			CurrSettings;
}DTFORMATS, P_DTFORMATS;

typedef struct _SEARCHSTRUCT {
	int			updown;
	int			matchcase;
	int			wholeword;
	int			alldocs;
}SEARCHSTRUCT, P_SEARCHSTRUCT;

//command line arguments
enum clargs {ARG_EXIT = 1, ARG_SILENT = 2, ARG_NO_SAVE = 4, ARG_INI_PATH = 8, ARG_DATA_PATH = 16, ARG_PROG_PATH = 32, ARG_LOAD_ON_START = 64};
enum line_num_align	{AL_LEFT, AL_CENTER, AL_RIGHT};
enum doc_status {ST_NEW, ST_FILE};
enum code_types {CODE_TEXT, CODE_UNICODE, CODE_UNICODE_BE, CODE_UTF8};
enum text_position_on_reload {POS_SAVED, POS_START, POS_END};
enum settings_bits {SB_AUTO_URL, SB_SHOW_FR, SB_SHOW_SNIPPETS};

HINSTANCE			g_hInstance;
HWND				g_hMain, g_hClient, g_hSettings, g_hFind, g_hSplit;
HFONT				g_hMenuFont, g_hTextFont, g_hLinesFont;
HBITMAP				g_hBmpNormal, g_hBmpGray, g_hBmpTab, g_hCR, g_hLF;
HIMAGELIST			g_ImlTab;
HMENU				g_hMenu;
TPSTRINGS			g_Strings;
TPFORMAT			g_TextAreaFormat, g_TempTextAreaFormat, g_LineNumbersFormat, g_TempLineNumbersFormat;
TPSETTINGS			g_Settings, g_TempSettings;
TPPATHS				g_Paths;
wchar_t				g_SearchString[512], g_ReplaceString[512];
int					g_FParam, g_FindIndex, g_RepIndex;
PAGESETUPDLGW		g_pSetup;
PRINTDLGW			g_pPdlg;
HANDLE				g_hHeap;
GCOLORTYPE			g_BreaksColor, g_TempBreaksColor, g_TabsColor, g_TempTabsColor, g_SpaceColor, g_TempSpaceColor, g_BMColor, g_TempBMColor;
int					g_TextHeight, g_LineNumberHeight;
BOOL				g_CheckingFromButton;
wchar_t				g_sDefBrowser[MAX_PATH], g_sTempDefBrowser[MAX_PATH];
wchar_t				*g_pDFormats, *g_pTFormats;
LPACCEL				g_TempAcc;
int					g_TempAccCount;
DTFORMATS			g_DTFormats, g_TempDTFormats;
SEARCHSTRUCT		g_SearchParams;
BOOL				g_VSEnabled;
#endif
