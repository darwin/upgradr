// UpgradrToolbar.cpp : Implementation of CUpgradrToolbar

#include "stdafx.h"
#include "UpgradrToolbar.h"
#include "Services.h"
#include "Script.h"
#include "ScriptManager.h"
#include "AboutBox.h"
#include "DialogManager.h"

//#include "Debug.h"

const wchar_t *g_wcToolbarWindowText = _T("UpgradrToolbarWindow");

// CUpgradrToolbar
CUpgradrToolbar::CUpgradrToolbar():
m_kToolbar(TOOLBARCLASSNAME, this, ALT_MAP_TB_TOOLBAR),
m_dwBandId(0), 
m_dwViewMode(0), 
m_iToolbarHeight(22)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: constructor"), this)));

	CBitmap toolbar;
	CBitmap mask;

	m_kAccelerator.LoadAccelerators(MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if (m_kAccelerator.IsNull()) throw CUpgradrWindowsError(GetLastError());

	toolbar.LoadBitmap(MAKEINTRESOURCE(IDB_TOOLBAR));
	if (toolbar.IsNull()) throw CUpgradrWindowsError(GetLastError());

	mask.LoadBitmap(MAKEINTRESOURCE(IDB_TOOLBARMASK));
	if (mask.IsNull()) throw CUpgradrWindowsError(GetLastError());

	CDPIHelper::ScaleBitmap(toolbar);
	CDPIHelper::ScaleBitmap(mask);

	int width = (int)CDPIHelper::ScaleX(16);
	int height = (int)CDPIHelper::ScaleY(16);
	m_kImageList.Create(width, height, ILC_COLOR24 | ILC_MASK, 3, 3);
	if (m_kImageList.IsNull()) throw CUpgradrWindowsError(GetLastError());
	if (m_kImageList.Add(toolbar, mask) == -1) throw CUpgradrWindowsError(GetLastError());
}

CUpgradrToolbar::~CUpgradrToolbar()
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: destructor"), this)));
	m_kToolbar.DestroyWindow();
	UnsubclassWindow();
}

STDMETHODIMP 
CUpgradrToolbar::GetWindow(HWND *phwnd)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: GetWindow(%08X)"), this, phwnd)));
	*phwnd = m_kToolbar;
	return S_OK;
}

STDMETHODIMP 
CUpgradrToolbar::ContextSensitiveHelp(BOOL fEnterMode)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: ContextSensitiveHelp(%s)"), this, fEnterMode?_T("TRUE"):_T("FALSE"))));
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrToolbar::ShowDW(BOOL fShow)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: ShowDW(%s)"), this, fShow?_T("TRUE"):_T("FALSE"))));
	if (m_kToolbar)
	{
		ShowWindow(fShow ? SW_SHOW : SW_HIDE);
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrToolbar::CloseDW(DWORD dwReserved)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: CloseDW(%08X)"), this, dwReserved)));
	ShowDW(FALSE);
	// CloseDW purposely does not destroy the window. We leave that to the destructor.
	return S_OK;
}

STDMETHODIMP 
CUpgradrToolbar::ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved)
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrToolbar::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
	if (fActivate)	SetFocus();
	return S_OK;
}

STDMETHODIMP 
CUpgradrToolbar::HasFocusIO()
{
	// generic implementation, override in base class
	return S_FALSE;
}

STDMETHODIMP 
CUpgradrToolbar::TranslateAcceleratorIO(LPMSG lpMsg)
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
CUpgradrToolbar::SetSite(IUnknown *pUnknownSite)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: SetSite(%08X)"), this, pUnknownSite)));
	try {
		if (!!pUnknownSite)
		{
			// attach the window
			HWND hMyWindow;
			CComPtr<IUnknown> site(pUnknownSite);
			CComQIPtr<IOleWindow> window(site);
			window->GetWindow(&hMyWindow);
			if (!hMyWindow) 
			{
				TRACE_E(FS(_T("Toolbar[%08X]: Cannot retrieve toolbar base window"), this));
				return E_FAIL;
			}
			SubclassWindow(hMyWindow);

			// get a WebBrowser reference
			CComQIPtr<IServiceProvider> serviceProvider(site);
			serviceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&m_spWebBrowser);
			site->QueryInterface(IID_IInputObjectSite, (void**)&m_spSite);

			// create main window
			CreateMainWindow();
		}
	}
	catch (CUpgradrRuntimeError &ex)
	{
		HandleError(ex.ErrorMessage());
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrToolbar::GetSite(const IID &riid, void **ppvSite)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: GetSite(...)"), this)));
	if (!ppvSite) return E_INVALIDARG;
	*ppvSite = NULL;
	return m_spSite->QueryInterface(riid, ppvSite);
}

STDMETHODIMP 
CUpgradrToolbar::GetBandInfo(DWORD dwBandId, DWORD dwViewMode, DESKBANDINFO *pdbi)
{
	DT(TRACE_I(FS(_T("Toolbar[%08X]: GetBandInfo(...)"), this)));
	try
	{
		if( pdbi == NULL )
			return E_INVALIDARG;

		m_dwBandId = dwBandId;
		m_dwViewMode = dwViewMode;

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
			pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_USECHEVRON | DBIMF_BREAK;
		}

		if( pdbi->dwMask & DBIM_BKCOLOR )
		{
			//Use the default background color by removing this flag.
			pdbi->dwMask &= ~DBIM_BKCOLOR;
		}

		return S_OK;
	}
	catch (CUpgradrRuntimeError &ex)
	{
		HandleError(ex.ErrorMessage());
		return E_FAIL;
	}
}

void 
CUpgradrToolbar::HandleError(const CString &errorMessage)
{
	CString message = LoadStringResource(IDS_ERROR_BASEMESSAGE);
	message += errorMessage;
	::MessageBox(NULL, message, LoadStringResource(IDS_TOOLBAR_NAME), MB_OK | MB_ICONERROR);
}

void 
CUpgradrToolbar::CreateMainWindow()
{
	DWORD style = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | CCS_TOP | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT;
	m_kToolbar.Create(*this, rcDefault, MAIN_TOOLBAR_WINDOW_NAME, style, 0, IDC_TB_TOOLBAR);
	RECT  rc;
	GetClientRect(&rc);
	
	m_kToolbar.SetWindowText(g_wcToolbarWindowText);
	m_kToolbar.SendMessage(TB_SETEXTENDEDSTYLE, 0, SendMessage(m_kToolbar, TB_GETEXTENDEDSTYLE, 0, 0) | TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);
	m_kToolbar.SendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
	m_kToolbar.SendMessage(TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(static_cast<HIMAGELIST>(m_kImageList)));

	TBBUTTON button[2];
	ZeroMemory(button, sizeof(button));

	button[0].iBitmap = 0;
	button[0].idCommand = IDC_TOOLBUTTON;
	button[0].fsState = TBSTATE_ENABLED;
	button[0].fsStyle = TBSTYLE_CHECK | BTNS_DROPDOWN | BTNS_AUTOSIZE;
	button[0].iString = 0;

	button[1].iBitmap = 1;
	button[1].idCommand = IDC_DISABLEBUTTON;
	button[1].fsState = TBSTATE_ENABLED;
	button[1].fsStyle = TBSTYLE_CHECK | BTNS_DROPDOWN | BTNS_AUTOSIZE;
	button[1].iString = 0;

	// Add the buttons
	if (!m_kToolbar.SendMessage(TB_ADDBUTTONS, sizeof(button) / sizeof(TBBUTTON), reinterpret_cast<LPARAM>(button))) 
		throw CUpgradrWindowsError(GetLastError());

	// In order to make sure the edit control has the right height regardless of font size settings,
	// we give it the same height as the toolbar buttons.
	LRESULT size = m_kToolbar.SendMessage(TB_GETBUTTONSIZE, 0, 0);
	int height = HIWORD(size);
	int width = LOWORD(size);
	m_iToolbarHeight = height;
}

LRESULT 
CUpgradrToolbar::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Notify IE of the focus change
	DT(TRACE_I(FS(_T("Toolbar[%08X]: Focus gained"), this)));
	m_spSite->OnFocusChangeIS(static_cast<IDockingWindow*>(this), TRUE);
	return S_OK;
}

LRESULT 
CUpgradrToolbar::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Notify IE of the focus change
	DT(TRACE_I(FS(_T("Toolbar[%08X]: Focus lost"), this)));
	m_spSite->OnFocusChangeIS(static_cast<IDockingWindow*>(this), FALSE);
	return S_OK;
}

LRESULT 
CUpgradrToolbar::OnToolbarDropdown(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled)
{
	// dispatcher for button popup menus
	LPNMTOOLBAR data = (LPNMTOOLBAR)lParam;
	switch (data->iItem) {
		case IDC_TOOLBUTTON: OnGeneralDropdown(data); break;
		case IDC_DISABLEBUTTON: OnDisableDropdown(data); break;
		default: ATLASSERT(0);
	}
	bHandled = TRUE;
	return 0;
}

bool
CUpgradrToolbar::OnGeneralDropdown(LPNMTOOLBAR data)
{
	RECT rect;
	m_kToolbar.SendMessage(TB_GETRECT, data->iItem, reinterpret_cast<LPARAM>(&rect));
	m_kToolbar.MapWindowPoints(HWND_DESKTOP, reinterpret_cast<LPPOINT>(&rect), 2);
	TPMPARAMS tpm;
	tpm.cbSize = sizeof(tpm);
	tpm.rcExclude = rect;

	CMenu menu;
	menu.LoadMenu(MAKEINTRESOURCE(IDR_TOOLMENU));
	if (menu.IsNull()) throw CUpgradrWindowsError(GetLastError());

	CMenuHandle popupMenu = menu.GetSubMenu(0);
	if (popupMenu.IsNull()) throw CUpgradrWindowsError(GetLastError());

	TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL, rect.left, rect.bottom, m_kToolbar, &tpm);
	return true;
}

bool
CUpgradrToolbar::OnDisableDropdown(LPNMTOOLBAR data)
{
	RECT rect;
	m_kToolbar.SendMessage(TB_GETRECT, data->iItem, reinterpret_cast<LPARAM>(&rect));
	m_kToolbar.MapWindowPoints(HWND_DESKTOP, reinterpret_cast<LPPOINT>(&rect), 2);
	TPMPARAMS tpm;
	tpm.cbSize = sizeof(tpm);
	tpm.rcExclude = rect;

	CMenu menu;
	menu.LoadMenu(MAKEINTRESOURCE(IDR_DISABLEMENU));
	if (menu.IsNull()) throw CUpgradrWindowsError(GetLastError());

	ScriptManagerLock scriptManager;
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	UINT pos = 0;
	UINT uID = 1000;

	CMenuHandle popupMenu = menu.GetSubMenu(0);
	if (popupMenu.IsNull()) throw CUpgradrWindowsError(GetLastError());

	CScriptManager::iterator i = scriptManager->begin();
	while (i!=scriptManager->end())
	{
		CScript* script = i->second;

		memset(&mii, 0, sizeof(MENUITEMINFO)); 
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID | MIIM_DATA;
		mii.wID = uID++;
		mii.fState = MFS_CHECKED;
		mii.dwItemData = i->first;
		TCHAR buf[1024];
		_tcscpy_s(buf, 1024, script->Name());
		mii.dwTypeData = buf;
		InsertMenuItem(popupMenu, pos++, TRUE, &mii);

		++i;
	}

	TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL, rect.left, rect.bottom, m_kToolbar, &tpm);
	return true;
}

CString 
CUpgradrToolbar::GetTitle()
{
	return LoadStringResource(IDS_TOOLBAR_CAPTION);
}

LRESULT
CUpgradrToolbar::OnToolbarMenu(WORD wCode, WORD wId, HWND hWnd, BOOL& bHandled)
{
	switch (wId) {
		case ID_POPUPMENU_OPTIONS:
			// bring options dialog
			{
				ServicesLock services;
				services->OpenSettingsDialog();
			}
			break;
		case ID_POPUPMENU_ABOUT:
			//::MessageBox(NULL, LoadStringResource(IDS_ABOUT), _T("About box"), MB_OK);
			{
				CAboutBox kAboutBox;
				kAboutBox.DoModal();
			}
			return true;
		case ID_POPUPMENU_GENERATECRASH:
			{
				int* p = 0;
				*p = 1;
			}
			break;
		case ID_POPUPMENU_TESTDIALOG:
			{
				//int nButtonPressed = 0;
				//::TaskDialog(NULL, GetBaseModule().GetModuleInstance(), _T("Title"), _T("Nejakej text"), _T("nevim"),
				//	TDCBF_OK_BUTTON, TD_INFORMATION_ICON, &nButtonPressed);

				DialogManagerLock dialogManager;
				dialogManager->TestDialog();
			}
			break;
		case ID_POPUPMENU_VISITSITE:
		case ID_POPUPMENU_CHECKFORUPDATES:
			{
				//ComPtr<IWebBrowser2> browser = GetBrowser();
				//BSTR url = SysAllocString(menu == ID_POPUPMENU_VISITOOKII ? LoadStringResource(IDS_URL_OOKII).c_str() : LoadStringResource(IDS_URL_UPDATE).c_str());
				//_variant_t target = _T("_blank");
				//browser->Navigate(url, NULL, &target, NULL, NULL);
				//SysFreeString(url);
				return true;
			}
	}
	return 0;
}
