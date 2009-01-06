///////////////////////////////////////////////////////////////
//
// BrowserWatcher.h
//
// Created: 23/09/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#pragma once

#include "DockingSite.h"
#include "MessageHook.h"

/**
 * Base class for classes to watch web browsers and record any
 * mouse gestures the user makes. Client code should derive from 
 * MouseTracker::Watcher and pass that to the CreateInstance factory
 * method.
 *
 * The <b>general</b> window hierarchy for a WebBrowser windows is:
 *
 * <pre>
 *      IEFrame
 *      |->Shell DocObject View
 *         |->Internet Explorer_Server (*)
 * </pre>
 *
 * We want to attach the gesture tracker to the Internet Explorer_Server
 * window (marked with a single asterisk (*))
 *
 * However, when we use a multiframe window, IE5.0 uses the following 
 * hierarchy (the example hierarchy assumes two frames):
 *
 * <pre>
 *      IEFrame
 *      |->Shell DocObject View
 *         |->Internet Explorer_Server
 *            |->Shell Embedding
 *            |  |->Shell DocObject View
 *            |     |->Internet Explorer_Server (**)
 *            |->Shell Embedding
 *               |->Shell DocObject View
 *                  |->Internet Explorer_Server (**)
 *
 * </pre>
 *
 * We want to attach the gesture tracker to the most descendant Internet 
 * Explorer_Server windows (marked with a double asterisk (**)) - IE5.5 and
 * IE6.0 keep the original window hierarchy. 
 *
 * The easiest way of keeping track of all these windows is to hook all
 * Windows messages (@see WindowHookManager) for the current thread 
 * (remember IE is a multi SDI app: each web-browser instance has it's 
 * own primary thread), and attach a gesture tracker to each Internet 
 * Explorer_Server window as it's created.
 *
 * The differing class hierarchies mean that we have to implement different
 * hit tests for IE5.0 and IE5.5/IE6.0 web browsers.
 **/

class BrowserWatcher : private MessageHook::Observer
{
/**
 * Protected constructor to prevent direct instantiation
 * @see CreateInstance for arguments
 **/
protected:
    BrowserWatcher(IWebBrowser2 *pWebBrowser, CDockingSite::Watcher *pObserver );

public:
    virtual ~BrowserWatcher();

/**
 * Retrieve the HTML element at the position in the web browser window
 * specified by hWnd and pt
 * @param hWnd handle to the window to be tested
 * @param pt point to be tested
 * @param ppElement address of a pointer to receive the element
 *        the user clicked on. It is the callers responsibility
 *        to decrement the reference count of this pointer if the
 *        call is successfull
 * @return a standard HRESULT
 **/
    virtual HRESULT HitTest( HWND hWnd, const POINT &pt, IHTMLElement **ppElement ) = 0;

public:
/** 
 * Factory method for classes deriving from BrowserWatcher. The factory
 * retrieves the version of the web browser and returns an appropriate
 * BrowserWatcher object.
 * @param pWebBrowser pointer to the web browser instance to be watched
 * @param pObserver pointer to the callback interface to use when a
 *        mouse gesture is observed
 * @return pointer to a BrowserWatcher derived object, or NULL if creation
 *         failed. It is the callers responsibility to free the memory
 *         associated with any returned objects.
 **/
    static BrowserWatcher * CreateInstance( 
                                    IWebBrowser2 *pWebBrowser,
                                    CDockingSite::Watcher *pObserver 
                                    );

// WindowHook::Observer
private:
    void OnCreate( HWND hWnd );
    void OnDestroy( HWND hWnd );

// Implementation
protected:
    HWND GetHwnd() const;   /// < retrieve the HWND of the web browser
    IWebBrowser2 * GetWebBrowser() const;

private:
    void Remove( HWND hWnd );

private:
    typedef std::map< HWND, CDockingSite * >  TrackersMap;

private:
    CComPtr< IWebBrowser2 >     m_pWebBrowser;
    CDockingSite::Watcher    *m_pObserver;
    TrackersMap                  m_gestureTracker;

    MessageHook                 m_messageHook;
};
