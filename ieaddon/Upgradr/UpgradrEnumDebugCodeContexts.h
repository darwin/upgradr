// UpgradrEnumDebugCodeContexts.h : Declaration of the CUpgradrEnumDebugCodeContexts

#pragma once
#include "resource.h"       // main symbols

#include "Upgradr.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CUpgradrEnumDebugCodeContexts

class ATL_NO_VTABLE CUpgradrEnumDebugCodeContexts :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrEnumDebugCodeContexts, &CLSID_UpgradrEnumDebugCodeContexts>,
	public IDispatchImpl<IUpgradrEnumDebugCodeContexts, &IID_IUpgradrEnumDebugCodeContexts, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IEnumDebugCodeContexts
{
public:
	CUpgradrEnumDebugCodeContexts();
	~CUpgradrEnumDebugCodeContexts();

DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRENUMDEBUGCODECONTEXTS)


BEGIN_COM_MAP(CUpgradrEnumDebugCodeContexts)
	COM_INTERFACE_ENTRY(IUpgradrEnumDebugCodeContexts)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IEnumDebugCodeContexts)
	COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_EnumDebugCodeContexts)  // TODO: possible COM rules violation (see transition rule)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	IMPLEMENT_CLASS_NAME(CUpgradrEnumDebugCodeContexts);

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	// IEnumDebugCodeContexts
	STDMETHOD(Next)(ULONG celt, IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc, ULONG __RPC_FAR *pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);

	CComPtr<IEnumDebugCodeContexts>                m_EnumDebugCodeContexts;
	ULONG                                          m_Pos;
	ULONG                                          m_Chars;
	TScriptId                                  m_ScriptId;
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrEnumDebugCodeContexts), CUpgradrEnumDebugCodeContexts)
