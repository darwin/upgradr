#include "StdAfx.h"
#include "WindowManager.h"
#include "Thread.h"
#include "UpgradrScriptSite.h"

//#include "Debug.h"

#define MUST_BE_UI_THREAD() ATLASSERT(GetCurrentThreadId()==m_UIThread->GetId());
#define MUST_BE_OTHER_THREAD() ATLASSERT(GetCurrentThreadId()!=m_UIThread->GetId());

DWORD gUIThreadId = 0; // for debug purposes
HWND gUIThreadHWND = 0;

class CUIThread : public CGuiThreadImpl<CUIThread> {
public:
	CUIThread(CWindowManager* manager) : m_WindowManager(manager), CGuiThreadImpl<CUIThread>(CREATE_SUSPENDED) {}

	BOOL InitializeThread()
	{
		gUIThreadId = GetId();

		// this thread may use COM, we need to join STA apartment
		// see: http://discuss.develop.com/archives/wa.exe?A2=ind0212c&L=atl&T=0&P=3696
		OleInitialize(NULL);

		// create message window
		m_WindowManager->CreateMessageWindow();
		m_WindowManager->SetReadyState(true);
		return TRUE;
	}

	void CleanupThread(DWORD dwExitCode)
	{ 
		// destroy message window
		m_WindowManager->SetReadyState(false);
		m_WindowManager->DestroyMessageWindow();

		// leave appartment
		OleUninitialize();

		gUIThreadId = 0;
	}

private:
	CWindowManager*										m_WindowManager;
};

#define REPLY2MESSAGE(res) ReplyMessage((LRESULT)res); bHasReplied = true;

LRESULT													
CWindowManager::UIThreadProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ATLASSERT(InSendMessage()); // all our messages must be synchronous
	bool bHasReplied = false;
	switch (uMsg) {
		case WMM_CREATEDOMEXPLORERWINDOW:	REPLY2MESSAGE(CreateDOMExplorerWindowUI((TBrowserId)wParam, (HWND)lParam));	break;
		case WMM_CREATECONSOLEWINDOW:			REPLY2MESSAGE(CreateConsoleWindowUI((TBrowserId)wParam, (HWND)lParam));	break;
		case WMM_CREATEPLAINTEXTVIEW:			REPLY2MESSAGE(CreatePlainTextViewUI((TBrowserId)wParam, (HWND)lParam));	break;
		case WMM_INITSCRIPTSITE:				REPLY2MESSAGE(InitScriptSite((CInitScriptSiteParams*)lParam)); break;
		case WMM_DONESCRIPTSITE:				REPLY2MESSAGE(DoneScriptSite((TScriptSite*)lParam)); break;
		default:
			ATLASSERT(0);
			return FALSE; // not handled
	}
	ATLASSERT(bHasReplied); // we need to reply for our messages
	return TRUE; // handled
}

bool														
CWindowManager::CreateMessageWindow()
{
	MUST_BE_UI_THREAD();
	ATLASSERT(!m_MessageWindow);
	CRect rcDefault(0,0,10,10);
	m_MessageWindow.Create(NULL, rcDefault, MESSAGE_WINDOW_WINDOW_NAME);
	ATLASSERT(!!m_MessageWindow);
	gUIThreadHWND = m_MessageWindow;
	return true;
}

bool														
CWindowManager::DestroyMessageWindow()
{
	MUST_BE_UI_THREAD();
	ATLASSERT(!!m_MessageWindow);
	m_MessageWindow.DestroyWindow();
	ATLASSERT(!m_MessageWindow);
	gUIThreadHWND = 0;
	return true;
}

bool                                         
CWindowManager::SetReadyState(bool state)
{
	MUST_BE_UI_THREAD();
	m_Ready = state;
	return true;
}

LRESULT 
CMessageWindow::UIThreadProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ATLASSERT(!!m_WindowManager);
	return m_WindowManager->UIThreadProc(uMsg, wParam, lParam);
}
//////////////////////////////////////////////////////////////////////////

CWindowManager::CWindowManager():
m_Inited(false),
m_LastWindowId(NULL_WINDOW),
m_UIThread(NULL),
m_Ready(false),
m_MessageWindow(this)
{
	m_Icons.Create(IDB_MSDEV_TAB_ICONS, 16, 4, RGB(0,255,0));
	m_Font.CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,	OUT_STROKE_PRECIS, CLIP_STROKE_PRECIS, PROOF_QUALITY, FF_MODERN, NULL);
}

CWindowManager::~CWindowManager()
{
}

bool														
CWindowManager::RunUIThread()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_UIThread) return false; // already running
	m_UIThread = new CUIThread(this);
	if (!m_UIThread) return false;

	// run thread
	m_UIThread->Resume();
	
	// wait for ready state
	int iCounter = 0;
	while (iCounter<100)  // 2sec of total time
	{
		if (m_Ready) return true;
		Sleep(WAIT_GRANULARITY); // give other thread some chance to respond
		iCounter++;
	} 

	ATLASSERT(0);
	// TODO: inform user that timeout has elapsed
	return false;
}

bool														
CWindowManager::KillUIThread()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_UIThread) return false;

	// want quit the thread
	m_UIThread->PostQuitMessage();

	// wait for thread to finish
	if (m_UIThread->Join(2000)==WAIT_TIMEOUT) // give the thread a couple of seconds to close
	{
		// the thread didn't respond to WM_QUIT
		DWORD dwForcefulExitCode = 1;        
		m_UIThread->Terminate(dwForcefulExitCode);
	} 

	delete m_UIThread;
	m_UIThread = NULL;
	m_Ready = false;
	return true;
}

bool 
CWindowManager::Init()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_Inited) return true;
	if (!RunUIThread()) 
	{
		TRACE_E(_T("Cannot run UI thread !"));
		return false;
	}
	m_Inited = true;
	return true;
}

bool 
CWindowManager::Done()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return true;
	KillUIThread();
	return true;
}

TWindowId 
CWindowManager::AllocWindowId()
{
	CSGUARD(m_CS);
	return ++m_LastWindowId;
}

bool 
CWindowManager::ReleaseWindowId( TWindowId id )
{
	CSGUARD(m_CS);
	return true;
}

bool 
CWindowManager::RemoveWindow( TWindowId id )
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	ATLASSERT(m_WindowsMap.find(id)!=m_WindowsMap.end());
	m_WindowsMap.erase(id);
	return true;
}

CManagedWindow<CDOMExplorerWindow>* 
CWindowManager::CreateDOMExplorerWindow(TBrowserId browser, HWND parent)
{
	CHECK_THREAD_OWNERSHIP;
	MUST_BE_OTHER_THREAD();
	// send request to UI thread
	HWND wnd = GetMessageWindowHWND();
	ResourceUnlock(SR_WINDOWMANAGER);
	LRESULT res = ::SendMessage(wnd, WMM_CREATEDOMEXPLORERWINDOW, (WPARAM)browser, (LPARAM)parent);
	return (CManagedWindow<CDOMExplorerWindow>*)res;
}

CManagedWindow<CDOMExplorerWindow>* 
CWindowManager::CreateDOMExplorerWindowUI(TBrowserId browser, HWND parent)
{
	MUST_BE_UI_THREAD();
	CSGUARD(m_CS);
	// DOM Exporer
	CManagedWindow<CDOMExplorerWindow>* pDOMExplorer;
	pDOMExplorer = InsertWindow<CDOMExplorerWindow>();
	ATLASSERT(pDOMExplorer);

	pDOMExplorer->SetBrowserId(browser);
	pDOMExplorer->Create(parent, CRect(0, 0, 500, 400), DOMEXPLORER_WINDOW_NAME);
	pDOMExplorer->Init(parent, EWF_DOMEXPLORER, browser, _T("DOM Explorer"), m_Icons.ExtractIcon(2), CRect(0, 0, 500, 400));
	pDOMExplorer->RequestDOM();

	return pDOMExplorer;
}

CManagedWindow<CConsoleWindow>* 
CWindowManager::CreateConsoleWindow(TBrowserId browser, HWND parent)
{
	CHECK_THREAD_OWNERSHIP;
	MUST_BE_OTHER_THREAD();
	// send request to UI thread
	HWND wnd = GetMessageWindowHWND();
	ResourceUnlock(SR_WINDOWMANAGER);
	LRESULT res = ::SendMessage(wnd, WMM_CREATECONSOLEWINDOW, (WPARAM)browser, (LPARAM)parent);
	return (CManagedWindow<CConsoleWindow>*)res;
}

CManagedWindow<CConsoleWindow>* 
CWindowManager::CreateConsoleWindowUI(TBrowserId browser, HWND parent)
{
	MUST_BE_UI_THREAD();
	CSGUARD(m_CS);
	CManagedWindow<CConsoleWindow>* pConsoleWindow;
	pConsoleWindow = InsertWindow<CConsoleWindow>();
	ATLASSERT(pConsoleWindow);

	DWORD consoleStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN;
	pConsoleWindow->Create(parent, CRect(0, 0, 800, 300), CONSOLE_WINDOW_NAME, consoleStyle);
	pConsoleWindow->SetFont(m_Font);
	pConsoleWindow->Init(parent, EWF_CONSOLE, browser, _T("Console"), m_Icons.ExtractIcon(7), CRect(0, 0, 800, 300));

	return pConsoleWindow;
}

CManagedWindow<CPlainTextView>* 
CWindowManager::CreatePlainTextView(TBrowserId browser, HWND parent)
{
	CHECK_THREAD_OWNERSHIP;
	MUST_BE_OTHER_THREAD();
	// send request to UI thread
	HWND wnd = GetMessageWindowHWND();
	ResourceUnlock(SR_WINDOWMANAGER);
	LRESULT res = ::SendMessage(wnd, WMM_CREATEPLAINTEXTVIEW, (WPARAM)browser, (LPARAM)parent);
	return (CManagedWindow<CPlainTextView>*)res;
}

CManagedWindow<CPlainTextView>* 
CWindowManager::CreatePlainTextViewUI(TBrowserId browser, HWND parent)
{
	MUST_BE_UI_THREAD();
	CSGUARD(m_CS);
	CManagedWindow<CPlainTextView>* pPlainTextView;
	pPlainTextView = InsertWindow<CPlainTextView>();
	ATLASSERT(pPlainTextView);

	pPlainTextView->Create(parent, CRect(0, 0, 500, 400), _T("plain text view class"));
	pPlainTextView->Init(parent, EWF_GENERAL, browser, _T("Plain Text"), m_Icons.ExtractIcon(11), CRect(0, 0, 500, 400));

	return pPlainTextView;
}

bool                                         
CWindowManager::RemoveBrowserWindows(TBrowserId browserId)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = m_WindowsMap.begin();
	while (i!=m_WindowsMap.end())
	{
		CManagedWindowHandle* pWindowHandle = i->second;
		if (pWindowHandle->m_BrowserId==browserId)
		{
			++i;
			RemoveWindow(pWindowHandle->m_Id); // this must not invalidate iterators
			continue;
		}
		++i;
	}
	
	return true;
}

TManagedWindowMap::iterator                  
CWindowManager::FindWindowType(TBrowserId browserId, EWindowFamily family)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = m_WindowsMap.begin();
	while (i!=m_WindowsMap.end())
	{
		CManagedWindowHandle* pWindowHandle = i->second;
		if (pWindowHandle->m_BrowserId==browserId && pWindowHandle->m_Family==family)
		{
			break;
		}
		++i;
	}
	return i;
}

CManagedWindow<CDOMExplorerWindow>*                
CWindowManager::GetDOMExplorerWindow(TBrowserId browserId)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = FindWindowType(browserId, EWF_DOMEXPLORER);
	if (i==m_WindowsMap.end()) return NULL;

	return (CManagedWindow<CDOMExplorerWindow>*)i->second;
}

CManagedWindow<CConsoleWindow>*                
CWindowManager::GetConsoleWindow(TBrowserId browserId)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = FindWindowType(browserId, EWF_CONSOLE);
	if (i==m_WindowsMap.end()) return NULL;

	return (CManagedWindow<CConsoleWindow>*)i->second;
}

CManagedWindow<CPlainTextView>*                
CWindowManager::GetPlainTextView(TBrowserId browserId)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = FindWindowType(browserId, EWF_GENERAL);
	if (i==m_WindowsMap.end()) return NULL;

	return (CManagedWindow<CPlainTextView>*)i->second;
}

bool                                         
CWindowManager::RefreshConsoles()
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = m_WindowsMap.begin();
	while (i!=m_WindowsMap.end())
	{
		CManagedWindowHandle* pWindowHandle = i->second;
		if (pWindowHandle->m_Family==EWF_CONSOLE)
		{
			CManagedWindow<CConsoleWindow>* pConsoleWindow = (CManagedWindow<CConsoleWindow>*)pWindowHandle;
			pConsoleWindow->Refresh();
		}
		++i;
	}

	return true;
}

bool 
CWindowManager::RefreshDOMExplorerWindow(TBrowserId browserId, IStream* pStream)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	CManagedWindow<CDOMExplorerWindow>* pDOMExporerWindow = GetDOMExplorerWindow(browserId);
	if (!pDOMExporerWindow)
	{
		// don't forget to flush the stream 
		// and release the document in the stream !
		IHTMLDocument2* pDoc = NULL;
		HRESULT hr = CoGetInterfaceAndReleaseStream(pStream, IID_IHTMLDocument2, (void**)&pDoc);
		pDoc->Release();
		return false;
	}
	pDOMExporerWindow->PostMessage(WM_UPDATEDOM, 0, (LPARAM)pStream);
	return true;
}

void                                         
CWindowManager::Float(TBrowserId browserId, HWND workspace)
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	TManagedWindowMap::iterator i = m_WindowsMap.begin();
	while (i!=m_WindowsMap.end())
	{
		CManagedWindowHandle* pWindowHandle = i->second;
		if (pWindowHandle->m_BrowserId==browserId)
		{
			HWND topMost;
			if (pWindowHandle->GetPaneWorkspace(topMost)==workspace)
			{
				pWindowHandle->GetPane().Float();// .m_docker.Undock();
			}
		}
		++i;
	}
}

HWND 
CWindowManager::GetMessageWindowHWND()
{
	CHECK_THREAD_OWNERSHIP;
	CSGUARD(m_CS);
	ATLASSERT(m_MessageWindow.m_hWnd); 
	return m_MessageWindow.m_hWnd;
}

HRESULT 
CWindowManager::InitScriptSite(CInitScriptSiteParams* params)
{
	ATLASSERT(I_AM_UI_THREAD);
	HRESULT res = params->m_Site->InitScriptSite(CComBSTR(params->m_Language), params->m_FrameId, params->m_ScriptId, params->m_Script);
	delete params;
	return res;
}

HRESULT 
CWindowManager::DoneScriptSite(TScriptSite* site)
{
	ATLASSERT(I_AM_UI_THREAD);
	return site->Done();
}