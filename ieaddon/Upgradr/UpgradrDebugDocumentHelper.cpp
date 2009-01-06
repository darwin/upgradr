// UpgradrDebugDocumentHelper.cpp : Implementation of CUpgradrDebugDocumentHelper

#include "stdafx.h"
#include "UpgradrDebugDocumentHelper.h"
#include "UpgradrDebugDocumentContext.h"

//////////////////////////////////////////////////////////////////////////
// CUpgradrDebugDocumentHelper

CUpgradrDebugDocumentHelper::CUpgradrDebugDocumentHelper()
{

}

CUpgradrDebugDocumentHelper::~CUpgradrDebugDocumentHelper()
{

}

//////////////////////////////////////////////////////////////////////////
// IDebugDocumentHelper

STDMETHODIMP
CUpgradrDebugDocumentHelper::Init( 
										 /* [in] */ IDebugApplication __RPC_FAR *pda,
										 /* [string][in] */ LPCOLESTR pszShortName,
										 /* [string][in] */ LPCOLESTR pszLongName,
										 /* [in] */ TEXT_DOC_ATTR docAttr)
{
	return m_DebugDocumentHelper->Init(pda, pszShortName, pszLongName, docAttr);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::Attach(/* [in] */ IDebugDocumentHelper __RPC_FAR *pddhParent)
{
	return m_DebugDocumentHelper->Attach(pddhParent);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::Detach( void)
{
	return m_DebugDocumentHelper->Detach();
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::AddUnicodeText(/* [string][in] */ LPCOLESTR pszText)
{
	return m_DebugDocumentHelper->AddUnicodeText(pszText);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::AddDBCSText(/* [string][in] */ LPCSTR pszText)
{
	return m_DebugDocumentHelper->AddDBCSText(pszText);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::SetDebugDocumentHost( 
	/* [in] */ IDebugDocumentHost __RPC_FAR *pddh)
{
	return m_DebugDocumentHelper->SetDebugDocumentHost(pddh);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::AddDeferredText( 
	/* [in] */ ULONG cChars,
	/* [in] */ DWORD dwTextStartCookie)
{
	return m_DebugDocumentHelper->AddDeferredText(cChars, dwTextStartCookie);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::DefineScriptBlock( 
	/* [in] */ ULONG ulCharOffset,
	/* [in] */ ULONG cChars,
	/* [in] */ IActiveScript __RPC_FAR *pas,
	/* [in] */ BOOL fScriptlet,
	/* [out] */ DWORD __RPC_FAR *pdwSourceContext)
{
	return m_DebugDocumentHelper->DefineScriptBlock(ulCharOffset, cChars, pas, fScriptlet, pdwSourceContext);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::SetDefaultTextAttr( 
	SOURCE_TEXT_ATTR staTextAttr)
{
	return m_DebugDocumentHelper->SetDefaultTextAttr(staTextAttr);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::SetTextAttributes( 
	/* [in] */ ULONG ulCharOffset,
	/* [in] */ ULONG cChars,
	/* [size_is][length_is][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr)
{
	return m_DebugDocumentHelper->SetTextAttributes(ulCharOffset, cChars, pstaTextAttr);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::SetLongName( 
												  /* [string][in] */ LPCOLESTR pszLongName)
{
	return m_DebugDocumentHelper->SetLongName(pszLongName);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::SetShortName( 
													/* [string][in] */ LPCOLESTR pszShortName)
{
	return m_DebugDocumentHelper->SetShortName(pszShortName);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::SetDocumentAttr( 
	/* [in] */ TEXT_DOC_ATTR pszAttributes)
{
	return m_DebugDocumentHelper->SetDocumentAttr(pszAttributes);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::GetDebugApplicationNode( 
	/* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdan)
{
	return m_DebugDocumentHelper->GetDebugApplicationNode(ppdan);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::GetScriptBlockInfo( 
	/* [in] */ DWORD dwSourceContext,
	/* [out] */ IActiveScript __RPC_FAR *__RPC_FAR *ppasd,
	/* [out] */ ULONG __RPC_FAR *piCharPos,
	/* [out] */ ULONG __RPC_FAR *pcChars)
{
	return m_DebugDocumentHelper->GetScriptBlockInfo(dwSourceContext, ppasd, piCharPos, pcChars);
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::CreateDebugDocumentContext( 
	/* [in] */ ULONG iCharPos,
	/* [in] */ ULONG cChars,
	/* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppddc)
{
	TRACE_I(FS(_T("CUpgradrDebugDocumentHelper::CreateDebugDocumentContext(pos=%d, chars=%d)"), iCharPos, cChars));

	// create instance of the delegate
	CComObject<CUpgradrDebugDocumentContext>* pDebugDocumentContext;
	CComObject<CUpgradrDebugDocumentContext>::CreateInstance(&pDebugDocumentContext);
	pDebugDocumentContext->AddRef();
	pDebugDocumentContext->m_Pos = iCharPos;
	pDebugDocumentContext->m_Chars = cChars;
	pDebugDocumentContext->m_ScriptId = m_ScriptId;

	HRESULT hr = m_DebugDocumentHelper->CreateDebugDocumentContext(iCharPos, cChars, &pDebugDocumentContext->m_DebugDocumentContext);
 
	pDebugDocumentContext->QueryInterface(IID_IDebugDocumentContext, (void**)ppddc);
	pDebugDocumentContext->Release();
	return hr;
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::BringDocumentToTop( void)
{
	return m_DebugDocumentHelper->BringDocumentToTop();
}

STDMETHODIMP
CUpgradrDebugDocumentHelper::BringDocumentContextToTop( 
	IDebugDocumentContext __RPC_FAR *pddc)
{
	return m_DebugDocumentHelper->BringDocumentContextToTop(pddc);
}