// UpgradrBHO.h : Declaration of the CUpgradrBHO

#pragma once
#include "resource.h"       // main symbols

#include "DOMRenderWindow.h"
#include "FrameTable.h"
#include "BrowserWatcher.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CUpgradrBHO
class ATL_NO_VTABLE CUpgradrBHO :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrBHO, &CLSID_UpgradrBHO>,
	public IObjectWithSiteImpl<CUpgradrBHO>,
	public IDispatchImpl<IUpgradrBHO, &IID_IUpgradrBHO, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispEventImpl<1, CUpgradrBHO, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>,
	public IDocHostUIHandler,
	public IOleCommandTarget,
	public WinTrace
{
public:
	CUpgradrBHO();
	virtual ~CUpgradrBHO();

	DECLARE_CLASS_SIGNATURE(CUpgradrBHO)
	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRBHO)
	DECLARE_NOT_AGGREGATABLE(CUpgradrBHO)

	BEGIN_COM_MAP(CUpgradrBHO)
		COM_INTERFACE_ENTRY(IUpgradrBHO)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IObjectWithSite)
		COM_INTERFACE_ENTRY(IDocHostUIHandler)
		COM_INTERFACE_ENTRY(IOleCommandTarget)
	END_COM_MAP()

	BEGIN_SINK_MAP(CUpgradrBHO)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTRELOAD, OnDocumentReload)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_ONQUIT, OnQuit)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOWNLOADBEGIN, OnDownloadBegin)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOWNLOADCOMPLETE, OnDownloadComplete)
		
//		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_COMMANDSTATECHANGE, OnCommandStateChange)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_PROGRESSCHANGE, OnProgressChange) // needed for processing F5 (refresh)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_PROPERTYCHANGE, OnPropertyChange)
	END_SINK_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	// IObjectWithSite 
	STDMETHOD(SetSite)(IUnknown *pUnkSite);

	// DWebBrowserEvents2
	STDMETHOD(OnDocumentComplete)(IDispatch* pDisp, VARIANT* URL);
	STDMETHOD(OnNavigateComplete2)(IDispatch* pDisp, VARIANT* URL);
	STDMETHOD(OnDocumentReload)(IDispatch* pDisp);
	STDMETHOD(OnQuit)(void);
	STDMETHOD(OnProgressChange)(long Progress, long ProgressMax);
	STDMETHOD(OnCommandStateChange)(long Command, VARIANT_BOOL Enable);
	STDMETHOD(OnBeforeNavigate2)(IDispatch* pDisp, VARIANT*& url, VARIANT*& Flags, VARIANT*& TargetFrameName, VARIANT*& PostData, VARIANT*& Headers, VARIANT_BOOL*& Cancel);
	STDMETHOD(OnPropertyChange)(BSTR szProperty);
	STDMETHOD(OnDownloadBegin)();
	STDMETHOD(OnDownloadComplete)();

	// IDocHostUIHandler
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT FAR* ppt, IUnknown FAR* pcmdTarget, IDispatch FAR* pdispReserved);
	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject FAR* pActiveObject, IOleCommandTarget FAR* pCommandTarget, IOleInPlaceFrame  FAR* pFrame, IOleInPlaceUIWindow FAR* pDoc);
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO FAR *pInfo);
	STDMETHOD(HideUI)();
	STDMETHOD(UpdateUI)();
	STDMETHOD(EnableModeless)(BOOL fEnable);
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow FAR* pUIWindow, BOOL fFrameWindow);
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID);
	STDMETHOD(GetOptionKeyPath)(LPOLESTR FAR* pchKey, DWORD dw);
	STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget);
	STDMETHOD(GetExternal)(IDispatch** ppDispatch);
	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut);
	STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet);

	// IOleCommandTarget
	STDMETHOD(QueryStatus)(/*[in]*/ const GUID *pguidCmdGroup, /*[in]*/ ULONG cCmds,	/*[in,out][size_is(cCmds)]*/ OLECMD *prgCmds, /*[in,out]*/ OLECMDTEXT *pCmdText);
	STDMETHOD(Exec)(/*[in]*/ const GUID *pguidCmdGroup, /*[in]*/ DWORD nCmdID, /*[in]*/ DWORD nCmdExecOpt, /*[in]*/ VARIANTARG *pvaIn, /*[in,out]*/ VARIANTARG *pvaOut);

	// helpers
	bool                                         RefreshDOMExplorer();
	CComPtr<IWebBrowser2>	                     GetTopBrowser() const { return m_TopBrowser; }

	void														FindMissingLinks(CComQIPtr<IHTMLDocument2> doc, TDocumentContainer& links);
	HRESULT                                      ProcessDocumentRefresh(IDispatch *pDisp, VARIANT *pvarURL);

private:
	// Helpers
	bool														SetupEnvironment();
	bool														OnRefreshStart();
	bool														OnRefreshEnd();

	// 
	HRESULT													ProcessDocument(IDispatch *pDisp, VARIANT *pvarURL);

	TBrowserId                                   m_BrowserId; ///< Id of this BHO 
	CComPtr<IWebBrowser2>								m_TopBrowser; ///< top level browser window in BHO's tab
	CFrameTable												m_FrameTable;

	// Default interface pointers
	CComPtr<IDocHostUIHandler>							m_DefaultDocHostUIHandler;
	CComPtr<IOleCommandTarget>							m_DefaultOleCommandTarget;
	CComPtr<ICustomDoc>									m_HookedDoc;
	BrowserWatcher*								      m_BrowserWatcher;

	bool														m_IsAdvised; 
	CDOMRenderWindow										m_DOMRenderWindow;
	bool														m_RequestReload;
	bool														m_DownloadInProgress;
	bool                                         m_DontResizeIEWindow;
	bool														m_FirstTimeNavigate;       ///< this is here for preventing document warning in GetDocument first time
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrBHO), CUpgradrBHO)
