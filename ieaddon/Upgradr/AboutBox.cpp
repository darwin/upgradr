// AboutBox.cpp : Implementation of CAboutBox

#include "stdafx.h"
#include "AboutBox.h"
#include "BitmapHyperLink.h"
#include "verinfo.h"

// CAboutBox
CAboutBox::CAboutBox()
{
}

CAboutBox::~CAboutBox()
{
}

LRESULT 
CAboutBox::OnClickedOK( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	EndDialog(wID);
	return 0;
}

LRESULT 
CAboutBox::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CAxDialogImpl<CAboutBox>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	DLLVERSIONINFO dllVerInfo;
	ZeroMemory(&dllVerInfo, sizeof(DLLVERSIONINFO));
	dllVerInfo.cbSize  = sizeof(DLLVERSIONINFO);
	DllGetVersion(GetBaseModule().GetModuleInstance(), &dllVerInfo);
	TCHAR version[100];
	_stprintf_s(version, _T("%d.%d.%d"), dllVerInfo.dwMajorVersion, dllVerInfo.dwMinorVersion, dllVerInfo.dwBuildNumber);
	::SetWindowText(GetDlgItem(IDC_VERSION), version);

	::SetWindowText(GetDlgItem(IDC_PRODUCTID), PRODUCT_ID_TEXT);

	::SetWindowText(GetDlgItem(IDC_CREDITS), 
		_T("CodeProject.com team\n")
		_T("Armen Hakobyan\n")
		_T("..."));
	// set icons
	CIcon icon;
	icon.LoadIcon(IDI_ABOUTBOX);
	SetIcon(icon);

	m_Homepage.SetHyperLink(_T("http://www.upgradr.com"));
	m_Homepage.SetLabel(_T("www.upgradr.com"));
	//m_Homepage.SetToolTipText(_T("go to Upgradr homepage"));
	m_Homepage.SubclassWindow(GetDlgItem(IDC_HOMEPAGE));
	m_Homepage.SetExtendedStyle(HLINK_UNDERLINEHOVER);

	m_Support.SetHyperLink(_T("mailto:support@upgradr.com"));
	m_Support.SetLabel(_T("support@upgradr.com"));
	//m_Support.SetToolTipText(_T("send email to Upgradr support"));
	m_Support.SubclassWindow(GetDlgItem(IDC_SUPPORT));
	m_Support.SetExtendedStyle(HLINK_UNDERLINEHOVER);

	//HBITMAP hBmp = m_Homepage.AddBitmap(IDB_TOOLBAR);
	//ATLASSERT(hBmp);

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT 
CAboutBox::OnEraseBackground( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	return 1; // don't clear background
}

LRESULT 
CAboutBox::OnCtlColor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	HDC hDC = (HDC)wParam;
	HWND hWnd = (HWND)lParam;

	if(uMsg == WM_CTLCOLORDLG)
	{
		bHandled = TRUE;
		return (LRESULT)(HBRUSH)GetStockObject(WHITE_BRUSH);
	}
	else if(uMsg == WM_CTLCOLORSTATIC)
	{
		SetBkMode(hDC,TRANSPARENT);
		bHandled = TRUE;
		return (LRESULT)(HBRUSH)GetStockObject(WHITE_BRUSH);
	}
	else
		return 0;
}