#include "StdAfx.h"
#include "CTaskDialog.h"

HRESULT WINAPI BackportTaskDialogIndirect(
	const TASKDIALOGCONFIG* config,
	int* button,
	int* radioButton,
	BOOL* verificationChecked)

{
	// show dialog

	CTaskDialog dialog(config);
	int result = static_cast<int>(dialog.DoModal(config->hwndParent));
	if (button) *button = result;
	if (radioButton) *radioButton = dialog.GetRadioButton();
	if (verificationChecked) *verificationChecked = dialog.IsVerificationChecked();

	return S_OK;
}