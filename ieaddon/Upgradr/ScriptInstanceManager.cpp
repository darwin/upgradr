#include "StdAfx.h"
#include "ScriptInstanceManager.h"

//#include "Debug.h"

CScriptInstanceManager::CScriptInstanceManager():
m_LastScriptInstanceId(0)
{
}

CScriptInstanceManager::~CScriptInstanceManager()
{
}

CScriptInstance* 
CScriptInstanceManager::FindScriptInstance(TScriptInstanceId id)
{
	CHECK_THREAD_OWNERSHIP;
	iterator i = find(id);
	if (i==end()) return NULL;
	return i->second;
}

TScriptInstanceId 
CScriptInstanceManager::InsertScriptInstance(CScriptInstance instance)
{
	CHECK_THREAD_OWNERSHIP;
	TScriptInstanceId newId = ++m_LastScriptInstanceId;
	ATLASSERT(find(newId)==end());
	CScriptInstance* pInstance = new CScriptInstance(instance);
	insert(make_pair(newId, pInstance));
	return newId;
}

bool 
CScriptInstanceManager::RemoveScriptInstance(TScriptInstanceId id)
{
	CHECK_THREAD_OWNERSHIP;
	iterator i = find(id);
	if (i==end()) return false;
	delete i->second;
	erase(i);
	return true;
}