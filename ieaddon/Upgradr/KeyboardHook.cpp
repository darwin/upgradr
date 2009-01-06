///////////////////////////////////////////////////////////////
//
// KeyboardHook.cpp
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
#include "KeyboardHook.h"

KeyboardHook::KeyboardHook() :
    WindowHook(),
    m_pObserver( NULL )
{
}

KeyboardHook::~KeyboardHook()
{
    Shutdown();
}

void KeyboardHook::Initialise( Observer *pObserver )
{
    m_pObserver = pObserver;
    SetWindowsHookEx( WH_KEYBOARD_LL, 0 );
}

void KeyboardHook::Shutdown()
{
    UnhookWindowsHookEx();
}

LRESULT KeyboardHook::OnHook( int nCode, WPARAM wParam, LPARAM lParam )
{
    if( ( nCode == HC_ACTION ) && ( m_pObserver != NULL ) )
    {
        LRESULT ret = m_pObserver->OnKeyHook( 
                                        static_cast< UINT >( wParam ), 
                                        reinterpret_cast< KBDLLHOOKSTRUCT * >( lParam ) 
                                        );

        if( ret > 0 )
        {
            return ret;
        }
    }

    return WindowHook::OnHook( nCode, wParam, lParam );
}