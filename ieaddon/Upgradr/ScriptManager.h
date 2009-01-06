#pragma once

#include "Script.h"

typedef hash_map<TScriptId, CScript*> TScriptContainer;

class CScriptManager : public TScriptContainer, 
	                    public CResourceInit<SR_SCRIPTMANAGER> {
public:
	enum EScriptType {
		ST_XML,
		ST_PLAIN,
		ST_URL,
		ST_UNKNOWN,
		ST_LAST
	};

	CScriptManager();
	~CScriptManager();

	bool                                           Init();
	bool                                           Done();

	bool                                           AddScript(CScript* script);

	bool                                           LoadUserSettings();
	bool                                           SaveUserSettings();

	CScript*                                       FindScript(TScriptId hScript);

	bool                                           SetScriptValue(CGuid guid, CString key, CString value);
	bool                                           GetScriptValue(CGuid guid, CString key, CString& value);

private:
	void                                           ReleaseScripts(CScript::ECategory cat = CScript::EC_ALL);

	// script parsers
	// delete
	CScript*                                       ParsePlainScript(LPCTSTR fileName);
};