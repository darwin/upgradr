#pragma once

class CDOMRenderWindow : public CWindowImpl<CDOMRenderWindow> {
public:
	CDOMRenderWindow(void);
	~CDOMRenderWindow(void);

	DECLARE_WND_CLASS(_T("CDOMRenderWindow"))

	BEGIN_MSG_MAP(CDOMRenderWindow)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
