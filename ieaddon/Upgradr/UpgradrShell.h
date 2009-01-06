// UpgradrShell.h : Declaration of the CUpgradrShell

#pragma once
#include "resource.h"       // main symbols

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CUpgradrShell
//
// shell je globalni objekt kazdeho spusteneho skriptu v UpgradrScriptSite
// shell v sobe obsahuje window, takze zpristupnuje podobne prostredi jako IE script site
// navic obsahuje jeste moje funkce (emulace GreaseMonkey apod)
// delam tady kompozici obou interfacu (neco jako http://www.microsoft.com/mind/1099/dynamicobject/dynamicobject.asp)
// prvnich 100 dipatch idcek je vyhrazeno pro moje funkce (self)
// idcka nad 100 jsou mapovanim idcek z window objektu
//
// shell je vyroben jeden pro jeden HTML frame
// tento objekt je urcen pro volani ze skriptu do upgradru
//
// Log - funkcionalita pro GM_Log
// 
class ATL_NO_VTABLE CUpgradrShell :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrShell, &CLSID_UpgradrShell>,
	public ISupportErrorInfo,
	public IUpgradrShell,
	public WinTrace
{
public:
	CUpgradrShell();
	~CUpgradrShell();

	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRSHELL)

	DECLARE_NOT_AGGREGATABLE(CUpgradrShell)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CUpgradrShell)
		COM_INTERFACE_ENTRY(IUpgradrShell)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IDispatchEx)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
	END_COM_MAP()

	IMPLEMENT_CLASS_NAME(CUpgradrShell)

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	TScriptInstanceId										m_ScriptInstanceId; ///< active script instance
	CCS														m_Lock;
public:
	IUpgradrScriptEngine*								m_ScriptEngine;
	CComPtr<IDispatchEx>									m_Window; ///< IHTMLWindow

	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
	STDMETHOD(Invoke)(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, UINT* puArgErr);

	// IDispatchEx
	STDMETHOD(GetDispID)(BSTR bstrName, DWORD grfdex, DISPID* pid);
	STDMETHOD(InvokeEx)(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes,	EXCEPINFO* pei, IServiceProvider* pspCaller);
	STDMETHOD(DeleteMemberByName)(BSTR bstr, DWORD grfdex);
	STDMETHOD(DeleteMemberByDispID)(DISPID id);
	STDMETHOD(GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD* pgrfdex);
	STDMETHOD(GetMemberName)(DISPID id, BSTR* pbstrName);
	STDMETHOD(GetNextDispID)(DWORD grfdex, DISPID id, DISPID* pid);
	STDMETHOD(GetNameSpaceParent)(IUnknown** ppunk);

	// IUpgradrShell
	STDMETHOD(Log)(BSTR message);
	STDMETHOD(GetValue)(BSTR key, BSTR* result);
	STDMETHOD(SetValue)(BSTR key, BSTR value);
	STDMETHOD(HasValue)(BSTR key, VARIANT_BOOL* result);

private:
	HRESULT													LoadTypeLib(LCID lcid);
	DISPID													MapIdFromWindowToShell(DISPID windowId);
	DISPID													MapIdFromSelfToShell(DISPID selfId);
	DISPID													MapIdFromShellToWindow(DISPID id);
	DISPID													MapIdFromShellToSelf(DISPID id);
	bool														IsWindowId(DISPID id);
	bool														IsSelfId(DISPID id);
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrShell), CUpgradrShell)
