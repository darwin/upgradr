#include "StdAfx.h"
#include "Script.h"

CScript::CScript()
{
}

CScript::~CScript()
{
}

bool                                           
CScript::AddProperty(LPCTSTR name, LPCTSTR value)
{
	if (_tcscmp(name, _T("name"))==0)
	{
		m_Name = value;
		return true;
	}
	if (_tcscmp(name, _T("namespace"))==0)
	{
		m_Namespace = value;
		return true;
	}
	if (_tcscmp(name, _T("description"))==0)
	{
		m_Description = value;
		return true;
	}
	if (_tcscmp(name, _T("guid"))==0)
	{
		m_Guid = value;
		return true;
	}
	if (_tcscmp(name, _T("include"))==0)
	{
		if (!value[0]) return true;
		m_Includes.push_back(value);
		return true;
	}
	if (_tcscmp(name, _T("exclude"))==0)
	{
		if (!value[0]) return true;
		m_Excludes.push_back(value);
		return true;
	}
	return false;
}

bool 
CScript::GuessGuid()
{
	CGuid::TGUID* pGuid = m_Guid;
	pGuid->Data1 = 0;
	pGuid->Data2 = 0;
	pGuid->Data3 = 0;
	pGuid->Data4 = 0;
	pGuid->Data5 = 0;
	*(Fnv64_t*)pGuid = fnv_64_buf((char*)m_Code.LockBuffer(), m_Code.GetLength()*sizeof(TCHAR), FNV1_64A_INIT);
   *((Fnv64_t*)pGuid+1) = ~(*(Fnv64_t*)pGuid);
	m_Code.UnlockBuffer();
	TRACE_I(FS(_T("Generated GUID=%s for script %s"), (CString)m_Guid, m_Name));
	return true;
}

CString 
CScript::FormTraceName()
{
	CString s;
	s.Format(_T("%s %s"), m_Name, (CString)m_Guid);
	return s;
}

void                                           
CScript::DetectCategory()
{
	m_Category = EC_USER;
	// test well known script guid against script guid
	if (m_Guid==CGuid(SCRIPT_GUID_XPATH)) { m_Category = EC_SYSTEM; return; }
	if (m_Guid==CGuid(SCRIPT_GUID_IE2W3C)) { m_Category = EC_SYSTEM; return; }
	if (m_Guid==CGuid(SCRIPT_GUID_MONKEYAPI)) { m_Category = EC_SYSTEM; return; }
}

bool 
CScript::SetDirectoryHandle(CDirectoryHandle directoryHandle)
{
	ATLASSERT(directoryHandle!=NULL_DIRECTORY);
	m_DirectoryHandle = directoryHandle;
	return true;
}