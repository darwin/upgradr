///////////////////////////////////////////////////////////////
//
// BrowserWatcher60.cpp
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
#include "BrowserWatcher60.h"
#include "WindowHelper.h"
#include "ComUtils.h"

namespace IE60
{
    HRESULT GetElementOffset( IHTMLElement *pElement, SIZE &offset )
    {
        if( !pElement )
        {
            return E_POINTER;
        }
        
    //
    // as we're zeroing the offset structure here, be sure
    // to perform the recursive step before adding the offset
    // of the current element
    //
        ::ZeroMemory( &offset, sizeof( SIZE ) );

        ThrowHResult    hr;

        try
        {
        //
        // do we need to recurse up the element hierarchy?
        //
        // thanks to dchris_med (christig@home.ro) for the
        // fix when the link is in a text container (e.g. something
        // like the TEXTAREA tag
        //
            CComPtr< IHTMLElement > pParent;

            hr = pElement->get_offsetParent( &pParent );
            if( pParent )
            {
                hr = GetElementOffset( pParent, offset );
            }

            POINT   ptScroll = { 0 };
            CComQIPtr< IHTMLTextContainer > pTC = pElement;
            if( pTC )
            {
                pTC->get_scrollLeft( &ptScroll.x );
                pTC->get_scrollTop( &ptScroll.y );
            }

            POINT   ptClient = { 0 };
            CComQIPtr< IHTMLControlElement >    pCE = pElement;
            if( pCE )
            {
                pCE->get_clientLeft( &ptClient.x );
                pCE->get_clientTop( &ptClient.y );
            }

            long    cx, cy;
            hr = pElement->get_offsetLeft( &cx );
            hr = pElement->get_offsetTop( &cy );

            offset.cx = offset.cx + cx - ptScroll.x + ptClient.x;
            offset.cy = offset.cy + cy - ptScroll.y + ptClient.y;
        }
        catch( HRESULT )
        {
        }

        return hr;
    }

/**
 * Ascend the frame hierarchy (if any) until we find the first non-frame HTML 
 * element at the specified position
 **/
    HRESULT HitTest( IWebBrowser2 *pWB, const POINT &pt, IHTMLElement **ppElement )
    {
        if( pWB == NULL )
        {
            return E_INVALIDARG;
        }

        if( !ppElement )
        {
            return E_POINTER;
        }

        *ppElement = NULL;

        ThrowHResult    hr;

        try
        {
        //
        // get the document associated with this web browser object
        //
            CComPtr< IDispatch >    pDisp;
            hr = pWB->get_Document( &pDisp );

        //
        // is it an HTML document?
        //
            CComQIPtr< IHTMLDocument2 >   pDoc = pDisp;

            if( pDoc == NULL )
            {
            //
            // no - nothing more doing
            //
                return S_OK;
            }

        //
        // get the element at the specified point
        //
            CComPtr< IHTMLElement > pElement;
            hr = pDoc->elementFromPoint( pt.x, pt.y, &pElement );

            if( pElement == NULL )
            {
                return S_OK;
            }

        //
        // it could be a nested frame - if so we need to
        // drill down
        //
            CComQIPtr< IWebBrowser2 >   pFrame = pElement;

            if( pFrame )
            {
            //
            // how far is the frame nested within the document?
            //
                SIZE    offset;
                hr = GetElementOffset( pElement, offset );

                POINT   ptInFrame = { pt.x - offset.cx, pt.y - offset.cy };

                return HitTest( pFrame, ptInFrame, ppElement );
            }

        //
        // return the element we've found to the user
        //
            hr = pElement->QueryInterface( 
                                    IID_IHTMLElement,
                                    reinterpret_cast< void ** >( ppElement )
                                    );
        }
        catch( HRESULT )
        {
        }

        return hr;
    }
}

BrowserWatcher60::BrowserWatcher60( 
                                IWebBrowser2 *pWebBrowser, 
                                CDockingSite::Watcher *pObserver 
                                ) :
    BrowserWatcher( pWebBrowser, pObserver )
{
}

HRESULT BrowserWatcher60::HitTest( HWND, const POINT &pt, IHTMLElement **ppElement )
{
    return IE60::HitTest( GetWebBrowser(), pt, ppElement );
}
