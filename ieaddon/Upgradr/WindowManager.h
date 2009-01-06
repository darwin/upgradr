#pragma once
#include "ManagedWindow.h"
#include "DOMExplorerWindow.h"
#include "ConsoleWindow.h"
#include "PlainTextView.h"

class CUIThread;
class CWindowManager;

typedef hash_map<TWindowId, CManagedWindowHandle*> TManagedWindowMap;
typedef hash_map<TBrowserId, TWindowId>         TWindowIdMap;

typedef CWinTraits<WS_OVERLAPPED, 0> TMessageWindowTraits; // must be hidden

#define WMM_BASE											(WM_APP+2000)
// script site related
#define WMM_INITSCRIPTSITE								(WMM_BASE+0)
#define WMM_DONESCRIPTSITE								(WMM_BASE+1)
// window related
#define WMM_CREATEDOMEXPLORERWINDOW					(WMM_BASE+10)
#define WMM_CREATECONSOLEWINDOW						(WMM_BASE+11)
#define WMM_CREATEPLAINTEXTVIEW						(WMM_BASE+12)
#define WMM_LAST											(WMM_BASE+100) // !!!

class CInitScriptSiteParams {
public:
	CInitScriptSiteParams(TScriptSite* site, CString language, TFrameId frameId, TScriptId scriptId, CScript* script) : 
		m_Site(site), m_Language(language), m_FrameId(frameId), m_ScriptId(scriptId), m_Script(script) {}

	CString													m_Language;
	TFrameId													m_FrameId;
	TScriptId												m_ScriptId;
	CScript*													m_Script;

	TScriptSite*											m_Site;
};


class CMessageWindow : public CWindowImpl<CMessageWindow, CWindow, TMessageWindowTraits> {
public:
	CMessageWindow(CWindowManager* manager) : m_WindowManager(manager) {}

	DECLARE_WND_CLASS(MESSAGE_WINDOW_CLASS_NAME);
	BEGIN_MSG_MAP(CMessageWindow)
		MESSAGE_RANGE_HANDLER_EX(WMM_BASE, WMM_LAST, UIThreadProc)
	END_MSG_MAP()

	LRESULT													UIThreadProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CWindowManager*                              m_WindowManager;
};

// singleton: stara se o okenka a dokovani
// ma UI thread ktery vytvari okna a stara se o jejich zpravy - toto je pozadavek dokovaciho manageru, jinak to crashuje
// ostatni thready mohou davat pozadavky na vytvareni oken, pristup k objektu je chranen kritickou sekci, ktera resi pristup z UI versus ostatni thread
class CWindowManager : public CResourceInit<SR_WINDOWMANAGER> {
public:
	CWindowManager();
	~CWindowManager();

	virtual bool											Init();
	virtual bool											Done();

	CManagedWindow<CDOMExplorerWindow>*				GetDOMExplorerWindow(TBrowserId browserId);
	CManagedWindow<CConsoleWindow>*              GetConsoleWindow(TBrowserId browserId);
	CManagedWindow<CPlainTextView>*              GetPlainTextView(TBrowserId browserId);

	CManagedWindow<CDOMExplorerWindow>*				CreateDOMExplorerWindow(TBrowserId browserId, HWND parent);
	CManagedWindow<CConsoleWindow>*              CreateConsoleWindow(TBrowserId browserId, HWND parent);
	CManagedWindow<CPlainTextView>*              CreatePlainTextView(TBrowserId browserId, HWND parent);

	HWND														GetMessageWindowHWND();

	bool                                         RemoveBrowserWindows(TBrowserId browserId);

	// utilities
	bool                                         RefreshConsoles();
	bool                                         RefreshDOMExplorerWindow(TBrowserId browserId, IStream* pStream);

	void                                         Float(TBrowserId browserId, HWND workspace);

protected:
	template<class T>	CManagedWindow<T>*			InsertWindow();
	template<class T>	CManagedWindow<T>*			FindWindow(TWindowId id);
	bool                                         RemoveWindow(TWindowId id);

	TManagedWindowMap::iterator                  FindWindowType(TBrowserId browserId, EWindowFamily family);

	TWindowId                                    AllocWindowId();
	bool                                         ReleaseWindowId(TWindowId id);

	bool														RunUIThread();
	bool														KillUIThread();

	//////////////////////////////////////////////////////////////////////////
	// these functions must be entered from UI thread !!!	

	CManagedWindow<CDOMExplorerWindow>*				CreateDOMExplorerWindowUI(TBrowserId browserId, HWND parent);
	CManagedWindow<CConsoleWindow>*              CreateConsoleWindowUI(TBrowserId browserId, HWND parent);
	CManagedWindow<CPlainTextView>*              CreatePlainTextViewUI(TBrowserId browserId, HWND parent);

	HRESULT													InitScriptSite(CInitScriptSiteParams* params);
	HRESULT													DoneScriptSite(TScriptSite* site);


public:
	LRESULT													UIThreadProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool														CreateMessageWindow();
	bool														DestroyMessageWindow();

	bool                                         SetReadyState(bool state);

	//////////////////////////////////////////////////////////////////////////

private:
	CFont                                        m_Font;
	CImageList												m_Icons;
	TWindowId                                    m_LastWindowId;
	TManagedWindowMap                            m_WindowsMap;
	CCS                                          m_CS;
	CMessageWindow                               m_MessageWindow;
	bool                                         m_Ready;
	CUIThread*                                   m_UIThread;
	bool														m_Inited;
};

//////////////////////////////////////////////////////////////////////////

template<class T> CManagedWindow<T>*
CWindowManager::InsertWindow()
{
	CSGUARD(m_CS);
	CManagedWindow<T>* pWindow = new CManagedWindow<T>();
	CManagedWindowHandle* pWindowHandle = pWindow;
	TWindowId windowId = AllocWindowId();
	pWindowHandle->m_Id = windowId;
	m_WindowsMap.insert(make_pair(windowId, pWindowHandle));
	return pWindow;
}

template<class T> CManagedWindow<T>* 
CWindowManager::FindWindow( TWindowId id )
{
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = m_WindowsMap.find(id);
	if (i==m_WindowsMap.end()) return NULL;
	return (CManagedWindow<T>*)i->second;
}