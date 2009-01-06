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
// This file uses CString, so #include <atlmisc.h> if you are ATL/WTL
// It also make use of message crackers, #include <atlcrack.h>
//
// Because the COptionSheet class is provided as source code, it has to be
// compiled in your project.
// To achieve this you can simply include the source file in your derived
// Sheet class. Put this line at the top of your file:
//		#include <OptionSheet.cpp>
// This has the advantage of not having copies of the file AND makes your
// project independant of your SDK include dirs.
//
//========================================================================================
//
// Modifications:
//
//	2004-06-11	By Pascal Hurni
//
//				Reworked the selection behaviour. It was lacking the functionality of aborting
//				a selection change. This was because only OnSelChanged() was implemented and not
//				of OnSelChanging(). This lead to the following modifications:
//
//					Changed COptionSheet::SetActiveItem() to return a bool that indicates if
//					the new item has been shown.
//
//					Implemented all the COptionSelectionCtrl::OnSelChanging().
//
//					Implemented COptionSheet::OnSelelectionChanging() whith code contained before
//					in OnSelectionChanged().
//
//					Modified OnOK(), OnUse() to not close the dialog when OnKillActive() fails.
//
//				Made COptionSheet::OnSelectionChanging(), COptionSheet::OnSelectionChanged(),
//				COptionSheet::OnDeleteItem() virtual, so that subclasses can reimplement them if needed.
//
//	2004-06-03	By Pascal Hurni
//
//				DoEndDialog() prototype changed.
//				Now returns a bool instead void.
//
//========================================================================================

#ifndef __MORTIMER_OPTIONSHEET_H__
#define __MORTIMER_OPTIONSHEET_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//========================================================================================
// Property Sheet control id's (determined with Spy++)

#ifndef ID_APPLY_NOW
	#define ID_APPLY_NOW   0x3021
#endif

// Reuse a defined ID for our own behaviour
#ifdef ID_WIZBACK
	#define IDC_OS_RELOAD ID_WIZBACK
#else
	#define IDC_OS_RELOAD 0x3023
#endif

// Reuse a defined ID for our own behaviour
#ifdef ID_WIZFINISH
	#define IDC_OS_USE ID_WIZFINISH
#else
	#define IDC_OS_USE 0x3025
#endif

// Name differs a little bit from ATL to MFC
#if defined(ATL_IDC_TAB_CONTROL)
	#define IDC_OS_PAGESELECTION ATL_IDC_TAB_CONTROL
#elif defined(AFX_IDC_TAB_CONTROL)
	#define IDC_OS_PAGESELECTION AFX_IDC_TAB_CONTROL
#else
	#define IDC_OS_PAGESELECTION 0x3020
#endif

// Option Sheet Selection ctrl notify codes
#define OSSN_SELCHANGING		2000
#define OSSN_SELCHANGED			2001
#define OSSN_DELETEITEM			2002

//========================================================================================
// encapsulate these classes in a namespace

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

//========================================================================================

// Forward declaration (to resolve recursive includes with #include "MOptionPage.h")
class COptionPage;

//========================================================================================
/// Container for the selection control items.
///
/// You can derive your own class from this one, if you wish to add private datas.
///
/// This base class encapsulate a Caption for the item and the related OptionPage.

class COptionItem
{
public:
	COptionItem();
	COptionItem(LPCTSTR Caption, COptionPage *pPage);
	virtual ~COptionItem();

	//---------------------------------------------------------------------------------------
	// Operations

	virtual void SetCaption(LPCTSTR NewCaption);	// when to call ? Anytime
	virtual void SetPage(COptionPage *pPage);		// when to call ? Before Sheet::AddItem()

	//---------------------------------------------------------------------------------------
	// Accessors

	virtual CString GetCaption();
	virtual COptionPage *GetPage();

protected:
	CString m_Caption;
	COptionPage *m_pPage;

};

//========================================================================================

/// structure of Notify messages for COptionSelectionCtrl
typedef struct tagNMOSS
{
	NMHDR Hdr;
	COptionItem *pItem;		///< Pointer to the related item. Note that you can dynamic_cast<> this to your subclass object (see COptionSheet::AddItem())
} NMOSS;

//---------------------------------------------------------------------------------------

class COptionSheet;

/// Class that represent the selection control used in the Option Sheet
class COptionSelectionCtrl
{
public:
	//---------------------------------------------------------------
	// Item related operations

	/// Item relation, see GetItem()
	enum
	{
		irPrevious,
		irNext,
		irParent,
		irChild,
	};
	/// Add pItem to the list. Note that pItem has to be allocated with the 'new' operator.
	/// You can call this member directly to add an item but you probably better use
	/// COptionSheet::AddItem() which will call this member.
	/// @param pItem will be deleted automatically by this class. It can also be done by calling DeleteItem().
	/// @param pParent can point to the desired parent for this new item. NULL indicates root.
	/// @param pAfter can point to an item which will be just before the new item. NULL indicates the last brother.
	virtual bool AddItem(COptionItem *pItem, COptionItem *pParent = NULL, COptionItem *pAfter = NULL) = 0;

	/// Removes and delete pItem from the list. Note that it will also remove all its childrens.
	virtual bool DeleteItem(COptionItem *pItem) = 0;

	/// Removes all items. The selection control is emptied.
	virtual bool DeleteAllItems() = 0;

	/// Update the display of the item passed. (example: Caption could have changed)
	virtual bool UpdateItem(COptionItem *pItem) = 0;

	/// Programmatically shows an item.
	/// Implementation must generate the OSSN_SELCHANGING notification message and return
	/// the status of the result. (WARNING: notification message return the inverted value).
	/// If the status is change made, it must also generate the and OSSN_SELCHANGED message.
	virtual bool SelectItem(COptionItem *pItem) = 0;

	/// Retrieve the currently shown item.
	virtual COptionItem *GetSelectedItem() = 0;

	/// Get an item related to pItem. Relation can be one of these
	///		- irPrevious:		Get the previous brother
	///		- irNext:			Get the next brother
	///		- irParent:			Get the parent
	///		- irChild:			Get the first child
	///
	/// You'll get NULL if no item is found.
	/// Set pItem to NULL to indicate the root item (Relation will be ignored).
	virtual COptionItem *GetItem(COptionItem *pItem, int Relation) = 0;
};

//========================================================================================

/// The Option Sheet base class.
/// Note that this class implements the logical behaviour of the sheet. To actually get a sheet displayed
/// you have to use a window handling implementation template. Here is an example:
/// @code
///		COptionSheetDialogImpl<COptionSelectionTabCtrl, CMyOptionSheet> Sheet(IDD_MYOPTIONSHEET);
///		Sheet.DoModal();
/// @endcode
class COptionSheet : public CDialogImplBase
{
public:

	//---------------------------------------------------------------
	/// @name Operations
	//@{

	COptionSheet();

	/// Adds an item to the sheet. pItem has to be allocated with the 'new' operator.
	/// Generally, you'll pass a specialized COptionItem subclass that contains your private datas.
	/// This function will call m_pSelection->AddItem(). It is here for your conveniance but adds one more feature: \n
	/// When supplying an empty Caption for the item, it gets the Caption out of the pPage Caption.
	/// @param pItem will be deleted automatically.
	/// @param pParent can point to the desired parent for this new item. NULL indicates root.
	/// @param pAfter can point to an item which will be just before the new item. NULL indicates the last brother.
	bool AddItem(COptionItem *pItem, COptionItem *pParent = NULL, COptionItem *pAfter = NULL);

	/// Programmatically shows an item
	/// @return Indicates wether the given item has been showed.
	bool SetActiveItem(COptionItem *pItem);

	/// Retrieve the currently shown item
	COptionItem *GetActiveItem();

	/// Defines the sheet caption
	void SetTitle(LPCTSTR Caption);

	/// Get the selection control to perform item operations.
	COptionSelectionCtrl *GetSelectionControl();

	/// Defines some flags. See COptionSheet::OSF flags.
	void SetFlags(DWORD Flags);

	/// Get defined flags. See COptionSheet::OSF flags.
	DWORD GetFlags();


	//@}
	/// Sheet Flags.
	/// Defaults to : OSF_HASBUTTON_OK|OSF_HASBUTTON_CANCEL|OSF_HASBUTTON_APPLY
	enum OSF
	{
		OSF_DISABLE_PAGE_THEME = 0x0001,	///< By default, COptionPage will draw the themed background, set this to disable it.

		OSF_HASBUTTON_OK     = 0x0020,		///< Set it to add the \b OK button to the sheet.
		OSF_HASBUTTON_CANCEL = 0x0040,		///< Set it to add the \b Cancel button to the sheet.
		OSF_HASBUTTON_APPLY  = 0x0080,		///< Set it to add the \b Apply button to the sheet.
		OSF_HASBUTTON_USE    = 0x0100,		///< Set it to add the \b Use button to the sheet.
		OSF_HASBUTTON_RELOAD = 0x0200,		///< Set it to add the \b Reload button to the sheet.
		OSF_HASBUTTON_MASK   = 0x0FF0,		///< Mask for all the \a OSF_HASBUTTON_* flags.
	};

	//---------------------------------------------------------------
	/// @name Window Handling Implementation
	/// Override these functions to create and destroy the window sheet.
	/// Note that implementations are available form the COptionSheetXXXImpl templates.
	/// So you don't need to override these yourself in the COptionSheet subclass.
	//@{

	/// Create sheet and display it.
	virtual int DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL) = 0;

	/// Create sheet as modeless.
	virtual HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) = 0;

	/// Called by any Handler in the sheet to close it (Modal or Modeless).
	/// @return Handled status.
	/// Implementation will skip their behaviour when receiving true. This let's the
	/// subclass implement a different behaviour than that in the COptionSheetXXXImpl templates.
	/// This base class simply returns false to indicate: not handled.
	virtual bool DoEndDialog(int nRetCode);

	//@}
	//---------------------------------------------------------------
	/// @name Overrideables
	//@{
protected:

	/// Called at OnInitDialog(), do your own init here
	/// @param FirstTime true at OnInitDialog() time. false every other time the dialog needs re-init like in OnReload();
	virtual bool DoInit(bool FirstTime);

	/// Called each time a new item is choosed.
	/// @param pNewItem the newly selected item.
	/// @param pOldItem the previously selected item. Can be NULL if no item was selected.
	virtual void OnItemChange(COptionItem *pNewItem, COptionItem *pOldItem);

	/// Called when the \b OK button is pressed.
	/// Don't forget to call super class implementation when overriding this method.
	virtual void OnOK(UINT uCode, int nID, HWND hWndCtl);

	/// Called when the \b Apply button is pressed.
	/// Don't forget to call super class implementation when overriding this method.
	virtual void OnApply(UINT uCode, int nID, HWND hWndCtl);

	/// Called when the \b Cancel button is pressed.
	/// Don't forget to call super class implementation when overriding this method.
	virtual void OnCancel(UINT uCode, int nID, HWND hWndCtl);

	/// Called when the \b Use button is pressed.
	/// Don't forget to call super class implementation when overriding this method.
	virtual void OnUse(UINT uCode, int nID, HWND hWndCtl);

	/// Called when the \b Reload button is pressed.
	/// To keep consistency with the other button handlers, call it at the beginning
	/// of your overridden method. But this let's YOU the responsability to call
	/// DoInit(false) at the end of your method to let the sheet re-init itself.
	virtual void OnReload(UINT uCode, int nID, HWND hWndCtl);

	//@}
	//---------------------------------------------------------------
	// Implementation

protected:
	BEGIN_MSG_MAP(COptionSheet)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(ID_APPLY_NOW, OnApply)
		COMMAND_ID_HANDLER_EX(IDC_OS_USE, OnUse)
		COMMAND_ID_HANDLER_EX(IDC_OS_RELOAD, OnReload)
		NOTIFY_HANDLER_EX(IDC_OS_PAGESELECTION, OSSN_SELCHANGING, OnSelectionChanging)
		NOTIFY_HANDLER_EX(IDC_OS_PAGESELECTION, OSSN_SELCHANGED, OnSelectionChanged)
		NOTIFY_HANDLER_EX(IDC_OS_PAGESELECTION, OSSN_DELETEITEM, OnDeleteItem)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	virtual LRESULT OnSelectionChanging(NMHDR* phdr);
	virtual LRESULT OnSelectionChanged(NMHDR* phdr);
	virtual LRESULT OnDeleteItem(NMHDR* phdr);

	bool InternalSetActiveItem(COptionItem *pItem);

protected:
	COptionSelectionCtrl *m_pSelection;
	COptionItem *m_pActiveItem;
	CString m_Caption;
	DWORD m_Flags;
	bool m_SelectionChanging;
};

//========================================================================================

/// Specialized version of COptionSheet to partially mimic the behaviour of
/// standard Property Sheets.
/// When you have only one set of settings per page, it can be simplier to use this sheet. \n
/// Note that when adding items with AddItem() the page pointer must point to an instance of CPropPage.
/// You can still pass COptionPage for those that do not need OnOK() or OnCancel() (like About Page).
///
/// When the user clicks on the \b OK button, every page's OnOK() are called and the dialog is closed. \n
/// When the user clicks on the \b Apply button, every page's OnOK() are called. \n
/// When the user clicks on the \b Cancel button, every page's OnCancel() are called and the dialog is closed. \n
class CPropSheet : public COptionSheet
{
protected:
	virtual void OnOK(UINT uCode, int nID, HWND hWndCtl);
	virtual void OnApply(UINT uCode, int nID, HWND hWndCtl);
	virtual void OnCancel(UINT uCode, int nID, HWND hWndCtl);

protected:
	void DoPageOK(COptionItem *pItem);
	void DoPageCancel(COptionItem *pItem);
};

//========================================================================================
/// Provide an implementation for the Selection control. This one uses a TreeCtrl

class COptionSelectionTreeCtrl : public COptionSelectionCtrl, public CWindowImpl<COptionSelectionTreeCtrl, WTL::CTreeViewCtrl>
{
public:
	//---------------------------------------------------------------------------------------
	/// Add tree data to COptionItem
	class CItem : virtual public COptionItem
	{
		friend class COptionSelectionTreeCtrl;

	public:
		/// Constructor, you can pass ImageList IDs to use for the Unselected and Selected images.
		CItem(int ILIDunselected = 0, int ILIDselected = 1);

		/// Supply the ImageList IDs to use for the Unselected and Selected images.
		/// Call it before COptionSheet::AddItem().
		virtual void SetImages(int ILIDunselected, int ILIDselected);

		//---------------------------------------------------------------------------------------
	protected:
		// Imagelist ID for the user images
		int m_UnselectedILID;
		int m_SelectedILID;
	};

	//---------------------------------------------------------------------------------------

	bool AddItem(COptionItem *pItem, COptionItem *pParent = NULL, COptionItem *pAfter = NULL);
	bool DeleteItem(COptionItem *pItem);
	bool DeleteAllItems();
	bool UpdateItem(COptionItem *pItem);
	bool SelectItem(COptionItem *pItem);
	COptionItem *GetSelectedItem();
	COptionItem *GetItem(COptionItem *pItem, int Relation);

protected:
	BEGIN_MSG_MAP_EX(COptionSelectionTreeCtrl)
		MSG_WM_SETFOCUS(OnSetFocus)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGING, OnSelChange)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnSelChange)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDeleteItem)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnSetFocus(HWND hLoose)
	{
		SetMsgHandled(FALSE);
	}

	LRESULT OnSelChange(NMHDR* phdr);
	LRESULT OnDeleteItem(NMHDR* phdr);

protected:
	struct CData
	{
		// NULL param to let the Map template 'return NULL' on CData
		CData(const int Val = NULL) : pCtrl(NULL), hItem(NULL) {}

		COptionSelectionTreeCtrl *pCtrl;
		HTREEITEM hItem;
	};

	CSimpleMap<COptionItem*, CData> m_Datas;
};

//========================================================================================
/// Provide an implementation for the Selection control. This one uses a TabCtrl

class COptionSelectionTabCtrl : public COptionSelectionCtrl, public CWindowImpl<COptionSelectionTabCtrl, WTL::CTabCtrl>
{
public:
	//---------------------------------------------------------------------------------------
	/// Add tab data to COptionItem
	class CItem : virtual public COptionItem
	{
		friend class COptionSelectionTabCtrl;

	public:
		/// Constructor, you can pass the ImageList ID to use for optional image.
		CItem(int ImageID = -1);

		/// Supply the ImageList ID to use for optional image.
		/// Call it before COptionSheet::AddItem().
		virtual void SetImage(int ID);

		//---------------------------------------------------------------------------------------
	protected:
		int m_ImageID;
	};

	//---------------------------------------------------------------------------------------

	bool AddItem(COptionItem *pItem, COptionItem *pParent = NULL, COptionItem *pAfter = NULL);
	bool DeleteItem(COptionItem *pItem);
	bool DeleteAllItems();
	bool UpdateItem(COptionItem *pItem);
	bool SelectItem(COptionItem *pItem);
	COptionItem *GetSelectedItem();
	COptionItem *GetItem(COptionItem *pItem, int Relation);

protected:
	BEGIN_MSG_MAP_EX(COptionSelectionTabCtrl)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGING, OnSelChange)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelChange)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnSelChange(NMHDR* phdr);

protected:
	COptionItem *GetItemData(int nItem);

	struct CData
	{
		// NULL param to let the Map template 'return NULL' on CData
		CData(const int Val = NULL) : pCtrl(NULL), nItem(-1) {}

		COptionSelectionTabCtrl *pCtrl;
		int nItem;
	};

	CSimpleMap<COptionItem*, CData> m_Datas;
};

//========================================================================================
// A window handling Implementation that uses an existing dialog template.

template<class TSelectionCtrl, class TBase = COptionSheet>
class COptionSheetDialogImpl : public TBase
{
public:
	COptionSheetDialogImpl(int IDD) : m_IDD(IDD) {}

	// Create sheet and display it.
	int DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_pSelection = &m_SelectionCtrl;

		_Module.AddCreateWndData(&m_thunk.cd, (TBase*)this);

		m_Modal = true;
		return ::DialogBoxParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(m_IDD),
					hWndParent, (DLGPROC)StartDialogProc, dwInitParam);
	}

	// Create sheet as modeless.
	HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_pSelection = &m_SelectionCtrl;

		_Module.AddCreateWndData(&m_thunk.cd, (TBase*)this);

		m_Modal = false;
		HWND hWnd = ::CreateDialogParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(m_IDD),
					hWndParent, (DLGPROC)StartDialogProc, dwInitParam);
		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}

protected:
	// Called by any Handler in the sheet to close it (Modal or Modeless).
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
	BEGIN_MSG_MAP((COptionSheetDialogImpl<TSelectionCtrl, TBase>))
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(TBase)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		if (!m_SelectionCtrl.SubclassWindow(::GetDlgItem(m_hWnd, IDC_OS_PAGESELECTION)))
		{
			DoEndDialog(IDCANCEL);
			return 0;
		}

		if (!DoInit(true))
		{
			DoEndDialog(IDCANCEL);
			return 0;
		}

		if (!m_Caption.IsEmpty())
			SetWindowText(m_Caption);

		CenterWindow();

		return 0;
	}

protected:
	bool m_Modal;
	TSelectionCtrl m_SelectionCtrl;
	int m_IDD;
};

//========================================================================================

}; // namespace Mortimer

#endif // __MORTIMER_OPTIONSHEET_H__
