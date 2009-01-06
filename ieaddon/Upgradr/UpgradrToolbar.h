// UpgradrToolbar.h : Declaration of the CUpgradrToolbar

#pragma once
#include "resource.h"       // main symbols

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#define ALT_MAP_TB_TOOLBAR 1
#define IDC_TB_TOOLBAR 1

// CUpgradrToolbar
class ATL_NO_VTABLE CUpgradrToolbar :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrToolbar, &CLSID_UpgradrToolbar>,
	public IObjectWithSiteImpl<CUpgradrToolbar>,
	public IDispatchImpl<IUpgradrToolbar, &IID_IUpgradrToolbar, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDeskBand,
	public CWindowImpl<CUpgradrToolbar>
{
public:
	CUpgradrToolbar();
	virtual ~CUpgradrToolbar();

DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRTOOLBAR)

DECLARE_NOT_AGGREGATABLE(CUpgradrToolbar)

BEGIN_COM_MAP(CUpgradrToolbar)
	COM_INTERFACE_ENTRY(IUpgradrToolbar)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IDeskBand)
END_COM_MAP()

BEGIN_MSG_MAP(CUpgradrToolbar)
	NOTIFY_HANDLER(IDC_TB_TOOLBAR, TBN_DROPDOWN, OnToolbarDropdown)
	ALT_MSG_MAP(ALT_MAP_TB_TOOLBAR)
	COMMAND_CODE_HANDLER(0, OnToolbarMenu) // 0 == from menu
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	// IOleWindow (inherited from IDeskBand)
	STDMETHODIMP GetWindow(HWND *phwnd);
	STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

	// IDockingWindow (inherited from IDeskBand)
	STDMETHODIMP ShowDW(BOOL fShow);
	STDMETHODIMP CloseDW(DWORD dwReserved);
	STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved);

	// IDeskBand
	STDMETHODIMP GetBandInfo(DWORD dwBandId, DWORD dwViewMode, DESKBANDINFO *pdbi);

	// IInputObject
	STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);
	// A deriving class should override HasFocusIO to properly indicate when its UI has focus
	STDMETHODIMP HasFocusIO();
	STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

	// IObjectWithSite
	STDMETHODIMP SetSite(IUnknown *punkSite);
	STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

	static void HandleError(const CString &errorMessage);

protected:
	CComPtr<IWebBrowser2> GetBrowser()
	{
		return m_spWebBrowser;
	}

	virtual POINTL GetMinSize() const
	{
		POINTL pt = { 20, m_iToolbarHeight };
		return pt;
	}
	virtual POINTL GetMaxSize() const
	{
		POINTL pt = { -1, m_iToolbarHeight };
		return pt;
	}
	virtual POINTL GetActualSize() const
	{
		POINTL pt = { -1, m_iToolbarHeight };
		return pt;
	}

	virtual void CreateMainWindow();
	virtual CString GetTitle();

	virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnToolbarDropdown(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled);
		virtual bool OnGeneralDropdown(LPNMTOOLBAR data);
		virtual bool OnDisableDropdown(LPNMTOOLBAR data);

	virtual LRESULT OnToolbarMenu(WORD wCode, WORD wId, HWND hWnd, BOOL& bHandled);

	CComPtr<IWebBrowser2>                          m_spWebBrowser;
	CComPtr<IInputObjectSite>                      m_spSite;
	DWORD                                          m_dwBandId;
	DWORD                                          m_dwViewMode;

	CContainedWindow                               m_kToolbar;

	int                                            m_iToolbarHeight;
	CAccelerator                                   m_kAccelerator;
	CImageList                                     m_kImageList;
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrToolbar), CUpgradrToolbar)
