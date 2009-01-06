// UpgradrHelperbar.cpp : Implementation of CUpgradrHelperbar

#include "stdafx.h"
#include "UpgradrHelperbar.h"
#include "BrowserManager.h"
#include "Module.h"

//#include "Debug.h"

// CUpgradrHelperbar
CUpgradrHelperbar::CUpgradrHelperbar():
m_BandId(0), 
m_ViewMode(0),
m_BrowserId(NULL_BROWSER),
m_Toolbar(TOOLBARCLASSNAME, this, ALT_MAP_HELPERBAR_TOOLBAR),
m_UnpinButtonIndex(-1)
{
	InitRoot();
	DT(TRACE_I(FS(_T("Helperbar[%08X]: constructor"), this)));
}

CUpgradrHelperbar::~CUpgradrHelperbar()
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: destructor"), this)));
	DoneRoot();
}

STDMETHODIMP 
CUpgradrHelperbar::GetWindow(HWND *phwnd)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: GetWindow(%08X)"), this, phwnd)));
	*phwnd = m_MainWindow;
	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::ContextSensitiveHelp(BOOL fEnterMode)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: ContextSensitiveHelp(%s)"), this, fEnterMode?_T("TRUE"):_T("FALSE"))));
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrHelperbar::ShowDW(BOOL fShow)
{
	if (m_MainWindow)
	{
		m_MainWindow.ShowWindow(fShow ? SW_SHOW : SW_HIDE);
	}
	if (m_Toolbar)
	{
		if (fShow) AddToolbarUnpinButton(); else RemoveToolbarUnpinButton();
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::CloseDW(DWORD dwReserved)
{
	ShowDW(FALSE);
	// CloseDW purposely does not destroy the window. We leave that to the destructor.
	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkHelperbarSite, BOOL fReserved)
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrHelperbar::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
	if( fActivate) SetFocus();

	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::HasFocusIO()
{
	// generic implementation, override in base class
	return S_FALSE;
}

STDMETHODIMP 
CUpgradrHelperbar::TranslateAcceleratorIO(LPMSG lpMsg)
{
	// generic implementation, override in base class to handle accelerators
	if( (lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_KEYUP) && 
		(lpMsg->wParam == VK_TAB || lpMsg->wParam == VK_F6) )
		return S_FALSE;
	else
	{
		TranslateMessage(lpMsg);
		DispatchMessage(lpMsg);
		return S_OK;
	}
}

STDMETHODIMP 
CUpgradrHelperbar::SetSite(IUnknown *pUnknownSite)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: SetSite(%08X)"), this, pUnknownSite)));
	
	if (!!pUnknownSite)
	{
		// get a WebBrowser reference
		CComPtr<IUnknown> site(pUnknownSite);
		CComQIPtr<IServiceProvider> serviceProvider(site);
		serviceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&m_Browser);
		site->QueryInterface(IID_IInputObjectSite, (void**)&m_Site);

		// retrive browser id
		{
			BrowserManagerLock browserManager;
			m_BrowserId = browserManager->AllocBrowserId(m_Browser, this);
			ATLASSERT(m_BrowserId!=NULL_BROWSER);
		}

		// attach the window
		HWND hHelperbarWindow;
		CComQIPtr<IOleWindow> window(site);
		window->GetWindow(&hHelperbarWindow);
		if (!hHelperbarWindow) 
		{
			TRACE_E(FS(_T("Helperbar[%08X]: Cannot retrieve helpbar base window"), this));
			return E_FAIL;
		}
		SubclassWindow(hHelperbarWindow);

		// create main window
		CreateMainWindow();
	}
	else
	{
		BrowserManagerLock browserManager;
		browserManager->ReleaseBrowserId(m_BrowserId);
		m_BrowserId = NULL_BROWSER;
	}

	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::GetSite(const IID &riid, void **ppvSite)
{
	if (!ppvSite) return E_INVALIDARG;
	*ppvSite = NULL;
	return m_Site->QueryInterface(riid, ppvSite);
}

STDMETHODIMP 
CUpgradrHelperbar::GetBandInfo(DWORD dwBandId, DWORD dwViewMode, DESKBANDINFO *pdbi)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: GetBandInfo(...)"), this)));
	if (!pdbi) return E_INVALIDARG;
	m_BandId = dwBandId;
	m_ViewMode = dwViewMode;

	if( pdbi->dwMask & DBIM_MINSIZE )
	{
		pdbi->ptMinSize = GetMinSize();
	}

	if( pdbi->dwMask & DBIM_MAXSIZE )
	{
		pdbi->ptMaxSize = GetMaxSize();
	}

	if( pdbi->dwMask & DBIM_INTEGRAL )
	{
		pdbi->ptIntegral.x = 1;
		pdbi->ptIntegral.y = 1;
	}

	if( pdbi->dwMask & DBIM_ACTUAL )
	{
		pdbi->ptActual = GetActualSize();
	}

	if( pdbi->dwMask & DBIM_TITLE )
	{
		wcscpy_s(pdbi->wszTitle, GetTitle());
	}

	if( pdbi->dwMask & DBIM_MODEFLAGS )
	{
		pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT | DBIMF_DEBOSSED;
	}

	if( pdbi->dwMask & DBIM_BKCOLOR )
	{
		//Use the default background color by removing this flag.
		pdbi->dwMask &= ~DBIM_BKCOLOR;
	}
	return S_OK;
}

void 
CUpgradrHelperbar::CreateMainWindow()
{
	// Only create the window if it doesn't exist yet.
	if (m_MainWindow) return;

	HWND parent = GetParent();
	if (!parent) throw CUpgradrRuntimeError(LoadStringResource(IDS_ERROR_NOPARENTWINDOW));

	ATLASSERT(m_BrowserId!=NULL_BROWSER);
	m_MainWindow.SetBrowserId(m_BrowserId);
	m_MainWindow.SetDisplayMode(CHelperbarWindow::EDM_PINNED);
	m_MainWindow.Create(parent, rcDefault, HELPERBAR_WINDOW_NAME);

	// subclass toolbar
	HWND hToolbarWnd = FindWindowEx(m_hWnd, NULL, TOOLBARCLASSNAME, NULL);
	m_Toolbar.SubclassWindow(hToolbarWnd);

	// make our new button visible
	m_Toolbar.ModifyStyle(0, TBSTYLE_WRAPABLE);
	m_Toolbar.ResizeClient(20, 40);
}

bool                                         
CUpgradrHelperbar::AddToolbarUnpinButton()
{
	if (m_UnpinButtonIndex!=-1) return true;

	TBBUTTON button[1];
	ZeroMemory(button, sizeof(button));

	button[0].iBitmap = 0;
	button[0].idCommand = IDC_UNPINBUTTON;
	button[0].fsState = TBSTATE_ENABLED;
	button[0].fsStyle = TBSTYLE_BUTTON;
	button[0].iString = (INT_PTR)_T("Unpin");

	m_UnpinButtonIndex = (int)m_Toolbar.SendMessage(TB_BUTTONCOUNT);
	//// Add the buttons
	if (!m_Toolbar.SendMessage(TB_ADDBUTTONS, sizeof(button) / sizeof(TBBUTTON), reinterpret_cast<LPARAM>(button))) 
		throw CUpgradrWindowsError(GetLastError());

	return true;
}

bool                                         
CUpgradrHelperbar::RemoveToolbarUnpinButton()
{
	if (m_UnpinButtonIndex==-1) return true;

	m_Toolbar.SendMessage(TB_DELETEBUTTON, m_UnpinButtonIndex, 0);
	m_UnpinButtonIndex = -1;
	return true;
}

CString 
CUpgradrHelperbar::GetTitle()
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: GetTitle()"), this)));
	return LoadStringResource(IDS_HELPERBAR_CAPTION);
}

STDMETHODIMP 
CUpgradrHelperbar::GetClassID(LPCLSID pClassId)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: GetClassID()"), this)));
	if (!pClassId)	return E_INVALIDARG;

	*pClassId = CLSID_UpgradrHelperbar;
	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::IsDirty()
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: IsDirty()"), this)));
	return S_FALSE;
}

STDMETHODIMP 
CUpgradrHelperbar::Load(IStream *pStm)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: Load(%08X)"), this, pStm)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::Save(IStream *pStm, BOOL fClearDirty)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: Save(%08X, %s)"), this, pStm, fClearDirty?_T("TRUE"):_T("FALSE"))));
	return S_OK;
}

STDMETHODIMP 
CUpgradrHelperbar::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
	DT(TRACE_I(FS(_T("Helperbar[%08X]: GetSizeMax(...)"), this)));
	return E_NOTIMPL;
}

CComPtr<IWebBrowser2> CUpgradrHelperbar::GetBrowser()
{
	return m_Browser;
}

POINTL CUpgradrHelperbar::GetMinSize() const
{
	POINTL pt = { -1, 100 };
	return pt;
}

POINTL CUpgradrHelperbar::GetMaxSize() const
{
	POINTL pt = { -1, -1 };
	return pt;
}

POINTL CUpgradrHelperbar::GetActualSize() const
{
	POINTL pt = { -1, -1 };
	return pt;
}

LRESULT 
CUpgradrHelperbar::OnUnpin(WORD wCode, WORD wId, HWND hWnd, BOOL& bHandled)
{
	// recreate window with floating mode
	m_MainWindow.Unpin();

	return S_OK;
}
