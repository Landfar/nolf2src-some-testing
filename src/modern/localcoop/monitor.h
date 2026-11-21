// ----------------------------------------------------------------------- //
//
// MODULE  : monitor.h
//
// PURPOSE : The modern build's choice of monitor for the game's window
//           (MODERN.md M-E73), for local co-op on one PC (M-G93): each
//           player's game in a window on a monitor of its own.  Included,
//           in NOLF2SRC_MODERN blocks, by the engine's render.cpp (the
//           size a borderless window takes) and the renderer's
//           common_init.cpp (where the window goes); each has its own copy.
//
//           WindowMonitor (a console variable):
//             0 (or unset)  the primary monitor, as before
//             -1            the first monitor that is not the primary one
//                           (counting from the left), or the primary one
//                           when there is no other
//             1, 2, ...     the monitors counted from the left (then from
//                           the top): 1 the leftmost
//           A monitor that is not there is the primary one.
//
//           EnumDisplayMonitors and GetMonitorInfo are Windows 98/2000's;
//           VC6's headers declare them only for WINVER 0x0500, which the
//           engine is not built for, so they are looked up in user32 at run
//           time and their types declared here.
//
//           Modern build only (NOLF2SRC_MODERN).
//
// ----------------------------------------------------------------------- //

#ifndef __NOLF2SRC_MONITOR_H__
#define __NOLF2SRC_MONITOR_H__

#include <windows.h>
#include <string.h>

namespace
{
	// MONITORINFO
	struct ModernMonitorInfo
	{
		DWORD	cbSize;
		RECT	rcMonitor;
		RECT	rcWork;
		DWORD	dwFlags;			// 1: MONITORINFOF_PRIMARY
	};

	typedef BOOL (CALLBACK *ModernMonitorEnumProc)(HANDLE, HDC, LPRECT, LPARAM);
	typedef BOOL (WINAPI *ModernEnumDisplayMonitorsFn)(HDC, LPCRECT, ModernMonitorEnumProc, LPARAM);
	typedef BOOL (WINAPI *ModernGetMonitorInfoFn)(HANDLE, ModernMonitorInfo*);

	enum { kModernMaxMonitors = 16 };

	struct ModernMonitorList
	{
		ModernGetMonitorInfoFn	pGetInfo;
		RECT					aRect[kModernMaxMonitors];
		BOOL					aPrimary[kModernMaxMonitors];
		int						nCount;
	};

	BOOL CALLBACK ModernMonitorEnum(HANDLE hMonitor, HDC, LPRECT, LPARAM lParam)
	{
		ModernMonitorList* pList = (ModernMonitorList*)lParam;
		ModernMonitorInfo info;

		if (pList->nCount >= kModernMaxMonitors)
			return FALSE;

		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		if (pList->pGetInfo(hMonitor, &info))
		{
			pList->aRect[pList->nCount]    = info.rcMonitor;
			pList->aPrimary[pList->nCount] = (info.dwFlags & 1) != 0;
			pList->nCount++;
		}

		return TRUE;
	}

	// The primary monitor, at (0, 0)
	void ModernPrimaryRect(RECT* pRect)
	{
		pRect->left   = 0;
		pRect->top    = 0;
		pRect->right  = GetSystemMetrics(SM_CXSCREEN);
		pRect->bottom = GetSystemMetrics(SM_CYSCREEN);
	}

	// The rectangle of the desktop's coordinates that monitor nMonitor
	// (WindowMonitor's numbering, above) covers; false when that is the
	// primary monitor (and *pRect the primary's)
	bool ModernMonitorRect(int nMonitor, RECT* pRect)
	{
		ModernMonitorList list;
		HMODULE hUser;
		ModernEnumDisplayMonitorsFn pEnum;
		int i, j, nFound;

		ModernPrimaryRect(pRect);

		if (nMonitor == 0)
			return false;

		hUser = GetModuleHandle("user32.dll");
		if (!hUser)
			return false;

		pEnum         = (ModernEnumDisplayMonitorsFn)GetProcAddress(hUser, "EnumDisplayMonitors");
		list.pGetInfo = (ModernGetMonitorInfoFn)GetProcAddress(hUser, "GetMonitorInfoA");
		list.nCount   = 0;
		if (!pEnum || !list.pGetInfo || !pEnum(NULL, NULL, ModernMonitorEnum, (LPARAM)&list))
			return false;

		// From the left, then from the top
		for (i = 1; i < list.nCount; i++)
		{
			for (j = i; j > 0; j--)
			{
				RECT rA = list.aRect[j - 1], rB = list.aRect[j];
				if (rA.left < rB.left || (rA.left == rB.left && rA.top <= rB.top))
					break;

				BOOL bPrimary       = list.aPrimary[j - 1];
				list.aRect[j - 1]    = rB;
				list.aPrimary[j - 1] = list.aPrimary[j];
				list.aRect[j]        = rA;
				list.aPrimary[j]     = bPrimary;
			}
		}

		nFound = -1;
		if (nMonitor < 0)
		{
			for (i = 0; i < list.nCount && nFound < 0; i++)
			{
				if (!list.aPrimary[i])
					nFound = i;
			}
		}
		else if (nMonitor <= list.nCount)
		{
			nFound = nMonitor - 1;
		}

		if (nFound < 0 || list.aPrimary[nFound])
			return false;

		*pRect = list.aRect[nFound];
		return pRect->right > pRect->left && pRect->bottom > pRect->top;
	}
}

#endif // __NOLF2SRC_MONITOR_H__
