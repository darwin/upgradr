// AboutBox.h : Declaration of the CAboutBox

#pragma once

#include "resource.h"       // main symbols
#include <atlhost.h>
#include "BitmapHyperLink.h"

// CAboutBox
class CAboutBox : public CAxDialogImpl<CAboutBox> {
public:
	CAboutBox();
	~CAboutBox();

	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedOK)
		CHAIN_MSG_MAP(CAxDialogImpl<CAboutBox>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	CBitmapHyperLink                               m_Homepage;
	CBitmapHyperLink                               m_Support;
};