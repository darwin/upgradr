//========================================================================================
//
// Module:			SettingsStorage
// Author:          Pascal Hurni
// Creation Date:	26.12.2003
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
// Modifications:
//
//	2004-10-07	By Pascal Hurni
//
//				SaveLoadItem() for bool was bugged. TCHAR sizeof with wchar_t
//				has been changed to get the real character count, not the byte size.
//
//	2004-05-25	By Pascal Hurni
//
//				SaveLoadItem() for bool was bugged. The return value was the data
//				and so the Variable parameter was never updated on load.
//
//	2004-05-11	By Pascal Hurni
//
//				Removed Collection related functions and subitem function.
//				Standard ones relies now in the base class CSettingsStorage.
//
//	2004-04-19	By Pascal Hurni
//
//				FIXED BUG: SaveLoadItem() for TCHAR. Bug fix was bugged!
//
//	2004-04-15	By Pascal Hurni
//
//				FIXED BUG: SaveLoadItem() for TCHAR would return an empty string with
//				success as return value when not founding the entry.
//				Returns now failure as expected. Thanx to Dezhi Zhao for the bug report.
//
//========================================================================================

#ifndef __MORTIMER_SETTINGSSTORAGEINIFILE_H__
#define __MORTIMER_SETTINGSSTORAGEINIFILE_H__

#include "MSettingsStorage.h"

// automatic BOOL to bool is ok for me
#pragma warning(disable: 4800)

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

/// CSettingsStorage implementation for INI files
class CSettingsStorageIniFile : public CSettingsStorage
{
protected:
	TCHAR m_IniFileName[MAX_PATH];
	TCHAR m_SectionName[MAX_PATH];

public:
	CSettingsStorageIniFile()
	{
		m_IniFileName[0] = 0;
		m_SectionName[0] = 0;
	}

	/// Constructor. You can already define which INI file to use. See SetIniFileName() for the parameters.
	CSettingsStorageIniFile(LPCTSTR IniName, LPCTSTR SectionName)
	{
		m_IniFileName[0] = 0;
		m_SectionName[0] = 0;
		SetIniFileName(IniName, SectionName);
	}

	CSettingsStorage *CreateSubStorage(LPCTSTR SubName)
	{
		TCHAR SubSectionName[MAX_PATH];
		_stprintf_s(SubSectionName, _T("%s.%s"), m_SectionName, SubName);
		return new CSettingsStorageIniFile(m_IniFileName, SubSectionName);
	}

	/// Defines the INI Filename related to this storage object.
	/// Call it before any calls to CSettings::Load() or CSettings::Save().
	/// @param IniName The INI complete Pathname.
	/// @param SectionName The root section name. Root items of the settings will appear under this section.
	void SetIniFileName(LPCTSTR IniName, LPCTSTR SectionName)
	{
		if (IniName) _tcsncpy_s(m_IniFileName, IniName, MAX_PATH);
		if (SectionName) _tcsncpy_s(m_SectionName, SectionName, MAX_PATH);
	}


	bool ContinueOnError()
	{
		// We want the system to read/write the most possible items, even if one fails
		return true;
	}

public:
	//------------------------------------------------------------------------------------
	// Here are the type specific SaveLoad functions

	// Handy macro used for all simple types
	#define SSIF_SAVELOADITEM(type, token) inline bool SaveLoadItem(LPCTSTR szName, type& Variable, bool bSave) \
	{	TCHAR Value[MAX_PATH]; \
		if (bSave) { \
			_stprintf_s(Value, _T(token), Variable); \
			return WritePrivateProfileString(m_SectionName, szName, Value, m_IniFileName); \
		} else { \
			TCHAR Null = 0; \
			if (0 == GetPrivateProfileString(m_SectionName, szName, &Null, Value, MAX_PATH, m_IniFileName)) return false; \
			return 1 == _stscanf_s(Value, _T(token), &Variable); \
	}	}

	SSIF_SAVELOADITEM(long, "%d")
	SSIF_SAVELOADITEM(unsigned long, "%u")

	SSIF_SAVELOADITEM(float, "%f")
	SSIF_SAVELOADITEM(double, "%lf")

	// bool
	inline bool SaveLoadItem(LPCTSTR szName, bool& Variable, bool bSave)
	{
		if (bSave)
			return WritePrivateProfileString(m_SectionName, szName, Variable ? _T("TRUE") : _T("FALSE"), m_IniFileName);
		else
		{
			TCHAR Value[16];
			ULONG Size = sizeof(Value)/sizeof(Value[0]);
			if (!SaveLoadItem(szName, Value, Size, bSave)) return false;
			Variable = _tcscmp(Value, _T("TRUE")) == 0;
			return true;
		}
	}

	// TCHAR*
	inline bool SaveLoadItem(LPCTSTR szName, TCHAR* Variable, ULONG& Size, bool bSave)
	{
		if (bSave)
			return WritePrivateProfileString(m_SectionName, szName, Variable, m_IniFileName);
		else
		{
			const ULONG MAX_SIZE = __max(1024, Size);
			TCHAR *pTemp = (TCHAR*)_alloca(MAX_SIZE*sizeof(TCHAR));
			TCHAR Magic[4] = { 0xFFu, 0x7Fu, 0x08u, 0x00u };	// This string 'should' never be read from an ini file
			ULONG CurrentSize = GetPrivateProfileString(m_SectionName, szName, Magic, pTemp, MAX_SIZE, m_IniFileName);
			if (CurrentSize == MAX_SIZE-1)
			{
				Size *= 2; // Completely arbitrary, but 'should' be enough
				return false;
			}
			if (_tcscmp(pTemp, Magic) == 0)
			{
				Size = 0;
				return false;
			}
			if (Variable == NULL)
			{
				Size = CurrentSize+1;
				return true;
			}
			if (CurrentSize+1 > Size)
			{
				Size = CurrentSize+1;
				return false;
			}

			_tcsncpy_s(Variable, MAX_SIZE, pTemp, Size);
			return true;
		}
	}

	// void*
	inline bool SaveLoadItem(LPCTSTR szName, void* Variable, ULONG size, bool bSave)
	{
		if (bSave)
			return WritePrivateProfileStruct(m_SectionName, szName, Variable, size, m_IniFileName);
		else
			return GetPrivateProfileStruct(m_SectionName, szName, Variable, size, m_IniFileName);
	}

}; // class CSettingsStorageIniFile


}; // namespace Mortimer

#endif // __MORTIMER_SETTINGSSTORAGEINIFILE_H__
