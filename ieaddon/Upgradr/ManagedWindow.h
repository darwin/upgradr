#pragma once

enum EWindowFamily {
	EWF_GENERAL,
	EWF_DOMEXPLORER,
	EWF_CONSOLE,
	EWF_LAST
};

class CManagedWindowHandle {
public:
	CManagedWindowHandle() : m_Pane(NULL) {}
	CTabbedAutoHideDockingWindow&						GetPane() { ATLASSERT(!!m_Pane); return *m_Pane; }
	HWND                                         GetPaneWorkspace(HWND& topMostBox);

	CTabbedAutoHideDockingWindow*                m_Pane;
	EWindowFamily											m_Family;
	TBrowserId                                   m_BrowserId; ///< id of browser which created this window

	TWindowId                                    m_Id;
};

template<class T>
class CManagedWindow : public CManagedWindowHandle, public T {
public:
	CManagedWindow();
	~CManagedWindow();

	virtual bool											Init(HWND workspaceWnd, EWindowFamily family, TBrowserId browserId, LPCTSTR name, HICON icon, CRect& rc);
	virtual bool											Done();
protected:
};

template<class T>
CManagedWindow<T>::CManagedWindow()
{

}

template<class T>
CManagedWindow<T>::~CManagedWindow()
{
	Done();
}

//////////////////////////////////////////////////////////////////////////

template <class T> bool 
CManagedWindow<T>::Init(HWND workspaceWnd, EWindowFamily family, TBrowserId browserId, LPCTSTR name, HICON icon, CRect& rc)
{
	ATLASSERT(!m_Pane);

	CManagedWindowHandle::m_BrowserId = browserId;
	m_Family = family;

	m_Pane = CTabbedAutoHideDockingWindow::CreateInstance(); // this is needed, docking manager will keep track of this window

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	HWND hWndPane = m_Pane->Create(workspaceWnd, rc, name, dwStyle);

	SetParent(hWndPane);
	SetIcon(icon, ICON_SMALL);

	m_Pane->SetReflectNotifications(false);
	m_Pane->SetClient(*this);

	return true;
}

template <class T> bool
CManagedWindow<T>::Done()
{
	ATLASSERT(!!m_Pane);
	// pane is deleted when window is destroyed - see TabbedDockingWindow line 292
	m_Pane = NULL;
	return true;
}


