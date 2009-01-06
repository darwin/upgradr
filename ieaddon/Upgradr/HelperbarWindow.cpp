#include "StdAfx.h"
#include "HelperbarWindow.h"

//#include "Debug.h"

CHelperbarWindow::CHelperbarWindow():
m_BrowserId(NULL_BROWSER)
{
	TRACE_I(FS(_T("HelperbarMainWindow[%08X]: constructor"), this));
}

CHelperbarWindow::~CHelperbarWindow()
{
	TRACE_I(FS(_T("HelperbarMainWindow[%08X]: destructor"), this));
}

LRESULT 
CHelperbarWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_Workspace.SetBrowserId(m_BrowserId);
	m_Workspace.Create(*this, rcDefault, HELPERBAR_WORKSPACE_WINDOW_NAME, 0, 0, IDC_HELPERBAR_WORKSPACE);

	UpdateLayout();
	return 0;
}

LRESULT 
CHelperbarWindow::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return FALSE;
}

LRESULT 
CHelperbarWindow::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS * lpwndpos = (WINDOWPOS *) lParam;
	if (!(lpwndpos->flags & SWP_NOSIZE))
	{
		UpdateLayout();
		bHandled = TRUE;
	}
	return 0;
}

void 
CHelperbarWindow::UpdateLayout()
{
	RECT  rc;
	GetClientRect(&rc);

	HDWP hdwp = BeginDeferWindowPos(1);
	hdwp = m_Workspace.DeferWindowPos(hdwp, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
}

bool 
CHelperbarWindow::SetBrowserId( TBrowserId browserId )
{
	m_BrowserId = browserId;
	return true;
}

bool 
CHelperbarWindow::SetDisplayMode(CHelperbarWindow::EDisplayMode mode)
{
	m_DisplayMode = mode;
	return true;
}

bool CHelperbarWindow::Unpin()
{
	m_Workspace.LetAllPanesFloat();
	return true;
}