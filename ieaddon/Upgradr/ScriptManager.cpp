#include "StdAfx.h"
#include <string>

#include "HandleManager.h"
#include "ScriptManager.h"
#include "Script.h"
#include "Database.h"

//#include "Debug.h"

#define USERSCRIPT_TOKEN "==UserScript=="

CScriptManager::CScriptManager()
{
	DT(TRACE_I(_T("CScriptManager constructor")));
}

CScriptManager::~CScriptManager()
{
	DT(TRACE_I(_T("CScriptManager destructor")));
}

bool                                           
CScriptManager::Init()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_Inited) return true;

	DT(TRACE_I(_T("CScriptManager::Init()")));

	//TCHAR path[_MAX_PATH];
	//DWORD size;

	//// load user scripts
	//size = _MAX_PATH;
	//if (GetStringValueFromRegistry(REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_USERSCRIPTSDIR, path, &size))
	//{
	//	TRACE_E(FS(_T("Cannot read user scripts location from HKCU\\%s%s\\%s"), REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_USERSCRIPTSDIR));
	//	// FATAL
	//	return false;
	//}
	//LoadFromDirectory(path, CScript::EC_USER);

	//// load system scripts
	//size = _MAX_PATH;
	//if (GetStringValueFromRegistry(REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_SYSTEMSCRIPTSDIR, path, &size))
	//{
	//	TRACE_E(FS(_T("Cannot read system scripts location from HKCU\\%s%s\\%s"), REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_SYSTEMSCRIPTSDIR));
	//	// FATAL
	//	return false;
	//}
	//LoadFromDirectory(path, CScript::EC_SYSTEM);

	// load user settings
	LoadUserSettings();

	m_Inited = true;
	return true;
}

bool                                           
CScriptManager::Done()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return true;

	DT(TRACE_I(_T("CScriptManager::Done()")));

	// save user settings
	SaveUserSettings();

	// release all
	ReleaseScripts();

	m_Inited = false;
	return true;
}

void                                           
CScriptManager::ReleaseScripts(CScript::ECategory cat)
{
	CHECK_THREAD_OWNERSHIP;
	if (cat==CScript::EC_ALL)
	{
		while (size())
		{
			iterator i = begin();
			delete i->second;
			erase(i);
		}
	}
	else
	{
		iterator i = begin();
		while (i!=end())
		{
			if (i->second->Category()==cat)
			{
				iterator s;
				++s;
				delete i->second;
				erase(i);
				i = s;
				continue;
			}
			++i;
		}
	}
}

#define INITIAL_COMMENT_BUFFER_COUNT (64*1024) // 64kb
#define INITIAL_CODE_BUFFER_COUNT (100*1024) // 100kb
#define PROPNAME_BUFFER_COUNT (1*1024) // 1kb
#define PROPVALUE_BUFFER_COUNT (4*1024) // 4kb

CScript*                                       
CScriptManager::ParsePlainScript(LPCTSTR fileName)
{
	CHECK_THREAD_OWNERSHIP;
	// well, open file in text mode
	tifstream file(fileName);
	if (!file) return NULL;
	
	// 0 = begin, 1 = userscript section, 2 = code
	int state = 0; 
	size_t lineCounter = 0;
	CScript* pScript = new CScript();

	pScript->Code().GetBuffer(INITIAL_CODE_BUFFER_COUNT);
	pScript->Code().ReleaseBuffer(0);

	pScript->Comment().GetBuffer(INITIAL_COMMENT_BUFFER_COUNT);
	pScript->Comment().ReleaseBuffer(0);

	TRACE_I(_T("Code"));
	TTrace::Debug()->Indent();
	while (!file.eof() && file.good())
	{
		TCHAR line[64*1024];
		lineCounter++;

		//file >> ws; // skip whitespace
		if (file.eof()) break;
		file.getline(line, 64*1024);

		TRACE_I(line);

		// gather code
		if (pScript->Code().GetLength()) pScript->Code()+=_T('\n');
		pScript->Code() += line;

		TCHAR* sws = line;
		while (*sws && *sws==_T(' ') || *sws==_T('\t')) sws++;
		TCHAR* linews = sws;

		if (state!=2 && linews[0]==0) continue; // skip empty lines
		if (linews[0]==_T('/') && linews[1]==_T('/'))
		{
			// we have javascript comment here
			if (_tcsstr(sws, _T(USERSCRIPT_TOKEN)))
			{
				state = 1;
				continue;
			}
		}
		else
		{
			// js code here
			if (!state) 
			{
				delete pScript;
				return NULL;
			}
			state = 2;
		}

		switch (state) {
		case 0:
			if (pScript->Comment().GetLength()) pScript->Comment()+=_T('\n');
			pScript->Comment()+=line;
			break;
		case 1:
			// look for userscript parameters
			{
				TCHAR* pos = linews+2;
				while (*pos && *pos==_T(' ') || *pos==_T('\t')) pos++;
				if (*pos)
				{
					if (*pos==_T('@'))
					{
						pos++;

						// read property name and value
						TCHAR propName[PROPNAME_BUFFER_COUNT];
						TCHAR* propNamePos = propName;
						while (*pos && !(*pos==_T(' ') || *pos==_T('\t'))) 
						{
							*propNamePos = *pos;
							propNamePos++;
							if (propNamePos==propName+PROPNAME_BUFFER_COUNT-1)
							{
								TRACE_E(FS(_T("PropName greater than %d characters at line %d -- skipping script"), PROPNAME_BUFFER_COUNT, lineCounter));
								delete pScript;
								return NULL;
							}
							pos++;
						}
						*propNamePos = 0;

						// skip whitespace
						while (*pos && *pos==_T(' ') || *pos==_T('\t')) pos++;

						TCHAR propValue[PROPVALUE_BUFFER_COUNT];
						TCHAR* propValuePos = propValue;
						while (*pos) 
						{
							*propValuePos = *pos;
							propValuePos++;
							if (propValuePos==propValue+PROPVALUE_BUFFER_COUNT-1)
							{
								TRACE_E(FS(_T("PropValue greater than %d characters at line %d -- skipping script"), PROPVALUE_BUFFER_COUNT, lineCounter));
								delete pScript;
								return NULL;
							}
							pos++;
						}
						*propValuePos = 0;

						pScript->AddProperty(propName, propValue);
					}
				}
			}
			break;
		case 2:
			// code is taken always
			break;
		}
	}

	TTrace::Debug()->UnIndent();

	// compact string buffers
	pScript->Code().FreeExtra();
	pScript->Comment().FreeExtra();

	// prepare guid if there is no specified
	if (pScript->Guid()==CGuid::m_Null)
	{
		pScript->GuessGuid();
	}
	return pScript;
}

bool                                           
CScriptManager::AddScript(CScript* pScript)
{
	// register newly created script into manager
	HandleManagerLock handleManager;
	TScriptId hScript = handleManager->GetHandle(pScript->Guid());
	iterator test = find(hScript);
	if (test!=end())
	{
		TRACE_E(FS(_T("Duplicit key found: %s AND %s"), test->second->FormTraceName(), pScript->FormTraceName()));
		return false;
	}
	pair<iterator, bool> res = insert(make_pair(hScript, pScript));
	return true;
}

bool                                           
CScriptManager::LoadUserSettings()
{
	CHECK_THREAD_OWNERSHIP;
	DatabaseLock db;
	CSqlStatement* statement = db->Statement(_T("SELECT ScriptGUID, URL FROM DisabledScripts ORDER BY ScriptGUID"));
	if (!statement) return false;
	statement->Execute();	

	CGuid guid;
	CScript* script = NULL; 
	while (statement->NextRow())
	{
		guid = statement->ValueString(0);
		LPCTSTR url = statement->ValueString(1);
		if (script && script->Guid()!=guid)
		{
			HandleManagerLock handleManager;
			THandle handle = handleManager->GetHandle(guid);
			script = FindScript(handle);
		}
		if (script)
		{
			script->Disables().push_back(url);
		}
	}
	delete statement;
	return true;
}

bool                                           
CScriptManager::SaveUserSettings()
{
	CHECK_THREAD_OWNERSHIP;
	DatabaseLock db;
	db->DirectStatement(_T("DELETE FROM DisabledScripts"));
	iterator i = begin();
	while (i!=end())
	{
		CScript* script = i->second;
		CDisableList::iterator j = script->Disables().begin();
		while (j!=script->Disables().end())
		{
			CSqlStatement* statement = db->Statement(_T("INSERT INTO DisabledScripts (ScriptGUID, URL) VALUES (?, ?)"));
			CString guidStr = script->Guid();
			statement->Bind(0, guidStr);
			statement->Bind(1, *j);
			statement->Execute();
			statement->Reset();
			delete statement;
			++j;
		}
		++i;
	}
	return true;
}

CScript*                                       
CScriptManager::FindScript(TScriptId hScript)
{
	CHECK_THREAD_OWNERSHIP;
	iterator l = find(hScript);
	if (l==end()) return NULL;
	return l->second;
}

bool                                           
CScriptManager::SetScriptValue(CGuid guid, CString key, CString value)
{
	CHECK_THREAD_OWNERSHIP;
	DatabaseLock db;
	CString guidStr = guid;
	CSqlStatement* statement;
	statement = db->Statement(_T("DELETE FROM ScriptValues WHERE ScriptGUID=? AND Key=?"));
	statement->Bind(0, guidStr);
	statement->Bind(1, key);
	statement->Execute();
	delete statement;

	statement = db->Statement(_T("INSERT INTO ScriptValues (ScriptGUID, Key, Value) VALUES (?, ?, ?)"));
	statement->Bind(0, guidStr);
	statement->Bind(1, key);
	statement->Bind(2, value);
	statement->Execute();
	delete statement;

	return true;
}

bool                                           
CScriptManager::GetScriptValue(CGuid guid, CString key, CString& value)
{
	CHECK_THREAD_OWNERSHIP;
	DatabaseLock db;
	CString guidStr = guid;
	CSqlStatement* statement = db->Statement(_T("SELECT Value FROM ScriptValues WHERE ScriptGUID=? AND Key=?"));
	if (!statement) return false;
	statement->Bind(0, guidStr);
	statement->Bind(1, key);
	if (!statement->NextRow()) 
	{
		delete statement;
		return false;	
	}
	value = statement->ValueString(0);
	delete statement;
	return true;
}
