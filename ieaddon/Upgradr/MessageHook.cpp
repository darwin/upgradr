///////////////////////////////////////////////////////////////
//
// MessageHook.cpp
//
// Created: rhare - 21/07/2005
// Copyright (c) 2005 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageHook.h"

MessageHook::MessageHook() :
    WindowHook(),
    m_hook( NULL ),
    m_pObserver( NULL )
{
}

MessageHook::~MessageHook()
{
    Shutdown();
}

void MessageHook::Initialise( Observer *pObserver )
{
    m_pObserver = pObserver;
    SetWindowsHookEx( WH_CALLWNDPROCRET );
}

void MessageHook::Shutdown()
{
    UnhookWindowsHookEx();
}

LRESULT MessageHook::OnHook( int nCode, WPARAM wParam, LPARAM lParam )
{
    if( nCode == HC_ACTION )
    {
        if( lParam && m_pObserver )
        {
            CWPRETSTRUCT    *pCWPR = reinterpret_cast< CWPRETSTRUCT * >( lParam );

            switch( pCWPR->message )                
            {
            case WM_CREATE:
            case WM_DESTROY:
                if( pCWPR->message == WM_CREATE )
                {
                    if( pCWPR->lResult != -1 )
                    {
                        m_pObserver->OnCreate( pCWPR->hwnd );
                    }
                }
                else
                {
                    if( pCWPR->lResult == 0 )
                    {
                        m_pObserver->OnDestroy( pCWPR->hwnd );
                    }
                }
                break;
            }
        }
    }

    return WindowHook::OnHook( nCode, wParam, lParam );
}