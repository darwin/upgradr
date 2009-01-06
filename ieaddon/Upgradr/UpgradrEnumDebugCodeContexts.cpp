// UpgradrEnumDebugCodeContexts.cpp : Implementation of CUpgradrEnumDebugCodeContexts

#include "stdafx.h"
#include "UpgradrEnumDebugCodeContexts.h"
#include "UpgradrDebugCodeContext.h"

//////////////////////////////////////////////////////////////////////////
// CUpgradrEnumDebugCodeContexts

CUpgradrEnumDebugCodeContexts::CUpgradrEnumDebugCodeContexts()
{

}

CUpgradrEnumDebugCodeContexts::~CUpgradrEnumDebugCodeContexts()
{

}

//////////////////////////////////////////////////////////////////////////
// IEnumDebugCodeContexts

STDMETHODIMP
CUpgradrEnumDebugCodeContexts::Next(ULONG celt, IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc, ULONG __RPC_FAR *pceltFetched)
{
	TRACE_I(FS(_T("CUpgradrEnumDebugCodeContexts::Next(celt=%d)"), celt));
	HRESULT hr = m_EnumDebugCodeContexts->Next(celt, pscc, pceltFetched);
	for (ULONG i=0; i<*pceltFetched; i++)
	{
		IDebugCodeContext*& origObject = pscc[i];

		// create instance of the delegate
		CComObject<CUpgradrDebugCodeContext>* pDebugCodeContext;
		CComObject<CUpgradrDebugCodeContext>::CreateInstance(&pDebugCodeContext);
		pDebugCodeContext->AddRef();
		pDebugCodeContext->m_Pos = m_Pos;
		pDebugCodeContext->m_Chars = m_Chars;
		pDebugCodeContext->m_ScriptId = m_ScriptId;

		pDebugCodeContext->m_DebugCodeContext = origObject; // here, internally was called AddRef
		origObject->Release(); // so we need to release it and leave ownership to the pDebugCodeContext->m_DebugCodeContext

		origObject = pDebugCodeContext;
	}
	return hr;
}

STDMETHODIMP
CUpgradrEnumDebugCodeContexts::Skip(ULONG celt)
{
	TRACE_I(FS(_T("CUpgradrEnumDebugCodeContexts::Skip(celt=%d)"), celt));
	return m_EnumDebugCodeContexts->Skip(celt);
}

STDMETHODIMP 
CUpgradrEnumDebugCodeContexts::Reset()
{
	TRACE_I(FS(_T("CUpgradrEnumDebugCodeContexts::Reset()")));
	return m_EnumDebugCodeContexts->Reset();
}

STDMETHODIMP
CUpgradrEnumDebugCodeContexts::Clone(IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc)
{
	TRACE_I(FS(_T("CUpgradrEnumDebugCodeContexts::Clone()")));

	// create instance of the delegate
	CComObject<CUpgradrEnumDebugCodeContexts>* pEnumDebugCodeContexts;
	CComObject<CUpgradrEnumDebugCodeContexts>::CreateInstance(&pEnumDebugCodeContexts);
	pEnumDebugCodeContexts->AddRef();
	pEnumDebugCodeContexts->m_Pos = m_Pos;
	pEnumDebugCodeContexts->m_Chars = m_Chars;
	pEnumDebugCodeContexts->m_ScriptId = m_ScriptId;

	HRESULT hr = m_EnumDebugCodeContexts->Clone(&pEnumDebugCodeContexts->m_EnumDebugCodeContexts);

	pEnumDebugCodeContexts->QueryInterface(IID_IEnumDebugCodeContexts, (void**)ppescc);
	pEnumDebugCodeContexts->Release();
	return hr;
}
