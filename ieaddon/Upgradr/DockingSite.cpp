///////////////////////////////////////////////////////////////
//
// MouseTracker.cpp
//
// Created: 08/07/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DockingSite.h"

LRESULT 
CIEServer::OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = (int)(short) LOWORD(lParam);   // horizontal position 
	int yPos = (int)(short) HIWORD(lParam);   // vertical position 
	TRACE_I(FS(_T("OnMove %d, %d"), xPos, yPos));
	m_DockingSite->SetWindowPos(NULL, 10, 10, 500, 500, 0);
//	m_DockingSite->GetWatcher()->SetIEWindowPos(0, 0, 500, 500);
	return S_OK;
}

LRESULT 
CIEServer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = (int)(short) LOWORD(lParam);   // horizontal position 
	int yPos = (int)(short) HIWORD(lParam);   // vertical position 
	TRACE_I(FS(_T("OnSize %d, %d"), xPos, yPos));
	m_DockingSite->SetWindowPos(NULL, 10, 10, 500, 500, 0);
	//m_DockingSite->GetWatcher()->SetIEWindowPos(0, 0, 500, 500);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

CDockingSite::CDockingSite() :
m_IsBound(false),
m_Watcher(NULL)
{
}

CDockingSite::~CDockingSite()
{
	if( m_IsBound )
	{
		if (!!m_IEServer) m_IEServer.UnsubclassWindow();
		UnsubclassWindow();
	}
}

LRESULT CDockingSite::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	if (m_IsBound)
	{
		//
		// force the unsubclass on shutdown - even if somebody else
		// has subclassed this window, we need to make sure we detach
		// from it otherwise things go bang in the destructor
		//
		if (!!m_IEServer) m_IEServer.UnsubclassWindow(TRUE);
		UnsubclassWindow(TRUE);
		m_IsBound = false;
	}

	return 0;
}

LRESULT CDockingSite::OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
//	m_Watcher->OnWindowPosChanging(uMsg, wParam, lParam, bHandled);
	//WINDOWPOS* pWindowPos = (WINDOWPOS*)lParam;
	//pWindowPos->flags|=(SWP_NOSIZE|SWP_NOMOVE);
	return S_OK; 
}

LRESULT 
CDockingSite::OnSetFocus( UINT, WPARAM, LPARAM, BOOL & )
{
	return 0;
}

LRESULT 
CDockingSite::OnKillFocus( UINT, WPARAM, LPARAM, BOOL & )
{
	return 0;
}

HRESULT 
CDockingSite::Advise( HWND hWnd, CDockingSite::Watcher *pWatcher )
{
	if (SubclassWindow(hWnd))
	{
		m_Watcher = pWatcher;
		m_IsBound = true;

		InitializeDockingFrame();

		return S_OK;
	}
	else
	{
		ATLASSERT( FALSE );
	}

	return E_FAIL;
}

HRESULT 
CDockingSite::Unadvise()
{
	if (m_IsBound)
	{
		if (!!m_IEServer) m_IEServer.UnsubclassWindow();
		UnsubclassWindow();
		m_IsBound = false;
	}
	return S_OK;
}

HRESULT 
CDockingSite::SetIEServer(HWND hWnd)
{
	//m_IEServer.SetDockingSite(this);
	//m_IEServer.SubclassWindow(hWnd);
	//m_IEServer.SetWindowPos(NULL, 10, 10, 500, 500, 0);
	return S_OK;
}

HRESULT 
CDockingSite::UnsetIEServer()
{
	//m_IEServer.SetDockingSite(NULL);
	//m_IEServer.UnsubclassWindow(TRUE);
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
