///////////////////////////////////////////////////////////////
//
// MessageHook.h
//
// Created: rhare - 21/07/2005
// Copyright (c) 2005 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#ifndef __MESSAGEHOOK_H_A8B30B6F_1F35_4CA9_ADFA_459ED6BED578_
#define __MESSAGEHOOK_H_A8B30B6F_1F35_4CA9_ADFA_459ED6BED578_

#include "WindowHook.h"

/**
 * Class to hook window creation/destruction for the current thread.
 **/
class MessageHook : public WindowHook
{
public:
/**
 * Callback interface to notify clients of window
 * creation/destruction
 **/
    struct Observer
    {
        virtual void OnCreate( HWND hWnd ) = 0;
        virtual void OnDestroy( HWND hWnd ) = 0;
    };

public:
    MessageHook();
    ~MessageHook();

    void Initialise( Observer *pObserver );
    void Shutdown();

// WindowHook
private:
/**
 * Callback function invoked by WindowHook when the windows hook is
 * invoked. Check the hook code and if appropriate inform the observer.
 **/
    LRESULT OnHook( int nCode, WPARAM wParam, LPARAM lParam );

private:
    HHOOK       m_hook;             /// < the windows hook
    Observer    *m_pObserver;       /// < the callback interface
};

#endif // __MESSAGEHOOK_H_A8B30B6F_1F35_4CA9_ADFA_459ED6BED578_