#include "StdAfx.h"
#include "ConsoleWindow.h"

//////////////////////////////////////////////////////////////////////////
// CConsoleWindow

CConsoleWindow::CConsoleWindow()
{
}

CConsoleWindow::~CConsoleWindow()
{
}

LRESULT 
CConsoleWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return S_OK;
}

LRESULT 
CConsoleWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(!!m_Logger);
	DWORD consoleStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS;
	m_LoggerConsole.Create(*this, rcDefault, CONSOLE_LIST_WINDOW_NAME, consoleStyle, 0, 1);

	if (!m_ImageList.CreateFromImage(IDB_EXAMPLE, 16, 0, RGB( 255, 0, 255 ), IMAGE_BITMAP, LR_CREATEDIBSECTION))
		return FALSE;

	InitList();
	return TRUE;
}

void 
CConsoleWindow::Layout()
{
	RECT  rc;
	GetClientRect(&rc);

	HDWP hdwp = BeginDeferWindowPos(1);
	hdwp = m_LoggerConsole.DeferWindowPos(hdwp, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
} 

LRESULT 
CConsoleWindow::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS * lpwndpos = (WINDOWPOS *)lParam;
	if (!(lpwndpos->flags & SWP_NOSIZE))
	{
		Layout();
		bHandled = TRUE;
	}
	return 0;
}

void 
CConsoleWindow::InitList()
{
	LOGFONT logFont = { 0 };
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = 90;
	lstrcpy(logFont.lfFaceName, _T("Courier New"));
	logFont.lfWeight = FW_NORMAL;
	logFont.lfItalic = (BYTE)FALSE;
	m_Font.CreatePointFontIndirect(&logFont);

	m_LoggerConsole.SetSmoothScroll(FALSE);
	m_LoggerConsole.SetFocusItem(FALSE);
	m_LoggerConsole.SetFocusSubItem(FALSE);
	m_LoggerConsole.SetSingleSelect(TRUE);
	m_LoggerConsole.SetDragDrop(FALSE);

	m_LoggerConsole.SetImageList(m_ImageList);
}

void                                   
CConsoleWindow::Refresh()
{
	// track new item
	m_LoggerConsole.EnsureItemVisible(m_LoggerConsole.GetItemCount()-1);
	m_LoggerConsole.ResetScrollBars();
	m_LoggerConsole.Invalidate();
}

bool 
CConsoleWindow::SetLogger( CLogger* logger )
{
	m_Logger = logger;
	return true;	
}