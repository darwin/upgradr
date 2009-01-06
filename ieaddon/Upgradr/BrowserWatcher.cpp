///////////////////////////////////////////////////////////////
//
// BrowserWatcher.cpp
//
// Created: 23/09/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrowserWatcher.h"
#include "WindowHelper.h"

#include "BrowserWatcher50.h"
#include "BrowserWatcher55.h"
#include "BrowserWatcher60.h"
#include "BrowserWatcher70.h"

namespace
{
	HWND get_HWND(IWebBrowser2 *pWebBrowser)
	{
		HWND hWnd = NULL;
		pWebBrowser->get_HWND(reinterpret_cast< long * >(&hWnd));
		return hWnd;
	}

	LPCTSTR     CLASS_SHELL_DOC_VIEW    = _T( "Shell DocObject View" );
	LPCTSTR     CLASS_SHELL_EMBEDDING   = _T( "Shell Embedding" );
	LPCTSTR     CLASS_IE_SERVER         = _T( "Internet Explorer_Server" );
}

BrowserWatcher::BrowserWatcher(IWebBrowser2 *pWebBrowser, CDockingSite::Watcher *pObserver) :
m_pWebBrowser(pWebBrowser),
m_pObserver(pObserver),
m_messageHook()
{
	m_messageHook.Initialise( this );
}

BrowserWatcher::~BrowserWatcher()
{
	m_messageHook.Shutdown();
}

BrowserWatcher* 
BrowserWatcher::CreateInstance(IWebBrowser2 *pWebBrowser, CDockingSite::Watcher *pObserver)
{
	if( pWebBrowser == NULL )
	{
		return NULL;
	}

	HWND    hWnd = get_HWND( pWebBrowser );
	if( hWnd == NULL )
	{
		return NULL;
	}

	WindowHelper::VersionInfo   versionInfo;
	if( WindowHelper::GetWindowModuleVersion( hWnd, versionInfo ) == false )
	{
		return NULL;
	}

	switch (versionInfo.majorVersion) {
	case 5:
		if (versionInfo.minorVersion == 0)
		{
			return new BrowserWatcher50(pWebBrowser, pObserver);
		}
		else
		{
			return new BrowserWatcher55(pWebBrowser, pObserver);
		}
		break;

	case 6:
		return new BrowserWatcher60(pWebBrowser, pObserver);
		break;

	case 7:
		return new BrowserWatcher70(pWebBrowser, pObserver);
		break;
	}

	return NULL;
}

HWND BrowserWatcher::GetHwnd() const
{
	return get_HWND( m_pWebBrowser );
}

static HWND token = NULL;
void BrowserWatcher::OnCreate( HWND hWnd )
{
	tstring className = WindowHelper::GetClassName( hWnd );

	if (className == CLASS_SHELL_DOC_VIEW)
	{
		//
		// attach a gesture tracker to the DOC_VIEW window so
		// we can use mouse gestures while we're waiting for
		// a page to load
		//
		Remove( hWnd );

		CDockingSite *pTracker = new CDockingSite();
		pTracker->Advise(hWnd, m_pObserver);
		m_gestureTracker[hWnd] = pTracker;
	}
	else if (className == CLASS_IE_SERVER)
	{
		HWND parentWnd = ::GetParent(hWnd);
		// try to find parent in tracked windows
		TrackersMap::iterator lookup = m_gestureTracker.find(parentWnd);
		ATLASSERT(lookup!=m_gestureTracker.end());
		if (lookup!=m_gestureTracker.end())
		{
			lookup->second->SetIEServer(hWnd);
			token = hWnd;
		}
	}
}

void BrowserWatcher::OnDestroy(HWND hWnd)
{
	if (hWnd==token)
	{
		HWND parentWnd = ::GetParent(hWnd);
		// try to find parent in tracked windows
		TrackersMap::iterator lookup = m_gestureTracker.find(parentWnd);
		ATLASSERT(lookup!=m_gestureTracker.end());
		if (lookup!=m_gestureTracker.end())
		{
			lookup->second->UnsetIEServer();
			token = hWnd;
		}
	}
	else
	{
		Remove(hWnd);
	}
}

IWebBrowser2* 
BrowserWatcher::GetWebBrowser() const
{
	return m_pWebBrowser;
}

void 
BrowserWatcher::Remove(HWND hWnd)
{
	TrackersMap::iterator it = m_gestureTracker.find(hWnd);
	if (it != m_gestureTracker.end())
	{
		(*it).second->Unadvise();
		delete (*it).second;
		m_gestureTracker.erase(it);
	}
}