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

class CSettingsfreedbOptionPage : public Mortimer::COptionPageSettingsImpl<CSettingsfreedbOptionPage, CSettingsfreedb>
                                 ,public WTL::CWinDataExchange<CSettingsfreedbOptionPage>
{
public:
    enum { IDD = IDD_OPTIONPAGE_FREEDB };

	DECLARE_EMPTY_MSG_MAP()

	BEGIN_DDX_MAP(CSettingsfreedbOptionPage)
		DDX_TEXT(IDC_EDIT_HOST, m_pSettings->Host)
		DDX_CHECKB(IDC_CHECK_USEPROXY, m_pSettings->UseProxy)
		if (m_pSettings->UseProxy) {
			DDX_UINT_RANGE(IDC_EDIT_PORT, m_pSettings->Port, 0, 65535) }
		DDX_TEXT(IDC_EDIT_SEARCHSERVER, m_pSettings->SearchSite)
		DDX_TEXT(IDC_EDIT_FREEDBSERVER, m_pSettings->CDDBSite)
		DDX_TEXT(IDC_EDIT_USERNAME, m_pSettings->UserName)
		DDX_TEXT(IDC_EDIT_PASSWORD, m_pSettings->PassWord)
	END_DDX_MAP()
};
