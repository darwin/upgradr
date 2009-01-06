// UpgradrShell.cpp : Implementation of CUpgradrShell

#include "stdafx.h"
#include "UpgradrShell.h"
#include "ConsoleWindow.h"
#include "ScriptInstanceManager.h"
#include "ScriptManager.h"
#include "Services.h"
#include "WindowManager.h"

//#include "Debug.h"

static CComTypeInfoHolder g_TypeInfoHolder = { &IID_IUpgradrShell, &LIBID_UpgradrLib, 1, 0, 0, 0, 0, 0};

// CUpgradrShell
CUpgradrShell::CUpgradrShell():
m_ScriptEngine(NULL),
m_ScriptInstanceId(NULL_SCRIPT_INSTANCE)
{
	DT(TRACE_LI(FS(_T("Shell[%08X]: constructor"), this)));
}

CUpgradrShell::~CUpgradrShell()
{
	DT(TRACE_LI(FS(_T("Shell[%08X]: destructor"), this)));
}

STDMETHODIMP 
CUpgradrShell::InterfaceSupportsErrorInfo(REFIID riid)
{
	DT(TRACE_LI(FS(_T("Shell[%08X]: InterfaceSupportsErrorInfo(riid=%08X)"), this, riid)));
	static const IID* arr[] = 
	{
		&IID_IUpgradrShell
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP 
CUpgradrShell::Log(BSTR message)
{
	CString msg(message);
	DT(TRACE_LI(FS(_T("Shell[%08X]: Log(%s)"), this, msg)));

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	// ask script engine for info
	ATLASSERT(!!m_ScriptEngine);
	CComBSTR urlWrapper;
	m_ScriptEngine->GetURL(&urlWrapper);
	LONG idWrapper;
	m_ScriptEngine->GetFrameId(&idWrapper);
	CString url = urlWrapper;
	CString frame;
	frame.Format(_T("%d"), idWrapper);
	CString scriptName = "global";
	if (m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE)
	{
		CScriptInstance* scriptInstance;
		{
			ScriptInstanceManagerLock scriptInstanceManager;
			scriptInstance = scriptInstanceManager->FindScriptInstance(m_ScriptInstanceId);
		}
		CScript* script = NULL;
		if (scriptInstance)
		{
			ScriptManagerLock scriptManager;
			script = scriptManager->FindScript(scriptInstance->m_ScriptId);
			if (script)
			{
				scriptName = script->Name();
			}
		}
	}

	// add message to logger
	// retrieve logger
	{
		LoggerLock logger;
		logger->AddMessage(time, msg, scriptName, frame, url);
	}

	// refresh the console (if available)
	WindowManagerLock windowManager;
	windowManager->RefreshConsoles();

	return S_OK;
}

STDMETHODIMP 
CUpgradrShell::GetValue(BSTR key, BSTR* result)
{
	DT(TRACE_LI(FS(_T("Shell[%08X]: GetValue(...)"), this)));
	ATLASSERT(m_ScriptEngine);

	ATLASSERT(m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE);
	CScriptInstance* scriptInstance;
	{
		ScriptInstanceManagerLock scriptInstanceManager;
		scriptInstance = scriptInstanceManager->FindScriptInstance(m_ScriptInstanceId);
	}
	ATLASSERT(scriptInstance);
	if (!scriptInstance) return S_OK;
	CScript* script = NULL;
	{
		ScriptManagerLock scriptManager;
		script = scriptManager->FindScript(scriptInstance->m_ScriptId);

		ATLASSERT(script);
		if (!script) return S_OK;

		CGuid& _guid = script->Guid();
		CString _key(key);
		CString _result;
		bool found = scriptManager->GetScriptValue(_guid, _key, _result);
		if (!found) return S_OK;
		CComBSTR str = _result;
		return str.CopyTo(result);;
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrShell::HasValue(BSTR key, VARIANT_BOOL* result)
{
	DT(TRACE_LI(FS(_T("Shell[%08X]: GetValue(...)"), this)));
	ATLASSERT(m_ScriptEngine);

	ATLASSERT(m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE);
	CScriptInstance* scriptInstance;
	{
		ScriptInstanceManagerLock scriptInstanceManager;
		scriptInstance = scriptInstanceManager->FindScriptInstance(m_ScriptInstanceId);
	}
	ATLASSERT(scriptInstance);
	if (!scriptInstance) return S_OK;
	CScript* script = NULL;
	{
		ScriptManagerLock scriptManager;
		script = scriptManager->FindScript(scriptInstance->m_ScriptId);

		ATLASSERT(script);
		if (!script) return S_OK;

		CGuid& _guid = script->Guid();
		CString _key(key);
		CString _result;
		*result = scriptManager->GetScriptValue(_guid, _key, _result)?VARIANT_TRUE:VARIANT_FALSE;
	}
	return S_OK;
}

STDMETHODIMP 
CUpgradrShell::SetValue(BSTR key, BSTR value)
{
	DT(TRACE_LI(FS(_T("Shell[%08X]: SetValue(...)"), this)));
	ATLASSERT(m_ScriptEngine);

	ATLASSERT(m_ScriptInstanceId!=NULL_SCRIPT_INSTANCE);
	CScriptInstance* scriptInstance;
	{
		ScriptInstanceManagerLock scriptInstanceManager;
		scriptInstance = scriptInstanceManager->FindScriptInstance(m_ScriptInstanceId);
	}
	ATLASSERT(scriptInstance);
	if (!scriptInstance) return S_OK;
	CScript* script = NULL;
	{
		ScriptManagerLock scriptManager;
		script = scriptManager->FindScript(scriptInstance->m_ScriptId);

		ATLASSERT(script);
		if (!script) return S_OK;

		CGuid& _guid = script->Guid();
		CString _key(key);
		CString _value(value);
		bool res = scriptManager->SetScriptValue(_guid, _key, _value);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP 
CUpgradrShell::GetTypeInfoCount(UINT* pctinfo)
{
	ATLASSERT(m_Window);
	if (!pctinfo) return E_POINTER;
	HRESULT res = m_Window->GetTypeInfoCount(pctinfo);
	if (res!=S_OK) return res;
	*pctinfo++;
	return S_OK;
}

STDMETHODIMP 
CUpgradrShell::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
	ATLASSERT(m_Window);
	LoadTypeLib(lcid);
	return m_Window->GetTypeInfo(itinfo, lcid, pptinfo);
}

STDMETHODIMP 
CUpgradrShell::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	ATLASSERT(m_Window);
	LoadTypeLib(lcid);

	if (cNames==1) // no named arguments, use GetDispID
	{
		// this will handle self+window commands
		return this->GetDispID(CComBSTR(rgszNames[0]), fdexNameCaseSensitive, rgdispid);
	}

	// well, named arguments may be supported only by window
	return m_Window->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP 
CUpgradrShell::Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, UINT* puArgErr)
{
	ATLASSERT(m_Window);
	LoadTypeLib(lcid);
	if (IsWindowId(id))
	{
		id = MapIdFromShellToWindow(id);
		return m_Window->Invoke(id, riid, lcid, wFlags, pdp, pvarRes, pei, puArgErr);
	}
	if (IsSelfId(id))
	{
		id = MapIdFromShellToSelf(id);
		// I believe Invoke call is thread-safe on constructed g_TypeInfoHolder
		return g_TypeInfoHolder.Invoke(static_cast<IUpgradrShell*>(this), id, IID_NULL, lcid, wFlags, pdp, pvarRes, pei, puArgErr);
	}

	ATLASSERT(0);
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrShell::GetDispID(BSTR bstrName, DWORD grfdex, DISPID* pid)
{
	ATLASSERT(m_Window);
	LoadTypeLib(0);

	// try to locate name in self
	// i believe Invoke call is thread-safe on constructed g_TypeInfoHolder
	HRESULT hr = g_TypeInfoHolder.GetIDsOfNames(IID_NULL, &bstrName, 1, 0, pid);
	if (hr==S_OK) 
	{
		*pid = MapIdFromSelfToShell(*pid);
		return hr;
	}

	// try to locate name in window
	HRESULT res = m_Window->GetDispID(bstrName, grfdex, pid);
	if (res!=S_OK) return res;
	*pid = MapIdFromWindowToShell(*pid);
	return res;
}

STDMETHODIMP 
CUpgradrShell::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, IServiceProvider* pspCaller)
{
	ATLASSERT(m_Window);
	DT(TRACE_LI(FS(_T("Shell[%08X]: InvokeEx(id=%d, pspCaller=%08X)"), this, id, pspCaller)));

	if (IsWindowId(id))
	{
		id = MapIdFromShellToWindow(id);
		return m_Window->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
	}

	if (IsSelfId(id))
	{
		id = MapIdFromShellToSelf(id);

		HRESULT res = S_OK;
		if (m_ScriptInstanceId != NULL_SCRIPT_INSTANCE)
		{
			UINT uArgErr;
			// I believe Invoke call is thread-safe on constructed g_TypeInfoHolder
			res = g_TypeInfoHolder.Invoke(static_cast<IUpgradrShell*>(this), id, IID_NULL, lcid, wFlags, pdp, pvarRes, pei, &uArgErr);
			return res;
		}
	}

	ATLASSERT(0);
	return E_NOTIMPL;
}

STDMETHODIMP 
CUpgradrShell::DeleteMemberByName(BSTR bstr, DWORD grfdex)
{
	ATLASSERT(m_Window);
	return m_Window->DeleteMemberByName(bstr, grfdex);
}

STDMETHODIMP 
CUpgradrShell::DeleteMemberByDispID(DISPID id)
{
	ATLASSERT(m_Window);
	id = MapIdFromShellToWindow(id);
	return m_Window->DeleteMemberByDispID(id);
}

STDMETHODIMP 
CUpgradrShell::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD* pgrfdex)
{
	ATLASSERT(m_Window);
	id = MapIdFromShellToWindow(id);
	return m_Window->GetMemberProperties(id, grfdexFetch, pgrfdex);
}

STDMETHODIMP 
CUpgradrShell::GetMemberName(DISPID id, BSTR* pbstrName)
{
	ATLASSERT(m_Window);
	id = MapIdFromShellToWindow(id);
	return m_Window->GetMemberName(id, pbstrName);
}

STDMETHODIMP 
CUpgradrShell::GetNextDispID(DWORD grfdex, DISPID id, DISPID* pid)
{
	ATLASSERT(m_Window);
	id = MapIdFromShellToWindow(id);
	return m_Window->GetNextDispID(grfdex, id, pid);
}

STDMETHODIMP 
CUpgradrShell::GetNameSpaceParent(IUnknown** ppunk)
{
	ATLASSERT(m_Window);
	return m_Window->GetNameSpaceParent(ppunk);
}

//////////////////////////////////////////////////////////////////////////

DISPID 
CUpgradrShell::MapIdFromWindowToShell(DISPID windowId)
{
	unsigned long uid = (unsigned long)windowId;
	return (DISPID)(uid+100);
}

DISPID 
CUpgradrShell::MapIdFromShellToWindow(DISPID id)
{
	unsigned long uid = (unsigned long)id;
	ATLASSERT(uid>=100);
	return (DISPID)(uid-100);
}

DISPID 
CUpgradrShell::MapIdFromSelfToShell(DISPID selfId)
{
	unsigned long uid = (unsigned long)selfId;
	ATLASSERT(uid<100);
	return (DISPID)(uid);
}

DISPID 
CUpgradrShell::MapIdFromShellToSelf(DISPID id)
{
	unsigned long uid = (unsigned long)id;
	ATLASSERT(uid<100);
	return (DISPID)(uid);
}

bool 
CUpgradrShell::IsWindowId(DISPID id)
{
	unsigned long uid = (unsigned long)id;
	return uid>=100;
}

bool 
CUpgradrShell::IsSelfId(DISPID id)
{
	return !IsWindowId(id);
}

// ensure typelib is loaded
HRESULT 
CUpgradrShell::LoadTypeLib(LCID lcid)
{
	CSGUARD(m_Lock); // ensure thread safety
	if (g_TypeInfoHolder.m_pInfo) return S_OK;
	
	// load TypeInfo
	CComPtr<ITypeInfo> spInfo;
	g_TypeInfoHolder.GetTI(lcid, &spInfo);
	return S_OK;
}
