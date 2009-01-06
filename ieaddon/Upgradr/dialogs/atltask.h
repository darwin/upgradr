#ifndef __ATLTASK_H__
#define __ATLTASK_H__

#include "commctrl_taskdialogs.h"

#ifndef __ATLWIN_H__
#pragma error atltask.h requires atlwin.h to be included first
#endif

#define ATL_NAMESPACE	ATL::

class ATL_NO_VTABLE CTaskDialogBase
{
// data members
protected:

	TASKDIALOGCONFIG m_config;
	int m_button;

// construction / destruction
public:

	CTaskDialogBase(ATL_NAMESPACE _U_STRINGorID instruction = 0U, ATL_NAMESPACE _U_STRINGorID content = 0U, ATL_NAMESPACE _U_STRINGorID title = 0U, ATL_NAMESPACE _U_STRINGorID icon = 0U)
	{
		::ZeroMemory(&m_config, sizeof(TASKDIALOGCONFIG));
		m_config.cbSize = sizeof(TASKDIALOGCONFIG);
		m_config.hInstance = GetBaseModule().GetModuleInstance();
		m_config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;

		SetWindowTitle(title);
		SetMainInstruction(instruction);
		SetContent(content);

		SetIcon(icon);
	}

	virtual ~CTaskDialogBase()
	{}

// attributes
public:

	virtual void SetWindowTitle(ATL_NAMESPACE _U_STRINGorID title)
		{ m_config.pszWindowTitle = title.m_lpstr; }
	virtual void SetMainInstruction(ATL_NAMESPACE _U_STRINGorID instruction)
		{ m_config.pszMainInstruction = instruction.m_lpstr; }
	virtual void SetContent(ATL_NAMESPACE _U_STRINGorID content)
		{ m_config.pszContent = content.m_lpstr; }

	virtual void SetIcon(ATL_NAMESPACE _U_STRINGorID icon)
	{
		m_config.dwFlags &= ~TDF_USE_HICON_MAIN;
		m_config.pszMainIcon = icon.m_lpstr;
	}

	virtual int GetButton(void) const
		{ return m_button; }

		// internal
	const TASKDIALOGCONFIG* GetTaskDialogConfig(void) const
		{ return &m_config; }
};

class CSimpleTaskDialog : public CTaskDialogBase
{
// construction / destruction
public:

	CSimpleTaskDialog(ATL_NAMESPACE _U_STRINGorID instruction = 0U, ATL_NAMESPACE _U_STRINGorID content = 0U, ATL_NAMESPACE _U_STRINGorID title = 0U, TASKDIALOG_COMMON_BUTTON_FLAGS buttons = TDCBF_OK_BUTTON, ATL_NAMESPACE _U_STRINGorID icon = 0U)
		: CTaskDialogBase(instruction, content, title, icon)
	{
		m_config.dwCommonButtons = buttons;
	}

// operations
public:

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		m_config.hwndParent = hWndParent;

		if (SUCCEEDED(::TaskDialog(hWndParent
						, m_config.hInstance
						, m_config.pszWindowTitle
						, m_config.pszMainInstruction
						, m_config.pszContent
						, m_config.dwCommonButtons
						, m_config.pszMainIcon
						, &m_button)))
			return (INT_PTR) m_button;

		if (SUCCEEDED(::BackportTaskDialog(hWndParent
			, m_config.hInstance
			, m_config.pszWindowTitle
			, m_config.pszMainInstruction
			, m_config.pszContent
			, m_config.dwCommonButtons
			, m_config.pszMainIcon
			, &m_button)))
			return (INT_PTR) m_button;

		return 0;
	}
};

template <class T, class TBase = ATL_NAMESPACE CWindow>
class ATL_NO_VTABLE CTaskDialogImpl
	: public CTaskDialogBase
	, public TBase
{
// data members
private:

	int m_radiobutton;
	BOOL m_verification;

	CSimpleArray<TASKDIALOG_BUTTON> m_buttons;
	CSimpleArray<TASKDIALOG_BUTTON> m_radioButtons;

// construction / destruction
public:

	CTaskDialogImpl(ATL_NAMESPACE _U_STRINGorID instruction = 0U, ATL_NAMESPACE _U_STRINGorID content = 0U, ATL_NAMESPACE _U_STRINGorID title = 0U, TASKDIALOG_COMMON_BUTTON_FLAGS buttons = TDCBF_OK_BUTTON, ATL_NAMESPACE _U_STRINGorID icon = 0U)
		: CTaskDialogBase(instruction, content, title, icon)
		, m_radiobutton(0)
		, m_verification(FALSE)
	{
		m_config.dwCommonButtons = buttons;

		m_config.pfCallback = TaskDialogCallbackProc;
		m_config.lpCallbackData = reinterpret_cast<LPARAM>(this);
	}

// attributes
public:

	int GetRadioButton(void) const
		{ return m_radiobutton; }

	BOOL IsVerificationChecked(void) const
		{ return m_verification; }

	void SetFlags(TASKDIALOG_FLAGS dwFlags, TASKDIALOG_FLAGS dwExMax = 0)
	{
		ATLASSERT(!::IsWindow(m_hWnd));
		m_config.dwFlags &= ~dwExMax;
		m_config.dwFlags |= dwFlags;
	}

	void SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS buttons)
	{
		ATLASSERT(!::IsWindow(m_hWnd));
		m_config.dwCommonButtons = buttons;
	}

	void SetWindowTitle(ATL_NAMESPACE _U_STRINGorID title)
	{
		if (!::IsWindow(m_hWnd)) {
			CTaskDialogBase::SetWindowTitle(title);
			return ;
		}

		if (IS_INTRESOURCE(title.m_lpstr)) {
			TCHAR szWindowTitle[1024] = { _T('\0') };

#pragma warning (push)
#pragma warning (disable: 4311) // truncation warning

			if (AtlLoadString(reinterpret_cast<UINT>(title.m_lpstr), szWindowTitle, sizeof(szWindowTitle)/sizeof(TCHAR) - sizeof(TCHAR)))
				::SetWindowText(m_hWnd, szWindowTitle);

#pragma warning (pop)

		}

		else
			::SetWindowText(m_hWnd, title.m_lpstr);
	}

	void SetIcon(ATL_NAMESPACE _U_STRINGorID icon)
	{
		if (!::IsWindow(m_hWnd)) {
			CTaskDialogBase::SetIcon(icon);
			return ;
		}

		UpdateIcon(TDIE_ICON_MAIN, icon.m_lpstr);
	}

	void SetIcon(HICON hIcon)
	{
		if (!::IsWindow(m_hWnd)) {
			m_config.dwFlags |= TDF_USE_HICON_MAIN;
			m_config.hMainIcon = hIcon;
			return ;
		}

		UpdateIcon(TDIE_ICON_MAIN, hIcon);
	}

	void SetMainInstruction(ATL_NAMESPACE _U_STRINGorID instruction)
	{
		if (!::IsWindow(m_hWnd)) {
			CTaskDialogBase::SetMainInstruction(instruction);
			return ;
		}

		UpdateElementText(TDE_MAIN_INSTRUCTION, instruction);
	}

	void SetContent(ATL_NAMESPACE _U_STRINGorID content)
	{
		if (!::IsWindow(m_hWnd)) {
			CTaskDialogBase::SetContent(content);
			return ;
		}

		UpdateElementText(TDE_CONTENT, content);
	}

	void AddButton(UINT nButtonID, LPCTSTR pszButtonText = 0)
	{
		TASKDIALOG_BUTTON button = { nButtonID, pszButtonText ? pszButtonText : MAKEINTRESOURCE(nButtonID) };
		m_buttons.Add(button);

		m_config.pButtons = m_buttons.GetData();
		m_config.cButtons = m_buttons.GetSize();

	}

	void SetDefaultButton(UINT nButtonID)
	{
		m_config.iDefaultButton = nButtonID;
	}

	void AddRadioButton(UINT nButtonID, LPCTSTR pszButtonText = 0)
	{
		TASKDIALOG_BUTTON button = { nButtonID, pszButtonText ? pszButtonText : MAKEINTRESOURCE(nButtonID) };
		m_radioButtons.Add(button);

		m_config.pRadioButtons = m_radioButtons.GetData();
		m_config.cRadioButtons = m_radioButtons.GetSize();
	}

	void SetDefaultRadioButton(UINT nButtonID)
	{
		m_config.nDefaultRadioButton = nButtonID;
	}

	void SetVerificationText(ATL_NAMESPACE _U_STRINGorID verification, BOOL bChecked = FALSE)
	{
		ATLASSERT(!::IsWindow(m_hWnd));
		m_config.pszVerificationText = verification.m_lpstr;
		if (bChecked)
			m_config.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
	}

	void SetFooterIcon(ATL_NAMESPACE _U_STRINGorID icon)
	{
		if (!::IsWindow(m_hWnd)) {
			m_config.dwFlags &= ~TDF_USE_HICON_FOOTER;
			m_config.pszFooterIcon = icon.m_lpstr;
			return ;
		}

		UpdateIcon(TDIE_ICON_FOOTER, icon.m_lpstr);
	}

	void SetFooterIcon(HICON hIcon)
	{
		if (!::IsWindow(m_hWnd)) {
			m_config.dwFlags |= TDF_USE_HICON_FOOTER;
			m_config.hFooterIcon = hIcon;
			return ;
		}

		UpdateIcon(TDIE_ICON_FOOTER, hIcon);
	}

	void SetFooter(ATL_NAMESPACE _U_STRINGorID footer)
	{
		if (!::IsWindow(m_hWnd))
			m_config.pszFooter = footer.m_lpstr;

		else
			UpdateElementText(TDE_FOOTER, footer.m_lpstr);
	}

// operations
public:

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(!::IsWindow(m_hWnd));
		m_config.hwndParent = hWndParent;
		if (SUCCEEDED(::TaskDialogIndirect(&m_config, &m_button, &m_radiobutton, &m_verification))) return (INT_PTR) m_button;
		if (SUCCEEDED(::BackportTaskDialogIndirect(&m_config, &m_button, &m_radiobutton, &m_verification))) return (INT_PTR) m_button;
		return 0;
	}

	template <typename U>
	void Navigate(const CTaskDialogImpl<U>& task_dialog)
	{
		TASKDIALOGCONFIG task_dialog_config;
		::CopyMemory(&task_dialog_config, task_dialog.GetTaskDialogConfig(), sizeof(TASKDIALOGCONFIG));
		task_dialog_config.hwndParent = m_config.hwndParent;

		Navigate(&task_dialog_config);
	}

	void ClickButton(UINT uID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_CLICK_BUTTON, (WPARAM) uID);
	}

	void ClickRadioButton(UINT uID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_CLICK_RADIO_BUTTON, (WPARAM) uID);
	}

	void ClickVerification(BOOL bState, BOOL bFocus = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_CLICK_VERIFICATION, (WPARAM) bState, (LPARAM) bFocus);
	}

	void EnableButton(UINT uID, BOOL bEnabled = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_ENABLE_BUTTON, (WPARAM) uID, (LPARAM) bEnabled);
	}

	void EnableRadioButton(UINT uID, BOOL bEnabled = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_ENABLE_RADIO_BUTTON, (WPARAM) uID, (LPARAM) bEnabled);
	}

	void SetProgressBarMarquee(BOOL bMarquee, UINT nSpeed)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_SET_PROGRESS_BAR_MARQUEE, (WPARAM) bMarquee, (LPARAM) nSpeed);
	}

	void SetProgressBarPosition(UINT nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_SET_PROGRESS_BAR_POS, (WPARAM) nPos);
	}

	void SetProgressBarRange(UINT nMinRange, UINT nMaxRange)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_SET_PROGRESS_BAR_RANGE, (WPARAM) 0, MAKELPARAM(nMinRange, nMaxRange));
	}

	void SetProgressBarState(UINT nState)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_SET_PROGRESS_BAR_STATE, (WPARAM) nState);
	}

	BOOL UpdateElementText(TASKDIALOG_ELEMENTS te, ATL_NAMESPACE _U_STRINGorID string)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL) SendMessage(TDM_UPDATE_ELEMENT_TEXT, (WPARAM) te, (LPARAM) string.m_lpstr);
	}

	void UpdateIcon(TASKDIALOG_ICON_ELEMENTS tie, ATL_NAMESPACE _U_STRINGorID icon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(
			((tie == TDIE_ICON_MAIN)   && (m_config.dwFlags & TDF_USE_HICON_MAIN) == 0) ||
			((tie == TDIE_ICON_FOOTER) && (m_config.dwFlags & TDF_USE_HICON_FOOTER) == 0)
			);
		SendMessage(TDM_UPDATE_ICON, (WPARAM) tie, (LPARAM) icon.m_lpstr);
	}

	void UpdateIcon(TASKDIALOG_ICON_ELEMENTS tie, HICON hIcon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(
			((tie == TDIE_ICON_MAIN)   && (m_config.dwFlags & TDF_USE_HICON_MAIN) == TDF_USE_HICON_MAIN) ||
			((tie == TDIE_ICON_FOOTER) && (m_config.dwFlags & TDF_USE_HICON_FOOTER) == TDF_USE_HICON_FOOTER)
			);
		SendMessage(TDM_UPDATE_ICON, (WPARAM) tie, (LPARAM) hIcon);
	}

	void Navigate(const TASKDIALOGCONFIG* task_dialog)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(TDM_NAVIGATE, (WPARAM) 0, (LPARAM) task_dialog);
	}

// notification handlers
public:

	void OnDialogConstructed(void)
		{ /* empty stub */ }
	void OnCreated(void)
		{ /* empty stub */ }
	void OnDestroyed(void)
		{ /* empty stub */ }
	void OnRadioButtonClicked(UINT uID)
		{ /* empty stub */ }
	BOOL OnButtonClicked(UINT uID)
		{ /* empty stub */ return /* should we close the dialog box */ FALSE; }
	void OnVerificationClicked(BOOL bChecked)
		{ /* empty stub */ }
	void OnHyperLinkClicked(LPCWSTR wszHREF)
		{ /* empty stub */ }
	void OnHelp(void)
		{ /* empty stub */ }
	BOOL OnTimer(DWORD dwTickCount)
		{ /* empty stub */ return /* should we reset the timer */ FALSE; }
	void OnNavigated(void)
		{ /* empty stub */ }

// notification callback
protected:

	static HRESULT __stdcall TaskDialogCallbackProc(HWND hWnd, UINT uCode, WPARAM wParam, LPARAM lParam, LONG_PTR data)
	{
		ATLASSERT(::IsWindow(hWnd));

		T* pT = reinterpret_cast<T*>(data);
		pT->m_hWnd = hWnd;

		ATLASSERT(::IsWindow(pT->m_hWnd));

		ATLTRACE(_T("TaskDialogCallbackProc(%s)\n"),
			(uCode == TDN_DIALOG_CONSTRUCTED ? _T("TDN_DIALOG_CONSTRUCTED") :
			(uCode == TDN_CREATED ? _T("TDN_CREATED") :
			(uCode == TDN_DESTROYED ? _T("TDN_DESTROYED") :
			(uCode == TDN_BUTTON_CLICKED ? _T("TDN_BUTTON_CLICKED") :
			(uCode == TDN_RADIO_BUTTON_CLICKED ? _T("TDN_RADIO_BUTTON_CLICKED") :
			(uCode == TDN_VERIFICATION_CLICKED ? _T("TDN_VERIFICATION_CLICKED") :
			(uCode == TDN_HYPERLINK_CLICKED ? _T("TDN_HYPERLINK_CLICKED") :
			(uCode == TDN_HELP ? _T("TDN_HELP") :
			(uCode == TDN_TIMER ? _T("TDN_TIMER") :
			(uCode == TDN_NAVIGATED ? _T("TDN_NAVIGATED") : _T("E_NOTIMPL")))))))))))
				);

		HRESULT hResult = S_OK;

		switch (uCode) {
			case TDN_DIALOG_CONSTRUCTED:
				pT->OnDialogConstructed();
				break;

			case TDN_CREATED:
				pT->OnCreated();
				break;

			case TDN_DESTROYED:
				pT->OnDestroyed();
				break;

			case TDN_BUTTON_CLICKED:
				hResult = (HRESULT) pT->OnButtonClicked((UINT) wParam);
				break;

			case TDN_RADIO_BUTTON_CLICKED:
				pT->OnRadioButtonClicked((UINT) wParam);
				break;

			case TDN_VERIFICATION_CLICKED:
				pT->OnVerificationClicked((BOOL) wParam);
				break;

			case TDN_HYPERLINK_CLICKED:
				pT->OnHyperLinkClicked((LPCWSTR) lParam);
				break;

			case TDN_EXPANDO_BUTTON_CLICKED:
				ATLASSERT(E_NOTIMPL);
				break;

			case TDN_HELP:
				pT->OnHelp();
				break;

			case TDN_TIMER:
				hResult = (HRESULT) pT->OnTimer((DWORD) wParam);
				break;

			case TDN_NAVIGATED:
				pT->OnNavigated();
				break;
		}

		ATLASSERT(::IsWindow(pT->m_hWnd));

		pT->m_hWnd = 0;

		return hResult;
	}
};

class CTaskDialog : public CTaskDialogImpl<CTaskDialog>
{
public:

	CTaskDialog(ATL_NAMESPACE _U_STRINGorID instruction = 0U, ATL_NAMESPACE _U_STRINGorID content = 0U, ATL_NAMESPACE _U_STRINGorID title = 0U, TASKDIALOG_COMMON_BUTTON_FLAGS buttons = TDCBF_OK_BUTTON, ATL_NAMESPACE _U_STRINGorID icon = 0U)
		: CTaskDialogImpl<CTaskDialog>(instruction, content, title, buttons, icon)
	{}
};

#endif /* __ATLTASK_H__ */
