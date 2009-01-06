#include "StdAfx.h"
#include "HandleManager.h"

//#include "Debug.h"

CHandleManager::CHandleManager():
m_LastHandle(0)
{
}

CHandleManager::~CHandleManager()
{
}

THandle                                        
CHandleManager::GetHandle(CGuid guid)
{
	CHECK_THREAD_OWNERSHIP;
	DT(TRACE_I(FS(_T("CHandleManager::GetHandle(%s)"), (LPCTSTR)guid)));
	iterator f = find(guid);
	if (f!=end()) return f->second;
	if (!m_FreeHandles.size()) 
	{
		m_LastHandle++;
		insert(make_pair(guid, m_LastHandle));
		m_ReverseMap.insert(make_pair(m_LastHandle, guid));
		return m_LastHandle;
	}
	THandle res = m_FreeHandles.top();
	m_FreeHandles.pop();
	insert(make_pair(guid, res));
	m_ReverseMap.insert(make_pair(res, guid));
	return res;
}

bool                                           
CHandleManager::FreeHandle(CGuid guid)
{
	CHECK_THREAD_OWNERSHIP;
	DT(TRACE_I(FS(_T("CHandleManager::FreeHandle(%s)"), (LPCTSTR)guid)));
	iterator f = find(guid);
	if (f==end()) return false;
	m_FreeHandles.push(f->second);
	erase(f);
	m_ReverseMap.erase(f->second);
	return true;
}

CGuid                                          
CHandleManager::LookupGUID(THandle handle) const
{
	CHECK_THREAD_OWNERSHIP;
	DT(TRACE_I(FS(_T("CHandleManager::LookupGUID(%08X)"), handle)));
	THandleGUIDMap::const_iterator i = m_ReverseMap.find(handle);
	if (i==m_ReverseMap.end()) return CGuid();
	return i->second;
}
