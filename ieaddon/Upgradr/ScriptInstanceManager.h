#pragma once

// zaznam o jedne instanci skriptu
class CScriptInstance {
public:
	TScriptSite*											m_ScriptSite;
	TScriptId												m_ScriptId;
	CUpgradrScriptEngine*							m_ScriptEngine;
};

// singleton: prirazuje Idcka instancim skriptu
typedef hash_map<TScriptInstanceId, CScriptInstance*> TScriptInstanceIdMap;

class CScriptInstanceManager : public TScriptInstanceIdMap, public CResourceInit<SR_SCRIPTINSTANCEMANAGER> {
public:
	typedef stack<TScriptInstanceId>					TScriptInstanceIdStack;

	CScriptInstanceManager();
	~CScriptInstanceManager();

	CScriptInstance*										FindScriptInstance(TScriptInstanceId id);
	TScriptInstanceId										InsertScriptInstance(CScriptInstance instance);
	bool														RemoveScriptInstance(TScriptInstanceId id);

private:
	TScriptInstanceIdStack								m_FreeScriptInstances;
	TScriptInstanceId										m_LastScriptInstanceId;
};
