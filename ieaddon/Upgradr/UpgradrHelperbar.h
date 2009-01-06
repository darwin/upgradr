// UpgradrHelperbar.h : Declaration of the CUpgradrHelperbar

#pragma once
#include "resource.h"       // main symbols

#include "HelperbarWindow.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CUpgradrHelperbar

#define ALT_MAP_HELPERBAR_TOOLBAR					1
#define IDC_UNPINBUTTON									187

class ATL_NO_VTABLE CUpgradrHelperbar :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrHelperbar, &CLSID_UpgradrHelperbar>,
	public IObjectWithSiteImpl<CUpgradrHelperbar>,
	public IDispatchImpl<IUpgradrHelperbar, &IID_IUpgradrHelperbar, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDeskBand,
	public IPersistStream,
	public CWindowImpl<CUpgradrHelperbar>
{
public:
	CUpgradrHelperbar();
	virtual ~CUpgradrHelperbar();

DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRHELPERBAR)

DECLARE_NOT_AGGREGATABLE(CUpgradrHelperbar)

BEGIN_COM_MAP(CUpgradrHelperbar)
	COM_INTERFACE_ENTRY(IUpgradrHelperbar)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IDeskBand)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(IDockingWindow)
END_COM_MAP()

BEGIN_MSG_MAP(CUpgradrHelperbar)
	COMMAND_ID_HANDLER(IDC_UNPINBUTTON, OnUnpin)
	ALT_MSG_MAP(ALT_MAP_HELPERBAR_TOOLBAR)
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
	STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkHelperbarSite, BOOL fReserved);

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

	// IPersistStream
	STDMETHODIMP GetClassID(LPCLSID pClassId);
	STDMETHODIMP IsDirty();
	STDMETHODIMP Load(IStream *pStm);
	STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
	STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);

	virtual LRESULT OnUnpin(WORD wCode, WORD wId, HWND hWnd, BOOL& bHandled);


protected:
	CComPtr<IWebBrowser2>								GetBrowser();

	virtual POINTL 										GetMinSize() const;
	virtual POINTL 										GetMaxSize() const;
	virtual POINTL 										GetActualSize() const;

	virtual void											CreateMainWindow();
	virtual CString										GetTitle();

	bool                                         AddToolbarUnpinButton();
	bool                                         RemoveToolbarUnpinButton();

	CComPtr<IWebBrowser2>                       	m_Browser;
	TBrowserId                                   m_BrowserId;
	CComPtr<IInputObjectSite>                   	m_Site;
	DWORD                                       	m_BandId;
	DWORD                                       	m_ViewMode;

	CHelperbarWindow                        		m_MainWindow;
	CContainedWindow 										m_Toolbar;
	int														m_UnpinButtonIndex;
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrHelperbar), CUpgradrHelperbar)
