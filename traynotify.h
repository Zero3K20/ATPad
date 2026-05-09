#ifndef __TRAYNOTIFY_H__
#define __TRAYNOTIFY_H__

//tray + shellnotify
#define IDI_TRAY					0
#define WM_SHELLNOTIFY				WM_USER + 5

void AddNotifyIcon(HWND hwnd, HINSTANCE hInstance, int iconID, NOTIFYICONDATAW * lpnid, wchar_t * lpTooltip);

#endif	// TRAYNOTIFY_H__
