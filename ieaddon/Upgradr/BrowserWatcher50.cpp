///////////////////////////////////////////////////////////////
//
// BrowserWatcher50.cpp
//
// Created: 23/09/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrowserWatcher50.h"
#include <oleacc.h>
#include "ComUtils.h"
#pragma comment( lib, "oleacc" )

namespace IE50
{
    const UINT  WM_HTML_GETOBJECT  = ::RegisterWindowMessage( _T( "WM_HTML_GETOBJECT" ) );

    HRESULT GetDocument( HWND hWnd, IHTMLDocument2 **ppDocument )
    {
        if( ppDocument == NULL )
        {
            return E_POINTER;
        }

        *ppDocument = NULL;

        DWORD   res = 0;
        if( ::SendMessageTimeout( 
                            hWnd, 
                            WM_HTML_GETOBJECT, 
                            0, 
                            0, 
                            SMTO_ABORTIFHUNG, 
                            1000, 
                            &res 
                            ) == 0 )
        {
            return E_FAIL;
        }

        return ::ObjectFromLresult( 
                                res, 
                                IID_IHTMLDocument2, 
                                0, 
                                reinterpret_cast< void ** >( ppDocument )
                                );
    }
}

BrowserWatcher50::BrowserWatcher50( IWebBrowser2 *pWebBrowser, CDockingSite::Watcher *pObserver ) :
    BrowserWatcher( pWebBrowser, pObserver )
{
}

HRESULT BrowserWatcher50::HitTest( HWND hWnd, const POINT &pt, IHTMLElement **ppElement )
{
    if( ppElement == NULL )
    {
        return E_POINTER;
    }

    *ppElement = NULL;

    ThrowHResult    hr;

    try
    {
        CComPtr< IHTMLDocument2 >    pDoc;
        hr = IE50::GetDocument( hWnd, &pDoc );

        if( pDoc )
        {
            hr = pDoc->elementFromPoint( pt.x, pt.y, ppElement );
        }
    }
    catch( HRESULT )
    {
    }

    return hr;
}
