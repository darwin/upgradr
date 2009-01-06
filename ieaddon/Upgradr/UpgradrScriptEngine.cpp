// UpgradrScriptEngine.cpp : Implementation of CUpgradrScriptEngine

#include "stdafx.h"
#include "UpgradrScriptEngine.h"
#include "HandleManager.h"
#include "ScriptInstanceManager.h"
#include "ScriptDebugger.h"

CUpgradrScriptEngine::CUpgradrScriptEngine():
m_Mode(EM_FREE),
m_Status(ES_DISABLED),
m_FrameId(0)
//m_Shell(NULL)
{
}

CUpgradrScriptEngine::~CUpgradrScriptEngine()
{
	ATLASSERT(m_ScriptInstances.size()==0);
}

bool                                           
CUpgradrScriptEngine::SetScriptManager(CScriptManager* pScriptManager)
{
	return true;
}

bool                                           
CUpgradrScriptEngine::Attach(CComPtr<IHTMLDocument2> spDocument, CString kURL)
{
	Detach();
	// TODO: need this ? if (!WaitForDocumentReadyState(spDocument)) return false;
	m_Document = spDocument;
	m_URL = kURL;
	EnableEngine();
	return true;
}

bool                                           
CUpgradrScriptEngine::Detach()
{
	DisableEngine();
	if (m_Mode==EM_FREE) return false;

	m_Document = NULL;
	m_URL = "";
	m_Mode = EM_FREE;
	return true;
}

bool                                           
CUpgradrScriptEngine::EnableEngine()
{
	if (m_Status==ES_ENABLED) return false;
	if (!TestEnvironment()) 
	{
		TRACE_E(FS(_T("Disabling scripts in '%s' because of bad enviroment (restricted access?)"), m_URL));
		return false;
	}
	if (!ExtendScriptingModel()) return false;
	if (!InjectSystemScripts()) return false;
	if (!InjectUserScripts()) return false;
	m_Status = ES_ENABLED;
	return true;
}

bool                                           
CUpgradrScriptEngine::DisableEngine()
{
	if (m_Status==ES_DISABLED) return false;
	if (!RemoveUserScripts()) return false;
	if (!RemoveSystemScripts()) return false;
	if (!RevertScriptingModel()) return false;
	ATLASSERT(m_ScriptInstances.size()==0);
	m_Status = ES_DISABLED;
	return true;
}

bool                                           
CUpgradrScriptEngine::InjectSystemScripts()
{
	ScriptManagerLock scriptManager;
	CScriptManager::iterator i = scriptManager->begin();
	for (;i!=scriptManager->end();++i)
	{
		CScript* pScript = i->second;
		if (pScript->Category()!=CScript::EC_SYSTEM) continue;

		// well, inject the script
		if (!InjectScript(pScript))
		{
			// TODO: 
			TRACE_E(FS(_T("Cannot parse script '%s'"), pScript->Name()));
		}
	}
	return true;
}

bool                                           
CUpgradrScriptEngine::RemoveSystemScripts()
{
	ScriptManagerLock scriptManager;
	CScriptManager::iterator i = scriptManager->begin();
	for (;i!=scriptManager->end();++i)
	{
		CScript* pScript = i->second;
		if (pScript->Category()!=CScript::EC_SYSTEM) continue;

		// well, remove script
		RemoveScript(pScript);
	}

	return true;
}

bool                                           
CUpgradrScriptEngine::InjectUserScripts()
{
	ScriptManagerLock scriptManager;
	CScriptManager::iterator i = scriptManager->begin();
	for (;i!=scriptManager->end();++i)
	{
		CScript* pScript = i->second;
		if (pScript->Category()!=CScript::EC_USER) continue;

		// test user settings

		// test matching site regexp

		// well, inject script
		if (!InjectScript(pScript))
		{
			// TODO: 
			TRACE_E(FS(_T("Cannot parse script '%s'"), pScript->Name()));
		}
	}

	return true;
}

bool                                           
CUpgradrScriptEngine::RemoveUserScripts()
{
	ScriptManagerLock scriptManager;
	CScriptManager::iterator i = scriptManager->begin();
	for (;i!=scriptManager->end();++i)
	{
		CScript* pScript = i->second;
		if (pScript->Category()!=CScript::EC_USER) continue;

		// TODO: test user settings

		// test matching site regexp

		// well, remove script
		RemoveScript(pScript);
	}

	return true;
}

CString                                        
CUpgradrScriptEngine::GetMangledScripCode(CScript* pScript)
{
	ATLASSERT(pScript);

	// wrap the original code into function wrapper
	CString code;

	// preallocate string => conservative estimate
	code.GetBuffer(pScript->Code().GetLength()+2*GUID_STRING_LEN+100);
	code.ReleaseBuffer(0);

	code += _T("(function(){\n");
	code += pScript->Code();
	code += _T("\n})()");
	return code;
}

bool                                           
CUpgradrScriptEngine::TestEnvironment()
{
	ATLASSERT(!!m_Document);
	CComPtr<IHTMLWindow2> window;
	CHECK_COM(m_Document->get_parentWindow(&window), _T("Cannot retrieve parent window"));
	ATLASSERT(!!window);

	CComVariant ret;
	HRESULT hr = window->execScript(CComBSTR(_T("window.length; // empty code")), CComBSTR(_T("JavaScript")), &ret);
	if (hr==E_ACCESSDENIED) return false;
	ATLASSERT(hr==S_OK);
	return true;
}

bool                                           
CUpgradrScriptEngine::ExecCode(const TCHAR* code)
{
	ATLASSERT(code);
	ATLASSERT(!!m_Document);

	// exec script
	CComPtr<IHTMLWindow2> window;
	CHECK_COM(m_Document->get_parentWindow(&window), _T("Cannot retrieve parent window"));
	ATLASSERT(!!window);
	CComVariant ret;
	CHECK_COM(window->execScript(CComBSTR(code), CComBSTR(_T("JavaScript")), &ret), _T("Script launcher has failed"));
	return true;
}

bool 
CUpgradrScriptEngine::ExecScript(CScript* pScript)
{
	ATLASSERT(pScript);

	// exec script
	CString launcher = GetMangledScripCode(pScript);
	return ExecCode(launcher);
}

bool                                           
CUpgradrScriptEngine::InjectScript(CScript* pScript)
{
	//////////////////////////////////////////////////////////////////////////
	// run a new script site

	// register script instance 
	TScriptId scriptId;
	{
		HandleManagerLock handleManager;
		scriptId = handleManager->GetHandle(pScript->Guid());
	}

	TScriptSite* pSite;
	CHECK(CComObject<CUpgradrScriptSite>::CreateInstance(&pSite));
	ATLASSERT(!!pSite);
	pSite->AddRef();

	// create new instance of upgradr shell object (automation client)
	CComObject<CUpgradrShell>*	shell;
	CComObject<CUpgradrShell>::CreateInstance(&shell);
	ATLASSERT(!!shell);
	//m_Shell->AddRef();
	shell->m_ScriptEngine = this;

	// retrieve document's window object
	CComPtr<IHTMLWindow2> window;
	CHECK_COM(m_Document->get_parentWindow(&window), _T("Cannot retrieve parent window"));
	if (!window) 
	{
		TRACE_E(_T("Cannot retrieve parent window for document"));
		return false;
	}

	// cast window object to IDispatchEx
	CComQIPtr<IDispatchEx> windowDispEx = window;
	if (!windowDispEx)
	{
		TRACE_E(_T("Cannot retrieve IDispatchEx on parent window of document"));
		return false;
	}

	shell->m_Window = windowDispEx; // TODO: nicer

	// initialize site - TODO: make it nicer
	pSite->m_Shell = shell;

	TScriptInstanceId scriptInstanceId;
	{
		ScriptInstanceManagerLock instanceManager;
		CScriptInstance scriptInstance;
		scriptInstance.m_ScriptId = scriptId;
		scriptInstance.m_ScriptSite = pSite;
		scriptInstance.m_ScriptEngine = this;
		scriptInstanceId = instanceManager->InsertScriptInstance(scriptInstance); 
	}
	m_ScriptInstances.insert(make_pair(scriptId, scriptInstanceId));

	shell->m_ScriptInstanceId = scriptInstanceId;

	{
		ScriptDebuggerLock scriptDebugger;
		if (scriptDebugger->m_DebugFlags&APPBREAKFLAG_DEBUGGER_HALT)
		{
			// tohle musim udelat, protoze debugger je po pripojeni v divnem stavu
			// proste naprazdno zinicializuju engine s jednoduchym skriptem, ktery spustim
			// debugger se probere a dalsi volani uz jsou v poradku

			CScript dummyScript;
			dummyScript.Code() = _T("var a = 0;\n");
			dummyScript.Name() = _T("dummy script");
			dummyScript.Description() = _T("dummy script");
			dummyScript.FileName() = _T("C:\\dummyscript.js");
			dummyScript.GuessGuid();

			pSite->InitInstanceSite(_T("jscript"), m_FrameId, scriptId, scriptInstanceId, &dummyScript);
			pSite->Done();
		}
	}

	// script debugger must be unlocked at this point
	HRESULT res = pSite->InitInstanceSite(_T("jscript"), m_FrameId, scriptId, scriptInstanceId, pScript);

	// register debug script site
	{
		ScriptDebuggerLock scriptDebugger;
		scriptDebugger->RegisterScriptInstance(scriptInstanceId, scriptId, m_FrameId, shell, pScript);
	}

	// don't forget to release shell !
	//m_Shell->m_ScriptEngine = NULL; 
	//CHECK_REFCOUNT_BEFORE_DELETE(m_Shell);
	//m_Shell->Release();	
	//m_Shell = NULL; // at this point shell can be still referenced by IE

	return res==S_OK;
}

bool                                           
CUpgradrScriptEngine::RemoveScript(CScript* pScript)
{
	TScriptId scriptId;
	{
		HandleManagerLock handleManager;
		scriptId = handleManager->GetHandle(pScript->Guid());
	}
	TScriptInstanceLookupMap::iterator lookup = m_ScriptInstances.find(scriptId);
	ATLASSERT(lookup!=m_ScriptInstances.end());
	if (lookup==m_ScriptInstances.end()) return false;
	TScriptInstanceId scriptInstanceId = lookup->second;

	// lookup instance id
	{
		ScriptInstanceManagerLock scriptInstanceManager;
		CScriptInstance* scriptInstance = scriptInstanceManager->FindScriptInstance(scriptInstanceId);
		ATLASSERT(!!scriptInstance);
		if (!scriptInstance) return false;
		CHECK(scriptInstance->m_ScriptSite->Done());
		scriptInstance->m_ScriptSite->Release();
		
		// remove instance
		scriptInstanceManager->RemoveScriptInstance(scriptInstanceId);
	}

	// unregister debug script site
	{
		ScriptDebuggerLock scriptDebugger;
		scriptDebugger->UnregisterScriptInstance(scriptInstanceId, scriptId);
	}

	m_ScriptInstances.erase(lookup);
	return false;
}

bool                                           
CUpgradrScriptEngine::EnterEditMode()
{
	return false;
}

bool                                           
CUpgradrScriptEngine::LeaveEditMode()
{
	return false;
}

bool                                           
CUpgradrScriptEngine::GetScriptDisables(TScriptDisables& scriptDisables)
{
	ScriptManagerLock scriptManager;
	CScriptManager::iterator i = scriptManager->begin();
	while (i!=scriptManager->end())
	{
		CScript* script = i->second;
		scriptDisables.push_back(std::make_pair(script->Name(), false));
		++i;
	}
	return true;
}

bool
CUpgradrScriptEngine::ExtendScriptingModel()
{
	ATLASSERT(m_Document);

	// create debug document helper
	{
		ScriptDebuggerLock scriptDebugger;
		CString shortName;
		shortName.Format(_T("[F%d] %s"), m_FrameId, m_URL);
		CString longName;
		longName = m_URL;
		BSTR content = GetUnicodeHTML(m_Document);
		scriptDebugger->CreateFrameDebugDocumentHelper(m_FrameId, shortName, longName, content);
	}
	return true;
}

bool                                           
CUpgradrScriptEngine::RevertScriptingModel()
{
	ATLASSERT(!!m_Document);

	// destroy debug document helper
	{
		ScriptDebuggerLock scriptDebugger;
		scriptDebugger->DestroyFrameDebugDocumentHelper(m_FrameId);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CUpgradrScriptEngine::SetFrameId(LONG id)
{
	m_FrameId = id;
	return S_OK;
}

STDMETHODIMP CUpgradrScriptEngine::GetFrameId(LONG* id)
{
	if (!id) return E_POINTER;
	*id = m_FrameId;
	return S_OK;
}

STDMETHODIMP CUpgradrScriptEngine::GetURL(BSTR* url)
{
	if (!url) return E_POINTER;
	CComBSTR str = m_URL;
	return str.CopyTo(url);;
}

HRESULT CUpgradrScriptEngine::FinalConstruct()
{
	return S_OK;
}

void CUpgradrScriptEngine::FinalRelease()
{

}