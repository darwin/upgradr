/**
 ** OptionSheetSettingsDemo
 **
 ** This is part of the demo app which shows the usage of:
 **		- Mortimer::COptionSheet(Settings)
 **		- Mortimer::CSettings
 **
 **/

#include <MOptionSheetSettings.h>

#include "SettingsGeneralOptionPage.h"
#include "SettingsSearchOptionPage.h"
#include "SettingsfreedbOptionPage.h"

class CCSNOptionSheet : public Mortimer::COptionSheetSettings<CRootSettings>
{
public:
	bool DoInit(bool FirstTime)
	{
		if (FirstTime)
		{
			CIcon icon;
			icon.LoadIcon(MAKEINTRESOURCE(IDI_OPTIONSHEET));
			SetIcon(icon);

			m_GeneralOptionPage.Create(this);
			m_SearchOptionPage.Create(this);
			m_freedbOptionPage.Create(this);
		}

		AddItem(new Mortimer::COptionItemSettings(NULL, &m_GeneralOptionPage, &m_Settings.General));
		AddItem(new Mortimer::COptionItemSettings(NULL, &m_SearchOptionPage, &m_Settings.Search));
		AddItem(new Mortimer::COptionItemSettings(NULL, &m_freedbOptionPage, &m_Settings.freedb));

		return COptionSheet::DoInit(FirstTime);
	}

protected:
	CSettingsGeneralOptionPage m_GeneralOptionPage;
	CSettingsSearchOptionPage m_SearchOptionPage;
	CSettingsfreedbOptionPage m_freedbOptionPage;

};