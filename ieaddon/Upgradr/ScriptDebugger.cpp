#include "StdAfx.h"
#include "ScriptDebugger.h"
#include "UpgradrDebugDocumentHelper.h"
#include "UpgradrScriptSite.h"
#include "WindowManager.h"

//#include "Debug.h"

CComObject<CUpgradrScriptDebugger>*			m_Debugger = NULL;

//////////////////////////////////////////////////////////////////////////
// CScriptBreakpoint

CScriptBreakpoint::CScriptBreakpoint(ULONG pos, ULONG chars, EBreakpointState state, CComPtr<IDebugCodeContext> context):
m_Pos(pos),
m_Chars(chars),
m_State(state),
m_DebugCodeContext(context)
{
	
}

//////////////////////////////////////////////////////////////////////////
// CScriptSiteInfo

CScriptSiteInfo::CScriptSiteInfo(TScriptId scriptId, TFrameId frameId, CComPtr<IUpgradrShell> shell, CScript* pScript)
{
	CHECK(CComObject<CUpgradrScriptSite>::CreateInstance(&m_Site));
	ATLASSERT(!!m_Site);
	m_Site->AddRef();

	m_Site->m_Shell = shell;

	// script sites must be inited from special thread !!!
	// this is limitation of scripting engine which must be destroyed from the same thread which has created it
	// we are creating it using UI thread
	// original code was: CHECK(m_Site->InitScriptSite(_T("jscript"), frameId, scriptId, pScript));
	CInitScriptSiteParams* pParams = new CInitScriptSiteParams(m_Site, _T("jscript"), frameId, scriptId, pScript);
	
	// we are going to call another thread (UI thread)
	// let him manipulate with some of our resources
	ResourceUnlock(SR_SCRIPTDEBUGGER);
	CHECK((HRESULT)SendMessage(UI_THREAD_HWND, WMM_INITSCRIPTSITE, 0, (LPARAM)pParams)); // callee is responsible for releasing params
}

CScriptSiteInfo::~CScriptSiteInfo()
{
	// script sites must be inited from special thread !!!
	// this is limitation of scripting engine which must be destroyed from the same thread which created it
	// we will create using UI thread
	// original code was: CHECK(m_Site->Done());

	// we are going to call another thread (UI thread)
	// let him manipulate with some of our resources
	{
		ResourceUnlock(SR_SCRIPTDEBUGGER);
		CHECK((HRESULT)SendMessage(UI_THREAD_HWND, WMM_DONESCRIPTSITE, 0, (LPARAM)m_Site)); // callee is responsible for releasing params
	}
	m_Site->Release();
	m_Site = NULL;
}

//////////////////////////////////////////////////////////////////////////
// CScriptDebugger

CScriptDebugger::CScriptDebugger():
m_ApplicationCookie(0),
m_DisableBreakpointTracking(false),
m_RunningScriptsHelper(NULL),
m_RunningFramesHelper(NULL),
INIT_TRACE(CScriptDebugger)
{
	DTI(TRACE_LI(FS(_T("Constructor()"))));
}

CScriptDebugger::~CScriptDebugger()
{
	DTI(TRACE_LI(FS(_T("Desctructor()"))));
	ATLASSERT(!m_Inited);
	ATLASSERT(!m_ScriptInstanceDocumentHelpers.size());
	ATLASSERT(!m_ScriptDocumentHelpers.size());
	ATLASSERT(!m_FrameDocumentHelpers.size());

	// Visual Web Developer 2005 - Express Edition, does not clear breakpoints on detach
	// this is not needed for Visual Studio 2005 - Professional, it clears breakpoints and reapplies them after attach
	TScriptDebugInfoMap::iterator i = m_ScriptDebugInfos.begin();
	while (i!=m_ScriptDebugInfos.end())
	{
		delete i->second;
		++i;
	}
	m_ScriptDebugInfos.clear();

	ATLASSERT(!m_ScriptDebugInfos.size());
}

bool                                         
CScriptDebugger::AdviseEvents()
{
	DTI(TRACE_LI(FS(_T("AdviseEvents()"))));
	CHECK_THREAD_OWNERSHIP;
	// sink application events
	CComObject<CUpgradrScriptDebugger>::CreateInstance(&m_Debugger);
	m_Debugger->AddRef();
	HRESULT hr = AtlAdvise(m_DebugApplication, m_Debugger->GetUnknown(), IID_IRemoteDebugApplicationEvents, &m_dwConnectCookie);
	return true;
}

bool                                         
CScriptDebugger::UnadviseEvents()
{
	DTI(TRACE_LI(FS(_T("UnadviseEvents()"))));
	CHECK_THREAD_OWNERSHIP;
	// stop sink application events
	if (m_dwConnectCookie) {
		AtlUnadvise(m_DebugApplication, IID_IRemoteDebugApplicationEvents, m_dwConnectCookie);
		m_dwConnectCookie = 0;
	}
	CHECK_REFCOUNT_BEFORE_DELETE(m_Debugger);
	m_Debugger->Release();
	m_Debugger = NULL;
	return true;
}

bool                                           
CScriptDebugger::Init()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_Inited) return true;
	DTI(TRACE_LI(FS(_T("Init()"))));

	HRESULT hr;
	// initialize the ProcessDebugManager
	hr = CoCreateInstance(CLSID_ProcessDebugManager, NULL, 
		CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER,
		IID_IProcessDebugManager,
		(void**)&m_ProcessDebugManager);
	if (!SUCCEEDED(hr)) 
	{
		TRACE_E(_T("Unable to create ProcessDebugManager"));
		return false;
	}

	CHECK(m_ProcessDebugManager->CreateApplication(&m_DebugApplication));
	ATLASSERT(!!m_DebugApplication);

	CHECK(m_DebugApplication->SetName(_T("Upgradr")));
	CHECK(m_ProcessDebugManager->AddApplication(m_DebugApplication, &m_ApplicationCookie));

	// Advise must be called after m_ProcessDebugManager->AddApplication(...) 
	// in other case we get deadlock in Visual Web Developer Express
	AdviseEvents();

	//////////////////////////////////////////////////////////////////////////

	m_RunningFramesHelper = new TDebugDocumentHelper();
	TDebugDocumentHelper& runningFramesHelper = *m_RunningFramesHelper;

	ATLASSERT(!runningFramesHelper);
	CHECK(ProcessDebugManager()->CreateDebugDocumentHelper(NULL, &runningFramesHelper));
	ATLASSERT(runningFramesHelper);

	CHECK(runningFramesHelper->Init(DebugApplication(), CComBSTR("Running Frames"), CComBSTR("Running Frames"), TEXT_DOC_ATTR_READONLY));
	CHECK(runningFramesHelper->Attach(NULL));
	CHECK(runningFramesHelper->SetDocumentAttr(TEXT_DOC_ATTR_READONLY));
	CHECK(runningFramesHelper->AddUnicodeText(CComBSTR("Running Frames")));

	//////////////////////////////////////////////////////////////////////////

	m_RunningScriptsHelper = new TDebugDocumentHelper();
	TDebugDocumentHelper& runningScriptsHelper = *m_RunningScriptsHelper;

	ATLASSERT(!runningScriptsHelper);
	CHECK(ProcessDebugManager()->CreateDebugDocumentHelper(NULL, &runningScriptsHelper));
	ATLASSERT(runningScriptsHelper);

	CHECK(runningScriptsHelper->Init(DebugApplication(), CComBSTR("Running Scripts"), CComBSTR("Running Scripts"), TEXT_DOC_ATTR_READONLY));
	CHECK(runningScriptsHelper->Attach(NULL));
	CHECK(runningScriptsHelper->SetDocumentAttr(TEXT_DOC_ATTR_READONLY));
	CHECK(runningScriptsHelper->AddUnicodeText(CComBSTR("Running scripts")));

	m_Inited = true;
	return true;
}

bool                                           
CScriptDebugger::Done()
{
	CHECK_THREAD_OWNERSHIP;
	ATLASSERT(m_Inited);
	if (!m_Inited) return true;
	DTI(TRACE_LI(FS(_T("Done()"))));
	m_RunningScriptsHelper->Release();
	delete m_RunningScriptsHelper;
	m_RunningFramesHelper->Release();
	delete m_RunningFramesHelper;
	CleanScriptSiteInfos();
	UnadviseEvents();
	CHECK(m_DebugApplication->Close());
	m_DebugApplication = NULL;
	CHECK(m_ProcessDebugManager->RemoveApplication(m_ApplicationCookie));
	m_ProcessDebugManager = NULL;
	m_Inited = false;
	return true;
}

bool                                          
CScriptDebugger::CreateFrameDebugDocumentHelper(TFrameId id, CString shortName, CString longName, BSTR content)
{
	DTI(TRACE_LI(FS(_T("CreateFrameDebugDocumentHelper(frameId=%d, shortName=%s, longName=%s, ...)"), id, shortName, longName)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TDebugDocumentHelper* debugDocumentHelperRef = new TDebugDocumentHelper();
	TDebugDocumentHelper& debugDocumentHelper = *debugDocumentHelperRef;
	
	m_FrameDocumentHelpers.insert(make_pair(id, debugDocumentHelperRef));

	ATLASSERT(!debugDocumentHelper);
	CHECK(ProcessDebugManager()->CreateDebugDocumentHelper(NULL, &debugDocumentHelper));
	ATLASSERT(debugDocumentHelper);

	CHECK(debugDocumentHelper->Init(DebugApplication(), CComBSTR(shortName), CComBSTR(longName), TEXT_DOC_ATTR_READONLY));

	CHECK(debugDocumentHelper->Attach(*RunningFramesHelper()));
	CHECK(debugDocumentHelper->SetDocumentAttr(TEXT_DOC_ATTR_READONLY));
	CHECK(debugDocumentHelper->AddUnicodeText(content));

	return true;
}

bool                                           
CScriptDebugger::DestroyFrameDebugDocumentHelper(TFrameId id)
{
	DTI(TRACE_LI(FS(_T("DestroyFrameDebugDocumentHelper(frameId=%d)"), id)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TFrameDebugDocumentHelperRefMap::iterator i = m_FrameDocumentHelpers.find(id);
	if (i==m_FrameDocumentHelpers.end()) return false;
	CComPtr<IDebugDocumentHelper>& documentHelper = *i->second;
	CHECK(documentHelper->Detach());
	documentHelper = NULL;
	delete i->second;
	m_FrameDocumentHelpers.erase(i);
	return true;
}

TDebugDocumentHelper*                          
CScriptDebugger::FindFrameDebugDocumentHelper(TFrameId id)
{
	DTI(TRACE_LI(FS(_T("FindFrameDebugDocumentHelper(frameId=%d)"), id)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TFrameDebugDocumentHelperRefMap::iterator i = m_FrameDocumentHelpers.find(id);
	if (i==m_FrameDocumentHelpers.end()) return NULL;
	return i->second;
}

bool                                           
CScriptDebugger::CreateScriptInstanceDebugDocumentHelper(TScriptInstanceId id, TScriptId scriptId, CString shortName, CString longName, BSTR content)
{
	DTI(TRACE_LI(FS(_T("CreateScriptInstanceDebugDocumentHelper(instanceId=%d, scriptId=%d, shortName=%s, longName=%s)"), id, scriptId, shortName, longName)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TDebugDocumentHelper* debugDocumentHelperRef = new TDebugDocumentHelper();
	TDebugDocumentHelper& debugDocumentHelper = *debugDocumentHelperRef;

	m_ScriptInstanceDocumentHelpers.insert(make_pair(id, make_pair(scriptId, debugDocumentHelperRef)));

	// create instance of out debug helper object
	CComObject<CUpgradrDebugDocumentHelper>* pDebugDocumentHelper;
	CComObject<CUpgradrDebugDocumentHelper>::CreateInstance(&pDebugDocumentHelper);
	pDebugDocumentHelper->AddRef();
	pDebugDocumentHelper->m_ScriptId = scriptId;
	EXPECTED_REFCOUNT(pDebugDocumentHelper, 1);

	CHECK(ProcessDebugManager()->CreateDebugDocumentHelper(NULL, &pDebugDocumentHelper->m_DebugDocumentHelper));

	pDebugDocumentHelper->QueryInterface(IID_IDebugDocumentHelper, (void**)&debugDocumentHelper);
	pDebugDocumentHelper->Release();

	EXPECTED_REFCOUNT(pDebugDocumentHelper, 1);

	CHECK(debugDocumentHelper->Init(DebugApplication(), CComBSTR(shortName), CComBSTR(longName), TEXT_DOC_ATTR_READONLY));
	CHECK(debugDocumentHelper->SetDocumentAttr(TEXT_DOC_ATTR_READONLY));
	CHECK(debugDocumentHelper->AddUnicodeText(content));
	return true;
}

bool                                           
CScriptDebugger::DestroyScriptInstanceDebugDocumentHelper(TScriptInstanceId id)
{
	DTI(TRACE_LI(FS(_T("DestroyScriptInstanceDebugDocumentHelper(instanceId=%d)"), id)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptInstanceDebugDocumentHelperRefMap::iterator i = m_ScriptInstanceDocumentHelpers.find(id);
	if (i==m_ScriptInstanceDocumentHelpers.end()) return false;
	CComPtr<IDebugDocumentHelper>& documentHelper = *i->second.second;
	CHECK(documentHelper->Detach());
	documentHelper = NULL;
	delete i->second.second;
	m_ScriptInstanceDocumentHelpers.erase(i);
	return true;
}

TDebugDocumentHelper*                          
CScriptDebugger::FindScriptInstanceDebugDocumentHelper(TScriptInstanceId id)
{
	DTI(TRACE_LI(FS(_T("FindScriptInstanceDebugDocumentHelper(instanceId=%d)"), id)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptInstanceDebugDocumentHelperRefMap::iterator i = m_ScriptInstanceDocumentHelpers.find(id);
	if (i==m_ScriptInstanceDocumentHelpers.end()) return NULL;
	return i->second.second;
}

bool                                           
CScriptDebugger::CreateScriptDebugDocumentHelper(TScriptId id, CString shortName, CString longName, BSTR content)
{
	DTI(TRACE_LI(FS(_T("CreateScriptDebugDocumentHelper(scriptId=%d, shortName=%s, longName=%s)"), id, shortName, longName)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TDebugDocumentHelper* debugDocumentHelperRef = new TDebugDocumentHelper();
	TDebugDocumentHelper& debugDocumentHelper = *debugDocumentHelperRef;

	m_ScriptDocumentHelpers.insert(make_pair(id, debugDocumentHelperRef));

	// create  of out debug helper object
	CComObject<CUpgradrDebugDocumentHelper>* pDebugDocumentHelper;
	CComObject<CUpgradrDebugDocumentHelper>::CreateInstance(&pDebugDocumentHelper);
	pDebugDocumentHelper->AddRef();
	pDebugDocumentHelper->m_ScriptId = id;
	EXPECTED_REFCOUNT(pDebugDocumentHelper, 1);

	CHECK(ProcessDebugManager()->CreateDebugDocumentHelper(NULL, &pDebugDocumentHelper->m_DebugDocumentHelper));

	pDebugDocumentHelper->QueryInterface(IID_IDebugDocumentHelper, (void**)&debugDocumentHelper);
	pDebugDocumentHelper->Release();
	EXPECTED_REFCOUNT(pDebugDocumentHelper, 1);

	CHECK(debugDocumentHelper->Init(DebugApplication(), CComBSTR(shortName), CComBSTR(longName), TEXT_DOC_ATTR_READONLY));
	CHECK(debugDocumentHelper->SetDocumentAttr(TEXT_DOC_ATTR_READONLY));
	CHECK(debugDocumentHelper->AddUnicodeText(content));
	return true;
}

bool                                           
CScriptDebugger::DestroyScriptDebugDocumentHelper(TScriptId id)
{
	DTI(TRACE_LI(FS(_T("DestroyScriptDebugDocumentHelper(scriptId=%d)"), id)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptDebugDocumentHelperRefMap::iterator i = m_ScriptDocumentHelpers.find(id);
	if (i==m_ScriptDocumentHelpers.end()) return false;
	CComPtr<IDebugDocumentHelper>& documentHelper = *i->second;
	CHECK(documentHelper->Detach());
	documentHelper = NULL;
	delete i->second;
	m_ScriptDocumentHelpers.erase(i);
	return true;
}

TDebugDocumentHelper*                          
CScriptDebugger::FindScriptDebugDocumentHelper(TScriptId id)
{
	DTI(TRACE_LI(FS(_T("FindScriptDebugDocumentHelper(scriptId=%d)"), id)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptDebugDocumentHelperRefMap::iterator i = m_ScriptDocumentHelpers.find(id);
	if (i==m_ScriptDocumentHelpers.end()) return NULL;
	return i->second;
}


class CBreakpointPositionMatcher {
public:
	CBreakpointPositionMatcher(ULONG pos, ULONG chars) : m_Pos(pos), m_Chars(chars) {}

	bool operator() (const CScriptBreakpoint& breakpoint) const {
		return breakpoint.m_Pos==m_Pos && breakpoint.m_Chars==m_Chars;
	}

	ULONG                                          m_Pos;
	ULONG                                          m_Chars;
};

class CBreakpointUpdater : public CBreakpointPositionMatcher {
public:
	CBreakpointUpdater(ULONG pos, ULONG chars, CScriptBreakpoint::EBreakpointState state) : CBreakpointPositionMatcher(pos, chars), m_State(state) {}

	void operator() (CScriptBreakpoint& breakpoint) const {
		if (CBreakpointPositionMatcher::operator()(breakpoint))
		{
			breakpoint.m_State = m_State;
		}
	}

	CScriptBreakpoint::EBreakpointState            m_State;
};

bool                                           
CScriptDebugger::SetupBreakpoint(TScriptId script, ULONG pos, ULONG chars, CScriptBreakpoint::EBreakpointState state, CComPtr<IDebugCodeContext> context)
{
	DTI(TRACE_LI(FS(_T("SetupBreakpoint(scriptId=%d, pos=%d, chars=%d, state=%d)"), script, pos, chars, state)));

	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	// this test is very tricky, 
	// script debugger tends to clear breakpoints after first setup after connection to debugee
	// we have observed that this strange behavior is when DebugApplication reports it's DebugFlags as "HALT"
	// in this situation we simply ignore request for removing breakpoints
	if (m_DebugFlags&APPBREAKFLAG_DEBUGGER_HALT && state==CScriptBreakpoint::DELETED) return true;

	// if we are re-applying breakpoints, we don't want them to be inserted again
	if (m_DisableBreakpointTracking) 
	{
		DT(TRACE_LI(FS(_T("  ... breakpoint tracking is disabled -> no action)"))));
		return true;
	}

	TScriptDebugInfoMap::iterator iScriptDebugInfo = m_ScriptDebugInfos.find(script);
	CScriptDebugInfo* pScriptDebugInfo = NULL;
	if (iScriptDebugInfo==m_ScriptDebugInfos.end()) 
	{
		if (state==CScriptBreakpoint::DELETED) return false;

		// we need to allocate new record
		pScriptDebugInfo = new CScriptDebugInfo();
		m_ScriptDebugInfos.insert(make_pair(script, pScriptDebugInfo));
	}
	else pScriptDebugInfo = iScriptDebugInfo->second;
	ATLASSERT(pScriptDebugInfo);
	CScriptDebugInfo& sdi = *pScriptDebugInfo;

	CBreakpointPositionMatcher matcher(pos, chars);

	if (state==CScriptBreakpoint::DELETED) 
	{
		TScriptBreakpointContainer::iterator iScriptBreakpoint = find_if(sdi.m_Breakpoints.begin(), sdi.m_Breakpoints.end(), matcher);
		if (iScriptBreakpoint==sdi.m_Breakpoints.end()) return false;

		// use erase-remove idiom
		sdi.m_Breakpoints.erase(remove_if(sdi.m_Breakpoints.begin(), sdi.m_Breakpoints.end(), matcher), sdi.m_Breakpoints.end());
		DT(TRACE_LI(FS(_T("  breakpoint#=%d"), sdi.m_Breakpoints.size())));
		if (sdi.m_Breakpoints.size()==0)
		{
			// get rid of whole debug info
			delete pScriptDebugInfo;
			m_ScriptDebugInfos.erase(iScriptDebugInfo);
		}
		DT(TRACE_LI(FS(_T("  debug infos#=%d"), m_ScriptDebugInfos.size())));
		return true;
	}

	// well, breakpoint was just added or updated
	TScriptBreakpointContainer::iterator iScriptBreakpoint = find_if(sdi.m_Breakpoints.begin(), sdi.m_Breakpoints.end(), matcher);
	if (iScriptBreakpoint==sdi.m_Breakpoints.end()) 
	{
		// breakpoint has to be added
		sdi.m_Breakpoints.push_back(CScriptBreakpoint(pos, chars, state, context));
	}
	else
	{
		CBreakpointUpdater updater(pos, chars, state);
		// we have some breakpoints matching, perform their update
		for_each(sdi.m_Breakpoints.begin(), sdi.m_Breakpoints.end(), updater);
	}
	DT(TRACE_LI(FS(_T("  breakpoint#=%d"), sdi.m_Breakpoints.size())));
	DT(TRACE_LI(FS(_T("  debug infos#=%d"), m_ScriptDebugInfos.size())));

	ApplyBreakpoints();
	return true;
}

bool                                         
CScriptDebugger::ApplyBreakpoints()
{
	DTI(TRACE_LI(FS(_T("ApplyBreakpoints()"))));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptInstanceDebugDocumentHelperRefMap::iterator i = m_ScriptInstanceDocumentHelpers.begin();
	while (i!=m_ScriptInstanceDocumentHelpers.end())
	{
		ApplyBreakpoints(*i->second.second, i->second.first, i->first);
		++i;
	}

	return true;
}

bool                                           
CScriptDebugger::ApplyBreakpoints(CComPtr<IDebugDocumentHelper> debugDocumentHelper, TScriptId scriptId, TScriptInstanceId scriptInstanceId)
{
	DTI(TRACE_LI(FS(_T("ApplyBreakpoints(docHelper=%08X, scriptId=%d, instanceId=%d)"), debugDocumentHelper, scriptId, scriptInstanceId)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	ATLASSERT(debugDocumentHelper);

	TScriptDebugInfoMap::iterator iScriptDebugInfo = m_ScriptDebugInfos.find(scriptId);
	if (iScriptDebugInfo==m_ScriptDebugInfos.end()) return true; // nothing to do
	CScriptDebugInfo& sdi = *iScriptDebugInfo->second;

	m_DisableBreakpointTracking = true;

	// iterate through enabled or disabled breakpoints
	TScriptBreakpointContainer::iterator iBreakpoint = sdi.m_Breakpoints.begin();
	while (iBreakpoint!=sdi.m_Breakpoints.end())
	{
		CScriptBreakpoint& breakpoint = *iBreakpoint;
		ApplyBreakpoint(breakpoint, scriptId, debugDocumentHelper);

		++iBreakpoint;
	}

	m_DisableBreakpointTracking = false;

	return true;
}

TDebugDocumentHelper*                          
CScriptDebugger::RunningScriptsHelper()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return NULL;
	return m_RunningScriptsHelper;
}

TDebugDocumentHelper*                          
CScriptDebugger::RunningFramesHelper()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return NULL;
	return m_RunningFramesHelper;
}

bool                                         
CScriptDebugger::RegisterScriptInstance(TScriptInstanceId scriptInstanceId, TScriptId scriptId, TFrameId frameId, CComPtr<IUpgradrShell> shell, CScript* pScript)
{
	DTI(TRACE_LI(FS(_T("RegisterScriptInstance(instanceId=%d, scriptId=%d, frameId=%d, shell=%08X, script=%08X)"), scriptInstanceId, scriptId, frameId, shell, pScript)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptSiteInfos::iterator i = m_ScriptSiteInfos.find(scriptId);
	CScriptSiteInfo* pScriptSiteInfo;
	if (i==m_ScriptSiteInfos.end())
	{
		// create a new site info
		pScriptSiteInfo = new CScriptSiteInfo(scriptId, frameId, shell, pScript);
		m_ScriptSiteInfos.insert(make_pair(scriptId, pScriptSiteInfo));
	}
	else
	{
		// update existing site
		pScriptSiteInfo = i->second;
	}

	pScriptSiteInfo->m_Instances.push_back(scriptInstanceId);
	return true;
}

bool                                         
CScriptDebugger::UnregisterScriptInstance(TScriptInstanceId scriptInstanceId, TScriptId scriptId)
{
	DTI(TRACE_LI(FS(_T("UnregisterScriptInstance(instanceId=%d, scriptId=%d)"), scriptInstanceId, scriptId)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptSiteInfos::iterator i = m_ScriptSiteInfos.find(scriptId);
	ATLASSERT(i!=m_ScriptSiteInfos.end());
	CScriptSiteInfo* pScriptSiteInfo = i->second;

	// use erase-remove idiom
	pScriptSiteInfo->m_Instances.erase(remove(pScriptSiteInfo->m_Instances.begin(), pScriptSiteInfo->m_Instances.end(), scriptInstanceId));
	return true;
}

bool                                         
CScriptDebugger::CleanScriptSiteInfos()
{
	DTI(TRACE_LI(FS(_T("CleanScriptSiteInfos()"))));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return false;
	TScriptSiteInfos::iterator i = m_ScriptSiteInfos.begin();
	while (i!=m_ScriptSiteInfos.end())
	{
		delete i->second;
		++i;
	}
	m_ScriptSiteInfos.clear();
	return true;
}

void 
CScriptDebugger::ApplyBreakpoint(CScriptBreakpoint& breakpoint, TScriptId scriptId, CComPtr<IDebugDocumentHelper> debugDocumentHelper)
{
	DTI(TRACE_LI(FS(_T("ApplyBreakpoint([pos=%d, chars=%d], scriptId=%d, docHelper=%08X)"), breakpoint.m_Pos, breakpoint.m_Chars, scriptId, debugDocumentHelper)));
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return;

	CComPtr<IDebugDocumentContext> debugDocumentContext;
	CHECK(debugDocumentHelper->CreateDebugDocumentContext(breakpoint.m_Pos, breakpoint.m_Chars, &debugDocumentContext));
	ATLASSERT(!!debugDocumentContext);

	CComPtr<IEnumDebugCodeContexts> enumDebugCodeContexts;
	CHECK(debugDocumentContext->EnumCodeContexts(&enumDebugCodeContexts));
	ATLASSERT(!!enumDebugCodeContexts);

	CComPtr<IDebugCodeContext> debugCodeContext;
	ULONG received = 0;
	CHECK(enumDebugCodeContexts->Next(1, &debugCodeContext, &received));
	ATLASSERT(received==1);
	ATLASSERT(!!debugCodeContext);

	CHECK(debugCodeContext->SetBreakPoint(breakpoint.m_State==CScriptBreakpoint::ENABLED?BREAKPOINT_ENABLED:BREAKPOINT_DISABLED));

	// test, there should be no more code contexts
#ifdef _DEBUG
	CComPtr<IDebugCodeContext> debugCodeContextTest;
	CHECK(enumDebugCodeContexts->Next(1, &debugCodeContextTest, &received));
	ATLASSERT(received==0);
	ATLASSERT(!debugCodeContextTest);
#endif
}