// UpgradrDebugCodeContext.cpp : Implementation of CUpgradrDebugCodeContext

#include "stdafx.h"
#include "UpgradrDebugCodeContext.h"
#include "ScriptDebugger.h"


// CUpgradrDebugCodeContext

CUpgradrDebugCodeContext::CUpgradrDebugCodeContext()
{
	TRACE_I(_T("CUpgradrDebugCodeContext::CUpgradrDebugCodeContext"));
}

CUpgradrDebugCodeContext::~CUpgradrDebugCodeContext()
{
	TRACE_I(_T("CUpgradrDebugCodeContext::~CUpgradrDebugCodeContext"));
}


//////////////////////////////////////////////////////////////////////////
// IDebugCodeContext

STDMETHODIMP
CUpgradrDebugCodeContext::GetDocumentContext(IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc)
{
	return m_DebugCodeContext->GetDocumentContext(ppsc);
}

STDMETHODIMP
CUpgradrDebugCodeContext::SetBreakPoint(BREAKPOINT_STATE bps)
{
	TRACE_I(FS(_T("CUpgradrDebugCodeContext::SetBreakPoint(%d) [pos=%d, chars=%d]"), bps, m_Pos, m_Chars));

	// yeaah, we have breakpoint info here, consult it with script debugger
	bool res = false; 
	{
		ScriptDebuggerLock scriptDebugger;
		switch (bps) {
		case BREAKPOINT_DELETED: res = scriptDebugger->SetupBreakpoint(m_ScriptId, m_Pos, m_Chars, CScriptBreakpoint::DELETED, this); break;
		case BREAKPOINT_ENABLED: res = scriptDebugger->SetupBreakpoint(m_ScriptId, m_Pos, m_Chars, CScriptBreakpoint::ENABLED, this); break;
		case BREAKPOINT_DISABLED: res = scriptDebugger->SetupBreakpoint(m_ScriptId, m_Pos, m_Chars, CScriptBreakpoint::DISABLED, this); break;
		default: ATLASSERT(0);
		}
	}

	// perform original action
	return m_DebugCodeContext->SetBreakPoint(bps);
}

