//========================================================================================
//
// Module:			OptionSheet
// Author:          Pascal Hurni
// Creation Date:	20.11.2003
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
//========================================================================================

#ifndef __MORTIMER_OPTIONSHEETSETTINGS_H__
#define __MORTIMER_OPTIONSHEETSETTINGS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// CSettings and CSettingsStorage must be defined before including this file
// When using Mortimer CSettings, the Mortimer namespace must be implicitely used.
// (issue a 'using Mortimer::CSettings' or a 'using namespace Mortimer')

#include <MOptionSheet.h>

//========================================================================================
// encapsulate these classes in a namespace

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

//========================================================================================
// Managed OptionSheet means that the settings are managed by the sheet. The buttons in the
// sheet will automatically act as they should (eg: the Save button WILL save the settings
// to the storage).

// same CItem class for all implemented COptionSheetSettings templates
class COptionItemSettings : virtual public COptionItem
{
public:
	COptionItemSettings(LPCTSTR Caption, COptionPage *pPage, CSettings *pSettings)
		: COptionItem(Caption, pPage), m_pSettings(pSettings) {}

	void SetSettings(CSettings *pSettings) { m_pSettings = pSettings; }
	CSettings* GetSettings() { return m_pSettings; }

protected:
	CSettings *m_pSettings;
};

// Specialized sheet class to handle TSettings (which must be a CSettings subclass)
template <class TSettings, class TOptionSheet = COptionSheet>
class COptionSheetSettings : public TOptionSheet
{
public:
	COptionSheetSettings() : m_pOriginalSettings(NULL), m_pStorage(NULL) {}

	// Call it before DoModal() or Create()
	// To enable correct useness of managed mode, your TSettings class MUST have a valid
	// assignement operator (operator=). This is because a copy of the settings is made internally.
	void SetManagedSettings(TSettings *pSettings, CSettingsStorage *pStorage)
	{
		ATLASSERT(pSettings != NULL);
		ATLASSERT(pStorage != NULL);

		m_pOriginalSettings = pSettings;
		m_pStorage = pStorage;

		// Create a copy of these settings
		m_Settings = *m_pOriginalSettings;
	}

protected:
	virtual void OnOK/*OnSave*/(UINT uCode, int nID, HWND hWndCtl)
	{
		TOptionSheet::OnOK(uCode, nID, hWndCtl);

		// Well managed ?
		if (m_pOriginalSettings && m_pStorage)
		{
			// Copy back and save
			*m_pOriginalSettings = m_Settings;
			m_pOriginalSettings->Save(*m_pStorage);
		}
	}

	virtual void OnUse(UINT uCode, int nID, HWND hWndCtl)
	{
		TOptionSheet::OnUse(uCode, nID, hWndCtl);

		// Well managed ?
		if (m_pOriginalSettings && m_pStorage)
		{
			// Copy back
			*m_pOriginalSettings = m_Settings;
		}
	}

	virtual void OnReload(UINT uCode, int nID, HWND hWndCtl)
	{
		TOptionSheet::OnReload(uCode, nID, hWndCtl);

		// Well managed ?
		if (m_pStorage)
		{
			// Tell current page to disappear
			SetActiveItem(NULL);

			// Clears the items
			m_pSelection->DeleteAllItems();

			// Reload original settings (directly in our copy)
			m_Settings.Load(*m_pStorage);

			// Re-init all the pages
			DoInit(false);
		}
	}

	virtual void OnApply(UINT uCode, int nID, HWND hWndCtl)
	{
		TOptionSheet::OnApply(uCode, nID, hWndCtl);

		// Well managed ?
		if (m_pOriginalSettings && m_pStorage)
		{
			// Copy back (do NOT save)
			*m_pOriginalSettings = m_Settings;
		}
	}

protected:
	TSettings m_Settings;
	TSettings *m_pOriginalSettings;
	CSettingsStorage *m_pStorage;
};

//========================================================================================

}; // namespace Mortimer

#endif // __MORTIMER_OPTIONSHEETSETINGS_H__
