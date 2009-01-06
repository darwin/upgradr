// UpgradrDebugDocumentContext.h : Declaration of the CUpgradrDebugDocumentContext

#pragma once
#include "resource.h"       // main symbols

#include "Upgradr.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CUpgradrDebugDocumentContext

class ATL_NO_VTABLE CUpgradrDebugDocumentContext :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrDebugDocumentContext, &CLSID_UpgradrDebugDocumentContext>,
	public IDispatchImpl<IUpgradrDebugDocumentContext, &IID_IUpgradrDebugDocumentContext, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDebugDocumentContext
{
public:
	CUpgradrDebugDocumentContext();
	~CUpgradrDebugDocumentContext();

	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRDEBUGDOCUMENTCONTEXT)


	BEGIN_COM_MAP(CUpgradrDebugDocumentContext)
		COM_INTERFACE_ENTRY(IUpgradrDebugDocumentContext)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IDebugDocumentContext)
		COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_DebugDocumentContext)  // TODO: possible COM rules violation (see transition rule)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	IMPLEMENT_CLASS_NAME(CUpgradrDebugDocumentContext)

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	// IDebugDocumentContext
	STDMETHOD(GetDocument)(/* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppsd);
	STDMETHOD(EnumCodeContexts)(/* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);

	CComPtr<IDebugDocumentContext>                 m_DebugDocumentContext;
	ULONG                                          m_Pos;
	ULONG                                          m_Chars;
	TScriptId                                  m_ScriptId;
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrDebugDocumentContext), CUpgradrDebugDocumentContext)
