#pragma once

#include "WorkspaceWindow.h"

#define IDC_HELPERBAR_WORKSPACE                         1

class CHelperbarWindow : public CWindowImpl<CHelperbarWindow, CWindow> {
public:
	enum EDisplayMode {
		EDM_PINNED,
		EDM_FLOATING
	};

	CHelperbarWindow();
	virtual ~CHelperbarWindow();

	DECLARE_WND_CLASS(HELPERBAR_MAIN_CLASS_NAME)

	BEGIN_MSG_MAP(CHelperbarWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
	END_MSG_MAP()

	virtual LRESULT										OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT										OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT										OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	bool														SetBrowserId(TBrowserId browserId);
	bool                                         SetDisplayMode(EDisplayMode mode);

	virtual bool                                 Unpin();
	
protected:
	virtual void											UpdateLayout();

	CWorkspaceWindow										m_Workspace;
	TBrowserId                                   m_BrowserId;
	EDisplayMode                                 m_DisplayMode;
};