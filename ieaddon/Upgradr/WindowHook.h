///////////////////////////////////////////////////////////////
//
// WindowHook.h
//
// Created: 29/07/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#ifndef __WINDOWHOOK_H_06EA5989_97D5_4C6D_B78E_0FF52C84FEAF_
#define __WINDOWHOOK_H_06EA5989_97D5_4C6D_B78E_0FF52C84FEAF_

/**
 * @brief Base class to encapsulate a Windows Hook.
 *
 * The internals of the WindowsHook ensures that only
 * one hook of a specific type (@see SetWindowsHookEx in MSDN
 * for details of hook types) is allowed per thread. This per-thread
 * instantiation is necessary to allow WindowHook to invoke
 * the correct WindowHook derivation when Windows invokes
 * the hook.
 **/

class WindowHook
{
protected:
    WindowHook();

public:
    virtual ~WindowHook() {}

public:
/**
 * OnHook is called when Windows invokes the hook. Derived
 * classes should override OnHook. The return value and arguments
 * are dependent on the type of hook (e.g. see CallWndProc
 * in MSDN for WH_CALLWNDPROC hooks)
 **/
    virtual LRESULT OnHook( int nCode, WPARAM wParam, LPARAM lParam );

protected:
    bool SetWindowsHookEx( int idHook, DWORD threadId = -1 );
    void UnhookWindowsHookEx();

    LRESULT CallNextHookEx( int nCode, WPARAM wParam, LPARAM lParam );

private:
    int         m_idHook;
    HHOOK       m_hook;     /// < the windows hook
};

#endif // __WINDOWHOOK_H_06EA5989_97D5_4C6D_B78E_0FF52C84FEAF_
