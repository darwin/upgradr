#include "StdAfx.h"
#include "commctrl_taskdialogs.h"

HRESULT WINAPI BackportTaskDialog(
    HWND hWndParent,
    HINSTANCE hInstance,
    PCWSTR pszWindowTitle,
    PCWSTR pszMainInstruction,
    PCWSTR pszContent,
    TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons,
    PCWSTR pszIcon,
    int *pnButton)

{
	// create task dialog configuration

	//CTaskDialog::Data config;
	TASKDIALOGCONFIG config;

	::ZeroMemory(&config, sizeof(TASKDIALOGCONFIG));

	config.cbSize = sizeof(TASKDIALOGCONFIG);
	config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW | TDF_ALLOW_DIALOG_CANCELLATION;
	config.hwndParent = hWndParent;
	config.hInstance = hInstance;
	config.pszWindowTitle = pszWindowTitle;
	config.pszMainInstruction = pszMainInstruction;
	config.pszContent = pszContent;
	config.dwCommonButtons = dwCommonButtons;
	config.pszMainIcon = pszIcon;

	// validate parameters

	if (config.hInstance == 0) {

		if (config.pszMainIcon != 0 && IS_INTRESOURCE(config.pszMainIcon))
			return E_INVALIDARG;

		if (config.pszWindowTitle != 0 && IS_INTRESOURCE(config.pszWindowTitle))
			return E_INVALIDARG;

		if (config.pszMainInstruction != 0 && IS_INTRESOURCE(config.pszMainInstruction))
			return E_INVALIDARG;
	}

	//if (config.dwCommonButtons == 0) {
	//	config.dwCommonButtons = TDCBF_OK_BUTTON;
	//	config.iDefaultButton = IDOK;
	//}

	// create, display and operate task dialog

	int n_radiobutton = 0;
	BOOL b_verification = FALSE;

	return ::BackportTaskDialogIndirect(&config, pnButton, &n_radiobutton, &b_verification);
}