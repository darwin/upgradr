#include "StdAfx.h"
#include "Database.h"

//#include "Debug.h"

#define SQL_CREATE_DISABLEDSCRIPTS_TABLE _T("\
CREATE TABLE IF NOT EXISTS [DisabledScripts] (\
	[ID] INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\
   [ScriptGUID] VARCHAR(60) NULL,\
   [URL] TEXT NOT NULL\
)")

#define SQL_CREATE_SCRIPTVALUES_TABLE _T("\
CREATE TABLE IF NOT EXISTS [ScriptValues] (\
	[ID] INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\
   [ScriptGUID] VARCHAR(60) NULL,\
   [Key] TEXT NOT NULL,\
   [Value] TEXT\
)")

CDatabase::CDatabase()
{

}

CDatabase::~CDatabase()
{

}

bool 
CDatabase::Init()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_Inited) return true;

	DT(TRACE_I(_T("CUpgradrDB::Init()")));
	ATLASSERT(!m_db);
	TCHAR path[_MAX_PATH];
	DWORD size = _MAX_PATH;
	if (GetStringValueFromRegistry(REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_DATABASEPATH, path, &size))
	{
		TRACE_E(FS(_T("Cannot read database location from HKCU\\%s%s\\%s"), REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_DATABASEPATH));
		// FATAL
		return false;
	}
	if (!Open(path))
	{
		TRACE_E(FS(_T("Cannot open database at location '%s'"), path));
		return false;
	}

	CheckDBSchema();
	m_Inited = true;
	return true;
}

bool 
CDatabase::Done()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return true;
	DT(TRACE_I(_T("CUpgradrDB::Done()")));
	Close();
	m_Inited = false;
	return true;
}

bool                                           
CDatabase::CheckDBSchema()
{
	CHECK_THREAD_OWNERSHIP;
	DT(TRACE_I(_T("CUpgradrDB::CheckDBSchema()")));
	DirectStatement(SQL_CREATE_DISABLEDSCRIPTS_TABLE);
	DirectStatement(SQL_CREATE_SCRIPTVALUES_TABLE);
	return true;
}
