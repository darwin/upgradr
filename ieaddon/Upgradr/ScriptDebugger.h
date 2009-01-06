#pragma once

#include "UpgradrScriptDebugger.h"
#include "Script.h"
#include "ScriptSite.h"

typedef CComPtr<IDebugDocumentHelper>				TDebugDocumentHelper;
typedef hash_map<TFrameId, TDebugDocumentHelper*> TFrameDebugDocumentHelperRefMap;
typedef hash_map<TScriptId, TDebugDocumentHelper*> TScriptDebugDocumentHelperRefMap;
typedef hash_map<TScriptInstanceId, pair<TScriptId, TDebugDocumentHelper*> > TScriptInstanceDebugDocumentHelperRefMap;

class CBreakpointPositionMatcher;
class CBreakpointUpdater;
class CScriptDebugger;

/// Class tracks info about one breakpoint
class CScriptBreakpoint {
public:
	friend CBreakpointPositionMatcher;
	friend CBreakpointUpdater;
	friend CScriptDebugger;

	enum EBreakpointState {
		DELETED,
		ENABLED,
		DISABLED,
	};

	CScriptBreakpoint(ULONG pos, ULONG chars, EBreakpointState state, CComPtr<IDebugCodeContext> context);

protected:
	ULONG														m_Pos;
	ULONG														m_Chars;
	EBreakpointState										m_State;

public:
	// we need to hold this because debugger can watch reference count of IDebugCodeContext
	// if it drops to 1, debugger knows, he is last client holding IDebugCodeContext, thus releasing it
	CComPtr<IDebugCodeContext>							m_DebugCodeContext; 
};

typedef vector<CScriptBreakpoint>					TScriptBreakpointContainer;

class CScriptDebugInfo {
public:
	TScriptBreakpointContainer							m_Breakpoints;
};

typedef hash_map<TScriptId, CScriptDebugInfo*>	TScriptDebugInfoMap;

typedef vector<TScriptInstanceId>               TScriptInstanceIdContainer;

class CScriptSiteInfo {
public:
	CScriptSiteInfo(TScriptId scriptId, TFrameId frameId, CComPtr<IUpgradrShell> shell, CScript* pScript);
	~CScriptSiteInfo();

	TScriptSite*                                 m_Site;
	TScriptInstanceIdContainer                   m_Instances;
};

typedef hash_map<TScriptId, CScriptSiteInfo*>	TScriptSiteInfos;

class CScriptDebugger : public CResourceInit<SR_SCRIPTDEBUGGER>,
								public WinTrace
{
public:
	CScriptDebugger();
	~CScriptDebugger();

	bool														Init();
	bool														Done();

	CComPtr<IProcessDebugManager>&					ProcessDebugManager() { return m_ProcessDebugManager; }
	CComPtr<IDebugApplication>&						DebugApplication() { return m_DebugApplication; }
	
	TDebugDocumentHelper*								RunningScriptsHelper();
	TDebugDocumentHelper*								RunningFramesHelper();

	// FrameDebugDocumentHelpers
	bool														CreateFrameDebugDocumentHelper(TFrameId id, CString shortName, CString longName, BSTR content);
	bool														DestroyFrameDebugDocumentHelper(TFrameId id);
	TDebugDocumentHelper*								FindFrameDebugDocumentHelper(TFrameId id);

	// ScriptDebugDocumentHelpers
	bool														CreateScriptDebugDocumentHelper(TScriptId id, CString shortName, CString longName, BSTR content);
	bool														DestroyScriptDebugDocumentHelper(TScriptId id);
	TDebugDocumentHelper*								FindScriptDebugDocumentHelper(TScriptId id);

	// ScriptInstanceDebugDocumentHelpers
	bool														CreateScriptInstanceDebugDocumentHelper(TScriptInstanceId id, TScriptId scriptId, CString shortName, CString longName, BSTR content);
	bool														DestroyScriptInstanceDebugDocumentHelper(TScriptInstanceId id);
	TDebugDocumentHelper*								FindScriptInstanceDebugDocumentHelper(TScriptInstanceId id);

	// ScriptDebugInfos
	bool														SetupBreakpoint(TScriptId script, ULONG pos, ULONG chars, CScriptBreakpoint::EBreakpointState state, CComPtr<IDebugCodeContext> context);
	bool														ApplyBreakpoints(CComPtr<IDebugDocumentHelper> debugDocumentHelper, TScriptId scriptId, TScriptInstanceId scriptInstanceId);
	bool                                         ApplyBreakpoints();
	void														ApplyBreakpoint(CScriptBreakpoint& breakpoint, TScriptId scriptId, CComPtr<IDebugDocumentHelper> debugDocumentHelper);

	// Script sites
	bool                                         RegisterScriptInstance(TScriptInstanceId scriptInstanceId, TScriptId scriptId, TFrameId frameId, CComPtr<IUpgradrShell> shell, CScript* pScript);
	bool                                         UnregisterScriptInstance(TScriptInstanceId scriptInstanceId, TScriptId scriptId);
	bool                                         CleanScriptSiteInfos();

	bool                                         AdviseEvents();
	bool                                         UnadviseEvents();

	APPBREAKFLAGS                                m_DebugFlags;

private:
	CComPtr<IProcessDebugManager>						m_ProcessDebugManager; 
	CComPtr<IDebugApplication>							m_DebugApplication;
	DWORD														m_ApplicationCookie;
	TFrameDebugDocumentHelperRefMap					m_FrameDocumentHelpers;
	TScriptDebugDocumentHelperRefMap					m_ScriptDocumentHelpers;
	TScriptInstanceDebugDocumentHelperRefMap		m_ScriptInstanceDocumentHelpers;
	TScriptDebugInfoMap									m_ScriptDebugInfos;
	bool														m_DisableBreakpointTracking;
	TDebugDocumentHelper*								m_RunningScriptsHelper;
	TDebugDocumentHelper*								m_RunningFramesHelper;
	TScriptSiteInfos										m_ScriptSiteInfos;
	DWORD                                        m_dwConnectCookie;
};
