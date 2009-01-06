// UpgradrScriptDebugger.h : Declaration of the CUpgradrScriptDebugger

#pragma once
#include "resource.h"       // main symbols
#include "Upgradr.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CUpgradrScriptDebugger

class ATL_NO_VTABLE CUpgradrScriptDebugger :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrScriptDebugger, &CLSID_UpgradrScriptDebugger>,
	public IDispatchImpl<IUpgradrScriptDebugger, &IID_IUpgradrScriptDebugger, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IRemoteDebugApplicationEvents
{
public:
	CUpgradrScriptDebugger();
	~CUpgradrScriptDebugger();

	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRSCRIPTDEBUGGER)

	BEGIN_COM_MAP(CUpgradrScriptDebugger)
		COM_INTERFACE_ENTRY(IUpgradrScriptDebugger)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IRemoteDebugApplicationEvents)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	IMPLEMENT_CLASS_NAME(CUpgradrScriptDebugger)

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	// IRemoteDebugApplicationEvents 
	STDMETHOD(OnConnectDebugger)(IApplicationDebugger __RPC_FAR *pad);
	STDMETHOD(OnDisconnectDebugger)();
	STDMETHOD(OnSetName)(LPCOLESTR pstrName);
	STDMETHOD(OnDebugOutput)(LPCOLESTR pstr);
	STDMETHOD(OnClose)(void);
	STDMETHOD(OnEnterBreakPoint)(IRemoteDebugApplicationThread __RPC_FAR *prdat);
	STDMETHOD(OnLeaveBreakPoint)(IRemoteDebugApplicationThread __RPC_FAR *prdat);
	STDMETHOD(OnCreateThread)(IRemoteDebugApplicationThread __RPC_FAR *prdat);
	STDMETHOD(OnDestroyThread)(IRemoteDebugApplicationThread __RPC_FAR *prdat);
	STDMETHOD(OnBreakFlagChange)(APPBREAKFLAGS abf, IRemoteDebugApplicationThread __RPC_FAR *prdatSteppingThread);
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrScriptDebugger), CUpgradrScriptDebugger)
