#include "StdAfx.h"
#include "DialogManager.h"

#include "atltask.h"

CDialogManager::CDialogManager()
{
}

CDialogManager::~CDialogManager()
{
}

void 
CDialogManager::TestDialog()
{
	CHECK_THREAD_OWNERSHIP;
	CSimpleTaskDialog WelcomeTaskDialog(
		_T("Instruction"), 
		_T("Welcome"), 
		_T("Title"),
		TDCBF_YES_BUTTON | TDCBF_NO_BUTTON,
		MAKEINTRESOURCE(IDI_ABOUTBOX)
		);

	WelcomeTaskDialog.DoModal();

	if (WelcomeTaskDialog.GetButton() == IDYES) {
		WarningBeep();
	}
}
