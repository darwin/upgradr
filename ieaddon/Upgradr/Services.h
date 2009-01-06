#pragma once
#include "RootSettings.h"

class CServices : public CResourceInit<SR_SERVICES> {
public:
	CServices();
	~CServices();

	bool                                           OpenSettingsDialog();

private:
	CRootSettings                                  m_Settings;
};
