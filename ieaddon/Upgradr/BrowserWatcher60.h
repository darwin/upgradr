///////////////////////////////////////////////////////////////
//
// BrowserWatcher60.h
//
// Created: 23/09/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#ifndef __BROWSERWATCHER60_H_2C25B32E_8C00_4E99_89C6_E8A0A9704E57_
#define __BROWSERWATCHER60_H_2C25B32E_8C00_4E99_89C6_E8A0A9704E57_

#include "BrowserWatcher.h"

/**
 * @brief Watcher class for IE5.5 and IE6.0 web browser windows.
 *
 * The window class hierarchy for a web browser window is:
 *      IEFrame
 *      |->Shell DocObject View
 *         |->Internet Explorer_Server (*)
 *
 * When we're dealing with pages with multiple frames, the web browser maintains
 * the window class hierarchy shown above, but nests multiple web browser 
 * (IWebBrowser2) elements within the document. When performing the HitTest
 * we (may) need to ascend the frame hierarchy to retrieve the correct element.
 * As we ascend the frame hierarchy we need to be sure to take account of the
 * offset of each frame within the document.
 **/

class BrowserWatcher60 : public BrowserWatcher
{
public:
    BrowserWatcher60( IWebBrowser2 *pWebBrowser, CDockingSite::Watcher *pObserver );

// BrowserWatcher
private:
    HRESULT HitTest( HWND, const POINT &pt, IHTMLElement **ppElement );
};

#endif // __BROWSERWATCHER60_H_2C25B32E_8C00_4E99_89C6_E8A0A9704E57_
