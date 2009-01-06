#include "StdAfx.h"
#include "resource.h"
#include "WorkspaceWindow.h"
#include "WindowManager.h"
#include "DOMExplorerWindow.h"
#include "ConsoleWindow.h"
#include "PlainTextView.h"

//#include "Debug.h"

CWorkspaceWindow::CWorkspaceWindow():
m_Ready(false),
m_BrowserId(NULL_BROWSER)
{
	TRACE_I(FS(_T("CHelperbarTabsWindow[%08X]: constructor"), this));
}

CWorkspaceWindow::~CWorkspaceWindow()
{
	TRACE_I(FS(_T("CHelperbarTabsWindow[%08X]: destructor"), this));
}

LRESULT 
CWorkspaceWindow::OnCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	return 0;
}

LRESULT 
CWorkspaceWindow::OnPostCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	InitializeDockingFrame();
	InitializeDefaultPanes();
	return 0;
}

LRESULT 
CWorkspaceWindow::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS * lpwndpos = (WINDOWPOS *) lParam;
	if (!(lpwndpos->flags & SWP_NOSIZE))
	{
		CRect r;
		GetClientRect(&r);
		if (r.Width() && r.Height() && !m_Ready)
		{
			PostMessage(UM_POSTCREATE);
			m_Ready = true;
		}
	}
	bHandled = FALSE;
	return 0;
}

void 
CWorkspaceWindow::InitializeDefaultPanes(void)
{
	ATLASSERT(m_BrowserId!=NULL_BROWSER);

	CRect r;
	GetClientRect(&r);

	// DOM Explorer
	CManagedWindow<CDOMExplorerWindow>* pDOMExplorer;
	WindowManagerLock windowManager;
	pDOMExplorer = windowManager->GetDOMExplorerWindow(m_BrowserId);
	if (!pDOMExplorer)
	{
		pDOMExplorer = windowManager->CreateDOMExplorerWindow(m_BrowserId, m_hWnd);
		ATLASSERT(pDOMExplorer);
		if (!pDOMExplorer) return;
		DockWindow(pDOMExplorer->GetPane(),	dockwins::CDockingSide(dockwins::CDockingSide::sLeft), 0, float(0.0), 400,	r.Height());
	}

	// Console Window
	CManagedWindow<CConsoleWindow>* pConsoleWindow;
	pConsoleWindow = windowManager->GetConsoleWindow(m_BrowserId);
	if (!pConsoleWindow)
	{
		pConsoleWindow = windowManager->CreateConsoleWindow(m_BrowserId, m_hWnd);
		ATLASSERT(pConsoleWindow);
		if (!pConsoleWindow) return;
		DockWindow(pConsoleWindow->GetPane(),	dockwins::CDockingSide(dockwins::CDockingSide::sLeft), 1, float(0.0), r.Width()-400, r.Height());
	}

	// ...
	CManagedWindow<CPlainTextView>* pPlainTextView;
	pPlainTextView = windowManager->GetPlainTextView(m_BrowserId);
	if (!pPlainTextView)
	{
		pPlainTextView = windowManager->CreatePlainTextView(m_BrowserId, m_hWnd);
		ATLASSERT(pPlainTextView);
		if (!pPlainTextView) return;
		pPlainTextView->GetPane().DockTo(pConsoleWindow->GetPane(), 1);
	}
}

bool 
CWorkspaceWindow::SetBrowserId( TBrowserId browserId )
{
	m_BrowserId = browserId;
	return true;
}

bool                                         
CWorkspaceWindow::LetAllPanesFloat()
{
	UndockAll();
//	FloatAll();
	//CWindowManager& windowManager = GetWindowManager();
	//windowManager.Float(m_BrowserId, m_hWnd);
	return true;
}
