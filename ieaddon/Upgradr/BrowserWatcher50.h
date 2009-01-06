///////////////////////////////////////////////////////////////
//
// BrowserWatcher50.h
//
// Created: 23/09/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#ifndef __BROWSERWATCHER50_H_83CB7948_6351_40C8_873C_51F3C9CB6CFF_
#define __BROWSERWATCHER50_H_83CB7948_6351_40C8_873C_51F3C9CB6CFF_

#include "BrowserWatcher.h"

/**
 * @brief Watcher class for IE5.0 web browser windows.
 *
 * When displaying a single frame window, the window class hierarchy of
 * a 5.0 window is:
 *
 *      IEFrame
 *      |->Shell DocObject View
 *         |->Internet Explorer_Server
 *
 * However, when displaying a multiframe window, the class hierarchy changes:
 *
 *      IEFrame
 *      |->Shell DocObject View
 *         |->Internet Explorer_Server
 *            |->Shell Embedding
 *               |->Shell DocObject View
 *                  |->Internet Explorer_Server (**)
 * 
 * When performing the hit test, we can retrieve the document associated 
 * with a specific Internet Explorer_Server instance and perform a direct 
 * hit test on that document @see HitTest. This is much simpler than the 
 * frame traversal we have to use in the BrowserWatcher60 class
 **/

class BrowserWatcher50 : public BrowserWatcher
{
public:
    BrowserWatcher50( IWebBrowser2 *pWebBrowser, CDockingSite::Watcher *pObserver );

// BrowserWatcher
private:
    HRESULT HitTest( HWND hWnd, const POINT &pt, IHTMLElement **ppElement );
};

#endif // __BROWSERWATCHER50_H_83CB7948_6351_40C8_873C_51F3C9CB6CFF_
