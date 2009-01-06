// UpgradrScriptEngine.h : Declaration of the CUpgradrScriptEngine

#pragma once
#include "resource.h"       // main symbols

#include "Upgradr.h"

#include "Script.h"
#include "ScriptManager.h"
#include "UpgradrScriptSite.h"
#include "UpgradrShell.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

class CUpgradrAutomation;

typedef vector<std::pair<CString, bool> >         TScriptDisables;
typedef hash_map<TScriptId, TScriptInstanceId> TScriptInstanceLookupMap;

// CUpgradrScriptEngine
class ATL_NO_VTABLE CUpgradrScriptEngine :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpgradrScriptEngine, &CLSID_UpgradrScriptEngine>,
	public IDispatchImpl<IUpgradrScriptEngine, &IID_IUpgradrScriptEngine, &LIBID_UpgradrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	enum EMode {
		EM_FREE,   // detached
		EM_NORMAL, // attached
		EM_EDIT,   // edit mode
		EM_LAST
	};

	enum EStatus {
		ES_ENABLED,
		ES_DISABLED,
		ES_LAST
	};

	CUpgradrScriptEngine();
	~CUpgradrScriptEngine();

	DECLARE_REGISTRY_RESOURCEID(IDR_UPGRADRSCRIPTENGINE)

	DECLARE_NOT_AGGREGATABLE(CUpgradrScriptEngine)

	BEGIN_COM_MAP(CUpgradrScriptEngine)
		COM_INTERFACE_ENTRY(IUpgradrScriptEngine)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	IMPLEMENT_CLASS_NAME(CUpgradrScriptEngine);

	HRESULT													FinalConstruct();
	void														FinalRelease();

public:
	bool                                         SetScriptManager(CScriptManager* pScriptManager);
	bool                                         Attach(CComPtr<IHTMLDocument2> spDocument, CString kURL);
	bool                                         Detach();

	bool                                         EnterEditMode();
	bool                                         LeaveEditMode();

	bool                                         EnableEngine();
	bool                                         DisableEngine();

	bool                                         GetScriptDisables(TScriptDisables& scriptDisables);

	bool                                         ExecScript(CScript* pScript);
	bool                                         ExecCode(const TCHAR* code);

	CString                                      URL() const { return m_URL; }
	void                                         URL(CString val) { m_URL = val; }

protected:
	bool                                         ExtendScriptingModel();
	bool                                         RevertScriptingModel();

	bool                                         InjectSystemScripts();
	bool                                         RemoveSystemScripts();
	bool                                         InjectUserScripts();
	bool                                         RemoveUserScripts();

	bool                                         InjectScript(CScript* pScript);
	bool                                         RemoveScript(CScript* pScript);
	CComPtr<IHTMLScriptElement>                  LookupScriptTag(CScript* pScript);

	CString                                      GetMangledScripCode(CScript* pScript);
	bool														TestEnvironment();

private:
	EMode                                        m_Mode;
	EStatus                                      m_Status;

	CComPtr<IHTMLDocument2>                      m_Document;
	CString                                      m_URL;
	TFrameId                                     m_FrameId;
//	CComObject<CUpgradrShell>*							m_Shell;
	TScriptInstanceLookupMap                     m_ScriptInstances;

public:

	// IUpgradrScriptEngine
	STDMETHOD(SetFrameId)(LONG id);
	STDMETHOD(GetFrameId)(LONG* id);
	STDMETHOD(GetURL)(BSTR* url);
};

OBJECT_ENTRY_AUTO(__uuidof(UpgradrScriptEngine), CUpgradrScriptEngine)
