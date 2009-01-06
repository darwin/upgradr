/**
 ** OptionSheetSettingsDemo
 **
 ** This is part of the demo app which shows the usage of:
 **		- Mortimer::COptionSheet(Settings)
 **		- Mortimer::CSettings
 **
 **/

#pragma once

#include "resource.h"
#include "RootSettings.h"
#include <MOptionPage.h>

class CSettingsGeneralOptionPage : public Mortimer::COptionPageSettingsImpl<CSettingsGeneralOptionPage, CSettingsGeneral>
                                  ,public WTL::CWinDataExchange<CSettingsGeneralOptionPage>
{
public:
    enum { IDD = IDD_OPTIONPAGE_GENERAL };

	DECLARE_EMPTY_MSG_MAP()

	BEGIN_DDX_MAP(CSettingsGeneralOptionPage)
		DDX_CHECKB(IDC_CHECK_DISABLEMSGBOX, m_pSettings->DisableMsgBox)
		DDX_CHECKB(IDC_CHECK_BACKUPCUESHEET, m_pSettings->BackupCueSheetFile)
		DDX_CHECKB(IDC_CHECK_NAMESFROMFREEDB, m_pSettings->UpdateNamesFromSearchResults)
		DDX_RADIO(IDC_RADIO_NAMES_1, m_pSettings->ExtractNamesFrom_Index)
	END_DDX_MAP()
};
