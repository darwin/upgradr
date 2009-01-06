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

class CSettingsSearchOptionPage : public Mortimer::COptionPageSettingsImpl<CSettingsSearchOptionPage, CSettingsSearch>
                                 ,public WTL::CWinDataExchange<CSettingsSearchOptionPage>
{
public:
    enum { IDD = IDD_OPTIONPAGE_SEARCH };

	DECLARE_EMPTY_MSG_MAP()

	BEGIN_DDX_MAP(CSettingsSearchOptionPage)
		DDX_CHECK_FLAGS(IDC_CHECK_SEARCH_ARTIST, m_pSettings->SearchFields, CSettingsSearch::fieldArtist)
		DDX_CHECK_FLAGS(IDC_CHECK_SEARCH_TITLE, m_pSettings->SearchFields, CSettingsSearch::fieldTitle)
		DDX_CHECK_FLAGS(IDC_CHECK_EXCLUDE_ARTIST, m_pSettings->ExcludeFields, CSettingsSearch::fieldArtist)
		DDX_CHECK_FLAGS(IDC_CHECK_EXCLUDE_TITLE, m_pSettings->ExcludeFields, CSettingsSearch::fieldTitle)
		DDX_CHECKB(IDC_CHECK_LIMITRESULTS, m_pSettings->LimitResults)
		DDX_UINT_RANGE(IDC_EDIT_ARTIST_THRESHOLD, m_pSettings->ArtistSimilarThreshold, 0, 100)
		DDX_UINT_RANGE(IDC_EDIT_TITLE_THRESHOLD, m_pSettings->TitleSimilarThreshold, 0, 100)
		DDX_UINT_RANGE(IDC_EDIT_LIMITRESULTS_THRESHOLD, m_pSettings->LimitResultsThreshold, 0, 100)
	END_DDX_MAP()
};
