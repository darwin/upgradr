//========================================================================================
//
// Module:			OptionSheet
// Author:          Pascal Hurni
// Creation Date:	20.04.2004
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
//========================================================================================

#ifndef __MORTIMER_OPTIONSHEETLAYOUTSIMPLE_H__
#define __MORTIMER_OPTIONSHEETLAYOUTSIMPLE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MOptionSheetLayout.h>

#include "GradientStatic.h"

//========================================================================================
// encapsulate these classes in a namespace

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

//========================================================================================

class COptionSelectionLayoutTab : public COptionSelectionLayout
{
protected:
	COptionSelectionTabCtrl m_TabCtrl;

	UINT m_Spacing;
	CSize m_ButtonSize;

	CRect m_PagesRect;
	CSize m_TabSize;

public:
	BEGIN_MSG_MAP(COptionSelectionLayoutTab)
	END_MSG_MAP()

	bool Create(COptionSheet *pSheet)
	{
		m_pSheet = pSheet;

		m_Spacing = GetCtrlSpacing();
		m_ButtonSize = GetButtonSize();

		CRect Rect(m_Spacing,m_Spacing, 100,100);
		bool Result = m_TabCtrl.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP, 0, IDC_OS_PAGESELECTION) != NULL;
		if (Result)
			m_TabCtrl.SetFont(pSheet->GetFont());

		return Result;
	}

	void Destroy()
	{
		m_TabCtrl.DestroyWindow();
	}

	COptionSelectionCtrl *GetSelectionControl()
	{
		return &m_TabCtrl;
	}

	void SetPagesSize(CSize PagesSize)
	{
		EnforcePagesSize(PagesSize);

		CRect Rect(0,0, PagesSize.cx, PagesSize.cy);
		m_TabCtrl.AdjustRect(TRUE, &Rect);

		m_TabSize = Rect.Size();
		m_PagesRect = CRect(CSize(m_Spacing, m_Spacing)-Rect.TopLeft(), PagesSize);
	}

	CRect GetPagesRect()
	{
		return m_PagesRect;
	}
	
	CSize GetSheetClientSize()
	{
		return m_TabSize + CSize(m_Spacing*2, m_Spacing*3+m_ButtonSize.cy);
	}

	void UpdateLayout()
	{
		m_TabCtrl.SetWindowPos(NULL, 0,0, m_TabSize.cx, m_TabSize.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		UpdateButtonsPos();
	}
};

//========================================================================================

class COptionSelectionLayoutTree : public COptionSelectionLayout
{
protected:
	COptionSelectionTreeCtrl m_TreeCtrl;
	WTL::CStatic m_SelectionFrame;
	WTL::CStatic m_PageFrame;

	UINT m_Spacing;
	CSize m_ButtonSize;

	CRect m_TreeRect;
	CRect m_PagesRect;

	UINT m_TreeWidth;
	double m_TreeWidthRatio;
	DWORD m_TreeStyle;
	DWORD m_TreeExStyle;

public:
	BEGIN_MSG_MAP(COptionSelectionLayoutTree)
	END_MSG_MAP()

	COptionSelectionLayoutTree() : m_TreeWidth(0), m_TreeWidthRatio(0.35), m_TreeStyle(WS_BORDER|TVS_FULLROWSELECT|TVS_SHOWSELALWAYS), m_TreeExStyle(0)
	{
	}

	void SetTreeWidth(UINT Width)
	{
		m_TreeWidth = Width;
		m_TreeWidthRatio = 0.0;
	}

	// The Ratio is the factor by which to multiply the Pages width to get the Tree Width.
	// example: Ratio = 0.35, PagesWidth = 400, TreeWidth becomes 400*0.35 = 140
	void SetTreeWidthRatio(double Ratio)
	{
		m_TreeWidthRatio = Ratio;
		m_TreeWidth = 0;
	}

	// Warning: The styles are not filtered before passed to the tree creation, so don't set non applicable styles.
	void ModifyTreeStyle(DWORD StyleRemove, DWORD StyleAdd, DWORD ExStyleRemove, DWORD ExStyleAdd)
	{
		m_TreeStyle &= ~StyleRemove;
		m_TreeStyle |= StyleAdd;

		m_TreeExStyle &= ~ExStyleRemove;
		m_TreeExStyle |= ExStyleRemove;
	}

	void GetTreeStyle(DWORD &Style, DWORD &ExStyle)
	{
		Style = m_TreeStyle;
		ExStyle = m_TreeExStyle;
	}


	bool Create(COptionSheet *pSheet)
	{
		m_pSheet = pSheet;

		m_Spacing = GetCtrlSpacing();
		m_ButtonSize = GetButtonSize();

		CRect Rect(m_Spacing,m_Spacing, 100,100);
		bool Result = m_TreeCtrl.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|m_TreeStyle, m_TreeExStyle, IDC_OS_PAGESELECTION) != NULL;
		if (Result)
			m_TreeCtrl.SetFont(pSheet->GetFont());

		m_PageFrame.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_ETCHEDFRAME);
		m_SelectionFrame.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_ETCHEDFRAME);

		return Result;
	}

	void Destroy()
	{
		m_TreeCtrl.DestroyWindow();
		m_PageFrame.DestroyWindow();
		m_SelectionFrame.DestroyWindow();
	}

	COptionSelectionCtrl *GetSelectionControl()
	{
		return &m_TreeCtrl;
	}

	void SetPagesSize(CSize PagesSize)
	{
		EnforcePagesSize(PagesSize);
		UINT TreeWidth = m_TreeWidth == 0 ? UINT(double(PagesSize.cx)*m_TreeWidthRatio) : m_TreeWidth;

		m_TreeRect.left = m_Spacing*2;
		m_TreeRect.top = m_Spacing*2;
		m_TreeRect.right = m_TreeRect.left+TreeWidth;
		m_TreeRect.bottom = m_TreeRect.top+PagesSize.cy;

		m_PagesRect.left = m_TreeRect.right+2*m_Spacing;
		m_PagesRect.top = m_TreeRect.top;
		m_PagesRect.right = m_PagesRect.left+PagesSize.cx;
		m_PagesRect.bottom = m_TreeRect.bottom;
	}

	CRect GetPagesRect()
	{
		return m_PagesRect;
	}
	
	CSize GetSheetClientSize()
	{
		return CSize(m_PagesRect.right+2*m_Spacing, m_PagesRect.bottom+m_Spacing*3+m_ButtonSize.cy);
	}

	void UpdateLayout()
	{
		CSize SheetSize = GetSheetClientSize();

		m_TreeCtrl.SetWindowPos(NULL, m_TreeRect.left,m_TreeRect.top, m_TreeRect.Width(),m_TreeRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		m_SelectionFrame.SetWindowPos(NULL, 0,0, SheetSize.cx-2*m_Spacing,m_PagesRect.Height()+2*m_Spacing, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		m_PageFrame.SetWindowPos(NULL, m_PagesRect.left-m_Spacing,m_PagesRect.top-m_Spacing, m_PagesRect.Width()+2*m_Spacing,m_PagesRect.Height()+2*m_Spacing, SWP_NOZORDER | SWP_NOACTIVATE);

		UpdateButtonsPos();
	}
};

//========================================================================================

class COptionSelectionLayoutTreeBanner : public COptionSelectionLayout, public CMessageMap
{
protected:
	COptionSelectionTreeCtrl m_TreeCtrl;
	CGradientStatic m_Banner;
	CFont m_BannerFont;
	CContainedWindowT<WTL::CStatic> m_PageBorder;

	UINT m_Spacing;
	CSize m_ButtonSize;

	CRect m_TreeRect;
	CRect m_PagesRect;
	CRect m_PaneRect;

	UINT m_TreeWidth;
	double m_TreeWidthRatio;
	DWORD m_TreeStyle;
	DWORD m_TreeExStyle;

public:
	BEGIN_MSG_MAP(COptionSelectionLayoutTreeBanner)
	ALT_MSG_MAP(1)
//		MSG_WM_ERASEBKGND(OnEraseBkgnd)
	END_MSG_MAP()

	COptionSelectionLayoutTreeBanner() : m_TreeWidth(0), m_TreeWidthRatio(0.35), m_TreeStyle(TVS_FULLROWSELECT|TVS_SHOWSELALWAYS), m_TreeExStyle(WS_EX_CLIENTEDGE)
	                                    ,m_PageBorder(this, 1)
	{
	}

	void SetTreeWidth(UINT Width)
	{
		m_TreeWidth = Width;
		m_TreeWidthRatio = 0.0;
	}

	// The Ratio is the factor by which to multiply the Pages width to get the Tree Width.
	// example: Ratio = 0.35, PagesWidth = 400, TreeWidth becomes 400*0.35 = 140
	void SetTreeWidthRatio(double Ratio)
	{
		m_TreeWidthRatio = Ratio;
		m_TreeWidth = 0;
	}

	// Warning: The styles are not filtered before passed to the tree creation, so don't set non applicable styles.
	void ModifyTreeStyle(DWORD StyleRemove, DWORD StyleAdd, DWORD ExStyleRemove, DWORD ExStyleAdd)
	{
		m_TreeStyle &= ~StyleRemove;
		m_TreeStyle |= StyleAdd;

		m_TreeExStyle &= ~ExStyleRemove;
		m_TreeExStyle |= ExStyleRemove;
	}

	void GetTreeStyle(DWORD &Style, DWORD &ExStyle)
	{
		Style = m_TreeStyle;
		ExStyle = m_TreeExStyle;
	}

	bool Create(COptionSheet *pSheet)
	{
		m_pSheet = pSheet;

		m_Spacing = GetCtrlSpacing();
		m_ButtonSize = GetButtonSize();

		CRect Rect(m_Spacing,m_ButtonSize.cy+2*m_Spacing, 100,100);
		bool Result = m_TreeCtrl.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|m_TreeStyle, m_TreeExStyle, IDC_OS_PAGESELECTION) != NULL;
		if (Result)
			m_TreeCtrl.SetFont(pSheet->GetFont());

		Result |= (m_Banner.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS) != NULL);
		Result |= (m_PageBorder.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_OWNERDRAW) != NULL); //SS_ETCHEDHORZ);
		if (!Result)
			return false;

		LOGFONT LogFont;
		WTL::CFontHandle DialogFont = m_pSheet->GetFont();
		DialogFont.GetLogFont(&LogFont);
		LogFont.lfWeight = FW_BOLD;
		LogFont.lfHeight *= 3;
		LogFont.lfHeight /= 2;
		m_BannerFont.CreateFontIndirect(&LogFont);
		m_Banner.SetFont(m_BannerFont);

		return Result;
	}

	void Destroy()
	{
		m_TreeCtrl.DestroyWindow();
		m_Banner.DestroyWindow();
		m_PageBorder.DestroyWindow();
	}

	COptionSelectionCtrl *GetSelectionControl()
	{
		return &m_TreeCtrl;
	}

	void SetPagesSize(CSize PagesSize)
	{
		EnforcePagesSize(PagesSize);
		UINT TreeWidth = m_TreeWidth == 0 ? UINT(double(PagesSize.cx)*m_TreeWidthRatio) : m_TreeWidth;

		CRect PaneSpacing(0,0,0,2);		// Should be a CMargins
#ifdef __ATLTHEME_H__
		if (IsAppReallyThemed())
		{
			CTheme Theme;
			if (Theme.OpenThemeData(m_pSheet->m_hWnd, L"Tab"))
			{
				CRect RectContent(0,0, PagesSize.cx, PagesSize.cy);
				CRect RectExtent;
				Theme.GetThemeBackgroundExtent(NULL, TABP_PANE, 0, RectContent, RectExtent);
				Theme.CloseThemeData();
				PaneSpacing.SetRect(-RectExtent.left, -RectExtent.top, RectExtent.right-RectContent.right, RectExtent.bottom-RectContent.bottom);
			}
		}
#endif

		m_TreeRect.left = m_Spacing;
		m_TreeRect.right = m_TreeRect.left+TreeWidth;

		m_PaneRect.left = m_TreeRect.right+m_Spacing*2/3;
		m_PaneRect.top = m_Spacing;
		m_PaneRect.right = m_PaneRect.left+PaneSpacing.left+PagesSize.cx+PaneSpacing.right;
		m_PaneRect.bottom = m_PaneRect.top+PaneSpacing.top+m_ButtonSize.cy+PagesSize.cy+PaneSpacing.bottom;

		m_PagesRect.left = m_PaneRect.left+PaneSpacing.left;
		m_PagesRect.top = m_PaneRect.top+PaneSpacing.top+m_ButtonSize.cy;
		m_PagesRect.right = m_PagesRect.left+PagesSize.cx;
		m_PagesRect.bottom = m_PagesRect.top+PagesSize.cy;

		m_TreeRect.top = m_PaneRect.top;
		m_TreeRect.bottom = m_PaneRect.bottom;
	}

	CRect GetPagesRect()
	{
		return m_PagesRect;
	}
	
	CSize GetSheetClientSize()
	{
		return CSize(m_PaneRect.right+m_Spacing, m_PaneRect.bottom+m_Spacing*2+m_ButtonSize.cy);
	}

	void UpdateLayout()
	{
		CSize SheetSize = GetSheetClientSize();

		m_TreeCtrl.SetWindowPos(NULL, m_TreeRect.left,m_TreeRect.top, m_TreeRect.Width(),m_TreeRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		m_Banner.SetWindowPos(NULL, m_PagesRect.left,m_PagesRect.top-m_ButtonSize.cy, m_PagesRect.Width(),m_ButtonSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
		m_PageBorder.SetWindowPos(NULL, m_PaneRect.left,m_PaneRect.top, m_PaneRect.Width(),m_PaneRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		UpdateButtonsPos();
	}

	void OnItemChange(COptionItem *pNewItem, COptionItem *pOldItem)
	{
		// Set the banner text to the item selected
		if (pNewItem) m_Banner.SetWindowText(pNewItem->GetCaption());
	}

protected:
	BOOL OnEraseBkgnd(HDC hDC) 
	{
//		SetMsgHandled(FALSE);
//		CRect Rect;
//		m_PageBorder.GetClientRect(&Rect);
//#ifdef __ATLTHEME_H__
//		if (IsAppReallyThemed())
//		{
//			CTheme Theme;
//			if (Theme.OpenThemeData(m_PageBorder.m_hWnd, L"Tab"))
//			{
//				Theme.DrawThemeBackground(hDC, TABP_PANE, 0, Rect, NULL);
//				Theme.CloseThemeData();
//				SetMsgHandled(TRUE);
//			}
//		}
//		else
//#endif
//		{
//			DrawEdge(hDC, &Rect, EDGE_ETCHED, BF_BOTTOM);
//			SetMsgHandled(TRUE);
//		}
		return TRUE;
	}
};

//========================================================================================

class COptionSelectionLayoutTreeBannerToolbar : public COptionSelectionLayoutTreeBanner
{
protected:
	WTL::CStatic m_ToolbarCaption;
	CRect m_ToolbarRect;

public:
	bool Create(COptionSheet *pSheet)
	{
		if (!COptionSelectionLayoutTreeBanner::Create(pSheet))
			return false;

		CRect Rect(m_Spacing,m_ButtonSize.cy+2*m_Spacing, 100,100);
		return m_ToolbarCaption.Create(pSheet->m_hWnd, Rect, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_CENTERIMAGE, WS_EX_STATICEDGE)?true:false;
	}

	void Destroy()
	{
		COptionSelectionLayoutTreeBanner::Destroy();
		m_ToolbarCaption.DestroyWindow();
	}

	void SetPagesSize(CSize PagesSize)
	{
		COptionSelectionLayoutTreeBanner::SetPagesSize(PagesSize);

		m_ToolbarRect.left = m_TreeRect.left;
		m_ToolbarRect.top = m_TreeRect.top;
		m_ToolbarRect.right = m_TreeRect.right;
		m_ToolbarRect.bottom = m_ToolbarRect.top+m_ButtonSize.cy;

		m_TreeRect.top = m_ToolbarRect.bottom;
	}

	void UpdateLayout()
	{
		COptionSelectionLayoutTreeBanner::UpdateLayout();

		m_ToolbarCaption.SetWindowPos(NULL, m_ToolbarRect.left,m_ToolbarRect.top, m_ToolbarRect.Width(),m_ToolbarRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

};

//========================================================================================

}; // namespace Mortimer

#endif // __MORTIMER_OPTIONSHEETLAYOUTSIMPLE_H__
