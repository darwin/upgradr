///////////////////////////////////////////////////////////////
//
// KeyboardHook.h
//
// Created: rhare - 21/07/2005
// Copyright (c) 2005 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#ifndef __KEYBOARDHOOK_H_15173C39_10ED_40AF_828B_B1B57CB3C2C6_
#define __KEYBOARDHOOK_H_15173C39_10ED_40AF_828B_B1B57CB3C2C6_

#include "WindowHook.h"

/**
 * Class to hook low-level keyboard events - Note: keyboard
 * events aren't specific to any thread
 **/
class KeyboardHook : public WindowHook
{
public:
    KeyboardHook();
    ~KeyboardHook();

    struct Observer
    {
        virtual ~Observer() {};
        virtual LRESULT OnKeyHook( UINT uMsg, const KBDLLHOOKSTRUCT *pks ) = 0;
    };

    void Initialise( Observer *pObserver );
    void Shutdown();

// WindowHook
private:
    LRESULT OnHook( int code, WPARAM wParam, LPARAM lParam );

private:
    Observer    *m_pObserver;
};

#endif // __KEYBOARDHOOK_H_15173C39_10ED_40AF_828B_B1B57CB3C2C6_