// UpgradrScriptSite.h : Declaration of the CUpgradrScriptSite

#pragma once
#include "resource.h"       // main symbols
#include "Upgradr.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// If you want a different application title, declare SCRIPTSITE_APPNAME in
// you code about the reference to ScriptSite.h
#ifndef SCRIPTSITE_APPNAME
#define SCRIPTSITE_APPNAME	_T("UpgradrScriptSite")
#endif

// CUpgradrScriptSite
// vyrabim jednu site pro jeden vlozeny skript (kazdy ma svou vlastni)
//
// site poskytuje IUpgradrScriptService pomoci IServiceProvider, 
// aby bezicimu kodu v shellu rekla, ktery skript ho vykonava
class ATL_NO_VTABLE CUpgradrScriptSite :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrScriptSite, &CLSID_UpgradrScriptSite>,
	public IDispatchImpl<IUpgradrScriptSite, &IID_IUpgradrScriptSite, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IActiveScriptSite,
	public IActiveScriptSiteWindow,
	public IActiveScriptSiteDebug,
	public IServiceProvider,
	public IUpgradrScriptService,
	public WinTrace
{
public:
	CUpgradrScriptSite();
	~CUpgradrScriptSite();

	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRSCRIPTSITE)

	DECLARE_NOT_AGGREGATABLE(CUpgradrScriptSite)

	BEGIN_COM_MAP(CUpgradrScriptSite)
		COM_INTERFACE_ENTRY(IUpgradrScriptSite)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IActiveScriptSite)
		COM_INTERFACE_ENTRY(IActiveScriptSiteWindow)
		COM_INTERFACE_ENTRY(IActiveScriptSiteDebug)
		COM_INTERFACE_ENTRY(IServiceProvider)
		COM_INTERFACE_ENTRY(IUpgradrScriptService)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	IMPLEMENT_CLASS_NAME(CUpgradrScriptSite)

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	// IActiveScriptSite
	STDMETHOD(GetLCID)(LCID *plcid);
	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwMask, LPUNKNOWN* ppunkItem,LPTYPEINFO* ppTypeInfo);
	STDMETHOD(GetDocVersionString)(BSTR *pbstrversionString);
	STDMETHOD(OnScriptError)(IActiveScriptError* pase);
	STDMETHOD(OnEnterScript)(void);
	STDMETHOD(OnLeaveScript)(void);
	STDMETHOD(OnScriptTerminate)(const VARIANT* pvarResult, const EXCEPINFO* pexcepinfo);
	STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState);

	// IActiveScriptSiteWindow
	STDMETHOD(GetWindow)(HWND *phWnd);
	STDMETHOD(EnableModeless)(BOOL fEnable);

	// IActiveScriptSiteDebug
	STDMETHOD(GetDocumentContextFromPosition)(DWORD dwSourceContext, ULONG uCharacterOffset, ULONG uNumChars, IDebugDocumentContext **ppsc);
	STDMETHOD(GetApplication)(IDebugApplication **ppda);
	STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **ppdanRoot);
	STDMETHOD(OnScriptErrorDebug)(IActiveScriptErrorDebug *pErrorDebug, BOOL*pfEnterDebugger,	BOOL *pfCallOnScriptErrorWhenContinuing);

	// IServiceProvider
	STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv);

	// IUpgradrScriptService
	STDMETHOD(GetScriptInstanceId)(LONG* id);

	//////////////////////////////////////////////////////////////////////////

	// IUpgradrScriptSite

	//////////////////////////////////////////////////////////////////////////

	// this must be called from helper thread (UI thread)
	STDMETHOD(InitScriptSite)(BSTR language, TFrameId frameId, TScriptId scriptId, CScript* pScript);
	// this must be called from browser's thread (BHO's thread)
	STDMETHOD(InitInstanceSite)(BSTR language, TFrameId frameId, TScriptId scriptId, TScriptInstanceId scriptInstanceId, CScript* pScript);
	// this must be called by creating thread
	STDMETHOD(Done)();
	STDMETHOD(ParseScriptText)(CScript* pScript);

protected:
	HRESULT CreateEngine(BSTR language, TFrameId frameId, TScriptId scriptId, TScriptInstanceId scriptInstanceId);
	HRESULT CreateCommonDebugHelper(CScript* pScript);
	HRESULT CreateInstanceDebugHelper(CScript* pScript);
	HRESULT Run();

public:
	CComPtr<IDebugDocumentHelper>						m_DebugDocumentHelper;
	CComPtr<IActiveScript>								m_Engine;
	CComPtr<IUpgradrShell>								m_Shell;
	TScriptId												m_ScriptId;
	TFrameId													m_FrameId;
	CString													m_FileName;
	SCRIPTSTATE                                  m_ConfirmedState;
private:
	TScriptInstanceId										m_ScriptInstanceId;
	DWORD														m_CreatingThreadId;
	bool														m_EngineConnected;
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrScriptSite), CUpgradrScriptSite)
