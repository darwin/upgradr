/**
 ** OptionSheetSettingsDemo
 **
 ** This is part of the demo app which shows the usage of:
 **		- Mortimer::COptionSheet(Settings)
 **		- Mortimer::CSettings
 **
 **/

#ifndef __OPTIONSHEETSETTINGSDEMO_ROOTSETTINGS_H__
#define __OPTIONSHEETSETTINGSDEMO_ROOTSETTINGS_H__

#include "MSettingsStorage.h"
using Mortimer::CSettings;
using Mortimer::CSettingsStorage;

//------------------------------------------------------------------------
// For the 'General' page
class CSettingsGeneral : public CSettings
{
public:
	bool DisableMsgBox;
	bool BackupCueSheetFile;
	bool UpdateNamesFromSearchResults;
	int ExtractNamesFrom_Index;

	// Not visible settings
	CString LastCuePath;
	int PreviewHeight;

	BEGIN_SETTING_MAP(CSettingsGeneral)
		SETTING_ITEM_DEFAULT(DisableMsgBox, true)
		SETTING_ITEM_DEFAULT(BackupCueSheetFile, true)
		SETTING_ITEM_DEFAULT(UpdateNamesFromSearchResults, false)
		SETTING_ITEM_DEFAULT(ExtractNamesFrom_Index, 0)
		SETTING_ITEM(LastCuePath)
		SETTING_ITEM_DEFAULT(PreviewHeight, 0)
	END_SETTING_MAP()
};

//------------------------------------------------------------------------
// For the 'Search' page
class CSettingsSearch : public CSettings
{
public:
	bool LimitResults;
	int LimitResultsThreshold;

	CString Artist;
	CString Title;
	int ArtistSimilarThreshold;
	int TitleSimilarThreshold;
	UINT SearchFields;
	UINT ExcludeFields;

	enum
	{
		fieldArtist = 1,
		fieldTitle = 2,
		/*fieldXXX = 4, */
	};

	BEGIN_SETTING_MAP(CSettingsSearch)
		SETTING_ITEM_DEFAULT(LimitResults, true)
		SETTING_ITEM_DEFAULT(LimitResultsThreshold, 20)
		SETTING_ITEM(Artist)
		SETTING_ITEM(Title)
		SETTING_ITEM_DEFAULT(ArtistSimilarThreshold, 75)
		SETTING_ITEM_DEFAULT(TitleSimilarThreshold, 75)
		SETTING_ITEM_DEFAULT(SearchFields, fieldArtist)
		SETTING_ITEM_DEFAULT(ExcludeFields, 0)
	END_SETTING_MAP()
};

//------------------------------------------------------------------------
// For the 'freedb' page

class CSettingsfreedb : public CSettings
{
public:
	// Both server
	CString SearchSite;
	CString CDDBSite;

	// Proxy details
	bool UseProxy;
	CString Host;
	int Port;
	CString UserName;
	CString PassWord;

	BEGIN_SETTING_MAP(CSettingsfreedb)
		SETTING_ITEM_DEFAULT(SearchSite, "www.freedb.org")
		SETTING_ITEM_DEFAULT(CDDBSite, "freedb.freedb.org")
		SETTING_ITEM_DEFAULT(UseProxy, false)
		SETTING_ITEM(Host)
		SETTING_ITEM_DEFAULT(Port, 80)
		SETTING_ITEM(UserName)
		SETTING_ITEM(PassWord)
	END_SETTING_MAP()
};

//------------------------------------------------------------------------
// The global settings class which references all others
class CRootSettings : public CSettings
{
public:
	CSettingsGeneral General;
	CSettingsSearch Search;
	CSettingsfreedb freedb;

	BEGIN_SETTING_MAP(CRootSettings)
		SETTING_ITEM_SUBITEM(General)
		SETTING_ITEM_SUBITEM(Search)
		SETTING_ITEM_SUBITEM(freedb)
	END_SETTING_MAP()
};


#endif // __OPTIONSHEETSETTINGSDEMO_ROOTSETTINGS_H__
