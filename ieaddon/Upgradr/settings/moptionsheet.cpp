//========================================================================================
//
// Module:			OptionSheet
// Author:          Pascal Hurni
// Creation Date:	20.11.2003
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
//========================================================================================
//
// Modifications:
//
//	2004-06-03	By Pascal Hurni
//
//				Implemented correct behaviour of COptionPage activation/deactivation.
//				That means returning false in COptionPage::OnXXXActive() will prevent
//				the page to appear/disappear.
//
//========================================================================================
//
// Because the COptionSheet class is provided as source code, it has to be
// compiled in your project.
// To achieve this you can simply include the source file in your derived
// Sheet class. Put this line at the top of your file:
//		#include <OptionSheet.cpp>
// This has the advantage of not having copies of the file AND makes your
// project independant of your SDK include dirs.

// re-enable this (or your own global header file) if you compile this file separately
//#include "stdafx.h"

#include "MOptionSheet.h"
#include "MOptionPage.h"


using Mortimer::COptionItem;
using Mortimer::COptionSelectionCtrl;
using Mortimer::COptionPage;
using Mortimer::COptionSheet;
using Mortimer::CPropSheet;

//========================================================================================

COptionItem::COptionItem() : m_pPage(NULL) {}

COptionItem::COptionItem(LPCTSTR Caption, COptionPage *pPage)
{
	m_Caption = Caption;
	m_pPage = pPage;
}

COptionItem::~COptionItem()
{
}

//---------------------------------------------------------------------------------------

void COptionItem::SetCaption(LPCTSTR NewCaption)
{
	m_Caption = NewCaption;
}

void COptionItem::SetPage(COptionPage *pPage)
{
	m_pPage = pPage;
}

//---------------------------------------------------------------------------------------

CString COptionItem::GetCaption()
{
	return m_Caption;
}

COptionPage *COptionItem::GetPage()
{
	return m_pPage;
}

//========================================================================================

COptionSheet::COptionSheet() : m_pActiveItem(NULL), m_pSelection(NULL), m_SelectionChanging(false),
                               m_Flags(OSF_HASBUTTON_OK|OSF_HASBUTTON_CANCEL|OSF_HASBUTTON_APPLY)
{
}

COptionSelectionCtrl *COptionSheet::GetSelectionControl()
{
	return m_pSelection;
}

void COptionSheet::SetTitle(LPCTSTR Caption)
{
	if (m_hWnd)
		SetWindowText(Caption);
	else
		m_Caption = Caption;
}

void COptionSheet::SetFlags(DWORD Flags)
{
	m_Flags = Flags;
}

DWORD COptionSheet::GetFlags()
{
	return m_Flags;
}

//---------------------------------------------------------------------------------------

bool COptionSheet::DoInit(bool FirstTime)
{
	// Any page made active ? If not let the root one be active
	if (!m_pActiveItem)
	{
		SetActiveItem(m_pSelection->GetItem(NULL, 0));
	}

	return true;
}

void COptionSheet::OnItemChange(COptionItem *pNewItem, COptionItem *pOldItem)
{
}

bool COptionSheet::DoEndDialog(int nRetCode)
{
	return false;
}

//---------------------------------------------------------------------------------------

void COptionSheet::OnOK(UINT uCode, int nID, HWND hWndCtl)
{
	// Tell current page to update the settings from the control contents
	if (m_pActiveItem && m_pActiveItem->GetPage())
		if (!m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem))
			return;

	DoEndDialog(IDOK);
}

void COptionSheet::OnCancel(UINT uCode, int nID, HWND hWndCtl)
{
	// Tell current page to update the settings from the control contents
	if (m_pActiveItem && m_pActiveItem->GetPage()) m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem);

	DoEndDialog(IDCANCEL);
}

void COptionSheet::OnApply(UINT uCode, int nID, HWND hWndCtl)
{
	// Tell current page to update the settings from the control contents
	if (m_pActiveItem && m_pActiveItem->GetPage()) m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem);
}

void COptionSheet::OnUse(UINT uCode, int nID, HWND hWndCtl)
{
	// Tell current page to update the settings from the control contents
	if (m_pActiveItem && m_pActiveItem->GetPage())
		if (!m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem))
			return;

	DoEndDialog(IDOK);
}

void COptionSheet::OnReload(UINT uCode, int nID, HWND hWndCtl)
{
	// Tell current page to update the settings from the control contents
	if (m_pActiveItem && m_pActiveItem->GetPage()) m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem);
}

//---------------------------------------------------------------------------------------

LRESULT COptionSheet::OnSelectionChanging(NMHDR* phdr)
{
	NMOSS* pnmoss = (NMOSS*)phdr;

	// Retrieve new item
	COptionItem *pItem = pnmoss->pItem;

	// Unknown new item is okay unless OnSelectionChanged() is to be called to terminate the change.
	if (!pItem)
	{
		// Tell current page it gonna die, if it doesn't want to, abort.
		if (m_pActiveItem && m_pActiveItem->GetPage())
			if (!m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem))
				return TRUE;

		m_SelectionChanging = true;
		return FALSE;
	}

	// When the user selects an item which contains no page, look for children with pages
	if (!pItem->GetPage())
	{
		NMOSS NMoss;		// Recursive parameter

		// Get the child item
		NMoss.pItem = m_pSelection->GetItem(pItem, COptionSelectionCtrl::irChild);

		// If there's a child, recurse in here
		if (NMoss.pItem)
			return OnSelectionChanging((NMHDR*)&NMoss);

		// No child, prevent item changing
		return TRUE;
	}

	// Do nothing if pItem is already the active item (mandatory because of recursion of notify messages)
	if (pItem == m_pActiveItem)
		return TRUE;

	// Tell current page it gonna die, if it doesn't want to, abort.
	if (m_pActiveItem && m_pActiveItem->GetPage())
	{
		if (!m_pActiveItem->GetPage()->OnKillActive(m_pActiveItem))
		{
			// Prevent item changing
			return TRUE;
		}
	}

	// Try to set the active item
	if (!InternalSetActiveItem(pItem))
	{
		// Doesn't want to be shown, revert to the previous page.
		// Say hi again to the active page, it SHOULD not fail (coz, OnKillActive() succeeded)
		if (m_pActiveItem && m_pActiveItem->GetPage())
			m_pActiveItem->GetPage()->OnSetActive(m_pActiveItem);

		// Prevent item changing
		return TRUE;
	}

	return FALSE;
}

LRESULT COptionSheet::OnSelectionChanged(NMHDR* phdr)
{
	// Do nothing when all was handled in OnSelectionChanging()
	if (!m_SelectionChanging)
		return 0;

	// Retrieve new item
	NMOSS* pnmoss = (NMOSS*)phdr;
	COptionItem *pItem = pnmoss->pItem;

	// When the user selects an item which contains no page, look for children with pages
	if (!pItem->GetPage())
	{
		NMOSS NMoss;		// Recursive parameter

		// Get the child item
		NMoss.pItem = m_pSelection->GetItem(pItem, COptionSelectionCtrl::irChild);

		// If there's a child, recurse in here
		if (NMoss.pItem)
			return OnSelectionChanged((NMHDR*)&NMoss);

		// No child! Re-select the previous item
		m_SelectionChanging = false;
		m_pSelection->SelectItem(m_pActiveItem);
		return 0;
	}

	// Now that we could have recursed, kill the flag
	m_SelectionChanging = false;

	// Try to set the active item
	if (!InternalSetActiveItem(pItem))
	{
		// Doesn't want to set the item

		// Clear m_pActiveItem so that the OnSelChanging() will not call OnKillActive() (already done)
		pItem = m_pActiveItem;
		m_pActiveItem = NULL;

		// Re-select the previous item (will generate OnSelChang*() so a new call to OnSetActive() will be done there)
		m_pSelection->SelectItem(pItem);
	}

	return 0;
}

LRESULT COptionSheet::OnDeleteItem(NMHDR* phdr)
{
	NMOSS* pnmoss = (NMOSS*)phdr;

	// Get our item, and go on only if we got it
	COptionItem *pItem = pnmoss->pItem;
	if (pItem)
	{
		// Is this one the displayed item ?
		if (pItem == m_pActiveItem)
		{
			// Yes, hide its page
			if (pItem->GetPage()) pItem->GetPage()->ShowWindow(SW_HIDE);

			// And clear the reference
			m_pActiveItem = NULL;
		}
	}

	return 0;
}

bool COptionSheet::InternalSetActiveItem(COptionItem *pItem)
{
	// Tell new page it gets active
	if (pItem && pItem->GetPage())
	{
		if (!pItem->GetPage()->OnSetActive(pItem))
		{
			// Doesn't want to be shown, tell it to the caller
			return false;
		}
	}

	// Show/Hide only if the new page is not the same as the current one
	if (!m_pActiveItem || !pItem || (m_pActiveItem->GetPage() != pItem->GetPage()))
	{
		if (m_pActiveItem && m_pActiveItem->GetPage()) m_pActiveItem->GetPage()->ShowWindow(SW_HIDE);
		if (pItem && pItem->GetPage()) pItem->GetPage()->ShowWindow(SW_SHOW);
	}

	// Notify subclasses
	OnItemChange(pItem, m_pActiveItem);

	// Commit change
	m_pActiveItem = pItem;

	return true;
}

//---------------------------------------------------------------------------------------
bool COptionSheet::AddItem(COptionItem *pItem, COptionItem *pParent, COptionItem *pAfter)
{
	// Check
	if (!pItem)
		return false;

	// Retrieve caption from Page if missing
	if (pItem->GetPage() && pItem->GetCaption().IsEmpty())
	{
		TCHAR Caption[256];
		pItem->GetPage()->GetWindowText(Caption, 255);
		pItem->SetCaption(Caption);
	}

	return m_pSelection->AddItem(pItem, pParent, pAfter);
}

COptionItem *COptionSheet::GetActiveItem()
{
	return m_pActiveItem;
}

bool COptionSheet::SetActiveItem(COptionItem *pItem)
{
	// Do nothing if pItem is already the active item (mandatory because of recursion of notify messages)
	if (pItem == m_pActiveItem)
		return true;

	// If for any reason (like first page for TabCtrl) the currently showed item was not tracked by us,
	// Select it without the notification messages
	if ((m_pSelection->GetSelectedItem() == pItem) && (m_pActiveItem != pItem))
		return InternalSetActiveItem(pItem);

	return m_pSelection->SelectItem(pItem);
}

//========================================================================================

void CPropSheet::OnOK(UINT uCode, int nID, HWND hWndCtl)
{
	COptionSheet::OnOK(uCode, nID, hWndCtl);

	COptionItem *pRoot = m_pSelection->GetItem(NULL, 0);
	if (pRoot)
	{
		DoPageOK(pRoot);
	}
}
void CPropSheet::OnApply(UINT uCode, int nID, HWND hWndCtl)
{
	COptionSheet::OnApply(uCode, nID, hWndCtl);

	COptionItem *pRoot = m_pSelection->GetItem(NULL, 0);
	if (pRoot)
	{
		DoPageOK(pRoot);
	}
}
void CPropSheet::OnCancel(UINT uCode, int nID, HWND hWndCtl)
{
	COptionSheet::OnCancel(uCode, nID, hWndCtl);

	COptionItem *pRoot = m_pSelection->GetItem(NULL, 0);
	if (pRoot)
	{
		DoPageCancel(pRoot);
	}
}

//---------------------------------------------------------------------------------------

void CPropSheet::DoPageOK(COptionItem *pItem)
{
	do
	{
		if (pItem->GetPage())
		{
			CPropPage *pPage = dynamic_cast<CPropPage*>(pItem->GetPage());
			if (pPage)
				pPage->OnOK();
		}

		// Any child ?
		COptionItem *pChild = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irChild);
		if (pChild)
			DoPageOK(pChild);

		// Get next item
	} while (pItem = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irNext));
}

void CPropSheet::DoPageCancel(COptionItem *pItem)
{
	do
	{
		if (pItem->GetPage())
		{
			CPropPage *pPage = dynamic_cast<CPropPage*>(pItem->GetPage());
			if (pPage)
				pPage->OnCancel();
		}

		// Any child ?
		COptionItem *pChild = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irChild);
		if (pChild)
			DoPageCancel(pChild);

		// Get next item
	} while (pItem = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irNext));
}

//========================================================================================

using Mortimer::COptionSelectionTreeCtrl;

COptionSelectionTreeCtrl::CItem::CItem(int ILIDunselected, int ILIDselected)
 : m_SelectedILID(ILIDselected), m_UnselectedILID(ILIDunselected) {}

void COptionSelectionTreeCtrl::CItem::SetImages(int Unselected, int Selected)
{
	m_UnselectedILID = Unselected;
	m_SelectedILID = Selected;
}

//---------------------------------------------------------------------------------------

bool COptionSelectionTreeCtrl::AddItem(COptionItem *pItem, COptionItem *pParent, COptionItem *pAfter)
{
	if (!pItem)
		return false;

	CData Data;
	Data.pCtrl = this;

	CData DataParent, DataAfter;
	DataParent = m_Datas.Lookup(pParent);
	DataAfter = m_Datas.Lookup(pAfter);

	HTREEITEM hParent = DataParent.hItem ? DataParent.hItem : TVI_ROOT;
	HTREEITEM hAfter = DataAfter.hItem ? DataAfter.hItem : TVI_LAST;
 
	COptionSelectionTreeCtrl::CItem* pTreeItem = dynamic_cast<COptionSelectionTreeCtrl::CItem*>(pItem);
	if (pTreeItem)
	{
		Data.hItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, pItem->GetCaption(),
										pTreeItem->m_UnselectedILID, pTreeItem->m_SelectedILID,
										0,0, (LPARAM)pItem, hParent, hAfter);
	}
	else
	{
		Data.hItem = InsertItem(TVIF_TEXT | TVIF_PARAM, pItem->GetCaption(), 0,0,
										0,0, (LPARAM)pItem, hParent, hAfter);
	}

	if (Data.hItem)
	{
		m_Datas.Add(pItem, Data);
	}

	return Data.hItem != NULL;
}

bool COptionSelectionTreeCtrl::DeleteItem(COptionItem *pItem)
{
	CData Data = m_Datas.Lookup(pItem);

	if (pItem)
	{
		m_Datas.Remove(pItem);
		if (Data.hItem)
			return CWindowImpl<COptionSelectionTreeCtrl, WTL::CTreeViewCtrl>::DeleteItem(Data.hItem)?true:false;
		else
			return true;
	}

	return false;
}

bool COptionSelectionTreeCtrl::DeleteAllItems()
{
	m_Datas.RemoveAll();
	return CWindowImpl<COptionSelectionTreeCtrl, WTL::CTreeViewCtrl>::DeleteAllItems()?true:false;
}

bool COptionSelectionTreeCtrl::UpdateItem(COptionItem *pItem)
{
	CData Data = m_Datas.Lookup(pItem);

	// For us, only a Caption change is considered.
	if (pItem && Data.hItem)
		return CWindowImpl<COptionSelectionTreeCtrl, WTL::CTreeViewCtrl>::SetItemText(Data.hItem, pItem->GetCaption())?true:false;

	return false;
}

bool COptionSelectionTreeCtrl::SelectItem(COptionItem *pItem)
{
	CData Data = m_Datas.Lookup(pItem);

	if (pItem && Data.hItem)
		return CWindowImpl<COptionSelectionTreeCtrl, WTL::CTreeViewCtrl>::SelectItem(Data.hItem)?true:false;

	return false;
}

COptionItem *COptionSelectionTreeCtrl::GetSelectedItem()
{
	HTREEITEM hItem = CWindowImpl<COptionSelectionTreeCtrl, WTL::CTreeViewCtrl>::GetSelectedItem();
	if (hItem == NULL)
		return NULL;

	return (COptionItem*)GetItemData(hItem);
}

COptionItem *COptionSelectionTreeCtrl::GetItem(COptionItem *pItem, int Relation)
{
	CData Data = m_Datas.Lookup(pItem);

	HTREEITEM hItem;

	if (!pItem)
		hItem = GetRootItem();
	else
	{
		if (!Data.hItem)
			return NULL;

		switch (Relation)
		{
			case irPrevious:	hItem = GetPrevSiblingItem(Data.hItem); break;
			case irNext:		hItem = GetNextSiblingItem(Data.hItem); break;
			case irParent:		hItem = GetParentItem(Data.hItem); break;
			case irChild:		hItem = GetChildItem(Data.hItem); break;
			default:			return NULL;
		}
	}

	if (!hItem)
		return NULL;

	return (COptionItem*)GetItemData(hItem);
}

LRESULT COptionSelectionTreeCtrl::OnSelChange(NMHDR *phdr)
{
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)phdr;
	NMOSS NMoss;
	NMoss.Hdr.hwndFrom = phdr->hwndFrom;
	NMoss.Hdr.idFrom = phdr->idFrom;
	switch (phdr->code)
	{
		case TVN_SELCHANGING:	NMoss.Hdr.code = OSSN_SELCHANGING;	break;
		case TVN_SELCHANGED:	NMoss.Hdr.code = OSSN_SELCHANGED;	break;
		default:	return 0;
	}

	// Retrieve item
	NMoss.pItem = (COptionItem*)GetItemData(pnmtv->itemNew.hItem);

	return ::SendMessage(GetParent(), WM_NOTIFY, NMoss.Hdr.idFrom, (LPARAM)&NMoss);
}

LRESULT COptionSelectionTreeCtrl::OnDeleteItem(NMHDR *phdr)
{
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)phdr;
	NMOSS NMoss;
	NMoss.Hdr.hwndFrom = phdr->hwndFrom;
	NMoss.Hdr.idFrom = phdr->idFrom;
	NMoss.Hdr.code = OSSN_DELETEITEM;

	// Retrieve item
	NMoss.pItem = (COptionItem*)GetItemData(pnmtv->itemOld.hItem);

	LRESULT lResult = ::SendMessage(GetParent(), WM_NOTIFY, NMoss.Hdr.idFrom, (LPARAM)&NMoss);

	// We can now delete it
	delete NMoss.pItem;

	return lResult;
}

//========================================================================================

using Mortimer::COptionSelectionTabCtrl;

COptionSelectionTabCtrl::CItem::CItem(int ImageID) : m_ImageID(ImageID) {}

void COptionSelectionTabCtrl::CItem::SetImage(int ImageID)
{
	m_ImageID = ImageID;
}

//---------------------------------------------------------------------------------------

bool COptionSelectionTabCtrl::AddItem(COptionItem *pItem, COptionItem *pParent, COptionItem *pAfter)
{
	if (!pItem || pParent)
		return false;

	CData Data;
	Data.pCtrl = this;

	CData DataAfter;
	DataAfter = m_Datas.Lookup(pAfter);
	int nAfter = DataAfter.nItem != -1 ? DataAfter.nItem : GetItemCount();

	TCITEM TCitem;
	TCitem.mask = TCIF_TEXT | TCIF_PARAM;
	TCitem.pszText = (LPTSTR)(LPCTSTR)pItem->GetCaption();
	TCitem.lParam = (LPARAM)pItem;

	COptionSelectionTabCtrl::CItem* pTabItem = dynamic_cast<COptionSelectionTabCtrl::CItem*>(pItem);
	if (pTabItem && (pTabItem->m_ImageID != -1))
	{
		TCitem.mask |= TCIF_IMAGE;
		TCitem.iImage = pTabItem->m_ImageID;
	}

	Data.nItem = InsertItem(nAfter, &TCitem);

	if (Data.nItem != -1)
	{
		m_Datas.Add(pItem, Data);
	}

	return Data.nItem != -1;
}

bool COptionSelectionTabCtrl::DeleteItem(COptionItem *pItem)
{
	CData Data = m_Datas.Lookup(pItem);

	if (pItem)
	{
		m_Datas.Remove(pItem);
		if (Data.nItem != -1)
			return CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>::DeleteItem(Data.nItem)?true:false;
		else
			return true;
	}

	return false;
}

bool COptionSelectionTabCtrl::DeleteAllItems()
{
	m_Datas.RemoveAll();
	return CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>::DeleteAllItems()?true:false;
}

bool COptionSelectionTabCtrl::UpdateItem(COptionItem *pItem)
{
	CData Data = m_Datas.Lookup(pItem);

	// For us, only a Caption change is considered.
	if (pItem && Data.nItem != -1)
	{
		TCITEM TCitem;
		TCitem.mask = TCIF_TEXT;
		TCitem.pszText = (LPTSTR)(LPCTSTR)pItem->GetCaption();
		return CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>::SetItem(Data.nItem, &TCitem)?true:false;
	}

	return false;
}

bool COptionSelectionTabCtrl::SelectItem(COptionItem *pItem)
{
	CData Data = m_Datas.Lookup(pItem);

	if (pItem && Data.nItem != -1)
		return CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>::SetCurSel(Data.nItem) != -1;

	return false;
}

COptionItem *COptionSelectionTabCtrl::GetSelectedItem()
{
	int nItem = CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>::GetCurSel();
	if (nItem == -1)
		return NULL;

	return (COptionItem*)GetItemData(nItem);
}

COptionItem *COptionSelectionTabCtrl::GetItem(COptionItem *pItem, int Relation)
{
	CData Data = m_Datas.Lookup(pItem);

	int nItem;

	if (!pItem)
		nItem = 0;
	else
	{
		if (Data.nItem == -1)
			return NULL;

		switch (Relation)
		{
			case irPrevious:	nItem = Data.nItem-1; break;
			case irNext:		nItem = Data.nItem+1; break;
			default:			return NULL;
		}
	}

	return (COptionItem*)GetItemData(nItem);
}

COptionItem *COptionSelectionTabCtrl::GetItemData(int nItem)
{
	if ((nItem < 0) || (nItem >= GetItemCount()))
		return NULL;

	TCITEM TCitem;
	TCitem.mask = TCIF_PARAM;
	if (!CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>::GetItem(nItem, &TCitem))
		return NULL;

	return (COptionItem*)TCitem.lParam;
}

LRESULT COptionSelectionTabCtrl::OnSelChange(NMHDR *phdr)
{
	NMOSS NMoss;
	NMoss.Hdr.hwndFrom = phdr->hwndFrom;
	NMoss.Hdr.idFrom = phdr->idFrom;
	NMoss.pItem = NULL;
	switch (phdr->code)
	{
		case TCN_SELCHANGING:	NMoss.Hdr.code = OSSN_SELCHANGING;	break;
		case TCN_SELCHANGE:		NMoss.Hdr.code = OSSN_SELCHANGED;	break;
		default:	return 0;
	}

	// Retrieve item
	if (phdr->code == TCN_SELCHANGE)
		NMoss.pItem = (COptionItem*)GetItemData(GetCurSel());

	return ::SendMessage(GetParent(), WM_NOTIFY, NMoss.Hdr.idFrom, (LPARAM)&NMoss);
}
