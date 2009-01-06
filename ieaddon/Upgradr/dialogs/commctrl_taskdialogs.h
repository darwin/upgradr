#ifndef _INC_COMMCTRL_TASKDIALOGS
#define _INC_COMMCTRL_TASKDIALOGS

#include <windows.h>
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TDM_FIRST						(CCM_LAST + 100U)
#define TDN_FIRST						(0U - 1800U)

#define TaskDialog_ClickButton(hWndTD, uID)	\
	(void)SNDMSG((hWndTD), TDM_CLICK_BUTTON, (WPARAM)(UINT)(uID), 0L)

#define TaskDialog_ClickRadioButton(hWndTD, uID)	\
	(void)SNDMSG((hWndTD), TDM_CLICK_RADIO_BUTTON, (WPARAM)(UINT)(uID), 0L)

#define TaskDialog_ClickVerification(hWndTD, bState, bFocus)	\
	(void)SNDMSG((hWndTD), TDM_CLICK_VERIFICATION, (WPARAM)(BOOL)(bState), (LPARAM)(BOOL)(bFocus))

#define TaskDialog_EnableButton(hWndTD, uID, bEnabled)	\
	(void)SNDMSG((hWndTD), TDM_ENABLE_BUTTON, (WPARAM)(UINT)(uID), (LPARAM)(BOOL)(bEnabled))

#define TaskDialog_EnableRadioButton(hWndTD, uID, bEnabled)	\
	(void)SNDMSG((hWndTD), TDM_ENABLE_RADIO_BUTTON, (WPARAM)(UINT)(uID), (LPARAM)(BOOL)(bEnabled))

#define TaskDialog_SetProgressBarMarquee(hWndTD, marquee, speed)	\
	(void)SNDMSG((hWndTD), TDM_SET_PROGRESS_BAR_MARQUEE, (WPARAM)(BOOL)(marquee), (LPARAM)(UINT)(speed))

#define TaskDialog_SetProgressBarPos(hWndTD, pos)	\
	(void)SNDMSG((hWndTD), TDM_SET_PROGRESS_BAR_POS, (WPARAM)(UINT)(pos), 0L)

#define TaskDialog_SetProgressBarRange(hWndTD, mini, maxi)	\
	(void)SNDMSG((hWndTD), TDM_SET_PROGRESS_BAR_RANGE, (WPARAM)0, MAKELPARAM((mini), (maxi)))

#define TaskDialog_SetProgressBarState(hWndTD, state)	\
	(void)SNDMSG((hWndTD), TDM_SET_PROGRESS_BAR_STATE, (WPARAM)state, 0L)

#define TaskDialog_UpdateElementText(hWndTD, tde, text)	\
	(void)SNDMSG((hWndTD), TDM_UPDATE_ELEMENT_TEXT, (WPARAM)(TASKDIALOG_ELEMENTS)(tde), (LPARAM)(LPCTSTR)(text))

#define TaskDialog_UpdateIcon(hWndTD, tdie, res)	\
	(void)SNDMSG((hWndTD), TDM_UPDATE_ICON, (WPARAM)(TASKDIALOG_ICON_ELEMENTS)(tdie), (LPARAM)res)

#define TDM_NAVIGATE							(TDM_FIRST + 53)

#define TaskDialog_Navigate(hWndTD, task_dialog)	\
	(void)SNDMSG((hWndTD), TDM_NAVIGATE, (WPARAM)0, (LPARAM)(const TASKDIALOGCONFIG*)(task_dialog))

//
// Callbak
//
typedef HRESULT (WINAPI* PFTASKDIALOGCALLBACK)(HWND handle,
                           UINT notification,
                           WPARAM wParam,
                           LPARAM lParam,
                           LONG_PTR data);

//
// Functions
//

HRESULT WINAPI BackportTaskDialogIndirect(
	const TASKDIALOGCONFIG* config,
	int* button,
	int* radioButton,
	BOOL* verificationChecked);

HRESULT WINAPI BackportTaskDialog(
	HWND hWndParent,
	HINSTANCE hInstance,
	PCWSTR pszWindowTitle,
	PCWSTR pszMainInstruction,
	PCWSTR pszContent,
	TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons,
	PCWSTR pszIcon,
	int *pnButton
);

#ifdef __cplusplus
}
#endif

#endif /* _INC_COMMCTRL_TASKDIALOGS */

