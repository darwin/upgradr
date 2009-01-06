#pragma once

#define UM_POSTCREATE                           WM_USER+333
class CWorkspaceWindow : 
	public dockwins::CDockingSiteImpl<CWorkspaceWindow>
{
	typedef dockwins::CDockingSiteImpl<CWorkspaceWindow> baseClass;
public:
	CWorkspaceWindow();
	~CWorkspaceWindow();

	DECLARE_WND_CLASS_EX(HELPERBAR_WORKSPACE_CLASS_NAME, 0, COLOR_APPWORKSPACE)

	BEGIN_MSG_MAP(CWorkspaceWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(UM_POSTCREATE, OnPostCreate)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT										         OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT													OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT													OnPostCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	bool                                         LetAllPanesFloat();

	void 														InitializeDefaultPanes();
	void														UninitializeDefaultPanes();

	bool														SetBrowserId(TBrowserId browserId);

protected:
	TBrowserId                                   m_BrowserId;
	bool                                         m_Ready;
};
