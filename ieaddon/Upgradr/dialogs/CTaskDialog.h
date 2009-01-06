#ifndef _INC_CTASKDIALOG
#define _INC_CTASKDIALOG

#include "../resource.h"
#include "commctrl_taskdialogs.h"
#include "MsgCrack.h"

class CTaskDialog : public CDialogImpl<CTaskDialog>
{
// type definitions
public:

	struct Data : public TASKDIALOGCONFIG
	{
		int radioButton;
		BOOL verificationChecked;
		BOOL allocated_;
		int iDefaultButton;
	};

// construction / destruction
public:

	enum { IDD = IDD_EMPTY_DIALOG };

	CTaskDialog(const TASKDIALOGCONFIG* config);
	~CTaskDialog();

// attributes
public:

	int GetRadioButton(void) const
		{ return config_.radioButton; }
	BOOL IsVerificationChecked(void) const
		{ return config_.verificationChecked; }

// implementation 
private:

		/// attributes

	BOOL HasFlag(DWORD dwFlags) const
		{ return ((config_.dwFlags & dwFlags) != 0); }

	HINSTANCE Instance(void) const
		{ return config_.hInstance; }

	HWND Parent(void) const
		{ return config_.hwndParent; }

	LPCWSTR WindowTitle(void) const
		{ return config_.pszWindowTitle; }

	HICON MainIcon(void) const
	{
		if (hMainIcon_ != 0)
			return hMainIcon_;

		if (HasFlag(TDF_USE_HICON_MAIN))
			hMainIcon_ = config_.hMainIcon;
		else if (config_.pszMainIcon != 0) {
			HINSTANCE hInstance = config_.hInstance;
			if (config_.pszMainIcon == TD_ERROR_ICON ||
				config_.pszMainIcon == TD_WARNING_ICON ||
				config_.pszMainIcon == TD_INFORMATION_ICON)
				hInstance = 0;
			hMainIcon_ = ::LoadIcon(hInstance, config_.pszMainIcon);
		}

		return hMainIcon_;
	}

	HICON FooterIcon(void) const
	{
		if (hFooterIcon_ != 0)
			return hFooterIcon_;

		if (HasFlag(TDF_USE_HICON_FOOTER))
			hFooterIcon_ = config_.hFooterIcon;
		else if (config_.pszFooterIcon != 0) {
			HINSTANCE hInstance = config_.hInstance;
			if (config_.pszFooterIcon == TD_ERROR_ICON ||
				config_.pszFooterIcon == TD_WARNING_ICON ||
				config_.pszFooterIcon == TD_INFORMATION_ICON)
				hInstance = 0;
			hFooterIcon_ = (HICON) ::LoadImage(hInstance, config_.pszFooterIcon, IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		}

		return hFooterIcon_;
	}

		/// dialog initialization / cleanup

	void Allocate(const TASKDIALOGCONFIG* config);
	void Free(void);

	void Initialize(void);

		/// operations

	HRESULT Callback(UINT nCode, WPARAM wParam = 0, LPARAM lParam = 0L);

	void SendControlCommand(UINT uID, UINT nCode);
	void SetControlFocus(UINT uID);
	
	void UpdateMainIcon(HICON hIcon);
	void UpdateFooterIcon(HICON hIcon);

	void UpdateWindowTitle(LPCWSTR szWindowTitle);
	void EnableSystemClose(BOOL bEnabled);

	void CreateCallbackTimer(void);
	
		/// dialog layout

	void LayoutWindow(void);

	void GenerateControlIDs(void);
	void LayoutControls(void);
	void SetFontControls(void);
	void SetZOrderControls(void);
	void DestroyControls(void);

	int CalculateDialogWidth(int& cx_buttons) const;
	int CalculateButtonExtent(LPCWSTR szText) const;
	int CalculateButtonsExtent(void) const;

	int AddMainInstruction(int x, int y, int cx);
	int AddContent(int x, int y, int cx);
	int AddRadioButtons(int x, int y, int cx);
	int AddProgressBar(int x, int y, int cx);
	int AddFrame(int x, int y, int cx, int cy);
	int AddDividingLine(int x, int y, int cx);
	int AddButtons(int x, int y, int cx);
	int AddVerificationCheckBox(int x, int y, int cx);
	int AddFooterDividingLine(int x, int y, int cx);
	int AddFooter(int x, int y, int cx);

	int AppendMainIcon(int& x, int y);
	int AppendMainInstruction(int x, int y, int cx, int cy);
	int AppendButton(LPCWSTR szText, UINT uID, int& x, int y);
	int AppendFooterIcon(int& x, int y);
	int AppendFooter(int x, int y, int cx, int cy);

		/// controls

	void AddControl(LPCWSTR class_name, LPCWSTR window_text, WORD wID, DWORD dwStyle, DWORD dwExStyle, WORD x, WORD y, WORD cx, WORD cy, DWORD dwHelpID = 0)
	{
		ATLASSERT(IsWindow());
		dwStyle |= WS_CHILD | WS_VISIBLE;
		HWND hWnd = ::CreateWindowEx(dwExStyle, class_name, window_text, dwStyle, x, y, cx, cy, m_hWnd, reinterpret_cast<HMENU>(wID), GetBaseModule().GetModuleInstance(), 0);
		ATLASSERT(::IsWindow(hWnd));
	}

	void AddButton(LPCWSTR window_text, WORD wID, DWORD dwStyle, DWORD dwExStyle, WORD x, WORD y, WORD cx, WORD cy, DWORD dwHelpID = 0)
		{ AddControl(L"BUTTON", window_text, wID, dwStyle, dwExStyle, x, y, cx, cy, dwHelpID); }
	void AddStatic(LPCWSTR window_text, WORD wID, DWORD dwStyle, DWORD dwExStyle, WORD x, WORD y, WORD cx, WORD cy, DWORD dwHelpID = 0)
		{ AddControl(L"STATIC", window_text, wID, dwStyle, dwExStyle, x, y, cx, cy, dwHelpID); }
	void AddHyperLink(LPCWSTR window_text, WORD wID, DWORD dwStyle, DWORD dwExStyle, WORD x, WORD y, WORD cx, WORD cy, DWORD dwHelpID = 0)
		{ AddControl(WC_LINK, window_text, wID, dwStyle, dwExStyle, x, y, cx, cy, dwHelpID); }
	void AddProgressBar(WORD wID, DWORD dwStyle, DWORD dwExStyle, WORD x, WORD y, WORD cx, WORD cy, DWORD dwHelpID = 0)
		{ AddControl(PROGRESS_CLASSW, L"", wID, dwStyle, dwExStyle, x, y, cx, cy, dwHelpID); }

		/// helpers

	static HFONT CreateSystemFont(WORD wSize, WORD wWeight = FW_NORMAL, BOOL bItalic = FALSE);

	CSize GetTextExtent(HFONT hFont, LPCWSTR szText, int cx_width = 65536) const;

		/// resource helpers

	class HeapResString
	{
	// construction / destruction
	private:

		HeapResString(const HeapResString&);
		HeapResString& operator =(const HeapResString&);

	public:

		HeapResString(HINSTANCE hInstance, LPCWSTR lpwsz);
		~HeapResString();

		operator LPCWSTR(void) const
			{ return lpwsz_; }

	// data members
	private:

		bool b_heap;
		union {
			WCHAR* wsz_;
			LPCWSTR lpwsz_;
		};
	};

// data members
private:

	Data config_;

	/// callback timer

	UINT_PTR hTimer_;
	DWORD dwTickCount_;

	/// progress bar

	BOOL marquee_;
	UINT state_;
	UINT pos_;
	UINT min_range_;
	UINT max_range_;

	/// one-time initialization

	bool b_window_style_updated;

	/// control IDs

	WORD IDC_FRAME;
	WORD IDC_MAIN_ICON;
	WORD IDC_MAIN_INSTRUCTION;
	WORD IDC_CONTENT;
	WORD IDC_PROGRESS;
	WORD IDC_DIVIDERX;
	WORD IDC_CHECK;
	WORD IDC_FOOTER_DIVIDER;
	WORD IDC_FOOTER_ICON;
	WORD IDC_FOOTER;

	/// cached attributes

	mutable HICON hMainIcon_;
	mutable HICON hFooterIcon_;
	
	CFontHandle hFont_;
	CFont hFontInstruction_;
	CFont hFontContent_;

	CBrush hBrushWhite_;

	/// graphics dimensions

	int cx_margin;
	int cy_margin;

	int cx_spacing;
	int cy_spacing;

	int cx_smspacing;
	int cy_smspacing;

	int cx_button;
	int cy_button;
	int cx_button_margin;

	int cy_progress_bar;

// message handlers
private:

	LRESULT OnButtonClicked(UINT uID);
	LRESULT OnRadioButtonClicked(UINT uID);
	LRESULT OnVerificationChecked(void);

// message handlers
private:

	BOOL OnInitDialog(HWND /* hWnd */, LPARAM /* lParam */);
	void OnWindowPosChanging(LPWINDOWPOS /* lpWindowPos */);
	void OnHelp(LPHELPINFO /* lpHelpInfo */);
	void OnTimer(UINT /* uID */, TIMERPROC /* lpTimerProc */);
	void OnSysCommand(UINT nCode, CPoint /* pt */);
	void OnDestroy(void);
	void OnNcDestroy(void);

	HBRUSH OnCtlColorStatic(HDC hDC, HWND hWnd);

	void OnClickButton(UINT uID);
	void OnClickVerification(BOOL bState, BOOL bFocus);
	void OnEnableButton(UINT uID, BOOL bEnabled);
	void OnUpdateElementText(TASKDIALOG_ELEMENTS tde, LPCWSTR pszText);
	void OnUpdateIcon(TASKDIALOG_ICON_ELEMENTS tdie, LPARAM lParam);
	void OnSetProgressBarPos(UINT pos);
	void OnSetProgressBarRange(UINT mini, UINT maxi);
	void OnSetProgressBarMarquee(BOOL bMarquee, UINT nSpeed);
	void OnSetProgressBarState(UINT nState);

	void OnNavigate(UINT uMsg, LPARAM lParam);

	LRESULT OnHyperLinkClicked(LPNMHDR lpnmhdr);

	LRESULT OnCancel(UINT /* nCode */, int nID, HWND /* hWnd */);
	LRESULT OnButtonClicked(UINT /* nCode */, int nID, HWND /* hWnd */);

// message handlers
private:

// message map
private:

	BEGIN_MSG_MAP_EX(CTaskDialog)

		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
		MSG_WM_HELP(OnHelp)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_NCDESTROY(OnNcDestroy)

		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)

		MSG_TDM_CLICK_BUTTON(OnClickButton)
		MSG_TDM_CLICK_RADIO_BUTTON(OnClickButton)
		MSG_TDM_CLICK_VERIFICATION(OnClickVerification)
		MSG_TDM_ENABLE_BUTTON(OnEnableButton)
		MSG_TDM_ENABLE_RADIO_BUTTON(OnEnableButton)
		MSG_TDM_UPDATE_ELEMENT_TEXT(OnUpdateElementText)
		MSG_TDM_UPDATE_ICON(OnUpdateIcon)
		MSG_TDM_SET_PROGRESS_BAR_POS(OnSetProgressBarPos)
		MSG_TDM_SET_PROGRESS_BAR_RANGE(OnSetProgressBarRange)
		MSG_TDM_SET_PROGRESS_BAR_MARQUEE(OnSetProgressBarMarquee)
		MSG_TDM_SET_PROGRESS_BAR_STATE(OnSetProgressBarState)

		MSG_TDM_NAVIGATE(OnNavigate)

		NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnHyperLinkClicked)
		NOTIFY_CODE_HANDLER_EX(NM_RETURN, OnHyperLinkClicked)

		COMMAND_HANDLER_EX(IDCANCEL, BN_CLICKED, OnCancel)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCLOSE, OnButtonClicked)
		COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnButtonClicked)

	END_MSG_MAP()

};

#endif // _INC_CTASKDIALOG
