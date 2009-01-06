#include "StdAfx.h"
#include "Services.h"
#include "MSettingsStorage.h"
#include "CSNOptionSheet.h"
#include "MOptionSheetLayoutSimple.h"
#include "MSettingsStorageRegistry.h"

CServices::CServices()
{
}

CServices::~CServices()
{
}

bool                                           
CServices::OpenSettingsDialog()
{
	CHECK_THREAD_OWNERSHIP;
	Mortimer::COptionSheetLayoutImpl<Mortimer::COptionSelectionLayoutTreeBanner, CCSNOptionSheet> Sheet(_T("Upgradr Settings"));
 
	// retrieve root key for registry
	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_ROOT_KEY, 0, KEY_ALL_ACCESS, &hKey);
	if (lRes != ERROR_SUCCESS)	
	{
		if (RegCreateKey(HKEY_CURRENT_USER, REGISTRY_ROOT_KEY, &hKey))
		{
			TRACE_E(_T("Cannot create registry key: HKCU\\") REGISTRY_ROOT_KEY);
			return false;
		}
	}

	// create storage
	Mortimer::CSettingsStorageRegistry storage(hKey, REGISTRY_OPTIONS_KEY);

	// load settings from storage
	m_Settings.Load(storage);

	// fire options dialog
	Sheet.SetManagedSettings(&m_Settings, &storage);
	if (IDOK!=Sheet.DoModal()) return true;

	// if OK pressed, save options into storage
	m_Settings.Save(storage);
	return true;
}
