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

#ifndef __MORTIMER_OPTIONPAGE_H__
#define __MORTIMER_OPTIONPAGE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOptionSheet.h"

//========================================================================================
// encapsulate these classes in a namespace

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

//========================================================================================

/// All OptionPages must be subclasses of COptionPage which is a CWindow
class COptionPage : public CWindow
{
public:
	/// @name Overrideables
	//@{

	/// Called each time this page is displayed.
	/// @param pItem Item that holds this page, you can dynamic_cast<> this to your subclass object (see COptionSheet::AddItem())
	virtual bool OnSetActive(COptionItem *pItem) = 0;

	/// Called each time this page will go away.
	/// @param pItem Item that holds this page, you can dynamic_cast<> this to your subclass object (see COptionSheet::AddItem())
	virtual bool OnKillActive(COptionItem *pItem) = 0;
	//@}
};

/// PropPage are specialized OptionPages, see overrideables
class CPropPage : public COptionPage
{
public:
	/// @name Overrideables
	//@{
	virtual void OnOK() = 0;		///< Will be called when the Sheet OK or Apply button is pressed
	virtual void OnCancel() = 0;	///< Will be called when the Sheet Cancel button is pressed
	//@}
};

//========================================================================================

// Implementation for COptionPage.
// Derive your dialog class from this template.
template <class T, class TBase = COptionPage>
class ATL_NO_VTABLE COptionPageImpl : public CDialogImplBaseT< TBase >
{
public:
	COptionPageImpl() : m_pSheet(NULL) {}

	// modeless dialogs
	HWND Create(COptionSheet *pSheet)	//HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(pSheet != NULL);
		m_pSheet = pSheet;

		ATLASSERT(m_hWnd == NULL);
		GetWinModule().AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
		HWND hWnd = ::CreateDialogParam(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
					pSheet->m_hWnd, (DLGPROC)T::StartDialogProc, 0);
		ATLASSERT(m_hWnd == hWnd);
#ifdef __ATLTHEME_H__
		if (hWnd && IsAppReallyThemed())
			::EnableThemeDialogTexture(hWnd, ETDT_ENABLETAB);
#endif
		return hWnd;
	}

	bool OnSetActive(COptionItem *pItem) { return true; }
	bool OnKillActive(COptionItem *pItem) { return true; }

protected:
	COptionSheet *m_pSheet;
};


//========================================================================================

// Implementation for COptionPage with automatic CSettings handling.
// Derive your dialog class from this template, passing the specialized CSettings subclass.
template <class T, class TSettings, class TBase = COptionPage>
class ATL_NO_VTABLE COptionPageSettingsImpl : public CDialogImplBaseT< TBase >
{
public:
	COptionPageSettingsImpl() : m_pSheet(NULL), m_pSettings(NULL) {}

	// modeless dialogs
	HWND Create(COptionSheet *pSheet)	//HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(pSheet != NULL);
		m_pSheet = pSheet;

		ATLASSERT(m_hWnd == NULL);
		GetWinModule().AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
		HWND hWnd = ::CreateDialogParam(GetBaseModule().GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
					pSheet->m_hWnd, (DLGPROC)T::StartDialogProc, 0);
		ATLASSERT(m_hWnd == hWnd);
#ifdef __ATLTHEME_H__
		if (hWnd && IsAppReallyThemed())
			::EnableThemeDialogTexture(hWnd, ETDT_ENABLETAB);
#endif
		return hWnd;
	}

	bool OnSetActive(COptionItem *pItem)
	{
		// Don't trust our CItem pointer
		COptionItemSettings *pSettingsItem = dynamic_cast<COptionItemSettings*>(pItem);
		if (!pSettingsItem)
			return false;
		m_pSettings = dynamic_cast<TSettings*>( pSettingsItem->GetSettings() );
		if (!m_pSettings)
			return false;	// No data provided, don't show page (if you don't have any data, use COptionPageImpl)

		T* pT = static_cast<T*>(this);
		return pT->DoDataExchange(DDX_LOAD)?true:false;
	}

	bool OnKillActive(COptionItem *pItem)
	{
		if (!m_pSettings)
			return true;	// No data to save, allow to change page

		T* pT = static_cast<T*>(this);
		return pT->DoDataExchange(DDX_SAVE)?true:false;
	}

protected:
	COptionSheet *m_pSheet;
	TSettings *m_pSettings;
};

//========================================================================================

}; // namespace Mortimer

#endif // __MORTIMER_OPTIONPAGE_H__
