// UpgradrDebugDocumentHelper.h : Declaration of the CUpgradrDebugDocumentHelper

#pragma once
#include "resource.h"       // main symbols
#include "Upgradr.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CUpgradrDebugDocumentHelper
class ATL_NO_VTABLE CUpgradrDebugDocumentHelper :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrDebugDocumentHelper, &CLSID_UpgradrDebugDocumentHelper>,
	public IDispatchImpl<IUpgradrDebugDocumentHelper, &IID_IUpgradrDebugDocumentHelper, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDebugDocumentHelper
{
public:
	CUpgradrDebugDocumentHelper();
	~CUpgradrDebugDocumentHelper();

	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRDEBUGDOCUMENTHELPER)
	BEGIN_COM_MAP(CUpgradrDebugDocumentHelper)
		COM_INTERFACE_ENTRY(IUpgradrDebugDocumentHelper)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IDebugDocumentHelper)
		COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_DebugDocumentHelper)  // TODO: possible COM rules violation (see transition rule)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	IMPLEMENT_CLASS_NAME(CUpgradrDebugDocumentHelper)

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	//////////////////////////////////////////////////////////////////////////

	// IDebugDocumentHelper
	STDMETHOD(Init)( 
		/* [in] */ IDebugApplication __RPC_FAR *pda,
		/* [string][in] */ LPCOLESTR pszShortName,
		/* [string][in] */ LPCOLESTR pszLongName,
		/* [in] */ TEXT_DOC_ATTR docAttr);

	STDMETHOD(Attach)( 
		/* [in] */ IDebugDocumentHelper __RPC_FAR *pddhParent);

	STDMETHOD(Detach)( void);

	STDMETHOD(AddUnicodeText)( 
		/* [string][in] */ LPCOLESTR pszText);

	STDMETHOD(AddDBCSText)( 
		/* [string][in] */ LPCSTR pszText);

	STDMETHOD(SetDebugDocumentHost)( 
		/* [in] */ IDebugDocumentHost __RPC_FAR *pddh);

	STDMETHOD(AddDeferredText)( 
		/* [in] */ ULONG cChars,
		/* [in] */ DWORD dwTextStartCookie);

	STDMETHOD(DefineScriptBlock)( 
		/* [in] */ ULONG ulCharOffset,
		/* [in] */ ULONG cChars,
		/* [in] */ IActiveScript __RPC_FAR *pas,
		/* [in] */ BOOL fScriptlet,
		/* [out] */ DWORD __RPC_FAR *pdwSourceContext);

	STDMETHOD(SetDefaultTextAttr)( 
		SOURCE_TEXT_ATTR staTextAttr);

	STDMETHOD(SetTextAttributes)( 
		/* [in] */ ULONG ulCharOffset,
		/* [in] */ ULONG cChars,
		/* [size_is][length_is][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr);

	STDMETHOD(SetLongName)( 
		/* [string][in] */ LPCOLESTR pszLongName);

	STDMETHOD(SetShortName)( 
		/* [string][in] */ LPCOLESTR pszShortName);

	STDMETHOD(SetDocumentAttr)( 
		/* [in] */ TEXT_DOC_ATTR pszAttributes);

	STDMETHOD(GetDebugApplicationNode)( 
		/* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdan);

	STDMETHOD(GetScriptBlockInfo)( 
		/* [in] */ DWORD dwSourceContext,
		/* [out] */ IActiveScript __RPC_FAR *__RPC_FAR *ppasd,
		/* [out] */ ULONG __RPC_FAR *piCharPos,
		/* [out] */ ULONG __RPC_FAR *pcChars);

	STDMETHOD(CreateDebugDocumentContext)( 
		/* [in] */ ULONG iCharPos,
		/* [in] */ ULONG cChars,
		/* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppddc);

	STDMETHOD(BringDocumentToTop)( void);

	STDMETHOD(BringDocumentContextToTop)( 
		IDebugDocumentContext __RPC_FAR *pddc);


	CComPtr<IDebugDocumentHelper>						m_DebugDocumentHelper;
	TScriptId												m_ScriptId;
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrDebugDocumentHelper), CUpgradrDebugDocumentHelper)
