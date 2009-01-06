#pragma once

#define BMM_BASE											(WM_APP+4000)
#define BMM_REQUESTDOMPROVIDER 						(BMM_BASE+0)
#define BMM_LAST											(BMM_BASE+100) // !!!

typedef CWinTraits<WS_OVERLAPPED, 0> TBrowserWindowTraits; // must be hidden

// CBrowserMessageWindow je okno vytvorene threadem prislusneho BHO/helperbaru
// na tomto okne budeme zpracovavat ukoly, ktere musi delat prislusny thread pro zbytek systemu
// napriklad prochazeni DOMu a dodani vysledku DOMExploreru
class CBrowserMessageWindow : public CWindowImpl<CBrowserMessageWindow, CWindow, TBrowserWindowTraits> {
public:
	CBrowserMessageWindow(IUnknown* browserInterface, CUpgradrBHO* pBHO);
	CBrowserMessageWindow(IUnknown* browserInterface, CUpgradrHelperbar* pHelperbar);
	virtual ~CBrowserMessageWindow();

	DECLARE_WND_CLASS(BROWSER_MESSAGE_WINDOW_CLASS_NAME);
	BEGIN_MSG_MAP(CBrowserMessageWindow)
		MESSAGE_RANGE_HANDLER_EX(BMM_BASE, BMM_LAST, WindowProc)
	END_MSG_MAP()

	LRESULT													WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	CUpgradrBHO*										GetBHO() const { return m_BHO; }
	void                                         SetBHO(CUpgradrBHO* pBHO);

	CUpgradrHelperbar*								GetHelperbar() const { return m_Helperbar; }
	void                                         SetHelperbar(CUpgradrHelperbar* pHelperbar) { m_Helperbar = pHelperbar; }

	unsigned int                                 AddRef() { return ++m_RefCount; }
	unsigned int                                 DecRef() { return --m_RefCount; }
	unsigned int                                 RefCount() const { return m_RefCount; }

	IUnknown*												GetBrowserInterface() const { return m_BrowserInterface; }
	DWORD														GetThreadId() { return m_ThreadId; }

protected:
	bool														CreateMessageWindow();
	bool														DestroyMessageWindow();

	unsigned int                                 m_RefCount;
	IUnknown*                                    m_BrowserInterface; ///< IUnknown to top level browser
	CUpgradrBHO*                            m_BHO;
	CUpgradrHelperbar*                      m_Helperbar;
	DWORD														m_ThreadId; ///< browser's thread
};

typedef hash_map<TBrowserId, CBrowserMessageWindow*> TBrowserMessageWindowMap;

//////////////////////////////////////////////////////////////////////////


class CBrowserManager : public CResourceInit<SR_BROWSERMANAGER>,
								public WinTrace
{
public:
	CBrowserManager();
	~CBrowserManager();

	TBrowserId                                   AllocBrowserId(IUnknown* browserInterface, CUpgradrBHO* pBHO);
	TBrowserId                                   AllocBrowserId(IUnknown* browserInterface, CUpgradrHelperbar* pHelperbar);
	bool                                         ReleaseBrowserId(TBrowserId browserId);
	CBrowserMessageWindow*								FindBrowserMessageWindow(TBrowserId browserId);
	bool														IsBrowserThread(DWORD threadId, TBrowserId browserId);

	// utilities
	bool                                         RequestDOM(TBrowserId browserId);

protected:
	TBrowserId                                   AllocBrowserId(IUnknown* browserInterface, CUpgradrBHO* pBHO, CUpgradrHelperbar* pHelperbar);
	TBrowserMessageWindowMap::iterator				FindBrowserId(IUnknown* browserInterface);

protected:
	TBrowserId                                   m_NextId;
	TBrowserMessageWindowMap							m_Browsers;
};
