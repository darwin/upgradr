// UpgradrBHO.cpp : Implementation of CUpgradrBHO
#include "stdafx.h"
#include "UpgradrBHO.h"
#include "BrowserManager.h"
#include "ScriptDebugger.h"
#include "WindowManager.h"
#include "Module.h"

//#include "Debug.h"

IStream* g_pIStream = NULL; // stream to (un)marshall interface pointer
static int BHOCounter = 0;

CUpgradrBHO::CUpgradrBHO():
m_RequestReload(false),
m_DownloadInProgress(false),
m_DontResizeIEWindow(false),
m_BrowserId(NULL_BROWSER),
m_FirstTimeNavigate(true)
{
	InitRoot();
	DT(TRACE_I(FS(_T("BHO[%08X]: constructor"), this)));
	DT(CREATE_DEBUG_TRACE("BHO"));
}

CUpgradrBHO::~CUpgradrBHO()
{
	DT(TRACE_I(FS(_T("BHO[%08X]: destructor"), this)));
	m_FrameTable.Clear(); // this must be here to force unloading frame resources before ***
	DoneRoot();
	DT(DELETE_DEBUG_TRACE());
}

// CUpgradrBHO
STDMETHODIMP 
CUpgradrBHO::SetSite(IUnknown* pUnkSite)
{
	WATCH_EXITS;
	DTI(TRACE_I(FS(_T("BHO[%08X]: SetSite(%08X)"), this, pUnkSite)));
	if (pUnkSite != NULL)
	{
		// cache the pointer to IWebBrowser2
		CHECK(pUnkSite->QueryInterface(IID_IWebBrowser2, (void **)&m_TopBrowser));
		// register to sink events from DWebBrowserEvents2
		CHECK(DispEventAdvise(m_TopBrowser));
		m_IsAdvised = true;

		// allocate browser id
		BrowserManagerLock browserManager;
		m_BrowserId = browserManager->AllocBrowserId(m_TopBrowser, this);
		ATLASSERT(m_BrowserId!=NULL_BROWSER);
	}
	else
	{
		// unregister event sink.
		if (m_IsAdvised)
		{
			DispEventUnadvise(m_TopBrowser);
			m_IsAdvised = false;
		}

		if (!!m_HookedDoc) 
		{
			m_HookedDoc->SetUIHandler(NULL);
			m_HookedDoc = NULL;
		}

		// remove associated windows
		WindowManagerLock windowManager;
		windowManager->RemoveBrowserWindows(m_BrowserId);

		BrowserManagerLock browserManager;
		browserManager->ReleaseBrowserId(m_BrowserId);
		m_BrowserId = NULL_BROWSER;

		// release cached pointers and other resources here.
		m_TopBrowser = NULL;
	}

	// Call base class implementation.
	return IObjectWithSiteImpl<CUpgradrBHO>::SetSite(pUnkSite);
}

STDMETHODIMP 
CUpgradrBHO::OnDocumentReload(IDispatch *pDisp)
{
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::OnProgressChange(long Progress, long ProgressMax)
{
	WATCH_EXITS;
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnProgressChange(Progress=%d, ProgressMax=%d)"), this, Progress, ProgressMax)));
	if (Progress==0 && ProgressMax==0)
	{
		if (m_RequestReload)
		{
			OnRefreshEnd();
		}
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrBHO::OnCommandStateChange(long Command, VARIANT_BOOL Enable)
{
	WATCH_EXITS;
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnCommandStateChange(Command=%d, Enable=%s)"), this, Command, Enable?_T("true"):_T("false"))));
	return S_OK;
}

bool
CUpgradrBHO::SetupEnvironment()
{
	if (!!m_HookedDoc) 
	{
		m_HookedDoc->SetUIHandler(NULL);
		m_HookedDoc = NULL;
	}

	HRESULT hr;

	// get the current document object from browser...
	CComPtr<IDispatch> spDispDoc;
	CHECK(m_TopBrowser->get_Document(&spDispDoc));

	// setup infrastructure
	// get pointers to default interfaces
	CComQIPtr<IOleObject> spOleObject(spDispDoc);
	if (spOleObject)
	{
		CComPtr<IOleClientSite> spClientSite;
		hr = spOleObject->GetClientSite(&spClientSite);
		if (SUCCEEDED(hr) && spClientSite)
		{
			m_DefaultDocHostUIHandler = spClientSite;
			m_DefaultOleCommandTarget = spClientSite;
		}
	}

	// set this class to be the IDocHostUIHandler
	CComQIPtr<ICustomDoc> spCustomDoc(spDispDoc);
	if (spCustomDoc)
	{
		spCustomDoc->SetUIHandler(this);
		m_HookedDoc = spCustomDoc;
	}

	// TODO: testing
	OnRefreshEnd();

	RefreshDOMExplorer();
	return true;
}

bool                                         
CUpgradrBHO::RefreshDOMExplorer()
{
	ATLASSERT(!!m_TopBrowser);

	CComPtr<IDispatch> spDispDoc;
	m_TopBrowser->get_Document(&spDispDoc);

	CComQIPtr<IHTMLDocument2> spHTMLDoc = spDispDoc;
	if (!spHTMLDoc) return false;

	IStream*	pStream = NULL;
	if (FAILED(CoMarshalInterThreadInterfaceInStream(IID_IHTMLDocument2, spHTMLDoc, &pStream)))
	{
		return false;
	}
	ATLASSERT(pStream);
	ATLASSERT(m_BrowserId!=NULL_BROWSER);

	WindowManagerLock windowManager;
	return windowManager->RefreshDOMExplorerWindow(m_BrowserId, pStream);
}

bool 
CUpgradrBHO::OnRefreshStart()
{
	m_FrameTable.Clear();
	return true;
}

bool 
CUpgradrBHO::OnRefreshEnd()
{
	m_RequestReload = false;

	// we need to detect missing documents and simulate lightweight OnDocumentComplete on them
	TDocumentContainer links;
	CComPtr<IDispatch> spDispDoc;
	CHECK(m_TopBrowser->get_Document(&spDispDoc));
	CComQIPtr<IHTMLDocument2> spDocument = spDispDoc;
	FindMissingLinks(spDocument, links);

	TDocumentContainer::iterator i = links.begin();
	while (i!=links.end())
	{
		CComQIPtr<IHTMLDocument2> spDoc = (CComPtr<IDispatch>)*i;
		CComBSTR url;
		CHECK(spDoc->get_URL(&url));
		CComVariant varUrl(url);
		// this is lightweight version of ProcessDocument
		ProcessDocumentRefresh((CComPtr<IDispatch>)*i, &varUrl);
		++i;
	}

	// rebuild top browser
	m_FrameTable.Create(GetDocument(m_TopBrowser));

	// perform final update of frame hierarchy
	m_FrameTable.UpdateLinks(); 

	RefreshDOMExplorer();
	return true;
}

STDMETHODIMP
CUpgradrBHO::OnPropertyChange(BSTR szProperty)
{
	WATCH_EXITS;
	//CString prop(szProperty);
	//CComVariant res;
	//CHECK_COM(m_TopBrowser->GetProperty(szProperty, &res), _T("<>"));
	//CString s = VariantToString(&res);
	//DTI(TRACE_I(FS(_T("BHO[%08X]: OnPropertyChange(property=%s) to %s"), this, prop, s)));
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::OnBeforeNavigate2(IDispatch *pDisp, VARIANT *&url, VARIANT *&Flags, VARIANT *&TargetFrameName, VARIANT *&PostData, VARIANT *&Headers, VARIANT_BOOL *&Cancel)
{
	WATCH_EXITS;
	CString sUrl = VariantToString((VARIANT*)&url);
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnBeforeNavigate2(pDisp=%08X, url='%s', ...)"), this, pDisp, sUrl)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: OnBeforeNavigate2(pDisp=%08X, url='%s', ...)"), this, pDisp, sUrl)));

	// remove frame from table
	CComQIPtr<IWebBrowser2> spBrowser = pDisp;
	ATLASSERT(!!spBrowser);
	if (!spBrowser) return S_OK;

	// m_FirstTimeNavigate stuff is here for preventing document warning in GetDocument first time
	CComPtr<IHTMLDocument2> spDocument = GetDocument(spBrowser, false, !m_FirstTimeNavigate);
	m_FirstTimeNavigate = false;
	if (!spDocument) return S_OK; // this is ok, when IE is starting with no document

	if (!m_FrameTable.IsManaged(spDocument)) 
		// this is ok, IE fires OnBeforeNavigate2 for document, which was not under our control (newly created document)
		return S_OK; 

	if (!m_FrameTable.Find(spDocument))
	{
		TFrameId id = m_FrameTable.GetFrameId(spDocument);
		// this should never happen
		TRACE_E(FS(_T("Unknown frame id=%d ptr=%08X (%s)"), id, spBrowser, GetURL(spBrowser)));
	}

	// document is under our control, release it from our frame table
	m_FrameTable.Destroy(spDocument);

	return S_OK;
}

HRESULT                                        
CUpgradrBHO::ProcessDocument(IDispatch *pDisp, VARIANT *pvarURL)
{
	CString sUrl = VariantToString(pvarURL);
	DTI(TRACE_I(FS(_T("BHO[%08X]: ProcessDocument(%08p, '%s')"), this, pDisp, sUrl)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: ProcessDocument(%08p, '%s')"), this, pDisp, sUrl)));
	// process ordinary frame => 
	//  1) build frame (apply script engine and detect it's children)
	//  2) register it into frame table
	CComQIPtr<IWebBrowser2> spBrowser = pDisp;
	if (!!spBrowser)
	{
		CComPtr<IHTMLDocument2> spDocument = GetDocument(spBrowser);
		if (!spDocument) return S_OK;

		if (m_FrameTable.IsManaged(spDocument))
		{
			// we've got document already present in the structure
			// this can happen from time to time
			TFrameId id = m_FrameTable.GetFrameId(spDocument);
			// this should never happen
			DT(TRACE_W(FS(_T("Unexpected destroy of document id=%d ptr=%08X (%s)"), id, spDocument, GetURL(spDocument))));
			m_FrameTable.Destroy(spDocument);
		}
		m_FrameTable.Create(spDocument);
	}

	// test for top-level window and do some environment setup
	// retrieve the top-level window from the site.
	HWND hwnd;
	HRESULT hr = m_TopBrowser->get_HWND((LONG_PTR*)&hwnd);
	if (SUCCEEDED(hr))
	{
		HRESULT hr = S_OK;

		// query for the IWebBrowser2 interface.
		CComQIPtr<IWebBrowser2> spTempWebBrowser = pDisp;

		// is this event associated with the top-level browser?
		if (spTempWebBrowser && m_TopBrowser &&
			m_TopBrowser.IsEqualObject(spTempWebBrowser))
		{
			DT(TRACE_I("This is top level browser => setup environment"));
			SetupEnvironment();
		}
	}

	return S_OK;
}

HRESULT                                        
CUpgradrBHO::ProcessDocumentRefresh(IDispatch *pDisp, VARIANT *pvarURL)
{
	CString sUrl = VariantToString(pvarURL);
	DTI(TRACE_I(FS(_T("BHO[%08X]: ProcessDocumentRefresh(%08p, '%s')"), this, pDisp, sUrl)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: ProcessDocumentRefresh(%08p, '%s')"), this, pDisp, sUrl)));

	CComQIPtr<IHTMLDocument2> spDocument = pDisp;
	if (!spDocument) return S_OK;

	if (!m_FrameTable.IsManaged(spDocument))
	{
		m_FrameTable.Create(spDocument);
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrBHO::OnDocumentComplete(IDispatch *pDisp, VARIANT *pvarURL)
{
	WATCH_EXITS;
	CString sUrl = VariantToString(pvarURL);
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnDocumentComplete(%08p, '%s')"), this, pDisp, sUrl)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: OnDocumentComplete(%08p, '%s')"), this, pDisp, sUrl)));

	if (m_DownloadInProgress)
	{
		// hack for IE7 - it gives false OnDocumentComplete events during download
		DTI(TRACE_LI(FS(_T("BHO[%08X]: ignoring ... download in progress"), this)));
		return S_OK;
	}

	ProcessDocument(pDisp, pvarURL);
	return S_OK;
}

STDMETHODIMP 
CUpgradrBHO::OnNavigateComplete2(IDispatch *pDisp, VARIANT *pvarURL)
{
	WATCH_EXITS;
	CString sUrl = VariantToString(pvarURL);
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnNavigateComplete2(%08p, '%s')"), this, pDisp, sUrl)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: OnNavigateComplete2(%08p, '%s')"), this, pDisp, sUrl)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrBHO::OnQuit()
{
	WATCH_EXITS;
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnQuit()"), this)));
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::OnDownloadBegin()
{
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnDownloadBegin()"), this)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: OnDownloadBegin()"), this)));
	ATLASSERT(!m_DownloadInProgress);
	m_DownloadInProgress = true;
	return S_OK;
}

void														
CUpgradrBHO::FindMissingLinks(CComQIPtr<IHTMLDocument2> doc, TDocumentContainer& links)
{
	// detect all child frames
	CComQIPtr<IOleContainer> spContainer = doc;
	if (!!spContainer)
	{
		// get an enumerator for the frames
		IEnumUnknown* pEnumerator;
		HRESULT hr = spContainer->EnumObjects(OLECONTF_EMBEDDINGS, &pEnumerator);

		if (SUCCEEDED(hr))
		{
			IUnknown* pUnknown;
			ULONG uFetched;

			// enumerate all the frames
			for (UINT i = 0; S_OK == pEnumerator->Next(1, &pUnknown, &uFetched); i++)
			{
				CComQIPtr<IWebBrowser2> spChildBrowser = pUnknown;
				pUnknown->Release();
				if (!!spChildBrowser)
				{
					// well done, store test child frame
					CComPtr<IDispatch> spChildDispDoc;
					CHECK_COM(spChildBrowser->get_Document(&spChildDispDoc), FS(_T("Cannot retrieve document object from %08X"), spChildBrowser));
					CComQIPtr<IHTMLDocument2> spChildDoc = spChildDispDoc;

					// sometimes IE can return NULL spChildDispDoc 
					// 
					// (observed on pages with file download blocking)
					// test for valid doc
					if (!!spChildDoc)
					{
						TFrameId id = m_FrameTable.GetFrameId(spChildDoc, false);
						if (id==NULL_FRAME)
						{
							links.push_back(spChildDispDoc);
						}

						// recurse into children
						FindMissingLinks(spChildDoc, links);
					}
				}
			}
		}
		pEnumerator->Release();
	}
}


STDMETHODIMP
CUpgradrBHO::OnDownloadComplete()
{
	DTI(TRACE_I(FS(_T("BHO[%08X]: OnDownloadComplete()"), this)));
	DTI(TRACE_LI(FS(_T("BHO[%08X]: OnDownloadComplete()"), this)));
	ATLASSERT(m_DownloadInProgress);
	m_DownloadInProgress = false;
	return S_OK;
}

// IDocHostUIHandler
// 
// Insert our Popup Blocker sub menu into the context menu
//
HRESULT 
CUpgradrBHO::ShowContextMenu(DWORD dwID,
										POINT *ppt,
										IUnknown *pcmdTarget,
										IDispatch *pdispObject) 
{
	return S_FALSE; 
/*
	// Return S_OK to tell MSHTML not to display its own menu
	// Return S_FALSE displays default MSHTML menu

#define IDR_BROWSE_CONTEXT_MENU  24641
#define IDR_FORM_CONTEXT_MENU    24640
#define SHDVID_GETMIMECSETMENU   27
#define SHDVID_ADDMENUEXTENSIONS 53

	HRESULT hr;

	CComPtr<IOleCommandTarget> spCT;
	hr = pcmdTarget->QueryInterface(IID_IOleCommandTarget, (void**)&spCT);
	if (FAILED(hr))
		return S_FALSE;

	CComPtr<IOleWindow> spWnd;
	hr = pcmdTarget->QueryInterface(IID_IOleWindow, (void**)&spWnd);
	if (FAILED(hr))
		return S_FALSE;

	HWND hwnd;
	hr = spWnd->GetWindow(&hwnd);
	if (FAILED(hr))
		return S_FALSE;

	hr = S_FALSE;

	HINSTANCE hinstSHDOCLC = LoadLibrary(_T("SHDOCLC.DLL"));
	if (hinstSHDOCLC)
	{
		HMENU hCtxMenu = LoadMenu(hinstSHDOCLC, MAKEINTRESOURCE(IDR_BROWSE_CONTEXT_MENU));
		if (hCtxMenu)
		{
			HMENU hSubMenu = GetSubMenu(hCtxMenu, dwID);
			if (hSubMenu)
			{
				// Get the language submenu
				CComVariant var;
				hr = spCT->Exec(&CGID_ShellDocView, SHDVID_GETMIMECSETMENU, 0, NULL, &var);
				if (SUCCEEDED(hr))
				{
					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask  = MIIM_SUBMENU;
					mii.hSubMenu = (HMENU) var.byref;

					// Add language submenu to Encoding context item
					SetMenuItemInfo(hSubMenu, IDM_LANGUAGE, FALSE, &mii);

					// Insert Shortcut Menu Extensions from registry
					CComVariant var1;
					V_VT(&var1) = VT_INT_PTR;
					V_BYREF(&var1) = hSubMenu;

					CComVariant var2;
					V_VT(&var2) = VT_I4;
					V_I4(&var2) = dwID;

					hr = spCT->Exec(&CGID_ShellDocView, SHDVID_ADDMENUEXTENSIONS, 0, &var1, &var2);
					if (SUCCEEDED(hr))
					{
						// Insert our menu at the top of the context menu
						g_hPubMenu = LoadMenu(g_hinstPub, MAKEINTRESOURCE(IDR_PUBMENU));
						if (g_hPubMenu)
						{
							::InsertMenu(hSubMenu, 0, MF_POPUP | MF_BYPOSITION, (UINT_PTR) g_hPubMenu, _T("Popup Blocker"));
							::InsertMenu(hSubMenu, 1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
						}

						// Subclass IE window.
						// This is required in order to enable our menu.  Otherwise, IE 
						// disregards menu items it doesn't recognize.
#pragma warning( push )
#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )
						g_lpPrevWndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)CtxMenuWndProc);
#pragma warning( pop ) 

						// Show shortcut menu
						int nCmd = ::TrackPopupMenu(hSubMenu,
							TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
							ppt->x,
							ppt->y,
							0,
							hwnd,
							(RECT*)NULL);

						// Unsubclass IE window
#pragma warning( push )
#pragma warning( disable : 4311 )
						SetWindowLong(hwnd, GWL_WNDPROC, (LONG)g_lpPrevWndProc);
#pragma warning( pop ) 

						if (g_hPubMenu)
						{
							DestroyMenu(g_hPubMenu);
							g_hPubMenu = NULL;
						}

						if (nCmd == ID_ENABLEPOPUPBLOCKER)
						{
							EnablePopupBlocker();
						}
						else if (nCmd == ID_PLAYSOUND)
						{
							EnableSound();
						}
						else if (nCmd == ID_VISIT)
						{
							m_bBlockNewWindow = FALSE;

							CComBSTR bsUrl = _T("http://www.osborntech.com");
							CComVariant vTarget = "_BLANK";
							CComVariant vEmpty;
							m_spWebBrowser2->Navigate(bsUrl, &vEmpty, &vTarget, &vEmpty, &vEmpty);
						}
						else if (nCmd == ID_ABOUT)
						{
							CAboutDlg dlg;
							dlg.DoModal();
						}
						else
						{
							ATLTRACE(_T("ContextMenu cmd = %d\n"), nCmd);

							if (nCmd == IDM_FOLLOWLINKN)
							{
								// Allow user to open link in new window
								m_bBlockNewWindow = FALSE;
							}

							// Send selected shortcut menu item command to shell
							LRESULT lr = ::SendMessage(hwnd, WM_COMMAND, nCmd, NULL);
						}
					}
				}
			}

			DestroyMenu(hCtxMenu);
		}

		FreeLibrary(hinstSHDOCLC);
	}
	return (SUCCEEDED(hr) ? hr : S_FALSE);*/
}

STDMETHODIMP
CUpgradrBHO::ShowUI(DWORD dwID, 
						 IOleInPlaceActiveObject FAR* pActiveObject,
						 IOleCommandTarget FAR* pCommandTarget,
						 IOleInPlaceFrame  FAR* pFrame,
						 IOleInPlaceUIWindow FAR* pDoc)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
	return S_FALSE;
}

STDMETHODIMP
CUpgradrBHO::GetHostInfo(DOCHOSTUIINFO FAR *pInfo)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->GetHostInfo(pInfo);
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::HideUI()
{
if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->HideUI();
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::UpdateUI()
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->UpdateUI();
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::EnableModeless(BOOL fEnable)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->EnableModeless(fEnable);
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::OnDocWindowActivate(BOOL fActivate)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->OnDocWindowActivate(fActivate);
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::OnFrameWindowActivate(BOOL fActivate)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->OnFrameWindowActivate(fActivate);
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow FAR* pUIWindow, BOOL fFrameWindow)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->ResizeBorder(prcBorder, pUIWindow, fFrameWindow);
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::TranslateAccelerator(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
	return E_NOTIMPL;
}

STDMETHODIMP
CUpgradrBHO::GetOptionKeyPath(LPOLESTR FAR* pchKey, DWORD dw)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->GetOptionKeyPath(pchKey, dw);
	return E_FAIL;
}

STDMETHODIMP
CUpgradrBHO::GetDropTarget(IDropTarget* pDropTarget, IDropTarget** ppDropTarget)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->GetDropTarget(pDropTarget, ppDropTarget);
	return S_OK;
}

STDMETHODIMP
CUpgradrBHO::GetExternal(IDispatch** ppDispatch)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->GetExternal(ppDispatch);
	return S_FALSE;
}

STDMETHODIMP
CUpgradrBHO::TranslateUrl(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
	return S_FALSE;
}

STDMETHODIMP
CUpgradrBHO::FilterDataObject(IDataObject* pDO, IDataObject** ppDORet)
{
	if (m_DefaultDocHostUIHandler)	return m_DefaultDocHostUIHandler->FilterDataObject(pDO, ppDORet);
	return S_FALSE;
}

//
// IOleCommandTarget
//
STDMETHODIMP
CUpgradrBHO::QueryStatus(/*[in]*/ const GUID *pguidCmdGroup, 
								/*[in]*/ ULONG cCmds,
								/*[in,out][size_is(cCmds)]*/ OLECMD *prgCmds,
								/*[in,out]*/ OLECMDTEXT *pCmdText)
{
	return m_DefaultOleCommandTarget->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
}

STDMETHODIMP
CUpgradrBHO::Exec(/*[in]*/ const GUID *pguidCmdGroup,
					  /*[in]*/ DWORD nCmdID,
					  /*[in]*/ DWORD nCmdExecOpt,
					  /*[in]*/ VARIANTARG *pvaIn,
					  /*[in,out]*/ VARIANTARG *pvaOut)
{
	// see http://www.codeproject.com/internet/detecting_the_ie_refresh.asp
	if(nCmdID==2300||nCmdID==6041||nCmdID==6042)
	{
		m_RequestReload = true;
		OnRefreshStart();
	}
	if (nCmdID == OLECMDID_SHOWSCRIPTERROR)
	{
		// Don't show the error dialog, but
		// continue running scripts on the page.
		(*pvaOut).vt = VT_BOOL;
		(*pvaOut).boolVal = VARIANT_TRUE;
		return S_OK;
	}
	return m_DefaultOleCommandTarget->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut);
}