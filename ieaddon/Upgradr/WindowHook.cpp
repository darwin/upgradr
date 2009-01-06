///////////////////////////////////////////////////////////////
//
// WindowHook.cpp
//
// Created: 29/07/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WindowHook.h"
#include <algorithm>

#define DEFINE_HOOK( idHook_ )              \
        if( idHook == idHook_ )             \
            return HookProc< idHook_ >;     \

namespace
{
    struct HookRec
    {
        int         idHook;
        DWORD       threadId;
        WindowHook  *pHook;
    };

    struct FindHook : public std::binary_function< HookRec, int, bool >
    {
        bool operator()( const HookRec &rec, int idHook ) const
        {
            if( rec.idHook != idHook )
            {
                return false;
            }

            return  ( rec.threadId == 0 ) ||
                    ( rec.threadId == ::GetCurrentThreadId() );
        }
    };

    class HookManager
    {
    private:
        typedef std::list< HookRec >    HookList;

    private:
        HookManager() :
            m_hookList()
        {
        }

        LRESULT CALLBACK OnHook( int idHook, int nCode, WPARAM wParam, LPARAM lParam )
        {
            HookList::iterator  it = Find( idHook );
            if( it != m_hookList.end() )
            {
                ATLASSERT( (*it).pHook != NULL );

                if( (*it).pHook != NULL )
                {
                    return (*it).pHook->OnHook( nCode, wParam, lParam );
                }
            }

            ATLASSERT( FALSE );
            return -1;
        }

        HookList::iterator Find( int idHook )
        {
            return std::find_if(
                            m_hookList.begin(),
                            m_hookList.end(),
                            std::bind2nd(
                                    FindHook(),
                                    idHook
                                    )
                            );
        }

    public:
        void Register( int idHook, WindowHook *pHook )
        {
            if( Find( idHook ) != m_hookList.end() )
            {
                ATLASSERT( FALSE );
                return;
            }

            HookRec rec = { idHook, ::GetCurrentThreadId(), pHook };
            m_hookList.push_back( rec );
        }

        void UnRegister( int idHook )
        {
            HookList::iterator  it = Find( idHook );
            if( it == m_hookList.end() )
            {
                ATLASSERT( FALSE );
                return;
            }

            m_hookList.erase( it );
        }

    public:
        static HookManager & GetInstance()
        {
            static HookManager  theInstance;
            return theInstance;
        }

        static HOOKPROC GetHookProc( int idHook )
        {
            DEFINE_HOOK( WH_CALLWNDPROC )
            DEFINE_HOOK( WH_CALLWNDPROCRET )
            DEFINE_HOOK( WH_CBT )
            DEFINE_HOOK( WH_DEBUG )
            DEFINE_HOOK( WH_FOREGROUNDIDLE )
            DEFINE_HOOK( WH_GETMESSAGE )
            DEFINE_HOOK( WH_JOURNALPLAYBACK )
            DEFINE_HOOK( WH_JOURNALRECORD )
            DEFINE_HOOK( WH_KEYBOARD )
            DEFINE_HOOK( WH_KEYBOARD_LL )
            DEFINE_HOOK( WH_MOUSE )
            DEFINE_HOOK( WH_MOUSE_LL )
            DEFINE_HOOK( WH_MSGFILTER )
            DEFINE_HOOK( WH_SHELL )
            DEFINE_HOOK( WH_SYSMSGFILTER )

            ATLASSERT( FALSE ); // unknown hook type
            return NULL;
        }

    private:
        template< int idHook >
        static LRESULT CALLBACK HookProc( int nCode, WPARAM wParam, LPARAM lParam )
        {
            return GetInstance().OnHook( idHook, nCode, wParam, lParam );
        }

    private:
        HookList    m_hookList;
    };
}

WindowHook::WindowHook() :
    m_idHook( -1 ),
    m_hook( NULL )
{
}

LRESULT WindowHook::OnHook( int nCode, WPARAM wParam, LPARAM lParam )
{
    return CallNextHookEx( nCode, wParam, lParam );
}

bool WindowHook::SetWindowsHookEx( int idHook, DWORD threadId )
{
    m_hook = ::SetWindowsHookEx( 
                        idHook,
                        HookManager::GetHookProc( idHook ),
                        _AtlBaseModule.GetModuleInstance(),
                        threadId == -1 ? ::GetCurrentThreadId() : 0
                        );

    ATLASSERT( m_hook != NULL );

    if( m_hook )
    {
        m_idHook = idHook;
        HookManager::GetInstance().Register( m_idHook, this );
        return true;
    }

    return false;
}

void WindowHook::UnhookWindowsHookEx()
{
    if( m_hook )
    {
        HookManager::GetInstance().UnRegister( m_idHook );
        ::UnhookWindowsHookEx( m_hook );

        m_hook = NULL;
        m_idHook = -1;
    }
}

LRESULT WindowHook::CallNextHookEx( int nCode, WPARAM wParam, LPARAM lParam )
{
    return ::CallNextHookEx( m_hook, nCode, wParam, lParam );
}