/////////////////////////////////////////////////////////////////////////////
// CSheetTabWindow - Draws the CFlatTabCtrl with Horizontal Scroll Support
//
// Written by Rashid Thadha (rashidthadha@hotmail.com)
// Copyright (c) August 2002.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __ATLAPP_H__
	#error atlsheettabwindow.h requires atlctrls.h to be included first
#endif

#if !defined(AFX_ATLSHEETTABWINDOW_H__DBEA58DA_DC43_4B5D_B5E9_B345BDD962F8__INCLUDED_)
#define AFX_ATLSHEETTABWINDOW_H__DBEA58DA_DC43_4B5D_B5E9_B345BDD962F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ATLFlatTab.h"
#define IDC_FLATTAB 4002

#define MINIMUM_SCROLLBAR 70
#define SPLITTER_BAR_WIDTH 3

template<class T>
class CSheetTabWindowImpl : 
   public CWindowImpl< T, CStatic, CControlWinTraits >
{
public:
    CSheetTabWindowImpl()  : m_nCurrentFlatTabWidth(0), m_hCursor(NULL), m_cxyDragOffset(0),
                             m_bMinimumScrollBarMode(false), m_nMaxFlatTabControlWidth(300),
                             m_nScrollMinRange(0), m_nScrollMaxRange(0), m_nPageSize(0)
    {
		if(m_hCursor == NULL)
		{
			// TODO ::EnterCriticalSection(GetModule().m_csStaticDataInit);
			if(m_hCursor == NULL)
				m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
			// TODO ::LeaveCriticalSection(GetModule().m_csStaticDataInit);
		}    
    }
    virtual ~CSheetTabWindowImpl() 
    {
    }
    CFlatTabCtrl* GetFlatTabCtrl() 
    {
        return &m_wndFlatTab;
    }
    void SetMaxTabControlWidth(int nWidth)
    {
        m_nMaxFlatTabControlWidth = nWidth;
    }    
    void SetViewScrollRange(int nMin, int nMax, int nPageSize)
    {
        m_nScrollMinRange = nMin;
        m_nScrollMaxRange = nMax;
        m_nPageSize = nPageSize;
        m_wndScroll.SetScrollRange(nMin, nMax);
    }

    CContainedWindowT<CScrollBar> m_wndScroll;

    typedef CSheetTabWindowImpl< T > thisClass;
    
    BEGIN_MSG_MAP(thisClass)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUP)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        ALT_MSG_MAP(1) // Scroll
    END_MSG_MAP()
    
	BOOL SubclassWindow(HWND hWnd)
	{
		BOOL bRet = CWindowImpl< T, CStatic, CControlWinTraits >::SubclassWindow(hWnd);
		if(bRet)
			_Init();
		return bRet;
	}
    bool _Init()
    {        
        // Setup Static control
        SetFont((HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0));
        ModifyStyle(WS_BORDER, ES_LEFT);
        
        // Create Scroll bar
        RECT rc = { 0, 0, 0, 0 };
        m_wndScroll.Create(this, 1, m_hWnd, &rc, NULL, WS_CHILD | WS_VISIBLE);
        ATLASSERT(m_wndScroll.IsWindow());
        m_wndScroll.SetFont((HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0));
        m_wndScroll.SetWindowPos(HWND_TOP, &rc, SWP_NOMOVE | SWP_NOSIZE);

        // Create Flat Tab Control
        if (!m_wndFlatTab.Create(*this, rc, NULL, WS_VISIBLE | WS_CHILD | FTS_HASARROWS | FTS_BOTTOM | TCS_TOOLTIPS,
           0, IDC_FLATTAB))
        {
            DWORD dErrorCode = GetLastError();
            // Find out why check the Win32 Error codes in MSDN
            return false;
        }

        ATLASSERT(m_wndFlatTab.IsWindow());
        m_wndFlatTab.SetFont((HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0));
        m_wndFlatTab.SetWindowPos(HWND_TOP, &rc, SWP_NOMOVE | SWP_NOSIZE);

        return true;
    }
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
        _Init();
        return lRes;
    }
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
        m_wndScroll.SetFocus(); // Set focus to button to prevent input
        return lRes;
    }	    
    void UpdateControlLayout(WORD cx, WORD cy, bool bMouseMove = false)
    {
        // first time set the sheet control to atleast three qurters
        if (m_nCurrentFlatTabWidth == 0)
        {
            m_nCurrentFlatTabWidth = (cx / 4) * 3;
        }
        else
        {            
            // Need to make sure that atleast a minimum scrollbar exist
            if (cx-m_nCurrentFlatTabWidth-SPLITTER_BAR_WIDTH < MINIMUM_SCROLLBAR)
            {
                m_nCurrentFlatTabWidth = cx-MINIMUM_SCROLLBAR;
                m_bMinimumScrollBarMode = true;
            }
            else
            {
                if (m_bMinimumScrollBarMode && !bMouseMove)
                {
                    if (m_nCurrentFlatTabWidth+SPLITTER_BAR_WIDTH+MINIMUM_SCROLLBAR < cx)
                        m_nCurrentFlatTabWidth = cx-MINIMUM_SCROLLBAR;                    

                    // Lets check the status again
                    if (cx > m_nMaxFlatTabControlWidth && cx > m_wndFlatTab.GetVisibleTabWidth()+100)
                        m_bMinimumScrollBarMode = false;
                }
            }

        }
        
        // Now lets move the windows
        if (m_wndFlatTab.m_hWnd)
        {
            m_wndFlatTab.MoveWindow(0, 0, m_nCurrentFlatTabWidth, ::GetSystemMetrics(SM_CXHSCROLL));
            m_wndScroll.MoveWindow(m_nCurrentFlatTabWidth+SPLITTER_BAR_WIDTH, 0,cx-m_nCurrentFlatTabWidth-SPLITTER_BAR_WIDTH, ::GetSystemMetrics(SM_CXHSCROLL));
        }    
    }
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		WORD cx = LOWORD(lParam);
        WORD cy = HIWORD(lParam);

        if (cx == 0)
            return 0;

	    if (wParam != SIZE_MINIMIZED)
        {
            // Need to size the Sheet Control and Scroll bar
            UpdateControlLayout(cx, cy);
        }

		return 0;
	}
    void DrawSplitterBar(CDCHandle dc)
	{
		RECT rect;
		if(GetSplitterBarRect(&rect))
		{
			dc.FillRect(&rect, (HBRUSH)LongToPtr(COLOR_3DFACE + 1));
			dc.DrawEdge(&rect, EDGE_RAISED, BF_LEFT | BF_RIGHT);
			dc.DrawEdge(&rect, BDR_RAISEDOUTER, BF_FLAT | BF_TOP);
		}
	}
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);
		DrawSplitterBar(dc.m_hDC);
		return 0;
	}

	bool GetSplitterBarRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);

		lpRect->left = m_nCurrentFlatTabWidth;
		lpRect->top = 0;
		lpRect->right = m_nCurrentFlatTabWidth + SPLITTER_BAR_WIDTH;
		lpRect->bottom = ::GetSystemMetrics(SM_CXHSCROLL);

		return true;
	}
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if((HWND)wParam == m_hWnd && LOWORD(lParam) == HTCLIENT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			ScreenToClient(&ptPos);
			if(IsOverSplitterBar(ptPos.x, ptPos.y))
				return 1;
		}

		bHandled = FALSE;
		return 0;
	}
	bool IsOverSplitterBar(int x, int y) const
	{
		int xyOff = m_nCurrentFlatTabWidth;
		return ((x >= (xyOff)) && (x < xyOff + SPLITTER_BAR_WIDTH));
	}
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if((wParam & MK_LBUTTON) && ::GetCapture() == m_hWnd)
		{
			int xyNewSplitPos = 0;
			xyNewSplitPos = xPos - m_nCurrentFlatTabWidth - m_cxyDragOffset;

			if(m_nCurrentFlatTabWidth != xyNewSplitPos && xyNewSplitPos != 0)
			{
				m_nCurrentFlatTabWidth += xyNewSplitPos;
                RECT rc, rcInvalidate;
		        GetClientRect(&rc);
				UpdateControlLayout((WORD)(rc.right - rc.left), ::GetSystemMetrics(SM_CXHSCROLL), true);
                
                rcInvalidate.left=m_nCurrentFlatTabWidth-xyNewSplitPos-10;
                rcInvalidate.top=0;
                rcInvalidate.bottom=::GetSystemMetrics(SM_CXHSCROLL);
                rcInvalidate.right=m_nCurrentFlatTabWidth+(SPLITTER_BAR_WIDTH*2);
                InvalidateRect(&rcInvalidate);
                UpdateWindow();                
			}
		}
		else		// not dragging, just set cursor
		{
			if(IsOverSplitterBar(xPos, yPos))
				::SetCursor(m_hCursor);
			bHandled = FALSE;
		}

		return 0;
	}
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if(IsOverSplitterBar(xPos, yPos))
		{
			SetCapture();
			::SetCursor(m_hCursor);
			m_cxyDragOffset = xPos - m_nCurrentFlatTabWidth;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonUP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::ReleaseCapture();
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        int nMax(0), nMin(0);
        m_wndScroll.GetScrollRange(&nMin, &nMax);        
        int nScrollPos = m_wndScroll.GetScrollPos();

        // set the position of the scroll bar
        switch ((int)LOWORD(wParam))
        {
        case SB_LINELEFT:      
            if (nScrollPos > nMin)
                nScrollPos--;
            break;        
        case SB_LINERIGHT:   
            if (nScrollPos < nMax)
                nScrollPos++;
            break;
        case SB_PAGELEFT:    
            if (nScrollPos > nMin)
                nScrollPos = max(nMin, nScrollPos-m_nPageSize);
            break;
        case SB_PAGERIGHT:      
            if (nScrollPos < nMax)
                nScrollPos = min(nMax, nScrollPos+m_nPageSize);
          break;
        case SB_LEFT: 
            nScrollPos = nMin;
            break;
        case SB_RIGHT: 
            nScrollPos = nMax;
            break;
        case SB_THUMBPOSITION: 
        case SB_THUMBTRACK:   
            nScrollPos = (short int)HIWORD(wParam);
            break;
        }

        // Set the new position of the thumb (scroll box).
        m_wndScroll.SetScrollPos(nScrollPos);

        // Allow the parent to actually scroll the view
        ::SendMessage(GetParent(), uMsg, wParam, lParam);
		return 0;
	}

	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		NMHDR* pNMHDR = (NMHDR*)lParam;

		LRESULT lResult = 0;
		
        // handle messages from the flat tab control itself
	    if (IDC_FLATTAB ==(UINT)wParam)
        {
            // Allow the parent to process the same event
            ::SendMessage(GetParent(), uMsg, wParam, lParam);
        }
		return lResult;
	}

protected:
    CFlatTabCtrl m_wndFlatTab;
    int m_nCurrentFlatTabWidth;
    HCURSOR m_hCursor;
	int m_cxyDragOffset;
    bool m_bMinimumScrollBarMode;
    int m_nMaxFlatTabControlWidth;
    int m_nScrollMinRange, m_nScrollMaxRange, m_nPageSize;
};

class CSheetTabWindow : 
   public CSheetTabWindowImpl<CSheetTabWindow>
{
public:
   typedef CSheetTabWindowImpl<CSheetTabWindow> baseClass;

    BEGIN_MSG_MAP(CSheetTabWindow)
    CHAIN_MSG_MAP(baseClass)
    ALT_MSG_MAP(1) // ScrollBar
        CHAIN_MSG_MAP_ALT(baseClass, 1)
    END_MSG_MAP()
};

#endif // !defined(AFX_ATLSHEETTABWINDOW_H__DBEA58DA_DC43_4B5D_B5E9_B345BDD962F8__INCLUDED_)
