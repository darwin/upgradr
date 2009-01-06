// UpgradrScriptSite.cpp : Implementation of CUpgradrScriptSite

#include "stdafx.h"
#include "UpgradrScriptSite.h"
#include "ScriptDebugger.h"

//#include "Debug.h"

//////////////////////////////////////////////////////////////////////////
// CUpgradrScriptSite

CUpgradrScriptSite::CUpgradrScriptSite():
m_ScriptInstanceId(NULL_SCRIPT_INSTANCE),
m_ConfirmedState(SCRIPTSTATE_UNINITIALIZED),
m_CreatingThreadId(0),
m_EngineConnected(false)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]: constructor"), this)));
}

CUpgradrScriptSite::~CUpgradrScriptSite()
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]: destructor"), this)));
	ATLASSERT(!m_Engine);
	ATLASSERT(!m_DebugDocumentHelper);
}

//////////////////////////////////////////////////////////////////////////
// IActiveScriptSite

STDMETHODIMP 
CUpgradrScriptSite::GetLCID(LCID *plcid)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetLCID()"), this)));
	*plcid = LANG_SYSTEM_DEFAULT;
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD dwMask, LPUNKNOWN* ppunkItem, LPTYPEINFO* ppTypeInfo)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetItemInfo(...)"), this)));
	ATLASSERT(m_Shell);

	if (dwMask&SCRIPTINFO_IUNKNOWN)
	{
		m_Shell->QueryInterface(IID_IDispatch, (void**)ppunkItem);
	}
	if (dwMask&SCRIPTINFO_ITYPEINFO)
	{
		// IE should never ask for type info
		ATLASSERT(0);
		*ppTypeInfo = NULL;
	}
	return S_OK;

	// old implementation was routing request directly to IE Script site
	// here must be forwarding to m_IESite, hence use "window" regardless of pstrName
	// return m_IESite->GetItemInfo(_T("window"), dwMask, ppunkItem, ppTypeInfo);
}

STDMETHODIMP 
CUpgradrScriptSite::GetDocVersionString(BSTR *pbstrversionString)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetDocVersionString(...)"), this)));
	return E_NOTIMPL;
}

// see http://support.microsoft.com/kb/232394
STDMETHODIMP 
CUpgradrScriptSite::OnScriptError(IActiveScriptError* pase)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::OnScriptError(...)"), this)));
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrScriptSite::OnEnterScript(void)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::OnEnterScript()"), this)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::OnLeaveScript(void)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::OnLeaveScript()"), this)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::OnScriptTerminate(const VARIANT* pvarResult, const EXCEPINFO* pexcepinfo)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::OnScriptTerminate(...)"), this)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::OnStateChange(%d)"), this, ssScriptState)));
	m_ConfirmedState = ssScriptState;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IActiveScriptWindow

STDMETHODIMP 
CUpgradrScriptSite::GetWindow(HWND *phWnd)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetWindow(...)"), this)));
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrScriptSite::EnableModeless(BOOL fEnable)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::EnableModeless(...)"), this)));
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// IActiveScriptSiteDebug

// Used by the language engine to delegate IDebugCodeContext::GetSourceContext
STDMETHODIMP 
CUpgradrScriptSite::GetDocumentContextFromPosition(DWORD dwSourceContext, ULONG uCharacterOffset, ULONG uNumChars, IDebugDocumentContext **ppsc)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetDocumentContextFromPosition(sourceContext=%d, CharOffset=%d, uNumChars=%d)"), this, dwSourceContext, uCharacterOffset, uNumChars)));
	if (!m_DebugDocumentHelper) 
	{
		TRACE_E(_T("GetDocumentContextFromPosition called when no DebugDocumentHelper"));
		return E_NOTIMPL;
	}

	ULONG ulStartPos = 0;
	HRESULT hr;
	hr = m_DebugDocumentHelper->GetScriptBlockInfo(dwSourceContext, NULL, &ulStartPos, NULL);
	hr = m_DebugDocumentHelper->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppsc);
	return hr;
}

// Returns the debug application object associated with this script site. Provides 
// a means for a smart host to define what application object each script belongs to. 
// Script engines should attempt to call this method to get their containing application 
// and resort to IProcessDebugManager::GetDefaultApplication if this fails. 
STDMETHODIMP 
CUpgradrScriptSite::GetApplication(IDebugApplication **ppda)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetApplication(...)"), this)));
	if (!ppda) 
	{
		TRACE_E(_T("GetApplication called when no DebugDocumentHelper"));
		WarningBeep();
		return E_INVALIDARG;
	}
	ScriptDebuggerLock scriptDebugger;
	if (!scriptDebugger->DebugApplication()) return E_FAIL;
	return scriptDebugger->DebugApplication()->QueryInterface(IID_IDebugApplication, (void**)ppda);
}

// Gets the application node under which script documents should be added 
// can return NULL if script documents should be top-level. 
STDMETHODIMP 
CUpgradrScriptSite::GetRootApplicationNode(IDebugApplicationNode **ppdanRoot)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetRootApplicationNode(...)"), this)));
	if (!ppdanRoot) return E_INVALIDARG;
	if (!m_DebugDocumentHelper) 
	{
		TRACE_E(_T("GetRootApplicationNode called when no DebugDocumentHelper"));
		return E_NOTIMPL;
	}
	//return m_DebugDocumentHelper->GetDebugApplicationNode(ppdanRoot);
	ScriptDebuggerLock scriptDebugger;
	if (!scriptDebugger->DebugApplication()) return E_FAIL;
	return scriptDebugger->DebugApplication()->GetRootNode(ppdanRoot);
}

// Allows a smart host to control the handling of runtime errors 
// see http://support.microsoft.com/kb/232394
STDMETHODIMP 
CUpgradrScriptSite::OnScriptErrorDebug(IActiveScriptErrorDebug *pErrorDebug, BOOL*pfEnterDebugger, BOOL *pfCallOnScriptErrorWhenContinuing)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::OnScriptErrorDebug(...)"), this)));
	//if (pfEnterDebugger) *pfEnterDebugger = TRUE;
	if (pfEnterDebugger) *pfEnterDebugger = FALSE;
	if (pfCallOnScriptErrorWhenContinuing) *pfCallOnScriptErrorWhenContinuing = TRUE;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IServiceProvider

STDMETHODIMP
CUpgradrScriptSite::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::QueryService(...)"), this)));
	if (guidService==IID_IUpgradrScriptService && 
		 riid==IID_IUpgradrScriptService)
	{
		AddRef();
		*ppv = (IUpgradrScriptService*)this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

//////////////////////////////////////////////////////////////////////////
// IUpgradrScriptService

STDMETHODIMP
CUpgradrScriptSite::GetScriptInstanceId(LONG* id)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::GetScriptInstanceId(...)"), this)));
	ATLASSERT(m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE);
	ATLASSERT(sizeof(*id)==sizeof(m_ScriptInstanceId));
	*id = m_ScriptInstanceId;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IUpgradrScriptSite

HRESULT
CUpgradrScriptSite::CreateEngine(BSTR language, TFrameId frameId, TScriptId scriptId, TScriptInstanceId scriptInstanceId)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::CreateEngine(...)"), this)));
	m_CreatingThreadId = GetCurrentThreadId();

	m_FrameId = frameId;
	m_ScriptInstanceId = scriptInstanceId;
	m_ScriptId = scriptId;

	// create new script engine
	CHECK(m_Engine.CoCreateInstance(language));
	ATLASSERT(!!m_Engine);

	// attach to site
	CComQIPtr<IActiveScriptSite> thisSite = this;
	ATLASSERT(!!thisSite);
	CHECK(m_Engine->SetScriptSite(thisSite));

	CHECK(m_Engine->AddNamedItem(CComBSTR(JAVASCRIPT_UPGRADRSHELL_OBJECT), /*SCRIPTITEM_GLOBALMEMBERS|*/SCRIPTITEM_NOCODE|SCRIPTITEM_ISVISIBLE));
	CHECK(m_Engine->AddNamedItem(CComBSTR(_T("window")), SCRIPTITEM_GLOBALMEMBERS|SCRIPTITEM_ISSOURCE|SCRIPTITEM_NOCODE|SCRIPTITEM_ISVISIBLE));

	return S_OK;
}

HRESULT
CUpgradrScriptSite::CreateInstanceDebugHelper(CScript* pScript)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::CreateInstanceDebugHelper(...)"), this)));
	ATLASSERT(m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE);

	CString& oleScript = pScript->Code();
	TDebugDocumentHelper* parentDebugDocumentHelper;
	{
		ScriptDebuggerLock scriptDebugger;

		parentDebugDocumentHelper = scriptDebugger->FindFrameDebugDocumentHelper(m_FrameId);
		if (!parentDebugDocumentHelper) return E_FAIL;

		CString shortName;
		shortName.Format(_T("[S%d] %s %s"), m_ScriptInstanceId, pScript->Name(), (CString)pScript->Guid());
		CString longName = pScript->FileName();
		m_FileName = longName;

		CComBSTR content(oleScript);
		scriptDebugger->CreateScriptInstanceDebugDocumentHelper(m_ScriptInstanceId, m_ScriptId, shortName, longName, content);
		// TODO: tests
		m_DebugDocumentHelper = *scriptDebugger->FindScriptInstanceDebugDocumentHelper(m_ScriptInstanceId);
		ATLASSERT(m_DebugDocumentHelper);
	}

	// attach it
	CHECK(m_DebugDocumentHelper->Attach(*parentDebugDocumentHelper));
	return S_OK;
}

HRESULT 
CUpgradrScriptSite::CreateCommonDebugHelper(CScript* pScript)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::CreateCommonDebugHelper(...)"), this)));
	ATLASSERT(m_ScriptInstanceId==NULL_SCRIPT_INSTANCE);

	CString& oleScript = pScript->Code();
	CString shortName;
	shortName.Format(_T("%s %s"), pScript->Name(), (CString)pScript->Guid());
	CString longName = pScript->FileName();
	m_FileName = longName;

	CComBSTR content(oleScript);
	ScriptDebuggerLock scriptDebugger;
	scriptDebugger->CreateScriptDebugDocumentHelper(m_ScriptId, shortName, longName, content);
	// TODO: tests
	m_DebugDocumentHelper = *scriptDebugger->FindScriptDebugDocumentHelper(m_ScriptId);
	ATLASSERT(m_DebugDocumentHelper);

	// attach it
	CHECK(m_DebugDocumentHelper->Attach(*scriptDebugger->RunningScriptsHelper()));
	return S_OK;
}

HRESULT
CUpgradrScriptSite::Run()
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::Run(...)"), this)));
	// don't run common script site !
	// common script site is only placeholder
	ATLASSERT(m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE);

	// reapply sticky breakpoints
	{
		ScriptDebuggerLock scriptDebugger;
		scriptDebugger->ApplyBreakpoints(m_DebugDocumentHelper, m_ScriptId, m_ScriptInstanceId);
	}

	// script debugger must be unlocked at this point !
	DT(TRACE_LI(FS(_T("Changing script state to SCRIPTSTATE_STARTED"))));
	HRESULT res = m_Engine->SetScriptState(SCRIPTSTATE_STARTED);
	if (res!=S_OK) return res;

	// connect - this makes the script engine handle incoming events
	DT(TRACE_LI(FS(_T("Changing script state to SCRIPTSTATE_CONNECTED"))));
	CHECK(m_Engine->SetScriptState(SCRIPTSTATE_CONNECTED));

	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::InitInstanceSite(BSTR language, TFrameId frameId, TScriptId scriptId, TScriptInstanceId scriptInstanceId, CScript* pScript)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::InitInstanceSite(...)"), this)));
	ATLASSERT(!I_AM_UI_THREAD); // must NOT be entered by UI thread
	ATLASSERT(!m_DebugDocumentHelper);

	CHECK(CreateEngine(language, frameId, scriptId, scriptInstanceId));
	CHECK(CreateInstanceDebugHelper(pScript));
	HRESULT res = ParseScriptText(pScript);
	if (res!=S_OK) return res;
	res = Run();
	if (res!=S_OK) return res;
	m_EngineConnected = true;
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::InitScriptSite(BSTR language, TFrameId frameId, TScriptId scriptId, CScript* pScript)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::InitScriptSite(...)"), this)));
	ATLASSERT(I_AM_UI_THREAD); // must be entered by UI thread
	ATLASSERT(!m_DebugDocumentHelper);

	CHECK(CreateEngine(language, frameId, scriptId, NULL_SCRIPT_INSTANCE));
	CHECK(CreateCommonDebugHelper(pScript));
	HRESULT res = ParseScriptText(pScript);
	if (res!=S_OK) return res;
	// don't run common script site !
	// CHECK(Run());
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::Done()
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::Done(...)"), this)));
	ATLASSERT(!!m_Engine);
	ATLASSERT(m_CreatingThreadId==GetCurrentThreadId()); // must be called in same thread

	// disconnect the host application from the engine. This will prevent
	// the further firing of events. Event sinks that are in progress will
	// be completed before the state changes.
	if (m_ScriptInstanceId && m_EngineConnected) 
	{
		CHECK(m_Engine->SetScriptState(SCRIPTSTATE_DISCONNECTED));
		m_EngineConnected = false;
	}

	// call to InterruptScriptThread to abandon any running scripts and
	// force cleanup of all script elements.
	//EXCEPINFO ei;
	//CHECK(m_Engine->InterruptScriptThread(SCRIPTTHREADID_ALL, &ei, 0));

	// decide which helper to destroy
	if (m_ScriptInstanceId)
	{
		ScriptDebuggerLock scriptDebugger;
		scriptDebugger->DestroyScriptInstanceDebugDocumentHelper(m_ScriptInstanceId);
	}
	else
	{
		ScriptDebuggerLock scriptDebugger;
		scriptDebugger->DestroyScriptDebugDocumentHelper(m_ScriptId);
	}

	// always call prior to release
	CHECK(m_Engine->Close());

	// wait for closed state
	// TODO: here add timeout
	while (m_ConfirmedState!=SCRIPTSTATE_CLOSED)
	{
		// give other thread some chance to respond
		Sleep(WAIT_GRANULARITY);
	}

	// release engine and helper
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X] => releasing m_Engine"), this)));
	m_Engine.Release();

	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X] => releasing m_DebugDocumentHelper"), this)));
	m_DebugDocumentHelper = NULL;

	m_FrameId = NULL_FRAME;
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptSite::ParseScriptText(CScript* pScript)
{
	DT(TRACE_LI(FS(_T("CUpgradrScriptSite[%08X]::ParseScriptText(...)"), this)));
	ATLASSERT(!!m_Engine);

	// define script block in debug helper
	DWORD dwCookie = 0;
	CString& oleScript = pScript->Code();
	CHECK(m_DebugDocumentHelper->DefineScriptBlock(0, oleScript.GetLength(), m_Engine, FALSE, &dwCookie));

	// query IActiveScriptParse interface
	CComQIPtr<IActiveScriptParse> spParse = m_Engine;
	if(!spParse) return E_NOINTERFACE;

	// init parser
	CHECK(spParse->InitNew());

	// parse script
	const DWORD dwFlags = SCRIPTTEXT_ISVISIBLE | SCRIPTTEXT_HOSTMANAGESSOURCE;
	EXCEPINFO einfo;
	return spParse->ParseScriptText(oleScript, _T(""), NULL, NULL, dwCookie, 0, dwFlags, NULL, &einfo);
}