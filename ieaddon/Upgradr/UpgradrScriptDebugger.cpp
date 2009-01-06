// UpgradrScriptDebugger.cpp : Implementation of CUpgradrScriptDebugger

#include "stdafx.h"
#include "UpgradrScriptDebugger.h"
#include "ScriptDebugger.h"

//#include "Debug.h"

// curious about Sleeps ?
//
// From:	 	Abraham Campbell
// Date:		Mon, Feb 11 2002 10:14 pm
//
//	I finally figured out what was going on.  My script host was *NOT* advising
//	the IRemoteDebugApplicationEvents.  When I added the implementation of
//	IRemoteDebugApplicationEvents to my component that implements
//	IApplicationDebugger and then I advised the events interface with my script
//	host, I got it to correctly single step--with one caveat.  Because this
//	problem is a timing issue with the PDM, I had to add "Sleep(0)" in the every
//	event method for IRemoteDebugApplicationEvents, whether or not I actually
//	did any processing in those methods.  When I did this, the "jumping" on
//	single step went away.   Since it is a timing issue, it could be affected by
//	your CPU speed.  So, you might try Sleep(50), Sleep(100), Sleep(150), etc.
//	But, Sleep(0) worked for me.   Reference Joel Alley's (from Microsoft)
//	"SampleHost.exe" on how to advise IRemoteDebugApplicationEvents.  This
//	example is still available on MSDN (luckily!)  Once I did all of this, it
//	finally worked for me.
//	
//	Microsoft has been able to confirm this problem and will log a bug against
//	the PDM.  But no promises on how long it will take the PDM group to resolve
//	the issue.  Best to go with the workaround. 
//
// see http://groups.google.com/group/microsoft.public.scripting.debugger/browse_thread/thread/4affce30c969c3cd/12307b652d370237?lnk=st&q=samplehost.exe&rnum=1&hl=en#12307b652d370237

//////////////////////////////////////////////////////////////////////////
// CUpgradrScriptDebugger

CUpgradrScriptDebugger::CUpgradrScriptDebugger()
{
}

CUpgradrScriptDebugger::~CUpgradrScriptDebugger()
{
}

//////////////////////////////////////////////////////////////////////////
// IRemoteDebugApplicationEvents

STDMETHODIMP 
CUpgradrScriptDebugger::OnConnectDebugger(IApplicationDebugger __RPC_FAR *pad)
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnConnectDebugger()"));
	return S_OK;
}


STDMETHODIMP 
CUpgradrScriptDebugger::OnDisconnectDebugger()
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnDisconnectDebugger()"));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnSetName(LPCOLESTR pstrName)
{
	Sleep(0);
	DT(TRACE_W(FS(_T("CUpgradrScriptDebugger::OnSetName(%S)"), pstrName)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnDebugOutput(LPCOLESTR pstr)
{
	Sleep(0);
	DT(TRACE_W(FS(_T("CUpgradrScriptDebugger::OnDebugOutput(%S)"), pstr)));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnClose()
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnClose()"));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnEnterBreakPoint(IRemoteDebugApplicationThread __RPC_FAR *prdat)
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnEnterBreakPoint()"));	
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnLeaveBreakPoint(IRemoteDebugApplicationThread __RPC_FAR *prdat)
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnLeaveBreakPoint()"));	
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnCreateThread(IRemoteDebugApplicationThread __RPC_FAR *prdat)
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnCreateThread()"));
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnDestroyThread(IRemoteDebugApplicationThread __RPC_FAR *prdat)
{
	Sleep(0);
	DT(TRACE_W("CUpgradrScriptDebugger::OnDestroyThread()"));	
	return S_OK;
}

STDMETHODIMP 
CUpgradrScriptDebugger::OnBreakFlagChange(APPBREAKFLAGS abf, IRemoteDebugApplicationThread __RPC_FAR *prdatSteppingThread)
{
	Sleep(0);
	DT(TRACE_W(FS(_T("CUpgradrScriptDebugger::OnBreakFlagChange(%ld)"), abf)));
	ScriptDebuggerLock scriptDebugger;
	scriptDebugger->m_DebugFlags = abf;
	return S_OK;
}