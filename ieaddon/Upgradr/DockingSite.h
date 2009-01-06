///////////////////////////////////////////////////////////////
//
// InternetExplorerSink.h
//
// Created: 12/07/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#pragma once
#include "DockingFrame.h"

class CDockingSite;

class CIEServer : public CWindowImpl<CIEServer> {
public:
	DECLARE_WND_CLASS(_T("CIEServerWnd"))
	BEGIN_MSG_MAP(CIEServer)
		MESSAGE_HANDLER(WM_MOVE, OnMove)
		MESSAGE_HANDLER(WM_MOVE, OnSize)
	END_MSG_MAP()

public:
	void                                         SetDockingSite(CDockingSite* site) { m_DockingSite = site; }

private:
	LRESULT OnMove(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL &);

	CDockingSite*                                m_DockingSite;
};

/**
* @brief Class to intercept mouse movements within a given window and
*        inform an observer of these movements.
*
* The MouseTracker class subclasses a window. The classes message pump
* intercepts all mouse messages and forwards these to the watcher.
* 
* Clients of the MouseTracker can disable mouse tracking by using the
* Suspend RAII class. Each Suspend object sets the m_isSuspended flag
* to true for its lifetime, and resets the flag to false when its
* destroyed. When this flag is set the message handlers ignore
* the current message by setting bHandled to FALSE and returning
* 1 (denoting that the message hasn't been processed).
*
* We can't simply temporarily unsubclass the (subclassed) Window. If
* we've got multiple ATL CWindowImpl derived objects subclassing a window
* (as we do in the MouseGestures app if (for example) the Google toolbar
* is installed), then there's no guarantee that we're the last object
* in the message chain. Unsubclass can fail (the top-most message proc
* will be different from the static WndProc function for this class),
* resulting in recursion (and ultimately a stack overflow) if we send
* our own messages to the subclassed window.
**/

class CDockingSite : public dockwins::CDockingSiteImpl<CDockingSite> {
	typedef dockwins::CDockingSiteImpl <CDockingSite> baseClass;
public:
	struct Watcher
	{
		virtual ~Watcher() {}

		virtual bool                              SetIEWindowPos(int x, int y, int sx, int sy) = 0;
		virtual LRESULT                           OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) = 0;
	};

public:
	CDockingSite();
	~CDockingSite();

	// Not implemented
private:
	CDockingSite(const CDockingSite &rhs);

public:
	DECLARE_WND_CLASS(_T("CDockSiteSampleWnd"))

	BEGIN_MSG_MAP(CDockingSite)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING , OnWindowPosChanging)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

private:
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnKillFocus(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnWindowPosChanging(UINT, WPARAM, LPARAM, BOOL &);

public:
	HRESULT Advise(HWND hWnd, Watcher *pWatcher);
	HRESULT Unadvise();
	HRESULT SetIEServer(HWND hWnd);
	HRESULT UnsetIEServer();

	Watcher* GetWatcher() const { return m_Watcher; }
	void SetWatcher(Watcher* val) { m_Watcher = val; }

private:
	bool														m_IsBound;   ///< are we bound to a subclassed window
	Watcher*													m_Watcher;   ///< the object watching the mouse
	CIEServer                                    m_IEServer;
};
