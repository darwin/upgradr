// UpgradrDebugDocumentContext.cpp : Implementation of CUpgradrDebugDocumentContext

#include "stdafx.h"
#include "UpgradrDebugDocumentContext.h"
#include "UpgradrEnumDebugCodeContexts.h"

// CUpgradrDebugDocumentContext
CUpgradrDebugDocumentContext::CUpgradrDebugDocumentContext()
{

}

CUpgradrDebugDocumentContext::~CUpgradrDebugDocumentContext()
{

}

// IDebugDocumentContext
STDMETHODIMP
CUpgradrDebugDocumentContext::GetDocument(/* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppsd)
{
	TRACE_I(FS(_T("CUpgradrDebugDocumentContext::GetDocument")));
	return m_DebugDocumentContext->GetDocument(ppsd);
}

STDMETHODIMP
CUpgradrDebugDocumentContext::EnumCodeContexts(/* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc)
{
	TRACE_I(FS(_T("CUpgradrDebugDocumentContext::EnumCodeContexts")));

	// create instance of the delegate
	CComObject<CUpgradrEnumDebugCodeContexts>* pEnumDebugCodeContexts;
	CComObject<CUpgradrEnumDebugCodeContexts>::CreateInstance(&pEnumDebugCodeContexts);
	pEnumDebugCodeContexts->AddRef();
	pEnumDebugCodeContexts->m_Pos = m_Pos;
	pEnumDebugCodeContexts->m_Chars = m_Chars;
	pEnumDebugCodeContexts->m_ScriptId = m_ScriptId;

	HRESULT hr = m_DebugDocumentContext->EnumCodeContexts(&pEnumDebugCodeContexts->m_EnumDebugCodeContexts);
	
	pEnumDebugCodeContexts->QueryInterface(IID_IEnumDebugCodeContexts, (void**)ppescc);
	pEnumDebugCodeContexts->Release();
	return hr;
}
