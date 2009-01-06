//========================================================================================
//
// Module:			SettingsStorage
// Author:          Pascal Hurni
// Creation Date:	20.12.2003
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
// This work is based on CRegSettings, Copyright (C) 2002 Magomed G. Abdurakhmanov, maq@mail.ru
//
// Modifications:
//
//	2004-05-11	By Pascal Hurni
//
//				Removed Collection related functions and subitem function.
//				Standard ones relies now in the base class CSettingsStorage.
//
//	2004-04-19	By Pascal Hurni
//
//				SaveLoadItem() for TCHAR updated to follow spec of CSettingsStorage.
//
//========================================================================================

#ifndef __MORTIMER_SETTINGSSTORAGEREGISTRY_H__
#define __MORTIMER_SETTINGSSTORAGEREGISTRY_H__

#include "MSettingsStorage.h"

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

/// CSettingsStorage implementation for the registry
class CSettingsStorageRegistry : public CSettingsStorage
{
protected:
	HKEY m_hKey;
	HKEY m_hKeyParent;
	TCHAR m_sKey[MAX_PATH];

public:
	CSettingsStorageRegistry() : m_hKey(NULL), m_hKeyParent(NULL)
	{
		m_sKey[0] = 0;
	}

	/// Constructor. You can already define which registry key to use. See SetBaseKeyName() for the parameters.
	CSettingsStorageRegistry(HKEY  hKeyParent, LPCTSTR KeyName)
	{
		SetBaseKeyName(hKeyParent, KeyName);
	}

	CSettingsStorage *CreateSubStorage(LPCTSTR SubName)
	{
		return new CSettingsStorageRegistry(m_hKey, SubName);
	}

	/// Defines under which registry key the settings are stored.
	/// Call it before any calls to CSettings::Load() or CSettings::Save().
	/// @param hKeyParent A valid HKEY. Generally a root key like HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE.
	/// @param KeyName A key name related to hKeyParent. Root items of settings will be stored under this key.
	void SetBaseKeyName(HKEY hKeyParent, LPCTSTR KeyName)
	{
		m_hKeyParent = hKeyParent;
		_tcsncpy_s(m_sKey, KeyName, MAX_PATH);
	}

	bool OnBeforeLoad()
	{
		return ERROR_SUCCESS == ::RegOpenKeyEx(m_hKeyParent, m_sKey, 0, KEY_READ, &m_hKey);
	}

	bool OnAfterLoad()
	{
		DWORD dwResult = ERROR_SUCCESS;
		if (m_hKey) dwResult = ::RegCloseKey(m_hKey);
		m_hKey = NULL;
		return ERROR_SUCCESS == dwResult;
	}

	bool OnBeforeSave()
	{
		return ERROR_SUCCESS == ::RegCreateKeyEx(m_hKeyParent, m_sKey, NULL, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &m_hKey, NULL);
	}

	bool OnAfterSave()
	{
		DWORD dwResult = ERROR_SUCCESS;
		if (m_hKey) dwResult = ::RegCloseKey(m_hKey);
		m_hKey = NULL;
		return ERROR_SUCCESS == dwResult;
	}

	bool ContinueOnError()
	{
		// We want the system to read/write the most possible items, even if one fails
		// (There's no serialization issue with the registry)
		return true;
	}

public:
	//------------------------------------------------------------------------------------
	// Here are the type specific SaveLoad functions

	// unsigned long (DWORD)
	inline bool SaveLoadItem(LPCTSTR szName, unsigned long& Variable, bool bSave)
	{
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		if(bSave)
			return ERROR_SUCCESS == ::RegSetValueEx(m_hKey, szName, NULL, dwType, (const BYTE*)&Variable, dwSize);
		else
			return ERROR_SUCCESS == ::RegQueryValueEx(m_hKey, szName, NULL, &dwType, (LPBYTE)&Variable, &dwSize);
	}
	inline bool SaveLoadItem(LPCTSTR szName, long& Variable, bool bSave)
	{
		return SaveLoadItem(szName, (unsigned long&)Variable, bSave);
	}

	// bool
	inline bool SaveLoadItem(LPCTSTR szName, bool& Variable, bool bSave)
	{
		DWORD dwValue = Variable ? 1 : 0;
		bool bResult = SaveLoadItem(szName, dwValue, bSave);
		if (!bSave && bResult)
			Variable = dwValue != 0;
		return bResult;
	}

	// float
	inline bool SaveLoadItem(LPCTSTR szName, float& Variable, bool bSave)
	{
		return SaveLoadItem(szName, (void*)&Variable, sizeof(float), bSave);
	}

	// double
	inline bool SaveLoadItem(LPCTSTR szName, double& Variable, bool bSave)
	{
		return SaveLoadItem(szName, (void*)&Variable, sizeof(double), bSave);
	}

	// TCHAR*
	inline bool SaveLoadItem(LPCTSTR szName, TCHAR* Variable, ULONG& Size, bool bSave)
	{
		DWORD dwType = REG_SZ;
		if(bSave)
			return ERROR_SUCCESS == ::RegSetValueEx(m_hKey, szName, NULL, dwType, (LPBYTE)Variable, Size*sizeof(TCHAR));
		else
		{
			DWORD ByteSize = Size*sizeof(TCHAR);
			DWORD ErrorCode = ::RegQueryValueEx(m_hKey, szName, NULL, &dwType, (LPBYTE)Variable, &ByteSize);
			Size = ByteSize/sizeof(TCHAR);
			if (ErrorCode == ERROR_SUCCESS)
				return true;
			if (ErrorCode == ERROR_MORE_DATA)
				return false;
			Size = 0;
			return false;
		}
	}

	// void*
	inline bool SaveLoadItem(LPCTSTR szName, void* Variable, ULONG size, bool bSave)
	{
		DWORD dwType = REG_BINARY;
		DWORD dwSize = (DWORD)size;
		if(bSave)
			return ERROR_SUCCESS == ::RegSetValueEx(m_hKey, szName, NULL, dwType, (LPBYTE)Variable, dwSize);
		else
			return ERROR_SUCCESS == ::RegQueryValueEx(m_hKey, szName, NULL, &dwType, (LPBYTE)Variable, &dwSize);
	}

}; // class CSettingsStorageRegistry


}; // namespace Mortimer

#endif // __MORTIMER_SETTINGSSTORAGEREGISTRY_H__
