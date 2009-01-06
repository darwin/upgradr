#include "StdAfx.h"
#include "CTaskDialog.h"
#include "commctrl_taskdialogs.h"

#pragma warning (disable: 4244) // conversion from 'double' to 'int', possible loss of data

/// obtains the previous non-space character
static LPCWSTR WINAPI GetWordWrapBoundary(LPCWSTR szStart, LPCWSTR szText)
{
	const WCHAR* _szText = szText;
	while (*_szText != L' ' && *_szText != L'\n' && _szText != szStart)
		_szText = ::CharPrevW(szStart, _szText);
	if (_szText != szText)
		_szText = ::CharNextW(_szText);
	return _szText;
}

/// obtains the string beginning at character index nAt
static LPCWSTR WINAPI GetCharAtW(LPCWSTR szText, int nAt)
{
	const WCHAR* _szText = szText;
	for (; nAt > 0; nAt--) {
		_szText = ::CharNextW(_szText);
		if (*_szText == L'\0')
			break;
	}
	return _szText;
}

/// implements GetTextExtentExPoint with additionnal support for newline-characters
static BOOL WINAPI GetTextExtentExPointExW(HDC hDC, LPCWSTR szText, int cchString, int nMaxExtent, LPINT lpnFit, LPINT alpDx, LPSIZE lpSize)
{
	LPWSTR _szNewLine = ::StrChr(szText, L'\n');
	if (_szNewLine != 0) {
		int cch = 0;
		const WCHAR* _szText = szText;
		while (_szText <= _szNewLine) {
			_szText = ::CharNextW(_szText);
			++cch;
		}

		cchString = min(cchString, cch);
	}

	BOOL bResult = ::GetTextExtentExPoint(hDC, szText, cchString, nMaxExtent, lpnFit, alpDx, lpSize);

	// a single newline-character should take up the same space as one line would

	if (_szNewLine != 0 && lpSize->cy == 0) {
		SIZE size;
		::GetTextExtentExPoint(hDC, L"|", 1, 65536, 0, 0, &size);
		lpSize->cy = size.cy;
	}

	return bResult;
}

/// implements GetTextExtent with additionnal support for word wrapped text
static BOOL WINAPI GetTextExtentEx(HDC hDC, LPCWSTR szText, int cchString, int nMaxExtent, LPSIZE lpSize)
{
	lpSize->cx = 0;
	lpSize->cy = 0;

	int nFit = 0;
	SIZE extent = { 0, 0 };
	const WCHAR* _szText = szText;

	if (::GetTextExtentExPointExW(hDC, _szText, cchString, nMaxExtent, &nFit, 0, &extent)) {

		lpSize->cx = min(nMaxExtent, extent.cx);

		if (nFit == cchString)
			lpSize->cy = extent.cy;

		else {

			// the specified text does not fit on a single line
			// break up each individual line and accumulate the
			// vertical dimension

			while (true) {

				lpSize->cx = min(nMaxExtent, max(lpSize->cx, extent.cx));
				lpSize->cy += extent.cy;

				const WCHAR* _szAt = 0;
				if (*( _szAt = ::GetCharAtW(_szText, nFit)) == L'\0')
					break;

				_szText = ::GetWordWrapBoundary(_szText, _szAt);
				::GetTextExtentExPointExW(hDC, _szText, ::lstrlenW(_szText), nMaxExtent, &nFit, 0, &extent);

			}
		}

		return TRUE;
	}

	return FALSE;
}

// TODO: fix footer standard icons (IDI_WARNING, etc.)
// TODO: expanded information

namespace {

	const COLORREF COLOR_WHITE = RGB(255, 255, 255);
	const COLORREF COLOR_BLUE = RGB(0, 25, 255);
	const COLORREF COLOR_RED = RGB(210, 25, 0);

	const int _cx_margin = 7;
	const int _cy_margin = 7;
	const int _cx_spacing = 7;
	const int _cy_spacing = 7;
	const int _cx_smspacing = 2;
	const int _cy_smspacing = 1;
	const int _cx_button = 50;
	const int _cy_button = 14;
	const int _cx_button_margin = 5;
	const int _cy_progress_bar = 8;


} // anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// HeapString helper class

CTaskDialog::HeapResString::HeapResString(HINSTANCE hInstance, LPCWSTR lpwsz)
	: b_heap(true)
	, lpwsz_(0)
{
	if (IS_INTRESOURCE(lpwsz)) {

		// load entire resource string in a heap-allocated buffer

#pragma warning (push)
#pragma warning (disable: 4311)

		DWORD dwSize = 128;
		DWORD dwLen = 0;
		wsz_ = reinterpret_cast<WCHAR*>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, (dwSize + 1) * sizeof(WCHAR)));
		while ((dwLen = ::LoadString(hInstance, reinterpret_cast<UINT>(lpwsz), wsz_, dwSize)) == dwSize - 1) {
			dwSize *= 1.50;
			wsz_ = reinterpret_cast<WCHAR*>(::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, wsz_, (dwSize + 1) * sizeof(WCHAR)));
		}

#pragma warning (pop)

	}

	else {

		b_heap = false;
		lpwsz_ = lpwsz;
	}
}

CTaskDialog::HeapResString::~HeapResString()
{
	if (b_heap)
		::HeapFree(::GetProcessHeap(), 0, wsz_);
}

//////////////////////////////////////////////////////////////////////////////
// construction / destruction

CTaskDialog::CTaskDialog(const TASKDIALOGCONFIG* config)
	: b_window_style_updated(false)
	, hMainIcon_(0)
	, hFooterIcon_(0)
{

	Allocate(config);
	Initialize();
}

CTaskDialog::~CTaskDialog()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////////
// dialog initialization / cleanup implementation 

void CTaskDialog::Allocate(const TASKDIALOGCONFIG* config)
{
	config_.allocated_ = false;
	::ZeroMemory(&config_, sizeof(Data));
	::CopyMemory(&config_, config, sizeof(TASKDIALOGCONFIG));
}

void CTaskDialog::Free(void)
{
	if (config_.allocated_) {

		FREEINTRESOURCE(config_.pszWindowTitle);
		FREEINTRESOURCE(config_.pszMainInstruction);
		FREEINTRESOURCE(config_.pszContent);
		FREEINTRESOURCE(config_.pszVerificationText);
		FREEINTRESOURCE(config_.pszExpandedInformation);
		FREEINTRESOURCE(config_.pszExpandedControlText);
		FREEINTRESOURCE(config_.pszCollapsedControlText);
		FREEINTRESOURCE(config_.pszFooter);

		for (DWORD cButton = 0; cButton < config_.cButtons; cButton++)
			FREEINTRESOURCE(config_.pButtons[cButton].pszButtonText);
		for (DWORD cRadioButton = 0; cRadioButton < config_.cRadioButtons; cRadioButton++)
			FREEINTRESOURCE(config_.pRadioButtons[cRadioButton].pszButtonText);

		delete [] config_.pButtons;
		delete [] config_.pRadioButtons;

	}
#ifdef _DEBUG
	::ZeroMemory(&config_, sizeof(Data));
	config_.allocated_ = false;
#endif

	b_window_style_updated = false;
}

void CTaskDialog::Initialize(void)
{
		// validate parameters

	if (config_.dwCommonButtons == 0 && config_.cButtons == 0) {
		config_.dwCommonButtons = TDCBF_OK_BUTTON;
		config_.iDefaultButton = IDOK;
	}

	if (config_.iDefaultButton == 0) {
		if (config_.cButtons != 0)
			config_.iDefaultButton = config_.pButtons[0].nButtonID;
		else {
			if (config_.dwCommonButtons & TDCBF_OK_BUTTON)
				config_.iDefaultButton = IDOK;

			else if (config_.dwCommonButtons & TDCBF_YES_BUTTON)
				config_.iDefaultButton = IDYES;

			else if (config_.dwCommonButtons & TDCBF_NO_BUTTON)
				config_.iDefaultButton = IDNO;

			else if (config_.dwCommonButtons & TDCBF_RETRY_BUTTON)
				config_.iDefaultButton = IDRETRY;

			else if (config_.dwCommonButtons & TDCBF_CANCEL_BUTTON)
				config_.iDefaultButton = IDCANCEL;

			else if (config_.dwCommonButtons & TDCBF_CLOSE_BUTTON)
				config_.iDefaultButton = IDCLOSE;

		}
	}

	if (config_.dwCommonButtons & TDCBF_CANCEL_BUTTON)
		config_.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;

}

	/// helpers

HFONT CTaskDialog::CreateSystemFont(WORD wSize, WORD wWeight, BOOL bItalic)
{
	// create the font that is used to calculate text extent

	LOGFONTW logFont = { 0 };
 
	NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
	::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, false );
	logFont = ncm.lfMessageFont;
 
	logFont.lfHeight = -wSize;
	logFont.lfWeight = wWeight;
	logFont.lfItalic = bItalic;

	return ::CreateFontIndirectW(&logFont);
}

CSize CTaskDialog::GetTextExtent(HFONT hFont, LPCWSTR szText, int cx_width) const
{
	CSize size(0, 0);

	// obtains the number of caracters that fit on each line

	CWindowDC hDC(m_hWnd);
	CFontHandle _hFont = hDC.SelectFont(hFont);

	if (!::GetTextExtentEx(hDC, szText, ::lstrlenW(szText), cx_width, &size)) {
		size.cx = 0;
		size.cy = 0;
	}

	hDC.SelectFont(_hFont);

	return size;
}

	/// dialog layout

void CTaskDialog::LayoutWindow(void)
{
	// customize post-creation window styles

	EnableSystemClose(HasFlag(TDF_ALLOW_DIALOG_CANCELLATION));

	// create and position controls

	GenerateControlIDs();
	LayoutControls();
	SetFontControls();
	SetZOrderControls();

	// update various graphical items

	UpdateMainIcon(MainIcon());
	UpdateWindowTitle(WindowTitle());
	UpdateFooterIcon(FooterIcon());

	// set the keyboard focus to the default button
	// and select the default radio button

	SetControlFocus(config_.iDefaultButton);

	if (!HasFlag(TDF_NO_DEFAULT_RADIO_BUTTON) && config_.nDefaultRadioButton != 0)
		CheckDlgButton(config_.nDefaultRadioButton, BST_CHECKED);

	// position dialog

	if (HasFlag(TDF_POSITION_RELATIVE_TO_WINDOW))
		CenterWindow(Parent());
	else
		CenterWindow(::GetDesktopWindow());

	if (HasFlag(TDF_CALLBACK_TIMER))
		CreateCallbackTimer();

	Callback(TDN_DIALOG_CONSTRUCTED);
}

void CTaskDialog::GenerateControlIDs(void)
{
	// custom control IDs start after the last custom button or radio button ID

	IDC_FRAME = 1000;

	for (DWORD cButton = 0; cButton < config_.cButtons; cButton++)
		IDC_FRAME = max(IDC_FRAME, config_.pButtons[cButton].nButtonID);

	for (DWORD cButton = 0; cButton < config_.cRadioButtons; cButton++)
		IDC_FRAME = max(IDC_FRAME, config_.pRadioButtons[cButton].nButtonID);

	IDC_FRAME++;
	IDC_MAIN_ICON			= IDC_FRAME++;
	IDC_MAIN_INSTRUCTION	= IDC_FRAME++;
	IDC_CONTENT				= IDC_FRAME++;
	IDC_PROGRESS			= IDC_FRAME++;
	IDC_DIVIDERX				= IDC_FRAME++;
	IDC_CHECK				= IDC_FRAME++;
	IDC_FOOTER_DIVIDER		= IDC_FRAME++;
	IDC_FOOTER_ICON			= IDC_FRAME++;
	IDC_FOOTER				= IDC_FRAME++;

	hTimer_					= IDC_FRAME++;

}

void CTaskDialog::LayoutControls(void)
{
	// layout controls

	int cx_buttons = 0;
	int cx_width = CalculateDialogWidth(cx_buttons);
	int cy_height = 0;

	int x = cx_margin;
	int y = cy_margin;

	// add content area

	{
		if (config_.pszMainInstruction != 0 || config_.pszMainIcon != 0)
			y += cy_spacing + AddMainInstruction(x, y, cx_width - cx_margin - cx_margin);
		if (config_.pszContent != 0)
			y += cy_spacing + AddContent(x, y, cx_width - cx_margin - cx_margin);
		if (config_.cRadioButtons != 0)
			y += cy_spacing + AddRadioButtons(x, y, cx_width - cx_margin - cx_margin);
		if (HasFlag(TDF_SHOW_PROGRESS_BAR))
			y += cy_spacing + AddProgressBar(x, y, cx_width - cx_margin - cx_margin);

		cy_height = y;
	}

	{
		y  = AddFrame(0, 0, cx_width, cy_height);
		y += AddDividingLine(0, y, cx_width);
		y += cy_spacing;
	}

	// add row of buttons

	{
		y += cy_spacing + AddButtons(cx_margin + cx_width - cx_buttons, y, cx_width);
	}

	// add verification text

	if (config_.pszVerificationText != 0) {
		y += cy_spacing + AddVerificationCheckBox(cx_margin, y, cx_width - cx_margin - cx_margin);
	}

	// add footer area

	if (config_.pszFooter != 0 || config_.pszFooterIcon != 0) {

		y += cy_spacing + AddFooterDividingLine(0, y, cx_width);
		y += cy_spacing + AddFooter(cx_margin, y, cx_width - cx_margin - cy_margin);
	}

	y += cy_margin - cy_spacing;
	y += ::GetSystemMetrics(SM_CYCAPTION);

	// update dialog dimension

	RECT rect = { 0, 0, cx_width, y };
	SetWindowPos(0, &rect, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CTaskDialog::SetFontControls(void)
{
	// update control font

	::SendMessage(GetDlgItem(IDC_MAIN_INSTRUCTION), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFontInstruction_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDC_CONTENT), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFontContent_), (LPARAM) 0L);

	for (DWORD cButton = 0; cButton < config_.cRadioButtons; cButton++)
		::SendMessage(GetDlgItem(config_.pRadioButtons[cButton].nButtonID), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);

	for (DWORD cButton = 0; cButton < config_.cButtons; cButton++)
		::SendMessage(GetDlgItem(config_.pButtons[cButton].nButtonID), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);

	::SendMessage(GetDlgItem(IDOK), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDYES), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDNO), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDRETRY), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDCANCEL), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDCLOSE), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);

	::SendMessage(GetDlgItem(IDC_CHECK), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
	::SendMessage(GetDlgItem(IDC_FOOTER), WM_SETFONT, (WPARAM) static_cast<HFONT>(hFont_), (LPARAM) 0L);
}

void CTaskDialog::SetZOrderControls(void)
{
	// restore correct window z-order
	// insert all controls after the frame in reversed order

	for (DWORD cButton = config_.cRadioButtons; cButton > 0; cButton--)
		::SetWindowPos(GetDlgItem(config_.pRadioButtons[cButton - 1].nButtonID), GetDlgItem(IDC_FRAME), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	::SetWindowPos(GetDlgItem(IDC_PROGRESS), GetDlgItem(IDC_FRAME), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::SetWindowPos(GetDlgItem(IDC_CONTENT), GetDlgItem(IDC_FRAME), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::SetWindowPos(GetDlgItem(IDC_MAIN_INSTRUCTION), GetDlgItem(IDC_FRAME), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::SetWindowPos(GetDlgItem(IDC_MAIN_ICON), GetDlgItem(IDC_FRAME), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CTaskDialog::DestroyControls(void)
{
	::DestroyWindow(GetDlgItem(IDC_MAIN_ICON));
	::DestroyWindow(GetDlgItem(IDC_MAIN_INSTRUCTION));
	::DestroyWindow(GetDlgItem(IDC_CONTENT));
	::DestroyWindow(GetDlgItem(IDC_PROGRESS));
	::DestroyWindow(GetDlgItem(IDC_FRAME));
	::DestroyWindow(GetDlgItem(IDC_DIVIDERX));
	::DestroyWindow(GetDlgItem(IDC_CHECK));
	::DestroyWindow(GetDlgItem(IDC_FOOTER_DIVIDER));
	::DestroyWindow(GetDlgItem(IDC_FOOTER_ICON));
	::DestroyWindow(GetDlgItem(IDC_FOOTER));

	for (DWORD cButton = 0; cButton < config_.cButtons; cButton++)
		::DestroyWindow(GetDlgItem(config_.pButtons[cButton].nButtonID));
	for (DWORD cButton = 0; cButton < config_.cRadioButtons; cButton++)
		::DestroyWindow(GetDlgItem(config_.pRadioButtons[cButton].nButtonID));

	::DestroyWindow(GetDlgItem(IDOK));
	::DestroyWindow(GetDlgItem(IDYES));
	::DestroyWindow(GetDlgItem(IDNO));
	::DestroyWindow(GetDlgItem(IDRETRY));
	::DestroyWindow(GetDlgItem(IDCANCEL));
	::DestroyWindow(GetDlgItem(IDCLOSE));
}

int CTaskDialog::CalculateDialogWidth(int& cx_buttons) const
{
	int cx_width = config_.cxWidth;

	cx_buttons = CalculateButtonsExtent() + cx_margin + cy_margin;
	cx_width = max(cx_width, cx_buttons);

	// main instruction should fit on one line
	if (config_.pszMainInstruction != 0 || config_.pszMainIcon != 0) {

		int cx = cx_margin + ::GetSystemMetrics(SM_CXICON) + cx_spacing + cx_margin;
		HeapResString szMainInstruction(Instance(), config_.pszMainInstruction);
		CSize extent = GetTextExtent(hFontInstruction_, szMainInstruction);
		cx_width = max(cx_width, extent.cx + cx);		

	}

	// try to maintain 16:9 aspect ratio for text content
	{
		HeapResString szContent(Instance(), config_.pszContent);
		CSize extent = GetTextExtent(hFontContent_, szContent, cx_width - cx_margin - cx_margin);
		if (extent.cy > extent.cx) {
			int cx = cx_width;
			double ratio = static_cast<double>(extent.cx)/static_cast<double>(extent.cy);
			while (ratio < 16.0/9.0) {
				cx *= 1.75;
				extent = GetTextExtent(hFontContent_, szContent, cx - cx_margin - cx_margin);
				if (static_cast<double>(extent.cx)/static_cast<double>(extent.cy) == ratio) {
					cx_width = min(cx_width, extent.cx + cx_margin + cx_margin);
					break;
				}
				cx_width = cx;
				ratio = static_cast<double>(extent.cx)/static_cast<double>(extent.cy);
			}
		}
	}

	return cx_width;
}

int CTaskDialog::CalculateButtonExtent(LPCWSTR szText) const
{
	HeapResString szButtonText(Instance(), szText);
	CSize extent = GetTextExtent(hFont_, szButtonText);
	return max(extent.cx + cx_button_margin + cx_button_margin, cx_button);
}

int CTaskDialog::CalculateButtonsExtent(void) const
{
	int cx = 0;

	// calculate custom buttons width

	for (DWORD cButton = 0; cButton < config_.cButtons; cButton++)
		cx += cx_spacing + CalculateButtonExtent(config_.pButtons[cButton].pszButtonText);

	// calculate common buttons width

	if (config_.dwCommonButtons & TDCBF_OK_BUTTON)
		cx += cx_spacing + CalculateButtonExtent(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDOK)));

	if (config_.dwCommonButtons & TDCBF_YES_BUTTON)
		cx += cx_spacing + CalculateButtonExtent(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDYES)));

	if (config_.dwCommonButtons & TDCBF_NO_BUTTON)
		cx += cx_spacing + CalculateButtonExtent(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDNO)));

	if (config_.dwCommonButtons & TDCBF_RETRY_BUTTON)
		cx += cx_spacing + CalculateButtonExtent(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDRETRY)));

	if (config_.dwCommonButtons & TDCBF_CANCEL_BUTTON)
		cx += cx_spacing + CalculateButtonExtent(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDCANCEL)));

	if (config_.dwCommonButtons & TDCBF_CLOSE_BUTTON)
		cx += cx_spacing + CalculateButtonExtent(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDCLOSE)));

	// return updated x-extent
	
	return cx - cx_spacing;
}

int CTaskDialog::AddMainInstruction(int x, int y, int cx)
{
	ATLASSERT(hFontInstruction_ != 0);

	int cy = 0;

	if (config_.pszMainIcon != 0) {
		int _cx = x;
		cy = AppendMainIcon(x, y);
		cx -= cx_spacing + x - _cx;
		x += cx_spacing;
	}

	if (config_.pszMainInstruction != 0)
		cy = AppendMainInstruction(x, y, cx, cy);

	return cy;
}

int CTaskDialog::AddContent(int x, int y, int cx)
{
	ATLASSERT(hFontContent_ != 0);

	HeapResString szStaticText(Instance(), config_.pszContent);
	CSize extent = GetTextExtent(hFontContent_, szStaticText, cx);

	if (HasFlag(TDF_ENABLE_HYPERLINKS))
		AddHyperLink(szStaticText, IDC_CONTENT, WS_TABSTOP, 0, x, y, extent.cx, extent.cy);
	else
		AddStatic(szStaticText, IDC_CONTENT, SS_LEFT, 0, x, y, extent.cx, extent.cy);

	return extent.cy;
}

int CTaskDialog::AddRadioButtons(int x, int y, int cx)
{
	int cy = 0;

	int _cy = ::GetSystemMetrics(SM_CXMENUCHECK);
	_cy += 2* ::GetSystemMetrics(SM_CXBORDER);
	_cy += 2* ::GetSystemMetrics(SM_CXEDGE);

	for (DWORD cButton = 0; cButton < config_.cRadioButtons; cButton++) {

		HeapResString szButtonText(Instance(), config_.pRadioButtons[cButton].pszButtonText);
		CSize extent = GetTextExtent(hFont_, szButtonText);

		AddButton(szButtonText, config_.pRadioButtons[cButton].nButtonID, BS_AUTORADIOBUTTON | (cButton == 0 ? WS_GROUP | WS_TABSTOP : 0), 0, x, y + cy, cx, _cy);
		cy += cy_smspacing + _cy;
	}

	return cy - cy_smspacing;
}

int CTaskDialog::AddProgressBar(int x, int y, int cx)
{
	marquee_ = (HasFlag(TDF_SHOW_MARQUEE_PROGRESS_BAR));

	AddProgressBar(IDC_PROGRESS, PBS_SMOOTH | (marquee_ ? PBS_MARQUEE : 0), 0, x, y, cx, cy_progress_bar);

	state_ = PBST_NORMAL;
	pos_ = ::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_GETPOS, (WPARAM) 0, (LPARAM) 0L);
	min_range_ = ::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_GETRANGE, (WPARAM) MAKEWPARAM(TRUE, 0), (LPARAM) 0L);
	max_range_ = ::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_GETRANGE, (WPARAM) MAKEWPARAM(FALSE, 0), (LPARAM) 0L);

	if (marquee_)
		OnSetProgressBarMarquee(TRUE, 25U);

	return cy_progress_bar;
}

int CTaskDialog::AddFrame(int x, int y, int cx, int cy)
{
	AddStatic(L"", IDC_FRAME, SS_WHITERECT, 0, x, y, cx, cy);
	return cy;
}

int CTaskDialog::AddDividingLine(int x, int y, int cx)
{
	int cy = ::GetSystemMetrics(SM_CYEDGE);
	AddStatic(L"", IDC_DIVIDERX, SS_ETCHEDHORZ, 0, x, y, cx, cy);
	return cy;
}

int CTaskDialog::AddButtons(int x, int y, int /* cx */)
{
	// add custom buttons

	for (DWORD cButton = 0; cButton < config_.cButtons; cButton++) {
		AppendButton(config_.pButtons[cButton].pszButtonText, config_.pButtons[cButton].nButtonID, x, y);
		x += cx_spacing;
	}

	// add common buttons

	if (config_.dwCommonButtons & TDCBF_OK_BUTTON) {
		AppendButton(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDOK)), IDOK, x, y);
		x += cx_spacing;
	}

	if (config_.dwCommonButtons & TDCBF_YES_BUTTON) {
		AppendButton(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDYES)), IDYES, x, y);
		x += cx_spacing;
	}

	if (config_.dwCommonButtons & TDCBF_NO_BUTTON) {
		AppendButton(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDNO)), IDNO, x, y);
		x += cx_spacing;
	}

	if (config_.dwCommonButtons & TDCBF_RETRY_BUTTON) {
		AppendButton(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDRETRY)), IDRETRY, x, y);
		x += cx_spacing;
	}

	if (config_.dwCommonButtons & TDCBF_CANCEL_BUTTON) {
		AppendButton(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDCANCEL)), IDCANCEL, x, y);
		x += cx_spacing;
	}

	if (config_.dwCommonButtons & TDCBF_CLOSE_BUTTON) {
		AppendButton(HeapResString(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCEW(IDCLOSE)), IDCLOSE, x, y);
		x += cx_spacing;
	}

	return cy_button;
}

int CTaskDialog::AddVerificationCheckBox(int x, int y, int cx)
{
	HeapResString szButtonText(Instance(), config_.pszVerificationText);

	int cy = ::GetSystemMetrics(SM_CXMENUCHECK);
	cy += 2* ::GetSystemMetrics(SM_CXBORDER);
	cy += 2* ::GetSystemMetrics(SM_CXEDGE);

	AddButton(szButtonText, IDC_CHECK, BS_AUTOCHECKBOX | WS_TABSTOP, 0, x, y, cx, cy);

	if (HasFlag(TDF_VERIFICATION_FLAG_CHECKED))
		::SendMessage(GetDlgItem(IDC_CHECK), BM_SETCHECK, (LPARAM) BST_CHECKED, 0L);

	return cy;
}

int CTaskDialog::AddFooterDividingLine(int x, int y, int cx)
{
	int cy = ::GetSystemMetrics(SM_CYEDGE);
	AddStatic(L"", IDC_FOOTER_DIVIDER, SS_ETCHEDHORZ, 0, x, y, cx, cy);
	return cy;
}

int CTaskDialog::AddFooter(int x, int y, int cx)
{
	int cy = 0;

	if (config_.pszFooterIcon != 0) {
		int _cx = x;
		cy = AppendFooterIcon(x, y);
		cx -= cx_smspacing + x - _cx;
		x += cx_smspacing;
	}

	if (config_.pszFooter != 0)
		cy = AppendFooter(x, y, cx, cy);

	return cy;
}

int CTaskDialog::AppendMainIcon(int& x, int y)
{
	int cx = ::GetSystemMetrics(SM_CXICON);
	int cy = ::GetSystemMetrics(SM_CYICON);
	AddStatic(L"", IDC_MAIN_ICON, SS_ICON | SS_CENTERIMAGE, 0, x, y, cx, cy);
	x += cx;
	return cy;
}

int CTaskDialog::AppendMainInstruction(int x, int y, int cx, int cy)
{
	ATLASSERT(hFontInstruction_ != 0);

	HeapResString szStaticText(Instance(), config_.pszMainInstruction);
	CSize extent = GetTextExtent(hFontInstruction_, szStaticText, cx);
	extent.cy = max(extent.cy, cy);
	AddStatic(szStaticText, IDC_MAIN_INSTRUCTION, SS_LEFT | SS_CENTERIMAGE, 0, x, y, cx, extent.cy);

	return extent.cy;
}

int CTaskDialog::AppendButton(LPCWSTR szText, UINT uID, int& x, int y)
{
	HeapResString szButtonText(Instance(), szText);
	CSize extent = GetTextExtent(hFont_, szButtonText);
	int _cx = max(extent.cx + cx_button_margin + cx_button_margin, cx_button);
	AddButton(szButtonText, uID, ((config_.iDefaultButton == uID) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON) | WS_TABSTOP, 0, x, y, _cx, cy_button);
	x += _cx;
	return cy_button;
}

int CTaskDialog::AppendFooterIcon(int& x, int y)
{
	int cx = ::GetSystemMetrics(SM_CXSMICON);
	int cy = ::GetSystemMetrics(SM_CYSMICON);
	AddStatic(L"", IDC_FOOTER_ICON, SS_ICON | SS_CENTERIMAGE, 0, x, y, cx, cy);
	x += cx;
	return cy;
}

int CTaskDialog::AppendFooter(int x, int y, int cx, int cy)
{
	HeapResString szStaticText(Instance(), config_.pszFooter);
	CSize extent = GetTextExtent(hFont_, szStaticText, cx);
	extent.cy = max(extent.cy, cy);

	if (HasFlag(TDF_ENABLE_HYPERLINKS))
		AddHyperLink(szStaticText, IDC_FOOTER, WS_TABSTOP, 0, x, y, cx, extent.cy);
	else
		AddStatic(szStaticText, IDC_FOOTER, SS_LEFT, 0, x, y, cx, extent.cy);

	return extent.cy;
}

	/// operations

HRESULT CTaskDialog::Callback(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if (config_.pfCallback)
		return config_.pfCallback(m_hWnd, nCode, wParam, lParam, config_.lpCallbackData);

	if (nCode == TDN_BUTTON_CLICKED)
		return (HRESULT) /* do not cancel */ FALSE;
	if (nCode == TDN_TIMER)
		return (HRESULT) /* do not reset */ FALSE;

	return S_OK;
}

void CTaskDialog::SendControlCommand(UINT uID, UINT nCode)
{
	SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(uID, nCode), (LPARAM) static_cast<HWND>(GetDlgItem(uID)));
}

void CTaskDialog::SetControlFocus(UINT uID)
{
	// the following line does not seem to be working
	//SendMessage(WM_NEXTDLGCTL, (WPARAM) static_cast<HWND>(GetDlgItem(uID)), (LPARAM) TRUE);

	// change the focus the hard way

	HWND hWndOldButton = 0;
	HWND hWndNewFocus = GetDlgItem(uID);

	if (HIWORD(SendMessage(DM_GETDEFID) == DC_HASDEFID))
		hWndOldButton = GetDlgItem(LOWORD(SendMessage(DM_GETDEFID)));

	if (::IsWindow(hWndOldButton)) {

		// if the new control is a push button
		// set the default push button style

		DWORD dwStyle = ::GetWindowLong(hWndNewFocus, GWL_STYLE);
		if (dwStyle & BS_PUSHBUTTON) {

			dwStyle &= ~BS_PUSHBUTTON;
			dwStyle |= BS_DEFPUSHBUTTON;
			::SendMessage(hWndNewFocus, BM_SETSTYLE, (WPARAM) dwStyle, (LPARAM) TRUE);

			// in that case, clear the previous default push button

			dwStyle = ::GetWindowLong(hWndOldButton, GWL_STYLE);
			dwStyle &= ~BS_DEFPUSHBUTTON;
			dwStyle |= BS_PUSHBUTTON;
			::SendMessage(hWndOldButton, BM_SETSTYLE, (WPARAM) dwStyle, (LPARAM) TRUE);

			// instruct the dialog of the new default push button

			SendMessage(DM_SETDEFID, (WPARAM) uID);

		}
	}

	// finally set keyboard focus to the specified control

	::SetFocus(hWndNewFocus);
}

void CTaskDialog::UpdateMainIcon(HICON hIcon)
{
	if (hIcon != 0 && HasFlag(TDF_CAN_BE_MINIMIZED)) {
		SetIcon(hIcon, TRUE);
		SetIcon(hIcon, FALSE);
	}

	::SendMessage(GetDlgItem(IDC_MAIN_ICON), STM_SETICON, (WPARAM) hIcon, (LPARAM) 0L);

	// update the position of the main instruction static

	if (config_.pszMainInstruction != 0) {

		RECT rect;
		::GetWindowRect(GetDlgItem(IDC_MAIN_INSTRUCTION), &rect);
		::MapWindowPoints(::GetDesktopWindow(), m_hWnd, reinterpret_cast<LPPOINT>(&rect), sizeof(RECT)/sizeof(POINT));
		rect.left = cx_margin;
		if (hIcon != 0)
			rect.left += ::GetSystemMetrics(SM_CXICON) + cx_spacing;
		::SetWindowPos(GetDlgItem(IDC_MAIN_INSTRUCTION), 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		::ShowWindow(GetDlgItem(IDC_MAIN_ICON), (hIcon != 0 ? SW_SHOW : SW_HIDE));

	}
}

void CTaskDialog::UpdateFooterIcon(HICON hIcon)
{
	::SendMessage(GetDlgItem(IDC_FOOTER_ICON), STM_SETICON, (WPARAM) hIcon, (LPARAM) 0L);

	if (config_.pszFooter != 0) {

		RECT rect;
		::GetWindowRect(GetDlgItem(IDC_FOOTER), &rect);
		::MapWindowPoints(::GetDesktopWindow(), m_hWnd, reinterpret_cast<LPPOINT>(&rect), sizeof(RECT)/sizeof(POINT));
		rect.left = cx_margin;
		if (hIcon != 0)
			rect.left += ::GetSystemMetrics(SM_CXSMICON) + cx_smspacing;
		::SetWindowPos(GetDlgItem(IDC_FOOTER), 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		::ShowWindow(GetDlgItem(IDC_FOOTER_ICON), (hIcon != 0 ? SW_SHOW : SW_HIDE));

	}
}

void CTaskDialog::UpdateWindowTitle(LPCWSTR szWindowTitle)
{
	// if no string specified, use the executable file name

	if (szWindowTitle == 0) {
		TCHAR szExecutablePath[MAX_PATH] = { _T('\0') };
		if (::GetModuleFileName(Instance(), szExecutablePath, sizeof(szExecutablePath)/sizeof(TCHAR) - sizeof(TCHAR)) != 0)
			SetWindowText(::PathFindFileName(szExecutablePath));
	}

	// otherwise, use the specified string

	else
		SetWindowText(HeapResString(Instance(), szWindowTitle));
}

void CTaskDialog::EnableSystemClose(BOOL bEnabled)
{
	HMENU hMenu = ::GetSystemMenu(m_hWnd, bEnabled);

	if (bEnabled)
		return ;

	if (hMenu != 0) {

		int count = ::GetMenuItemCount(hMenu);
		int index;
		for (index = 0; index < count; index++) {
			DWORD dwID = ::GetMenuItemID(hMenu, index);
			if (::GetMenuItemID(hMenu, index) == SC_CLOSE)
				break;
		}

		if (index < count) {
			::RemoveMenu(hMenu, index, MF_BYPOSITION);
			{
				MENUITEMINFO mnuItemInfo = { sizeof(MENUITEMINFO), MIIM_FTYPE };
				::GetMenuItemInfo(hMenu, index - 1, TRUE, &mnuItemInfo);
				if (mnuItemInfo.fType == MFT_SEPARATOR)
					::RemoveMenu(hMenu, index - 1, MF_BYPOSITION);
			}
		}
	}
}

void CTaskDialog::CreateCallbackTimer(void)
{
	dwTickCount_ = ::GetTickCount();
	::SetTimer(m_hWnd, hTimer_, 200UL, 0);
}

//////////////////////////////////////////////////////////////////////////////
// message handlers

LRESULT CTaskDialog::OnButtonClicked(UINT uID)
{
	if (!(BOOL)Callback(TDN_BUTTON_CLICKED, (WPARAM) uID) /* cancel dialog destruction */)
		EndDialog(uID);

	return 0L;
}

LRESULT CTaskDialog::OnRadioButtonClicked(UINT uID)
{
	config_.radioButton = uID;
	Callback(TDN_RADIO_BUTTON_CLICKED, (WPARAM) uID);

	return 0L;
}

LRESULT CTaskDialog::OnVerificationChecked(void)
{
	config_.verificationChecked = (IsDlgButtonChecked(IDC_CHECK) == BST_CHECKED);
	Callback(TDN_VERIFICATION_CLICKED, (WPARAM) config_.verificationChecked);

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////
// message handlers

BOOL CTaskDialog::OnInitDialog(HWND /* hWnd */, LPARAM /* lParam */)
{
	// update dialog font

	SetFont(hFont_, FALSE);

	// calculate various dimensions

	{
		RECT rect = { 0, 0, _cx_margin, _cy_margin };
		MapDialogRect(&rect);
		cx_margin = rect.right;
		cy_margin = rect.bottom;
	}

	{
		RECT rect = { 0, 0, _cx_spacing, _cy_spacing };
		MapDialogRect(&rect);
		cx_spacing = rect.right;
		cy_spacing = rect.bottom;
	}

	{
		RECT rect = { 0, 0, _cx_smspacing, _cy_smspacing };
		MapDialogRect(&rect);
		cx_smspacing = rect.right;
		cy_smspacing = rect.bottom;
	}

	{
		RECT rect = { 0, 0, _cx_button, _cy_button };
		MapDialogRect(&rect);
		cx_button = rect.right;
		cy_button = rect.bottom;
	}

	{
		RECT rect = { 0, 0, _cx_button_margin, 0 };
		MapDialogRect(&rect);
		cx_button_margin = rect.right;
	}

	{
		RECT rect = { 0, 0, 0, _cy_progress_bar };
		MapDialogRect(&rect);
		cy_progress_bar = rect.bottom;
	}

	// create useful GDI objects

	hFont_ = GetFont();
	hFontInstruction_ = CreateSystemFont(16, FW_BOLD);
	hFontContent_ = CreateSystemFont(12);

	hBrushWhite_ = ::CreateSolidBrush(COLOR_WHITE);

	// layout controls

	LayoutWindow();

	// callback 

	Callback(TDN_CREATED);

	return FALSE;
}

void CTaskDialog::OnWindowPosChanging(LPWINDOWPOS /* lpWindowPos */)
{
	if (b_window_style_updated)
		return ;

	b_window_style_updated = true;

	// determines window styles

	DWORD dwStyle = GetWindowLong(GWL_STYLE);
	dwStyle &= ~WS_SYSMENU;
	dwStyle &= ~WS_MINIMIZEBOX;

	if (HasFlag(TDF_ALLOW_DIALOG_CANCELLATION))
		dwStyle |= WS_SYSMENU;
	if (HasFlag(TDF_CAN_BE_MINIMIZED))
		dwStyle |= WS_SYSMENU | WS_MINIMIZEBOX;

	// update dialog box window styles

	if (GetWindowLong(GWL_STYLE) != dwStyle)
		SetWindowLong(GWL_STYLE, dwStyle);
}

void CTaskDialog::OnHelp(LPHELPINFO /* lpHelpInfo */)
{
	Callback(TDN_HELP);
}

void CTaskDialog::OnTimer(UINT /* uID */, TIMERPROC /* lpTimerProc */)
{
	if ((BOOL)Callback(TDN_TIMER, ::GetTickCount() - dwTickCount_) /* RESET_TIMER */)
		dwTickCount_ = ::GetTickCount();
}

void CTaskDialog::OnSysCommand(UINT nCode, CPoint /* pt */)
{
	SetMsgHandled(FALSE);
	if ((nCode & 0xFFF0) == SC_CLOSE) {
		SetMsgHandled(TRUE);
		if (HasFlag(TDF_ALLOW_DIALOG_CANCELLATION))
			OnCancel(BN_CLICKED, IDCANCEL, m_hWnd);
	}
}

void CTaskDialog::OnDestroy(void)
{
	SetMsgHandled(FALSE);
	KillTimer(hTimer_); hTimer_ = 0;
	DestroyControls();
}

void CTaskDialog::OnNcDestroy(void)
{
	SetMsgHandled(FALSE);
	Callback(TDN_DESTROYED);
}

HBRUSH CTaskDialog::OnCtlColorStatic(HDC hDC, HWND hWnd)
{
	SetMsgHandled(FALSE);

	BOOL bBrushWhite = FALSE;
	DWORD dwID = ::GetWindowLong(hWnd, GWL_ID);

	for (DWORD cButton = 0; cButton < config_.cRadioButtons; cButton++) {
		if (dwID == config_.pRadioButtons[cButton].nButtonID)
			bBrushWhite = TRUE;
	}

	if (dwID == IDC_MAIN_ICON || dwID == IDC_MAIN_INSTRUCTION || dwID == IDC_CONTENT)
		bBrushWhite = TRUE;

	if (bBrushWhite) {

		SetMsgHandled(TRUE);

		// the main instruction is drawn in blue

		if (dwID == IDC_MAIN_INSTRUCTION)
			::SetTextColor(hDC, COLOR_BLUE);

		// these controls appear on a white background

		return hBrushWhite_;

	}

	return 0L;
}

void CTaskDialog::OnClickButton(UINT uID)
{
	if (!::IsWindow(GetDlgItem(uID)))
		return ;

	SendControlCommand(uID, BN_CLICKED);
}

void CTaskDialog::OnClickVerification(BOOL bState, BOOL bFocus)
{
	if (!::IsWindow(GetDlgItem(IDC_CHECK)))
		return ;

	CheckDlgButton(IDC_CHECK, (bState ? BST_CHECKED : BST_UNCHECKED));
	if (bFocus)
		SetControlFocus(IDC_CHECK);
}

void CTaskDialog::OnEnableButton(UINT uID, BOOL bEnabled)
{
	if (!::IsWindow(GetDlgItem(uID)))
		return ;

	::EnableWindow(GetDlgItem(uID), bEnabled);

	if (uID == IDCANCEL) {
		if (bEnabled)
			config_.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;
		else
			config_.dwFlags &= ~TDF_ALLOW_DIALOG_CANCELLATION;
		EnableSystemClose(HasFlag(TDF_ALLOW_DIALOG_CANCELLATION));
	}
}

void CTaskDialog::OnUpdateElementText(TASKDIALOG_ELEMENTS tde, LPCWSTR pszText)
{
	switch (tde) {
		case TDE_MAIN_INSTRUCTION:
			::SetWindowTextW(GetDlgItem(IDC_MAIN_INSTRUCTION), HeapResString(Instance(), pszText));
			break;
		case TDE_CONTENT:
			::SetWindowTextW(GetDlgItem(IDC_CONTENT), HeapResString(Instance(), pszText));
			break;
		case TDE_EXPANDED_INFORMATION:
			break;
		case TDE_FOOTER:
			::SetWindowTextW(GetDlgItem(IDC_FOOTER), HeapResString(Instance(), pszText));
			break;
	}
}

void CTaskDialog::OnUpdateIcon(TASKDIALOG_ICON_ELEMENTS tdie, LPARAM lParam)
{
	if (tdie = TDIE_ICON_MAIN) {

		hMainIcon_ = 0;
		
		if (HasFlag(TDF_USE_HICON_MAIN))
			config_.hMainIcon = (HICON) lParam;

		else
			config_.pszMainIcon = reinterpret_cast<LPWSTR>((LPSTR) lParam);

		UpdateMainIcon(MainIcon());
	}

	if (tdie = TDIE_ICON_FOOTER) {

		hFooterIcon_ = 0;
		
		if (HasFlag(TDF_USE_HICON_FOOTER))
			config_.hFooterIcon = (HICON) lParam;

		else
			config_.pszFooterIcon = reinterpret_cast<LPWSTR>((LPSTR) lParam);

		UpdateFooterIcon(FooterIcon());
	}
}

void CTaskDialog::OnSetProgressBarPos(UINT pos)
{
	if (!::IsWindow(GetDlgItem(IDC_PROGRESS)))
		return ;

	if ((state_ & PBST_PAUSED) == PBST_PAUSED)
		return ;

	ATLTRACE(_T("OnSetProgressBarPos(%d)\n"), pos);

	pos_ = pos;
	::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_SETPOS, (WPARAM) pos_, (LPARAM) 0L);
}

void CTaskDialog::OnSetProgressBarRange(UINT mini, UINT maxi)
{
	if (!::IsWindow(GetDlgItem(IDC_PROGRESS)))
		return ;

	min_range_ = mini;
	max_range_ = maxi;

	::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_SETRANGE, (WPARAM) 0, (LPARAM) MAKELPARAM(min_range_, max_range_));
}

void CTaskDialog::OnSetProgressBarMarquee(BOOL bMarquee, UINT nSpeed)
{
	if (!::IsWindow(GetDlgItem(IDC_PROGRESS)))
		return ;

	::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_SETMARQUEE, (WPARAM) bMarquee, (LPARAM) nSpeed);
}

void CTaskDialog::OnSetProgressBarState(UINT nState)
{
	if (!::IsWindow(GetDlgItem(IDC_PROGRESS)))
		return ;

	ATLTRACE(_T("OnSetProgressBarState(%d)\n"), nState);
	state_ = nState;

	if ((nState & PBST_PAUSED) == PBST_NORMAL && marquee_)
		OnSetProgressBarMarquee(TRUE, 25U);

	if ((nState & PBST_PAUSED) == PBST_PAUSED)
		OnSetProgressBarMarquee(FALSE, 0U);

	::SendMessage(GetDlgItem(IDC_PROGRESS), PBM_SETBARCOLOR, (WPARAM) 0, (LPARAM) (nState & PBST_ERROR ? COLOR_RED : CLR_DEFAULT));
}

void CTaskDialog::OnNavigate(UINT uMsg, LPARAM lParam)
{
	OnDestroy();
	Free();

	Allocate(reinterpret_cast<const TASKDIALOGCONFIG*>(lParam));

	Initialize();
	LayoutWindow();
	Invalidate(TRUE);

	Callback(TDN_NAVIGATED);
}

LRESULT CTaskDialog::OnHyperLinkClicked(LPNMHDR lpnmhdr)
{
	NMLINK* pnmlink = reinterpret_cast<NMLINK*>(lpnmhdr);
	Callback(TDN_HYPERLINK_CLICKED, (WPARAM) 0, (LPARAM) pnmlink->item.szUrl);
	return 0L;
}

LRESULT CTaskDialog::OnCancel(UINT /* nCode */, int /* nID */, HWND /* hWnd */)
{
	if ((::GetKeyState(VK_ESCAPE) & 0x80000) == 0 || HasFlag(TDF_ALLOW_DIALOG_CANCELLATION))
		return OnButtonClicked(IDCANCEL);
	return 0L;
}

LRESULT CTaskDialog::OnButtonClicked(UINT /* nCode */, int nID, HWND /* hWnd */)
{
	if (nID == IDC_CHECK)
		return OnVerificationChecked();

	for (DWORD cButton = 0; cButton < config_.cRadioButtons; cButton++) {
		if (nID == config_.pRadioButtons[cButton].nButtonID)
			return OnRadioButtonClicked(nID);
	}

	for (DWORD cButton = 0; cButton < config_.cButtons; cButton++) {
		if (nID == config_.pButtons[cButton].nButtonID)
			return OnButtonClicked(nID);
	}

	if (nID >= IDOK && nID <= IDCLOSE)
		return OnButtonClicked(nID);

	return 0L;
}
