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

#ifndef __MORTIMER_OPTIONSHEETLAYOUT_H__
#define __MORTIMER_OPTIONSHEETLAYOUT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <MOptionSheet.h>

// Needed by COptionSheetLayoutImpl::CreateDialogTemplate()
#include <atlcom.h>
#include <atlhost.h>

//========================================================================================
// encapsulate these classes in a namespace

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

//========================================================================================

class COptionSelectionLayout
{
	// Client operations
public:
	COptionSelectionLayout() : m_pSheet(NULL), m_PagesMinSize(0,0), m_PagesMaxSize(0,0)
	{
	}

	void SetPagesMinSizes(SIZE *pMin)
	{
		if (pMin)
			m_PagesMinSize = *pMin;
	}

	void SetPagesMaxSizes(SIZE *pMax)
	{
		if (pMax)
			m_PagesMaxSize = *pMax;
	}

	CSize GetPagesMinSize()
	{
		if ((m_PagesMinSize.cx + m_PagesMaxSize.cy) == 0)
		{
			if (m_pSheet)
			{
				CRect Rect(0,0,195,127);
				if (m_pSheet->MapDialogRect(&Rect))
					return CSize(Rect.right, Rect.bottom);
			}

			return CSize(293, 206);
		}

		return m_PagesMinSize;
	}

	CSize GetPagesMaxSize()
	{
		if ((m_PagesMinSize.cx + m_PagesMaxSize.cy) == 0)
		{
			CRect DesktopRect;
			::GetClientRect(GetDesktopWindow(), &DesktopRect);

			return CSize(int(double(DesktopRect.right)*0.9), int(double(DesktopRect.bottom)*0.9));
		}

		return m_PagesMaxSize;
	}

	// Sheet callback operations
public:
	virtual bool Create(COptionSheet *pSheet) { return true; }
	virtual void Destroy() {}

	virtual COptionSelectionCtrl *GetSelectionControl() = 0;

	virtual void SetPagesSize(CSize PagesSize) {}
	virtual CRect GetPagesRect() = 0;
	virtual CSize GetSheetClientSize() = 0;

	virtual void UpdateLayout() {}
	virtual void OnItemChange(COptionItem *pNewItem, COptionItem *pOldItem) {}

	// Some usefull helpers
protected:

	UINT GetCtrlSpacing()
	{
		UINT Spacing = 8;	// Defaults to 8 pixels (should be overridden by the code below)
		CRect RectOK, RectCancel;

		ATLASSERT(m_pSheet != NULL);
		if (m_pSheet)
			return Spacing;

		GetWindowRect(m_pSheet->GetDlgItem(IDOK), &RectOK);
		GetWindowRect(m_pSheet->GetDlgItem(IDCANCEL), &RectCancel);

		// Horizontal ?
		if (RectOK.top == RectCancel.top)
		{
			Spacing = __max(RectCancel.left-RectOK.right, RectOK.left-RectCancel.right);
		}
		// Vertical ?
		else if (RectOK.left == RectCancel.left)
		{
			Spacing = __max(RectCancel.top-RectOK.bottom, RectOK.top-RectCancel.bottom);
		}

		return Spacing;
	}

	CSize GetButtonSize()
	{
		CRect Rect(0,0,0,0);

		ATLASSERT(m_pSheet != NULL);
		if (m_pSheet)
			GetClientRect(m_pSheet->GetDlgItem(IDOK), &Rect);
		return Rect.Size();
	}

	// Ensure the given pages size is in the range MinSize-MaxSize
	void EnforcePagesSize(CSize &PagesSize)
	{
		CSize MinSize = GetPagesMinSize();
		CSize MaxSize = GetPagesMaxSize();

		if (PagesSize.cx < MinSize.cx)
			PagesSize.cx = MinSize.cx;
		if (PagesSize.cx > MaxSize.cx)
			PagesSize.cx = MaxSize.cx;

		if (PagesSize.cy < MinSize.cy)
			PagesSize.cy = MinSize.cy;
		if (PagesSize.cy > MaxSize.cy)
			PagesSize.cy = MaxSize.cy;
	}

	// Uses GetSheetClientSize()...
	void UpdateButtonsPos()
	{
		CSize SheetSize = GetSheetClientSize();
		CSize ButtonSize = GetButtonSize();
		UINT Spacing = GetCtrlSpacing();

		DWORD WhichButton = m_pSheet->GetFlags() & COptionSheet::OSF_HASBUTTON_MASK;

		int NumButtons = 1;
		if (WhichButton & COptionSheet::OSF_HASBUTTON_RELOAD)
			SetWindowPos(GetDlgItem(m_pSheet->m_hWnd, IDC_OS_RELOAD), NULL, SheetSize.cx-(ButtonSize.cx+Spacing)*NumButtons++, SheetSize.cy-ButtonSize.cy-Spacing, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		if (WhichButton & COptionSheet::OSF_HASBUTTON_APPLY)
			SetWindowPos(GetDlgItem(m_pSheet->m_hWnd, ID_APPLY_NOW), NULL, SheetSize.cx-(ButtonSize.cx+Spacing)*NumButtons++, SheetSize.cy-ButtonSize.cy-Spacing, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		if (WhichButton & COptionSheet::OSF_HASBUTTON_CANCEL)
			SetWindowPos(GetDlgItem(m_pSheet->m_hWnd, IDCANCEL), NULL, SheetSize.cx-(ButtonSize.cx+Spacing)*NumButtons++, SheetSize.cy-ButtonSize.cy-Spacing, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		if (WhichButton & COptionSheet::OSF_HASBUTTON_USE)
			SetWindowPos(GetDlgItem(m_pSheet->m_hWnd, IDC_OS_USE), NULL, SheetSize.cx-(ButtonSize.cx+Spacing)*NumButtons++, SheetSize.cy-ButtonSize.cy-Spacing, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		if (WhichButton & COptionSheet::OSF_HASBUTTON_OK)
			SetWindowPos(GetDlgItem(m_pSheet->m_hWnd, IDOK), NULL, SheetSize.cx-(ButtonSize.cx+Spacing)*NumButtons++, SheetSize.cy-ButtonSize.cy-Spacing, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}


protected:
	COptionSheet *m_pSheet;

	CSize m_PagesMinSize;
	CSize m_PagesMaxSize;

};

//========================================================================================

template<class TLayout, class TBase = COptionSheet>
class COptionSheetLayoutImpl : public TBase
{
public:
	COptionSheetLayoutImpl(LPCTSTR Title)
	{
		m_Caption = Title;
	}

	/// Create sheet and display it.
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_pSelection = m_Layout.GetSelectionControl();

		DLGTEMPLATE *pTemplate = CreateDialogTemplate();
		if (!pTemplate)
			return -1;

		GetWinModule().AddCreateWndData(&m_thunk.cd, (TBase*)this);

		m_Modal = true;
		INT_PTR Result = ::DialogBoxIndirectParam(GetBaseModule().GetResourceInstance(), 
			pTemplate,
		   hWndParent, 
			StartDialogProc, 
			dwInitParam);

		DestroyDialogTemplate(pTemplate);

		return Result;
	}

	/// Create sheet as modeless.
	HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_pSelection = m_Layout.GetSelectionControl();

		DLGTEMPLATE *pTemplate = CreateDialogTemplate();
		if (!pTemplate)
			return NULL;

		GetWinModule().AddCreateWndData(&m_thunk.cd, (TBase*)this);

		m_Modal = false;
		HWND hWnd = ::CreateDialogParam(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCE(0),
					hWndParent, (DLGPROC)StartDialogProc, dwInitParam);
		ATLASSERT(m_hWnd == hWnd);

		DestroyDialogTemplate(pTemplate);

		return hWnd;
	}

	TLayout *GetLayout()
	{
		return &m_Layout;
	}

protected:
	/// Called by any Handler in the sheet to close it (Modal or Modeless).
	///
	/// If you change the behaviour of closing the dialog, override this method.
	bool DoEndDialog(int nRetCode)
	{
		if (TBase::DoEndDialog(nRetCode))
			return true;

		ATLASSERT(::IsWindow(m_hWnd));

		if (m_Modal)
			::EndDialog(m_hWnd, nRetCode);
		else
			DestroyWindow();

		return true;
	}

protected:
	bool m_Modal;
	TLayout m_Layout;
	DLGTEMPLATE *m_pDlgTemplate;

	BEGIN_MSG_MAP((COptionSheetLayoutImpl<TBase>))
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP_MEMBER(m_Layout)
		CHAIN_MSG_MAP(TBase)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		CString Caption;
		if (GetFlags() & OSF_HASBUTTON_USE)
		{
			Caption.LoadString(IDC_OS_USE);
			if (Caption.IsEmpty())
				Caption = "Use";
			SetDlgItemText(IDC_OS_USE, Caption);

			Caption.LoadString(ID_FILE_SAVE);
			if (Caption.IsEmpty())
				Caption = "Save";
			else
			{
				int Pos = Caption.Find('\n');
				if (Pos != -1)	Caption.Delete(0, Pos+1);
			}
			SetDlgItemText(IDOK, Caption);
		}
		if (GetFlags() & OSF_HASBUTTON_RELOAD)
		{
			Caption.LoadString(IDC_OS_RELOAD);
			if (Caption.IsEmpty())
				Caption = "Reload";
			SetDlgItemText(IDC_OS_RELOAD, Caption);
		}


		if (!m_Layout.Create(this))
		{
			DoEndDialog(IDABORT);
			return 0;
		}

		m_pSelection = m_Layout.GetSelectionControl();

		if (!TBase::DoInit(true))
		{
			DoEndDialog(IDABORT);
			return 0;
		}

		// Browse every pages, to get the max size
		CSize PageSize(0,0);
		RetrievePageSize(m_pSelection->GetItem(NULL, 0), PageSize);
		if ((PageSize.cx == 0) && (PageSize.cy == 0))
		{
			// Bad sizes, fail
			DoEndDialog(IDABORT);
			return 0;
		}

		m_Layout.SetPagesSize(PageSize);
		CSize SheetSize = m_Layout.GetSheetClientSize();

		CRect SheetRect, ClientRect;
		GetWindowRect(&SheetRect);
		ScreenToClient(&SheetRect);
		GetClientRect(&ClientRect);
		SheetRect -= ClientRect;

		SetWindowPos(NULL, 0, 0, SheetSize.cx+SheetRect.Width(), SheetSize.cy+SheetRect.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		m_Layout.UpdateLayout();

		// Place every pages according to the layout
		UpdatePageRect(m_pSelection->GetItem(NULL, 0), m_Layout.GetPagesRect());

		SetWindowText(m_Caption);
		CenterWindow();

		return 0;
	}

	void OnDestroy()
	{
		m_Layout.Destroy();
	}

	void OnItemChange(COptionItem *pNewItem, COptionItem *pOldItem)
	{
		m_Layout.OnItemChange(pNewItem, pOldItem);
	}

	void RetrievePageSize(COptionItem *pItem, CSize &Size)
	{
		if (!pItem)
			return;

		do
		{
			COptionPage *pPage = pItem->GetPage();
			if (pPage)
			{
				CRect Rect;
				if (pPage->GetClientRect(&Rect))
				{
					if (Rect.right > Size.cx)
						Size.cx = Rect.right;
					if (Rect.bottom > Size.cy)
						Size.cy = Rect.bottom;
				}
			}

			// Any child ?
			COptionItem *pChild = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irChild);
			if (pChild)
				RetrievePageSize(pChild, Size);

			// Get next item
		} while (pItem = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irNext));
	}

	void UpdatePageRect(COptionItem *pItem, CRect &Rect)
	{
		if (!pItem)
			return;

		do
		{
			COptionPage *pPage = pItem->GetPage();
			if (pPage)
				pPage->SetWindowPos(NULL, Rect.left, Rect.top, Rect.Width(), Rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

			// Any child ?
			COptionItem *pChild = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irChild);
			if (pChild)
				UpdatePageRect(pChild, Rect);

			// Get next item
		} while (pItem = m_pSelection->GetItem(pItem, Mortimer::COptionSelectionCtrl::irNext));
	}

	DLGTEMPLATE *CreateDialogTemplate()
	{
		// Mimic the behaviour of MFC
		HINSTANCE hInst = GetModuleHandle(_T("COMCTL32.DLL"));
		if (!hInst)
			return NULL;

		HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(1006), RT_DIALOG);
		HGLOBAL hTemplate = LoadResource(hInst, hResource);
		if (!hTemplate)
			return NULL;

		DLGTEMPLATE *pDlg = (DLGTEMPLATE*)LockResource(hTemplate);

		// Let's copy this template but only with the desired buttons
		DLGITEMTEMPLATE *pFirstItem;
		pFirstItem = _DialogSplitHelper::FindFirstDlgItem(pDlg);

		ULONG HeaderSize = (ULONG)((BYTE*)pFirstItem - (BYTE*)pDlg);
		ULONG TemplateSize = HeaderSize;

		BOOL bDialogEx = _DialogSplitHelper::IsDialogEx(pDlg);
		UINT DlgId;
		UINT iItem;
		UINT nItems = _DialogSplitHelper::DlgTemplateItemCount(pDlg);
		DLGITEMTEMPLATE* pItem = pFirstItem;
		DLGITEMTEMPLATE* pNextItem = pItem;

		DWORD WhichButton = GetFlags() & OSF_HASBUTTON_MASK;
		UINT ItemSize;

		for (iItem = 0; iItem < nItems; iItem++)
		{
			pNextItem = _DialogSplitHelper::FindNextDlgItem(pItem, bDialogEx);

			DlgId = bDialogEx ? ((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pItem)->id: pItem->id;
			switch (DlgId)
			{
				case IDOK:			ItemSize = WhichButton & OSF_HASBUTTON_OK ? (UINT)((BYTE*)pNextItem - (BYTE*)pItem) : 0;		break;
				case IDCANCEL:		ItemSize = WhichButton & OSF_HASBUTTON_CANCEL ? (UINT)((BYTE*)pNextItem - (BYTE*)pItem) : 0;
									if (WhichButton & OSF_HASBUTTON_RELOAD) ItemSize += (UINT)((BYTE*)pNextItem - (BYTE*)pItem);
									if (WhichButton & OSF_HASBUTTON_USE) ItemSize += (UINT)((BYTE*)pNextItem - (BYTE*)pItem);
									break;
				case ID_APPLY_NOW:	ItemSize = WhichButton & OSF_HASBUTTON_APPLY ? (UINT)((BYTE*)pNextItem - (BYTE*)pItem) : 0;		break;
				default:			ItemSize = 0;	break;
			}
			TemplateSize += ItemSize;

			pItem = pNextItem;
		}

		// Alloc 
		BYTE *pNewDlg;
		pNewDlg = (BYTE*)GlobalAlloc(GMEM_FIXED, TemplateSize);
		if (!pNewDlg)
			return NULL;

		memcpy(pNewDlg, pDlg, HeaderSize);
		BYTE *pNewDlgItem = pNewDlg+HeaderSize;

		pItem = pFirstItem;
		pNextItem = pItem;

		UINT CopiedItemCount = 0;

		DLGITEMTEMPLATE* pItemDump = NULL;
		UINT pItemDumpSize;

		for (iItem = 0; iItem < nItems; iItem++)
		{
			pNextItem = _DialogSplitHelper::FindNextDlgItem(pItem, bDialogEx);

			DlgId = bDialogEx ? ((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pItem)->id: pItem->id;

			bool AddIt = false;
			switch (DlgId)
			{
				case IDOK:			AddIt = (WhichButton & OSF_HASBUTTON_OK)?true:false;		break;
				case IDCANCEL:		AddIt = (WhichButton & OSF_HASBUTTON_CANCEL)?true:false;
									pItemDump = pItem;
									pItemDumpSize = (UINT)((BYTE*)pNextItem - (BYTE*)pItem);
									if (WhichButton & OSF_HASBUTTON_USE)
									{
										memcpy(pNewDlgItem, pItemDump, pItemDumpSize);
										if (bDialogEx)
										{
											((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pNewDlgItem)->id = IDC_OS_USE;
											((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pNewDlgItem)->style &= ~WS_DISABLED;
										}
										else
										{
											((DLGITEMTEMPLATE*)pNewDlgItem)->id = IDC_OS_USE;
											((DLGITEMTEMPLATE*)pNewDlgItem)->style &= ~WS_DISABLED;
										}
										pNewDlgItem += pItemDumpSize;
										CopiedItemCount++;
									}
									break;
				case ID_APPLY_NOW:	AddIt = (WhichButton & OSF_HASBUTTON_APPLY)?true:false;	break;
			}
			if (AddIt)
			{
				memcpy(pNewDlgItem, pItem, (BYTE*)pNextItem - (BYTE*)pItem);
				if (bDialogEx)
					((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pNewDlgItem)->style &= ~WS_DISABLED;
				else
					((DLGITEMTEMPLATE*)pNewDlgItem)->style &= ~WS_DISABLED;
				pNewDlgItem += (BYTE*)pNextItem - (BYTE*)pItem;
				CopiedItemCount++;
			}

			pItem = pNextItem;
		}
		if (pItemDump && (WhichButton & OSF_HASBUTTON_RELOAD))
		{
			memcpy(pNewDlgItem, pItemDump, pItemDumpSize);
			if (bDialogEx)
			{
				((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pNewDlgItem)->id = IDC_OS_RELOAD;
				((_DialogSplitHelper::DLGITEMTEMPLATEEX*)pNewDlgItem)->style &= ~WS_DISABLED;
			}
			else
			{
				((DLGITEMTEMPLATE*)pNewDlgItem)->id = IDC_OS_RELOAD;
				((DLGITEMTEMPLATE*)pNewDlgItem)->style &= ~WS_DISABLED;
			}
			pNewDlgItem += pItemDumpSize;
			CopiedItemCount++;
		}

		bDialogEx ? ((_DialogSplitHelper::DLGTEMPLATEEX*)pNewDlg)->cDlgItems = CopiedItemCount : ((DLGTEMPLATE*)pNewDlg)->cdit = CopiedItemCount;

		return (DLGTEMPLATE*)pNewDlg;
	}

	void DestroyDialogTemplate(DLGTEMPLATE *pTemplate)
	{
		GlobalFree(pTemplate);
	}

};


//========================================================================================

}; // namespace Mortimer

#endif // __MORTIMER_OPTIONSHEETLAYOUT_H__
