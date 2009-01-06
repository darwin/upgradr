
// FlatTab.h : header file
//
// Copyright © 1999 Ian Brumby
//
// This source code may be used in compiled form in any way you desire. 
// Source file(s) may be redistributed unmodified by any means PROVIDING
// they are not sold for profit without the authors expressed written consent,
// and providing that this notice and the authors name and all copyright
// notices remain intact.
//
// ==========================================================================  
// HISTORY:	  
// ==========================================================================  
//			1.4 	21 Jul 1999	- Initial release.
//			1.5		30 Aug 1999 - Several enhancements by Phil Baxter 
//					<Phil.Baxter@mrjinx.demon.co.uk>
// ==========================================================================  

// Excel uses FF_ROMAN and Font Height = ~(control's height - 4)
// Excel draws its tabs right to left
// Excel has 3DLIGHT highlighting on the left diagonal
// Excel has home/end buttons
// Excel's buttons change width as the control's height changes
//
/////////////////////////////////////////////////////////////////////////////
/****************************************************************************
 *
 * $Date: 10/26/99 10:40p $
 * $Revision: 8 $
 * $Archive: /CodeJock/Include/CJFlatTabCtrl.h $
 *
 * $History: CJFlatTabCtrl.h $
 * 
 * *****************  Version 8  *****************
 * User: Kirk Stowell Date: 10/26/99   Time: 10:40p
 * Updated in $/CodeJock/Include
 * Made class methods virtual for inheritance purposes.
 * 
 * *****************  Version 7  *****************
 * User: Kirk Stowell Date: 10/24/99   Time: 12:15a
 * Updated in $/CodeJock/Include
 * Fixed minor bug with tab font creation.
 * 
 * *****************  Version 6  *****************
 * User: Kirk Stowell Date: 10/24/99   Time: 12:01a
 * Updated in $/CodeJock/Include
 * Fixed potential resource and memory leak problems.
 * 
 * *****************  Version 5  *****************
 * User: Kirk Stowell Date: 10/14/99   Time: 12:25p
 * Updated in $/CodeJock/Include
 * Added source control history to file header.
 *
 ***************************************************************************/
/////////////////////////////////////////////////////////////////////////////
// ==========================================================================  
// Port to ATL/WTL (ATLFlatTab.h)
//
// Rashid Thadha 03/02/2001
// rashidthadha@hotmail.com
//
// Rashid Thadha 3/8/2002
// rashidthadha@hotmail.com
// few bug fixes and enhancement for Horizontal scroll bar capabilities
// ==========================================================================  

#if !defined(ATL_FLATTAB_H)
#define ATL_FLATTAB_H

#include "ATLMetaFileButton.h"

#include <list>
using namespace std;

#define IDC_LEFTBUTTON                  5031
#define IDC_RIGHTBUTTON                 5032
#define IDC_HOMEBUTTON                  5033
#define IDC_ENDBUTTON                   5034

// control styles
#define FTS_BOTTOM              0x0002
#define FTS_HASARROWS           0x0004
#define FTS_HASHOMEEND          0x0008 // Must be used with FTS_HASARROWS

typedef list<CString*, allocator<CString*> > CTabList;

// CCJFlatTabCtrl is a CWnd derived class which is used to create a flat tab
// control simular to DevStudio.
class CFlatTabCtrl : public CWindowImpl<CFlatTabCtrl, CWindow>
{
    // Construction
public:
    // ===========================================================================
    // Function name	: CFlatTabCtrl
    // Description	  : Constructor 
    // Return type		: 
    // ===========================================================================
    
    CFlatTabCtrl() 
    {
        m_bHasArrows          = false;
        m_bHasHomeEnd         = false;
        m_bTabsOnBottom       = false;
        m_iCurSel             = -1;
        m_pLeftButton         = NULL;
        m_pRightButton        = NULL;
        m_pHomeButton         = NULL;
        m_pEndButton          = NULL;
        m_hLeftArrow          = NULL;
        m_hLeftArrowDisabled  = NULL;
        m_hRightArrow         = NULL;
        m_hRightArrowDisabled = NULL;
        m_hHomeArrow          = NULL;
        m_hHomeArrowDisabled  = NULL;
        m_hEndArrow           = NULL;
        m_hEndArrowDisabled   = NULL;
        m_iXOffset            = 0;
    }
    
    
    BEGIN_MSG_MAP(CFlatTabCtrl)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        COMMAND_HANDLER(IDC_LEFTBUTTON, BN_CLICKED, OnLeftArrow)
        COMMAND_HANDLER(IDC_RIGHTBUTTON, BN_CLICKED, OnRightArrow)
        COMMAND_HANDLER(IDC_HOMEBUTTON, BN_CLICKED, OnHomeArrow)
        COMMAND_HANDLER(IDC_ENDBUTTON, BN_CLICKED, OnEndArrow)
        REFLECT_NOTIFICATIONS()
        END_MSG_MAP()
        
        DECLARE_WND_CLASS_EX(_T("MFXFlatTabCtrl"), CS_DBLCLKS, NULL);
    
    // Attributes
public:
    
    // Operations
public:
    BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (::IsWindow(m_ToolTip.m_hWnd))                                                                                                                                                                                                                                                                                                                                                                                                                         
        {
            if (pMsg->message == WM_MOUSEMOVE && pMsg->hwnd == m_hWnd)
            {
                CPoint Point(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
                
                TCHITTESTINFO test;
                test.pt = Point;
                int iTab = HitTest(&test);
                
                if (iTab >= 0)
                {
                    CString strText;
                    strText = (LPCTSTR) * (GetTabTipListItem(iTab));
                    
                    _U_STRINGorID stext(strText);
                    m_ToolTip.UpdateTipText(stext, m_hWnd);
                    m_ToolTip.RelayEvent(pMsg);
                } 
                else  
                {
                    // VC5 compatibility - Kirk Stowell - 10-21-99
#ifndef _MSC_VER
                    ::SendMessage(m_ToolTip.m_hWnd, TTM_POP, 0, 0L);
#endif
                }
            }
            else 
            {
                // VC5 compatibility - Kirk Stowell - 10-21-99
#ifndef _MSC_VER
                ::SendMessage(m_ToolTip.m_hWnd, TTM_POP, 0, 0L);
#endif
            }
        }
        
        return (BOOL)SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
    }
    
    
    // ===========================================================================
    // Function name	: InsertItem
    // Description	  : 
    // Return type		: virtual BOOL 
    // Argument       : const int nItem
    // Argument       : LPCTSTR lpszItem
    // ===========================================================================
    virtual BOOL InsertItem(const int nItem, LPCTSTR lpszItem)
    {
        if (nItem < 0 || nItem > TabListCount())
            return -1;
        
        // add tab to our list
        CString* pItem = new CString;
        ATLASSERT(pItem);
        
        *pItem = lpszItem;
        m_TabList.insert(m_TabList.end(), pItem);
        
        // PGB - Add the tabs tip text to the list
        
        CString* pTipItem = new CString;
        ATLASSERT(pTipItem);
        
        *pTipItem = lpszItem;
        m_TabTipList.insert(m_TabTipList.end(), pTipItem);
        
        if (TabListCount() == 1)
            m_iCurSel = 0;
        
        m_iTotalTabWidth = GetTotalTabWidth();      // PGB - Recalculate the total tab width now we have added a new item
        
        EnableButtons();
        InvalidateTabs();
        
        return nItem;
    }
    
    // ===========================================================================
    // Function name	: TabListCount
    // Description	  : 
    // Return type		: int 
    // ===========================================================================
    int TabListCount()
    {
        return (int)m_TabList.size();
    }
    
    // ===========================================================================
    // Function name	: DeleteItem
    // Description	  : 
    // Return type		: virtual BOOL 
    // Argument       : int nItem
    // ===========================================================================
    virtual BOOL DeleteItem(int nItem)
    {
        if (nItem < 0 || nItem >= TabListCount())
            return FALSE;
        
        int nIndex = 0;
        CTabList::iterator ListIterator;	
        for (ListIterator = m_TabList.begin(); ListIterator != m_TabList.end(); ++ListIterator)        
        {
            if (nIndex == nItem)
            {
                CString* pItem = *ListIterator;
                
                if (pItem)
                {
                    delete pItem;
                    pItem = NULL;
                }
                if (ListIterator != m_TabList.end())
                    m_TabList.erase(ListIterator);
                
                break;
            }
            nIndex++;
        }
        
        nIndex = 0;
        CTabList::iterator ListIterator2;	
        for (ListIterator2 = m_TabTipList.begin(); ListIterator2 != m_TabTipList.end(); ++ListIterator2)        
        {
            if (nIndex == nItem)
            {
                CString* pItem = *ListIterator2;
                
                if (pItem)
                {
                    delete pItem;
                    pItem = NULL;
                }
                if (ListIterator2 != m_TabTipList.end())
                    m_TabTipList.erase(ListIterator2);
                
                break;
            }
            nIndex++;
        }
        
        // PGB - Stole the following code from the OnLButtonDown() function
        // in order to cause the tab control to show the newly selected item
        // after deleting an item from the list
        
        // warn parent that the selection is about to change
        int id = GetDlgCtrlID();
        NMHDR hdr;
        hdr.hwndFrom = m_hWnd;
        hdr.idFrom = id;
        hdr.code = TCN_SELCHANGING;
        CWindow wnd(GetParent());
        if (wnd.SendMessage(WM_NOTIFY, id, (LPARAM)&hdr) == 0)
        {
            int nTabListCount = TabListCount();
            if (m_iCurSel >= nTabListCount)
            {
                SetCurSel(nTabListCount - 1); // PGB - Reset the currently selected tab to the last one in the list
                InvalidateTabs();
            }
            
            // notify parent that the selection has changed
            hdr.hwndFrom = m_hWnd;
            hdr.idFrom = id;
            hdr.code = TCN_SELCHANGE;
            wnd.SendMessage(WM_NOTIFY, id, (LPARAM)&hdr);
        }
        
        m_iTotalTabWidth = GetTotalTabWidth();      // PGB - Recalculate the total tab width now we have deleted an item
        
        EnableButtons();
        InvalidateTabs();
        
        return TRUE;
    }
    
    // ===========================================================================
    // Function name	: DeleteAllItems
    // Description	  : 
    // Return type		: virtual BOOL 
    // ===========================================================================
    virtual BOOL DeleteAllItems()
    {
        CTabList::iterator ListIterator;	
        for (ListIterator = m_TabList.begin(); ListIterator != m_TabList.end(); ++ListIterator)        
        {
            CString* pItem = *ListIterator;
            
            if (pItem)
            {
                delete pItem;
                pItem = NULL;
            }
        }
        m_TabList.erase(m_TabList.begin(), m_TabList.end());
        
        CTabList::iterator ListIterator2;	
        for (ListIterator2= m_TabTipList.begin(); ListIterator2 != m_TabTipList.end(); ++ListIterator2)        
        {
            CString* pItem = *ListIterator2;
            
            if (pItem)
            {
                delete pItem;
                pItem = NULL;
            }
        }
        m_TabTipList.erase(m_TabTipList.begin(), m_TabTipList.end());
        
        m_iCurSel = -1;                             // PGB - Reset the currently selected tab to -1 as we have no tabs in our list now.
        
        m_iTotalTabWidth = GetTotalTabWidth();      // PGB - Recalculate the total tab width now we have deleted all items
        
        EnableButtons();
        InvalidateTabs();
        
        return TRUE;
    }
    
    // ===========================================================================
    // Function name	: GetItemRect
    // Description	  : 
    // Return type		: virtual BOOL 
    // Argument       : int nItem
    // Argument       : LPRECT lpRect
    // ===========================================================================
    virtual BOOL GetItemRect(int nItem, LPRECT lpRect)
    {
        int nTabListCount = TabListCount();
        if (nItem < 0 || nItem >= nTabListCount)
            return FALSE;
        
        int x = GetTotalArrowWidth();
        for (int i = 0; i < nItem; i++)
        {
            x += GetTabWidth(i);
            if (i != nTabListCount)
                x -=(m_iClientHeight / 2) + 2;
        }
        lpRect->left = x - m_iXOffset;
        lpRect->top = 0;
        lpRect->right = lpRect->left + GetTabWidth(nItem);
        lpRect->bottom = m_iClientHeight;
        return TRUE;
    }
    
    // ===========================================================================
    // Function name	: HitTest
    // Description	  : 
    // Return type		: virtual int 
    // Argument       : TCHITTESTINFO *pHitTestInfo
    // ===========================================================================
    virtual int HitTest(TCHITTESTINFO *pHitTestInfo)
    {
        int nTabListCount = TabListCount();
        // ignore hits on the buttons
        int iHitX = pHitTestInfo->pt.x;
        if (iHitX < GetTotalArrowWidth())
            return -1;
        
        // check if any tabs were hit
        int x = GetTotalArrowWidth() - m_iXOffset;
        for (int i = 0; i < nTabListCount; i++)
        {
            int iTabWidth = GetTabWidth(i);
            if (i != nTabListCount)
                iTabWidth -=(m_iClientHeight / 2) + 2;
            if ((x <= iHitX) && (iHitX <= x + iTabWidth))
                return i;
            x += iTabWidth;
        }
        
        // hit point is right of rightmost tab
        return -1;
    }
    
    // ===========================================================================
    virtual int GetCurSel() const {return m_iCurSel;}
    
    // ===========================================================================
    // Function name	: SetCurSel
    // Description	  : 
    // Return type		: virtual int 
    // Argument       : int nItem
    // ===========================================================================
    virtual int SetCurSel(int nItem)
    {
        if (nItem < 0 || nItem >= TabListCount())
            return -1;
        
        int iPrevSel = m_iCurSel;
        m_iCurSel = nItem;
        
        // test if we need to center on the selected tab
        CRect rcItem;
        if (GetItemRect(nItem, &rcItem))
        {
            // test if the tab is off on the left
            int iTotalArrowWidth = GetTotalArrowWidth();
            rcItem.left -= iTotalArrowWidth;
            if (rcItem.left < 0)
                m_iXOffset += rcItem.left;
            else
            {
                // test if the tab is off on the right
                rcItem.right -= iTotalArrowWidth;
                int iTabAreaWidth = m_iClientWidth - iTotalArrowWidth;
                if (rcItem.right > iTabAreaWidth)
                    m_iXOffset +=(rcItem.right - iTabAreaWidth);
            }
        }
        
        EnableButtons();
        InvalidateTabs();
        
        return iPrevSel;
    }
    
    // ===========================================================================
    virtual int GetItemCount() { return TabListCount();}
    
    // ===========================================================================
    // Function name	: SetTipText
    // Description	  : 
    // Return type		: virtual void 
    // Argument       : int nItem
    // Argument       : LPCTSTR lpszTabTip
    // ===========================================================================
    virtual void SetTipText(int nItem, LPCTSTR lpszTabTip)
    {
        int nTabListCount = TabListCount();
        if (nItem < nTabListCount)
        {
            for (int i = 0; i < nTabListCount; i++)
            {
                if (i == nItem)
                {
                    CString *pTipItem = GetTabTipListItem(nItem);
                    *pTipItem = lpszTabTip;
                    break;
                }
            }
        }
    }
    
    CString* GetTabTipListItem(int nItem)
    {
        int nIndex = 0;
        CTabList::iterator ListIterator;	
        for (ListIterator = m_TabTipList.begin(); ListIterator != m_TabTipList.end(); ++ListIterator)        
        {
            if (nIndex == nItem)
            {
                return *ListIterator;
            }
            nIndex++;
        }
        
        return NULL;
    }
    
    CString* GetTabListItem(int nItem)
    {
        int nIndex = 0;
        CTabList::iterator ListIterator;	
        for (ListIterator = m_TabList.begin(); ListIterator != m_TabList.end(); ++ListIterator)        
        {
            if (nIndex == nItem)
            {
                return *ListIterator;
            }
            nIndex++;
        }
        
        return NULL;
    }
    
    // ===========================================================================
    // Function name	: GetTipText
    // Description	  : 
    // Return type		: virtual CString 
    // Argument       : int nItem
    // ===========================================================================
    virtual CString GetTipText(int nItem)
    {
        int nTabListCount = TabListCount();
        CString strText;
        if (nItem < nTabListCount)
        {
            for (int i = 0; i < nTabListCount; i++)
            {
                if (i == nItem)
                {
                    strText = * (GetTabTipListItem(nItem));
                    break;
                }
            }
        }
        
        return strText;
    }
    
    // ===========================================================================
    // Function name	: Home
    // Description	  : 
    // Return type		: virtual void 
    // ===========================================================================
    virtual void Home()
    {
        m_iXOffset = 0;
        EnableButtons();
        InvalidateTabs();
    }
    
    // Overrides
    protected:
        
        // Implementation
    public:
        // ===========================================================================
        // Function name	: ~CFlatTabCtrl
        // Description	  : Destructor
        // Return type		: virtual 
        // ===========================================================================
        virtual ~CFlatTabCtrl()
        {
            if (m_pLeftButton)
                delete m_pLeftButton;
            if (m_pRightButton)
                delete m_pRightButton;
            if (m_pHomeButton)
                delete m_pHomeButton;
            if (m_pEndButton)
                delete m_pEndButton;
            
            // free HENHMETAFILE handles.
            if (m_hLeftArrow)
            {
                ::DeleteEnhMetaFile(m_hLeftArrow);
            }
            if (m_hLeftArrowDisabled)
            {
                ::DeleteEnhMetaFile(m_hLeftArrowDisabled);
            }
            if (m_hRightArrow)
            {
                ::DeleteEnhMetaFile(m_hRightArrow);
            }
            if (m_hRightArrowDisabled)
            {
                ::DeleteEnhMetaFile(m_hRightArrowDisabled);
            }
            if (m_hHomeArrow)
            {
                ::DeleteEnhMetaFile(m_hHomeArrow);
            }
            if (m_hHomeArrowDisabled)
            {
                ::DeleteEnhMetaFile(m_hHomeArrowDisabled);
            }
            if (m_hEndArrow)
            {
                ::DeleteEnhMetaFile(m_hEndArrow);
            }
            if (m_hEndArrowDisabled)
            {
                ::DeleteEnhMetaFile(m_hEndArrowDisabled);
            }
            
            CTabList::iterator ListIterator;	
            for (ListIterator = m_TabList.begin(); ListIterator != m_TabList.end(); ++ListIterator)        
            {		
                CString* pItem = *ListIterator;
                if (pItem)
                    delete pItem;
            }
            
            CTabList::iterator ListIterator2;	
            for (ListIterator2 = m_TabTipList.begin(); ListIterator2 != m_TabTipList.end(); ++ListIterator2)        
            {		
                CString* pItem = *ListIterator2;
                if (pItem)
                    delete pItem;
            }
            
            // fix potential resource leak - KStowell - 10-21-99
            if (m_Font)
                m_Font.DeleteObject();
            if (m_BoldFont)
                m_BoldFont.DeleteObject();
        }

        int GetVisibleTabWidth()
        {
            return m_iTotalTabWidth + GetTotalArrowWidth() + 1;
        }
        
        // Generated message map functions
    protected:
        // ===========================================================================
        // Function name	: OnPaint
        // Description	  : 
        // Return type		: LRESULT 
        // Argument       : UINT Msg
        // Argument       : WPARAM wParam
        // Argument       : LPARAM lParam
        // Argument       : BOOL& bHandled
        // ===========================================================================
        LRESULT OnPaint(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            CPaintDC dc(m_hWnd); // device context for painting
            
            CBrush brush;
            brush.CreateSysColorBrush(COLOR_3DFACE);
            CPen penBlack;
            penBlack.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            
            // draw the black line along the left of the control
            HPEN pOldPen = dc.SelectPen(penBlack);
            dc.MoveTo(0, 0);
            dc.LineTo(0, m_iClientHeight);
            
            // draw the black line along the top/bottom of the control
            int iTotalArrowWidth = GetTotalArrowWidth();
            int iEndOfTab = iTotalArrowWidth + m_iTotalTabWidth - m_iXOffset;
            dc.MoveTo(iEndOfTab, m_bTabsOnBottom ? 0 : m_iClientHeight - 1);
            dc.LineTo(m_iClientWidth, m_bTabsOnBottom ? 0 : m_iClientHeight - 1);
            

            // fill the empty background area
            int iFillHeight = m_iClientHeight - (m_bTabsOnBottom ? 0 : 1);
            CRect rcBack(iEndOfTab, m_bTabsOnBottom ? 1 : 0, m_iClientWidth, iFillHeight);
            dc.FillRect(rcBack, brush);
            
            // create a bitmap of all the tabs
            CDC dcMem;
            dcMem.CreateCompatibleDC(NULL);
            CBitmap bitmap;
            bitmap.CreateCompatibleBitmap(dc.m_hDC, m_iTotalTabWidth, m_iClientHeight);
            HBITMAP pOldBitmap = dcMem.SelectBitmap(bitmap);
            HBRUSH pOldBrush = dcMem.SelectBrush(brush);
            CRect rc(0, m_bTabsOnBottom ? 1 : 0, m_iTotalTabWidth, iFillHeight);
            dcMem.FillRect(rc, brush);
            
            int iOverlap = (m_iClientHeight / 2) + 2;
            int x = 0;
            int iSelX = 0;
            for (int i = 0; i < TabListCount(); i++)
            {
                if (i != m_iCurSel)
                    x += DrawTab(dcMem, x, 0, m_iClientHeight, false, m_bTabsOnBottom, * (GetTabListItem(i))) - iOverlap;
                else
                {
                    iSelX = x;
                    x += GetTabWidth(i) - iOverlap;
                }
            }
            if (m_iCurSel >= 0 && TabListCount())
                DrawTab(dcMem, iSelX, 0, m_iClientHeight, true, m_bTabsOnBottom, * (GetTabListItem(m_iCurSel)));
            
            // blit the bitmap onto the control
            dc.BitBlt(iTotalArrowWidth, 0, m_iClientWidth - iTotalArrowWidth, m_iClientHeight, dcMem.m_hDC, m_iXOffset, 0, SRCCOPY);
            
            // draw the final line on the right
            dc.MoveTo(m_iClientWidth-1, 0);
            dc.LineTo(m_iClientWidth-1, m_iClientHeight);

            // fix potential resource leak - KStowell - 10-21-99.
            dcMem.SelectPen(pOldPen);
            dcMem.SelectBitmap(pOldBitmap);
            dcMem.SelectBrush(pOldBrush);
            dcMem.DeleteDC();
            brush.DeleteObject();
            penBlack.DeleteObject();
            bitmap.DeleteObject();
            
            return 0;
        }
        
        // ===========================================================================
        // Function name	: OnLButtonDown
        // Description	  : 
        // Return type		: LRESULT 
        // Argument       : UINT Msg
        // Argument       : WPARAM wParam
        // Argument       : LPARAM lParam
        // Argument       : BOOL& bHandled
        // ===========================================================================
        LRESULT OnLButtonDown(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            TCHITTESTINFO test;
            WORD x, y;
            x = LOWORD(lParam);
            y = HIWORD(lParam);
            CPoint point(x, y);
            test.pt = point;
            int iTab = HitTest(&test);
            if ((iTab != -1) && (iTab != m_iCurSel))
            {
                // warn parent that the selection is about to change
                int id = GetDlgCtrlID();
                NMHDR hdr;
                hdr.hwndFrom = m_hWnd;
                hdr.idFrom = id;
                hdr.code = TCN_SELCHANGING;
                CWindow wnd(GetParent());
                
                if (wnd.SendMessage(WM_NOTIFY, id, (LPARAM)&hdr) == 0)
                {
                    // parent has given permission for the selection to change
                    SetCurSel(iTab);
                    InvalidateTabs();
                    
                    // notify parent that the selection has changed
                    hdr.hwndFrom = m_hWnd;
                    hdr.idFrom = id;
                    hdr.code = TCN_SELCHANGE;
                    wnd.SendMessage(WM_NOTIFY, id, (LPARAM)&hdr);
                }
            }
            
            return 0;
        }
        
        // ===========================================================================
        // Function name	: OnSize
        // Description	  : 
        // Return type		: LRESULT 
        // Argument       : UINT Msg
        // Argument       : WPARAM wParam
        // Argument       : LPARAM lParam
        // Argument       : BOOL& bHandled
        // ===========================================================================
        LRESULT OnSize(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            WORD cx, cy;
            cx = LOWORD(lParam);
            cy = HIWORD(lParam);
            
            // TODO: Add your message handler code here
            if (m_bHasHomeEnd)
            {
                m_pHomeButton->MoveWindow(1, 0, 16, cy);
                m_pLeftButton->MoveWindow(17, 0, 16, cy);
                m_pRightButton->MoveWindow(33, 0, 16, cy);
                m_pEndButton->MoveWindow(49, 0, 16, cy);
            }
            else 
            {
                if (m_bHasArrows)
                {
                    // TCC: ADDED to handle situations where ther are no arrows
                    m_pLeftButton->MoveWindow(1, 0, 16, cy);
                    m_pRightButton->MoveWindow(17, 0, 16, cy);
                }
            }
            
            m_iClientWidth  = cx;
            m_iClientHeight = cy;
            
            CreateTabFont(cy);
            
            m_iTotalTabWidth = GetTotalTabWidth();
            EnableButtons();
            return 0;
        }
        
        // ===========================================================================
        // Function name	: OnCreate
        // Description	  : 
        // Return type		: LRESULT 
        // Argument       : UINT Msg
        // Argument       : WPARAM wParam
        // Argument       : LPARAM lParam
        // Argument       : BOOL& bHandled
        // ===========================================================================
        LRESULT OnCreate(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {    
            LPCREATESTRUCT lpCreateStruct = (LPCREATESTRUCT)lParam;
            
            // TODO: Add your specialized creation code here
            if (lpCreateStruct->style & FTS_BOTTOM)
            {
                m_bTabsOnBottom = true;
            }
            
            if (lpCreateStruct->style & TCS_TOOLTIPS)
            {
                m_ToolTip.Create(m_hWnd);
                m_ToolTip.Activate(TRUE);
                m_ToolTip.AddTool(m_hWnd, _T("Flat Tab Tool Tip"));
                m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // Allow multiline tooltips
            }
            
            if (lpCreateStruct->style & FTS_HASARROWS)
            {
                m_bHasArrows = true;
                POINT points[4];
                points[0].x = 100;
                points[0].y = 0;
                points[1].x = 0;
                points[1].y = 50;
                points[2].x = 100;
                points[2].y = 100;
                points[3].x = 100;
                points[3].y = 0;
                
                ////////////////////////////////////////////////////////////////////////////////
                // This portion of the code was re-written to help eliminate a nasty resource
                // leak. It would seem that the CMetaFileDC was not freeing the GDI resources
                // completely whenever CMetaFileDC::CloseEnhanced() was called. This may be due 
                // to a BUG in MFC or perhaps this class, in any event, this seems to resolve 
                // the issue. - KStowell - 10-23-99.
                ////////////////////////////////////////////////////////////////////////////////
                HPEN   hPen			  = ::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HPEN   hPenDisabled   = ::CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
                HBRUSH hBrush		  = ::CreateSolidBrush(RGB(0, 0, 0));
                HBRUSH hBrushDisabled = ::CreateSolidBrush(RGB(128, 128, 128));;
                
                // create left arrow metafile
                HDC hMetaDC	     = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                HPEN hOldPen     = (HPEN)::SelectObject(hMetaDC, hPen);
                HBRUSH hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrush);
                ::Polygon(hMetaDC, points, 4);
                ::SelectObject(hMetaDC, hOldPen);
                ::SelectObject(hMetaDC, hOldBrush);
                m_hLeftArrow = ::CloseEnhMetaFile(hMetaDC);
                
                // create disabled left arrow metafile
                hMetaDC	  = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                hOldPen   = (HPEN)::SelectObject(hMetaDC, hPenDisabled);
                hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrushDisabled);
                ::Polygon(hMetaDC, points, 4);
                ::SelectObject(hMetaDC, hOldPen);
                ::SelectObject(hMetaDC, hOldBrush);
                m_hLeftArrowDisabled = ::CloseEnhMetaFile(hMetaDC);
                
                points[0].x = 0;
                points[0].y = 0;
                points[1].x = 100;
                points[1].y = 50;
                points[2].x = 0;
                points[2].y = 100;
                points[3].x = 0;
                points[3].y = 0;
                
                // create right arrow metafile
                hMetaDC	  = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                hOldPen   = (HPEN)::SelectObject(hMetaDC, hPen);
                hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrush);
                ::Polygon(hMetaDC, points, 4);
                ::SelectObject(hMetaDC, hOldPen);
                ::SelectObject(hMetaDC, hOldBrush);
                m_hRightArrow = ::CloseEnhMetaFile(hMetaDC);
                
                // create disabled right arrow metafile
                hMetaDC	  = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                hOldPen   = (HPEN)::SelectObject(hMetaDC, hPenDisabled);
                hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrushDisabled);
                ::Polygon(hMetaDC, points, 4);
                ::SelectObject(hMetaDC, hOldPen);
                ::SelectObject(hMetaDC, hOldBrush);
                m_hRightArrowDisabled = ::CloseEnhMetaFile(hMetaDC);
                
                CRect rcButton(0, 0, 16, 16);
                m_pLeftButton = new CMetaFileButton;
                m_pLeftButton->Create(m_hWnd, rcButton, NULL, WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_OWNERDRAW, 0, IDC_LEFTBUTTON);
                m_pLeftButton->SetMetaFiles(m_hLeftArrow, 0, 0, m_hLeftArrowDisabled);
                
                m_pRightButton = new CMetaFileButton;
                m_pRightButton->Create(m_hWnd, rcButton, NULL, WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_OWNERDRAW, 0, IDC_RIGHTBUTTON);
                m_pRightButton->SetMetaFiles(m_hRightArrow, 0, 0, m_hRightArrowDisabled);
                
                if (lpCreateStruct->style & FTS_HASHOMEEND)
                {
                    m_bHasHomeEnd = true;
                    
                    // Setup the Home arrow button
                    points[0].x = 100;
                    points[0].y = 0;
                    points[1].x = 0;
                    points[1].y = 50;
                    points[2].x = 100;
                    points[2].y = 100;
                    points[3].x = 100;
                    points[3].y = 0;
                    
                    // create right arrow metafile
                    HDC hMetaDC	     = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                    HPEN hOldPen     = (HPEN)::SelectObject(hMetaDC, hPen);
                    HBRUSH hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrush);
                    ::Polygon(hMetaDC, points, 4);
                    ::MoveToEx(hMetaDC, 0, 0, NULL);
                    ::LineTo(hMetaDC, 0, 100);
                    ::SelectObject(hMetaDC, hOldPen);
                    ::SelectObject(hMetaDC, hOldBrush);
                    m_hHomeArrow = ::CloseEnhMetaFile(hMetaDC);
                    
                    // create disabled right arrow metafile
                    hMetaDC	  = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                    hOldPen   = (HPEN)::SelectObject(hMetaDC, hPenDisabled);
                    hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrushDisabled);
                    ::Polygon(hMetaDC, points, 4);
                    ::MoveToEx(hMetaDC, 0, 0, NULL);
                    ::LineTo(hMetaDC, 0, 100);
                    ::SelectObject(hMetaDC, hOldPen);
                    ::SelectObject(hMetaDC, hOldBrush);
                    m_hHomeArrowDisabled = ::CloseEnhMetaFile(hMetaDC);
                    
                    m_pHomeButton = new CMetaFileButton;
                    m_pHomeButton->Create(m_hWnd, rcButton, NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, IDC_HOMEBUTTON);
                    m_pHomeButton->SetMetaFiles(m_hHomeArrow, 0, 0, m_hHomeArrowDisabled);
                    
                    // Setup the End arrow button
                    points[0].x = 0;
                    points[0].y = 0;
                    points[1].x = 100;
                    points[1].y = 50;
                    points[2].x = 0;
                    points[2].y = 100;
                    points[3].x = 0;
                    points[3].y = 0;
                    
                    // create right arrow metafile
                    hMetaDC	  = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                    hOldPen   = (HPEN)::SelectObject(hMetaDC, hPen);
                    hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrush);
                    ::Polygon(hMetaDC, points, 4);
                    ::MoveToEx(hMetaDC, 100, 0, NULL);
                    ::LineTo(hMetaDC, 100, 100);
                    ::SelectObject(hMetaDC, hOldPen);
                    ::SelectObject(hMetaDC, hOldBrush);
                    m_hEndArrow = ::CloseEnhMetaFile(hMetaDC);
                    
                    // create disabled right arrow metafile
                    hMetaDC	  = ::CreateEnhMetaFile(NULL, NULL, NULL, NULL);
                    hOldPen   = (HPEN)::SelectObject(hMetaDC, hPenDisabled);
                    hOldBrush = (HBRUSH)::SelectObject(hMetaDC, hBrushDisabled);
                    ::Polygon(hMetaDC, points, 4);
                    ::MoveToEx(hMetaDC, 100, 0, NULL);
                    ::LineTo(hMetaDC, 100, 100);
                    ::SelectObject(hMetaDC, hOldPen);
                    ::SelectObject(hMetaDC, hOldBrush);
                    m_hEndArrowDisabled = ::CloseEnhMetaFile(hMetaDC);
                    
                    m_pEndButton = new CMetaFileButton;
                    m_pEndButton->Create(m_hWnd, rcButton, NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, IDC_ENDBUTTON);
                    m_pEndButton->SetMetaFiles(m_hEndArrow, 0, 0, m_hEndArrowDisabled);
                    
                    m_pHomeButton->MoveWindow(1, 0, 16, 16);
                    m_pLeftButton->MoveWindow(17, 0, 16, 16);
                    m_pRightButton->MoveWindow(33, 0, 16, 16);
                    m_pEndButton->MoveWindow(49, 0, 16, 16);
                }
                else 
                {
                    m_pLeftButton->MoveWindow(1, 0, 16, 16);
                    m_pRightButton->MoveWindow(17, 0, 16, 16);
                }
                
                // fix potential resource leak - KStowell - 10-23-99
                ::DeleteObject(hPen);
                ::DeleteObject(hPenDisabled);
                ::DeleteObject(hBrush);
                ::DeleteObject(hBrushDisabled);
      }
      
      return 0;
  }
  
  // ===========================================================================
  // Function name	: OnLeftArrow
  // Description	  : 
  // Return type		: LRESULT 
  // Argument       : WORD wNotifyCode
  // Argument       : WORD wID
  // Argument       : HWND hWndCtl
  // Argument       : BOOL& bHandled
  // ===========================================================================
  LRESULT OnLeftArrow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
  {
      // Move the tabs right, ensuring that we move right by one
      // whole tab each time ala Microsoft Access
      
      CPoint Point(GetTotalArrowWidth() + 1, 1);
      
      TCHITTESTINFO test;
      test.pt = Point;
      int iTab = HitTest(&test);
      
      if (iTab != -1)
      {
          m_iXOffset = 0;
          for (int iLoop = 0; iLoop <= iTab - 1; iLoop++)
          {
              RECT rect;
              if (GetItemRect(iLoop, &rect))
              {
                  m_iXOffset += rect.left;
              }
          }
          m_iXOffset -= GetTotalArrowWidth();
          
          EnableButtons();
          InvalidateTabs();
      }
      
      return 0;
  }
  
  // ===========================================================================
  // Function name	: OnRightArrow
  // Description	  : 
  // Return type		: LRESULT 
  // Argument       : WORD wNotifyCode
  // Argument       : WORD wID
  // Argument       : HWND hWndCtl
  // Argument       : BOOL& bHandled
  // ===========================================================================
  LRESULT OnRightArrow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
  {
      // Move the tabs left, ensuring that we move left by one
      // whole tab each time ala Microsoft Access
      
      CPoint Point(GetTotalArrowWidth() + 1, 1);
      
      TCHITTESTINFO test;
      test.pt = Point;
      int iTab = HitTest(&test);
      
      if (iTab != -1)
      {
          m_iXOffset = 0;
          for (int iLoop = 0; iLoop <= iTab + 1; iLoop++)
          {
              RECT rect;
              if (GetItemRect(iLoop, &rect))
              {
                  m_iXOffset += rect.left;
              }
          }
          m_iXOffset -= GetTotalArrowWidth();
          
          EnableButtons();
          InvalidateTabs();
      }
      
      return 0;
  }
  
  // ===========================================================================
  // Function name	: OnHomeArrow
  // Description	  : 
  // Return type		: LRESULT 
  // Argument       : WORD wNotifyCode
  // Argument       : WORD wID
  // Argument       : HWND hWndCtl
  // Argument       : BOOL& bHandled
  // ===========================================================================
  LRESULT OnHomeArrow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
  {
      m_iXOffset = 0;
      EnableButtons();
      InvalidateTabs();
      return 0;
  }
  
  // ===========================================================================
  // Function name	: OnEndArrow
  // Description	  : 
  // Return type		: LRESULT 
  // Argument       : WORD wNotifyCode
  // Argument       : WORD wID
  // Argument       : HWND hWndCtl
  // Argument       : BOOL& bHandled
  // ===========================================================================
  LRESULT OnEndArrow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
  {
      // TODO: Add your control notification handler code here
      m_iXOffset = m_iTotalTabWidth + GetTotalArrowWidth() - m_iClientWidth + 1;
      EnableButtons();
      InvalidateTabs();
      return 0;
  }
  
private:
    
    // ===========================================================================
    // Function name	: CreateTabFont
    // Description	  : 
    // Return type		: void 
    // Argument       : const int iTabHeight
    // ===========================================================================
    void CreateTabFont(const int iTabHeight)
    {
        int iHeight = - (iTabHeight - 6);
        
        if (m_Font)
        {
            m_Font.DeleteObject();
            m_Font = NULL;
        }
        m_Font.CreateFont(iHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_STROKE_PRECIS, CLIP_STROKE_PRECIS, PROOF_QUALITY, FF_SWISS, NULL);
        
        if (m_BoldFont)
            m_BoldFont.DeleteObject();
        m_BoldFont.CreateFont(iHeight, 0, 0, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_STROKE_PRECIS, CLIP_STROKE_PRECIS, PROOF_QUALITY, FF_SWISS, NULL);
    }
    
    // ===========================================================================
    // Function name	: GetTabWidth
    // Description	  : 
    // Return type		: int 
    // Argument       : const int nItem
    // ===========================================================================
    int GetTabWidth(const int nItem)
    {
        CDC dc;
        dc.CreateCompatibleDC(NULL);
        HFONT pOldFont = dc.SelectFont(m_BoldFont);
        CSize size;
        dc.GetTextExtent(* (GetTabListItem(nItem)), -1, &size);
        
        // fix potential resource leak - KStowell - 10-20-99.
        dc.SelectFont(pOldFont);
        dc.DeleteDC();
        
        return size.cx + m_iClientHeight + (m_iClientHeight / 2);
    }
    
    // ===========================================================================
    // Function name	: GetTotalArrowWidth
    // Description	  : 
    // Return type		: int 
    // Argument       : void
    // ===========================================================================
    int GetTotalArrowWidth(void) const
    {
        int iWidth = 1;
        if (m_bHasArrows)
            iWidth += 32;
        if (m_bHasHomeEnd)
            iWidth += 32;
        return iWidth;
    }
    // ===========================================================================
    // Function name	: GetTotalTabWidth
    // Description	  : 
    // Return type		: int 
    // Argument       : void
    // ===========================================================================
    int GetTotalTabWidth(void)
    {
        int iWidth = 0;
        int nTabListCount = TabListCount();
        for (int i = 0; i < nTabListCount; i++)
        {
            iWidth += GetTabWidth(i);
            if (i != nTabListCount - 1)
            {
                iWidth -=(m_iClientHeight / 2);
            }
        }
        return iWidth + 2;  
    }
    
    // ===========================================================================
    // Function name	: InvalidateTabs
    // Description	  : 
    // Return type		: void 
    // Argument       : void
    // ===========================================================================
    void InvalidateTabs(void)
    {
        if (m_hWnd)
        {
            // invalidate the visible tab area
            // to minimise flicker - don't erase the background
            CRect rcTabs;
            
            rcTabs.left = GetTotalArrowWidth();
            rcTabs.top = 0;
            rcTabs.right = rcTabs.left + (m_iTotalTabWidth - m_iXOffset);
            rcTabs.bottom = m_iClientHeight;
            InvalidateRect(&rcTabs, false);
            // invalidate the blank area to the right of the tabs
            if (rcTabs.right < m_iClientWidth)
            {
                rcTabs.left = rcTabs.right;
                rcTabs.right = m_iClientWidth;
                InvalidateRect(&rcTabs, true);
            }
        }
    }
    
    // ===========================================================================
    // Function name	: EnableButtons
    // Description	  : 
    // Return type		: void 
    // Argument       : void
    // ===========================================================================
    void EnableButtons(void)
    {
        if (m_bHasArrows)  // TCC: ADDED to handle situations where ther are no arrows
        {
            m_pLeftButton->EnableWindow(m_iXOffset);
            if (m_iClientWidth + m_iXOffset < m_iTotalTabWidth + GetTotalArrowWidth() + 1)
                m_pRightButton->EnableWindow(true);
            else
                m_pRightButton->EnableWindow(false);
            if (m_bHasHomeEnd)
            {
                m_pHomeButton->EnableWindow(m_iXOffset);
                m_pEndButton->EnableWindow(m_pRightButton->IsWindowEnabled());
            }
        }
    }
    
    // ===========================================================================
    // Function name	: DrawTab
    // Description	    : 
    // Return type		: int 
    // Argument         : CDC &dc
    // Argument         : const int x
    // Argument         : const int y
    // Argument         : const int iHeight
    // Argument         : bool bSelected
    // Argument         : bool bBottom
    // Argument         : LPCTSTR lpszTab
    // ===========================================================================
    int DrawTab(CDC &dc, const int x, const int y, const int iHeight, bool bSelected, bool bBottom, LPCTSTR lpszTab) const
    {
        HFONT pOldFont = dc.SelectFont(m_BoldFont);
        CSize size;
        dc.GetTextExtent(lpszTab, -1, &size);
        CRect rcText;
        rcText.left = iHeight + x - 4;
        rcText.top = 2 + y;
        rcText.right = size.cx + iHeight + x - 4;
        rcText.bottom = size.cy + 2 + y;
        
        int iHalf = iHeight / 2;
        int iWidth = iHeight + iHalf;
        
        COLORREF crBack;
        COLORREF crFore;
        if (bSelected)
        {
            crBack = GetSysColor(COLOR_WINDOW);
            crFore = GetSysColor(COLOR_WINDOWTEXT);
        }
        else 
        {
            crBack = GetSysColor(COLOR_3DFACE);
            crFore = GetSysColor(COLOR_BTNTEXT);
        }
        
        CPen penOutline;
        penOutline.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
        CPen penShadow;
        penShadow.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
        CBrush brush;
        brush.CreateSolidBrush(crBack);
        
        POINT points[4];
        points[0].x = x;
        points[0].y = iHeight + y - 1;
        points[1].x = iHalf + x;
        points[1].y = y;
        points[2].x = size.cx + iHeight + x;
        points[2].y = y;
        points[3].x = size.cx + iWidth + x;
        points[3].y = iHeight + y - 1;
        
        if (bBottom)
        {
            // swap vertical coordinates
            points[0].y = points[1].y;
            points[2].y = points[3].y;
            points[1].y = points[2].y;
            points[3].y = points[0].y;
        }
        
        HPEN pOldPen = dc.SelectPen(penOutline);
        dc.SetBkColor(crBack);
        HBRUSH pOldBrush = dc.SelectBrush(brush);
        dc.Polygon(points, 4);
        dc.SelectFont(bSelected ? m_BoldFont : m_Font);
        dc.SetTextColor(crFore);
        dc.DrawText(lpszTab, -1, rcText, DT_CENTER);
        dc.SelectPen(penShadow);
        
        if (bSelected)
        {
            dc.MoveTo(iHalf + x + 1, points[1].y);
            dc.LineTo(size.cx + iHeight + x, points[1].y);
            CPen penBack;
            penBack.CreatePen(PS_SOLID, 1, crBack);
            dc.SelectPen(penBack);
            dc.MoveTo(x + 1, points[0].y);
            dc.LineTo(size.cx + iWidth + x, points[0].y);
        }
        else 
        {
            dc.MoveTo(iHalf + x + 1, points[1].y);
            dc.LineTo(size.cx + iHeight + x - 1, points[1].y);
            dc.LineTo(size.cx + iWidth + x - 1, points[0].y);
        }
        
        // fix potential resource leak - KStowell - 10-21-99
        dc.SelectFont(pOldFont);
        dc.SelectPen(pOldPen);
        dc.SelectBrush(pOldBrush);
        penOutline.DeleteObject();
        penShadow.DeleteObject();
        brush.DeleteObject();
        
        return size.cx + iWidth;
    }
    
    bool m_bHasArrows;
    bool m_bHasHomeEnd;
    bool m_bTabsOnBottom;
    int m_iCurSel;
    int m_iClientWidth;
    int m_iClientHeight;
    int m_iTotalTabWidth;
    int m_iXOffset;
    CTabList m_TabList;
    CTabList m_TabTipList;
    CFont m_Font;
    CFont m_BoldFont;
    CMetaFileButton *m_pLeftButton;
    CMetaFileButton *m_pRightButton;
    CMetaFileButton *m_pHomeButton;
    CMetaFileButton *m_pEndButton;
    HENHMETAFILE m_hLeftArrow;
    HENHMETAFILE m_hLeftArrowDisabled;
    HENHMETAFILE m_hRightArrow;
    HENHMETAFILE m_hRightArrowDisabled;
    HENHMETAFILE m_hHomeArrow;
    HENHMETAFILE m_hHomeArrowDisabled;
    HENHMETAFILE m_hEndArrow;
    HENHMETAFILE m_hEndArrowDisabled;
    
    CToolTipCtrl m_ToolTip;
};


#endif
