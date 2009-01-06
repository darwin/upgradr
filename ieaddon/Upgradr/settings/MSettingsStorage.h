//========================================================================================
//
// Module:			SettingsStorage
// Author:          Pascal Hurni
// Creation Date:	20.12.2003
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
// This work is based on CRegSettings, Copyright (C) 2002 Magomed G. Abdurakhmanov, maq@mail.ru
//
// Modifications:
//
//	2004-10-18	By Pascal Hurni
//
//				Added ICollection pseudo-interface to let any collection class to be used.
//				Two steps for this:
//				  1. create a class that subclass your collection class, add all the following members
//				     (this is the ICollection interface but you don't have to derive from it because of the template instanciation)
//
//				class CMyColSettings : public CMyCol
//				{
//				public:
//					// TCollectionItem must be the type for the collection items and thus must derive from CSettings
//					typedef CItemSettings TCollectionItem;
//					// Members called by the CSettings system to manage your collection
//					void CollectionRemoveAll()							{ call the CMyCol clear }
//					int CollectionGetSize()								{ call the CMyCol getsize }
//					TCollectionItem& CollectionGetAt(int Index)			{ call the CMyCol getvalueat }
//					bool CollectionAdd(const TCollectionItem &Element)	{ call the CMyCol additem }
//				};
//
//				  2. Add SETTING_ITEM_COLLECTION() macros for your collection to your CSettings child class
//
//	2004-10-17	By Pascal Hurni
//
//				CSettings and CSettingsStorage have now virtual destructors because they have virtual members.
//				Thanx to the unknown man for pointing this issue.
//
//	2004-09-10	By Pascal Hurni
//
//				CSettings overrideables made public. This way they can be called by emulation of Load() or Save().
//
//	2004-06-24	By Pascal Hurni
//
//				SaveLoadItem() for subitems (CSettings) returned true when Load() failed.
//				Fixed, it now returns false like expected.
//
//	2004-05-25	By Pascal Hurni
//
//				Load() and Save() updated. The storage OnAfterXXX() was not called if OnBeforeXXX()
//				returned false. It now is like the documentation tells.
//
//				Added GenerateAloneItemName() virtual functions, so that subclasses can override the
//				way items are named. Usefull for the XML subclass.
//
//				Collections: if OnItemCollection() returned no more item (<0), the item was still
//				added to the collection. Fixed, so that empty non-count based collection are possible.
//
//	2004-05-11	By Pascal Hurni
//
//				Added mandatory function for CSettingsStorage subclasses: CreateSubStorage()
//				This enables the followings:
//				  1. Implemented Collection related functions in the base class CSettingsStorage.
//				  2. Implemented SaveLoadItem() for subitem in the base class CSettingsStorage.
//				This saves subclasses from implmenting them. For special cases, subclasses
//				can still override the standard behaviour.
//
//				SaveLoadItemSTL() and SaveLoadItemCArray(): When an item failed, the rest
//				of the collection was skipped. Skipping is now determined by Storage::ContinueOnError().
//
//				SaveLoadItem() with std:string and std::wstring missed conversion from/to UNICODE when
//				saving. Loading was fine.
//
//	2004-04-19	By Pascal Hurni
//
//				SaveLoadItem() for CString and std::(w)string updated to follow spec
//				of TCHAR SaveLoadItem().
//
//	2004-04-16	By Pascal Hurni
//
//				FIXED BUG: The SETTING_ITEM_BINARY macro used SaveLoadItemSTL() instead of
//				SaveLoadItem().
//
//========================================================================================

#ifndef __MORTIMER_SETTINGSSTORAGE_H__
#define __MORTIMER_SETTINGSSTORAGE_H__

//========================================================================================
// encapsulate these classes in a namespace

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

//========================================================================================

class CSettingsStorage;

/// Settings base class.
/// To add useness, you have to derive from this base class and add some member variables and
/// a MAP to relate these members to the Load/Save behaviour.
/// 
/// Each map item macro has three forms. As an example, here are the macros for simple types:
///		SETTING_ITEM()				\n
///		SETTING_ITEM_DEFAULT()		\n
///		SETTING_ITEM_REQUIRE()		\n
class CSettings
{
public:
	// Virtual destructor so that subclasses have they own destructors called
	virtual ~CSettings() {}

	/// @name Overrideables
	/// These are called before and after the load or save operations inside Load() and Save().
	/// You get a chance here to be notified of the load and save operations. \n
	/// \a StorageOk indicates if opening or closing the storage was successful.
	/// If it wasn't the real load or save operations are not executed.
	//@{

	virtual void OnBeforeSave(bool StorageOk) {}
	virtual void OnAfterSave(bool StorageOk) {}
	virtual void OnBeforeLoad(bool StorageOk) {}
	virtual void OnAfterLoad(bool StorageOk) {}

	//@}

	// The map will override this one
	virtual bool InternalSaveLoad(bool Continue, CSettingsStorage *pStorage, bool bSave, bool bOnlyInit) = 0;

public:
	/// Load the settings from Storage.
	/// Note that it's up to the caller to init the Storage and to move to the right position.
	/// @param Storage Any initialized storage object. (Can be different from the one passed in Save()).
	/// @return Success status.
	virtual bool Load(CSettingsStorage &Storage);

	/// Save the settings to \a Storage.
	/// Note that it's up to the caller to init the \a Storage and to move to the right position.
	/// @param Storage Any initialized storage object. (Can be different from the one passed in Load()).
	/// @return Success status.
	virtual bool Save(CSettingsStorage &Storage);

}; // class CSettings

// Forward declaration
//class CSettings;

/// Abstract class that represent a storage mechanism.
/// Derive from it and implement the Integer, Simple and Extended types SaveLoadItem() functions.
/// Implement also the Implementation overrideables functions.
///
/// It's up to the derived class to add methods for opening and closing the storage.
class CSettingsStorage
{
public:
	// Virtual destructor so that subclasses have they own destructors called
	virtual ~CSettingsStorage() {}

	/// @name Implementation overrideables
	//@{
	/// Subclasses must override this function to return a heap allocated object that
	/// represent a substorage of this. SaveLoadItem() functions will be called from
	/// this substorage. So the returned Storage object must be opened with the SubName
	/// and ready to operate, but OnBeforeXXX() will be called before any SaveLoadItem().
	virtual CSettingsStorage *CreateSubStorage(LPCTSTR SubName) = 0;

	/// The result of this function is used to determine if the storage may continue
	/// to read/write items after a failed one.
	virtual bool ContinueOnError() = 0;

	//@}
	//------------------------------------------------------------------------------------
	/// @name Overrideables
	/// Returning false in OnBeforeXXX() will disable the calls to SaveLoadItem()
	/// but OnAfterXXX() will still be called.

	virtual bool OnBeforeSave() { return true; }
	virtual bool OnAfterSave() { return true; }
	virtual bool OnBeforeLoad() { return true; }
	virtual bool OnAfterLoad() { return true; }

public:
	//------------------------------------------------------------------------------------
	// Type specific SaveLoad functions.

	/// @name Integer types
	/// The derived class must implement them.
	//@{
	virtual bool SaveLoadItem(LPCTSTR szName, long& Variable, bool bSave) = 0;
	virtual bool SaveLoadItem(LPCTSTR szName, unsigned long& Variable, bool bSave) = 0;
	//@}

	/// @name Simple types
	/// The derived class must implement them.
	//@{
	virtual bool SaveLoadItem(LPCTSTR szName, bool& Variable, bool bSave) = 0;
	virtual bool SaveLoadItem(LPCTSTR szName, double& Variable, bool bSave) = 0;
	//@}

	/// @name Extended types
	/// The derived class must implement them.
	//@{

	/// Strings.
	/// When saving, \a size contains the length of the string. This function can ignore it. \n
	/// When loading, \a size contains the maximum size \a Variable can handle. If it's not enough, this function
	/// should set \a size to the requested size (with NUL) and return false.
	/// If you fail to load the value, return false and set \a size to 0. \n
	/// When \a Variable contains NULL, you must only set \a size to the requested size and return true. You can return false to indicate failure.
	virtual bool SaveLoadItem(LPCTSTR szName, TCHAR* Variable, ULONG& size, bool bSave) = 0;

	/// void* (binary).
	virtual bool SaveLoadItem(LPCTSTR szName, void* Variable, ULONG size, bool bSave) = 0;

	//@}
	//------------------------------------------------------------------------------------
	/// @name	Collections related functions
	///			The derived class can implement them directly. If it does not, standard behaviour
	///			is provided. The standard behaviour is 'Count' based, so the derived storage has
	///			to be Name/Value pair behavioured.
	//@{

protected:
	/// Called before browsing items.
	/// When saving: If \a ItemCount contains 0xFFFFFFFF, it indicates that
	///		the count is not known, but the implementor should handle this case. He should
	///		count the number of OnItemCollection() calls to determine the count (if needed). \n
	/// When loading: \a ItemCount must contain a valid value on return. It can contain 0xFFFFFFFF
	///		to indicate that the count is not known.
	virtual bool OnBeginCollection(LPCTSTR szName, UINT& ItemCount, bool bSave)
	{
		m_pCollectionStorage = CreateSubStorage(szName);
		if (!m_pCollectionStorage || (ItemCount == 0xFFFFFFFF))
			return false;

		if (bSave)
			m_pCollectionStorage->OnBeforeSave();
		else
			m_pCollectionStorage->OnBeforeLoad();

		// Save or Load the item count. Without Count items are useless, so fail.
		if (!m_pCollectionStorage->SaveLoadItem(_T("Count"), ItemCount, bSave))
			return false;

		return true;
	}

	/// Called for each item in the collection.
	/// @return
	///		- 0	indicates failure.
	///		- >0	indicates success.
	///		- <0	indicates 'no more items' (only used on Load if ItemCount was 0xFFFFFFFF). The current item is ignored.
	virtual int OnItemCollection(UINT Index, CSettings &variable, bool bSave)
	{
		if (!m_pCollectionStorage)
			return false;

		TCHAR ItemName[16];
		_stprintf_s(ItemName, _T("%i"), Index);
		CSettingsStorage *pSubStorage = m_pCollectionStorage->CreateSubStorage(ItemName);
		if (!pSubStorage)
			return 0;

		int Result;
		if (bSave)
			Result = variable.Save(*pSubStorage) ? 1 : 0;
		else
			Result = variable.Load(*pSubStorage) ? 1 : 0;

		delete pSubStorage;
		return Result;
	}

	/// Called after browsing all items.
	virtual bool OnEndCollection(bool bSave)
	{
		if (!m_pCollectionStorage)
			return false;

		if (bSave)
			m_pCollectionStorage->OnAfterSave();
		else
			m_pCollectionStorage->OnAfterLoad();

		delete m_pCollectionStorage;
		m_pCollectionStorage = NULL;

		return true;
	}

	/// Internal function for stand-alone types collections
	virtual bool OnBeginAloneCollection(LPCTSTR szName, UINT& ItemCount, bool bSave)
	{
		m_pCollectionStorage = CreateSubStorage(szName);
		if (!m_pCollectionStorage)
			return false;

		bool bResult;
		if (bSave)
			bResult = m_pCollectionStorage->OnBeforeSave();
		else
			bResult = m_pCollectionStorage->OnBeforeLoad();

		// Save or Load the item count. Without Count items are useless, so fail.
		if (!m_pCollectionStorage->SaveLoadItem(_T("Count"), ItemCount, bSave))
		{
			if (bSave)
				m_pCollectionStorage->OnAfterSave();
			else
				m_pCollectionStorage->OnAfterLoad();
			delete m_pCollectionStorage;
			m_pCollectionStorage = NULL;
			return false;
		}

		return bResult;
	}

	/// Called before each item in the stand-alone collection.
	/// Generate here the item name and put it in ItemName.
	/// Subclasses can override it to changes the standard behaviour which is: "Item.%i" where %i is the Index.
	/// @return
	///		- 0	indicates failure. The item will not be processed.
	///		- >0	indicates success.
	///		- <0	indicates 'no more items'. The loop will be broken and the current item is ignored.
	/// Note that the string you pass in ItemName will not be freed at any time, so the best way is to have a subclass member for that string.
	virtual int OnItemAloneCollection(UINT Index, bool bSave, LPCTSTR &ItemName)
	{
		_stprintf_s(m_ItemName, _T("Item.%i"), Index);
		ItemName = m_ItemName;
		return 1;
	}

	/// Internal function for stand-alone types collections
	virtual bool OnEndAloneCollection(bool bSave)
	{
		if (!m_pCollectionStorage)
			return false;

		bool bResult;
		if (bSave)
			bResult = m_pCollectionStorage->OnAfterSave();
		else
			bResult = m_pCollectionStorage->OnAfterLoad();

		delete m_pCollectionStorage;
		m_pCollectionStorage = NULL;

		return bResult;
	}

	//@}
	//------------------------------------------------------------------------------------
	/// @name Helpers functions
	///	These functions handles conversion from/to similar types of the
	/// handled ones (the ones that are pure virtual).
	/// The derived class does not need to override them, but they are still
	/// made virtual in case special handling is desired.
	//@{

public:
	virtual bool SaveLoadItem(LPCTSTR szName, int& Variable, bool bSave)
	{
		long Value = (long)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (int)Value;
		return bResult;
	}

	virtual bool SaveLoadItem(LPCTSTR szName, unsigned int& Variable, bool bSave)
	{
		unsigned long Value = (unsigned long)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (unsigned int)Value;
		return bResult;
	}

	virtual bool SaveLoadItem(LPCTSTR szName, short& Variable, bool bSave)
	{
		long Value = (long)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (short)Value;
		return bResult;
	}

	virtual bool SaveLoadItem(LPCTSTR szName, unsigned short& Variable, bool bSave)
	{
		unsigned long Value = (unsigned long)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (unsigned short)Value;
		return bResult;
	}

	virtual bool SaveLoadItem(LPCTSTR szName, signed char& Variable, bool bSave)
	{
		long Value = (long)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (signed char)Value;
		return bResult;
	}

	virtual bool SaveLoadItem(LPCTSTR szName, unsigned char& Variable, bool bSave)
	{
		unsigned long Value = (unsigned long)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (unsigned char)Value;
		return bResult;
	}

	virtual bool SaveLoadItem(LPCTSTR szName, LONGLONG& Variable, bool bSave)
	{
		return SaveLoadItem(szName, &Variable, sizeof(Variable), bSave);
	}

	virtual bool SaveLoadItem(LPCTSTR szName, ULONGLONG& Variable, bool bSave)
	{
		return SaveLoadItem(szName, &Variable, sizeof(Variable), bSave);
	}

	virtual bool SaveLoadItem(LPCTSTR szName, float& Variable, bool bSave)
	{
		double Value = (double)Variable;
		bool bResult = SaveLoadItem(szName, Value, bSave);
		if (!bSave && bResult)
			Variable = (float)Value;
		return bResult;
	}

	/// CSettings (sub items), 
	/// If overrideen, the derived class must (at a point that make sense) call the Variable.InternalSaveLoad() function.
	virtual bool SaveLoadItem(LPCTSTR szName, CSettings& Variable, bool bSave)
	{
		CSettingsStorage *pSubStorage = CreateSubStorage(szName);
		if (!pSubStorage)
			return false;

		bool Result;
		if (bSave)
			Result = Variable.Save(*pSubStorage);
		else
			Result = Variable.Load(*pSubStorage);

		delete pSubStorage;
		return Result;
	}

#if defined(_MFC_VER) || defined(_WTL_USE_CSTRING)
	virtual bool SaveLoadItem(LPCTSTR szName, CString& Variable, bool bSave)
	{
		if(bSave)
		{
			ULONG Size = Variable.GetLength();
			return SaveLoadItem(szName, (TCHAR*)(LPCTSTR)Variable, Size, bSave);
		}
		else
		{
			DWORD dwSize = 128;
			bool bResult = SaveLoadItem(szName, Variable.GetBuffer(dwSize), dwSize, bSave);
			if (!bResult)
			{
				if (dwSize == 0) return false;
				bResult = SaveLoadItem(szName, Variable.GetBuffer(dwSize), dwSize, bSave);
			}
			Variable.ReleaseBuffer();
			return bResult;
		}
	}

#endif
#ifdef _STRING_
	virtual bool SaveLoadItem(LPCTSTR szName, std::string& Variable, bool bSave)
	{
		if(bSave)
		{
			ULONG Size = (ULONG)Variable.size()+1;
			TCHAR *pString;
			#ifdef _UNICODE
				pString = (wchar_t*)_alloca(Size*2);
				size_t chars;
				mbstowcs_s(&chars, pString, Size, Variable.c_str(), Size);
			#else
				pString = const_cast<TCHAR*>(Variable.c_str());
			#endif
			return SaveLoadItem(szName, pString, Size, bSave);
		}
		else
		{
			TCHAR Buffer[1024];
			TCHAR *pBuf = Buffer;
			ULONG Size = sizeof(Buffer)/sizeof(Buffer[0]);
			bool bResult = SaveLoadItem(szName, Buffer, Size, bSave);
			if (!bResult)
			{
				if (Size == 0) return false;
				pBuf = (TCHAR*)_alloca(Size);
				bResult = SaveLoadItem(szName, pBuf, Size, bSave);
			}
			#ifdef _UNICODE
				CHAR *pMB = (CHAR*)_alloca(Size*2);
				size_t chars;
				wcstombs_s(&chars, pMB, Size, pBuf, Size);
				Variable = pMB;
			#else
				Variable = pBuf;
			#endif
			return bResult;
		}
	}
	virtual bool SaveLoadItem(LPCTSTR szName, std::wstring& Variable, bool bSave)
	{
		if(bSave)
		{
			ULONG Size = (ULONG)Variable.size()+1;
			TCHAR *pString;
			#ifdef _UNICODE
				pString = const_cast<TCHAR*>(Variable.c_str());
			#else
				pString = (char*)_alloca(Size*2);
				wcstombs(pString, Variable.c_str(), Size);
			#endif
			return SaveLoadItem(szName, pString, Size, bSave);
		}
		else
		{
			TCHAR Buffer[1024];
			TCHAR *pBuf = Buffer;
			ULONG Size = sizeof(Buffer)/sizeof(Buffer[0]);
			bool bResult = SaveLoadItem(szName, Buffer, Size, bSave);
			if (!bResult)
			{
				if (Size == 0) return false;
				pBuf = (TCHAR*)_alloca(Size);
				bResult = SaveLoadItem(szName, pBuf, Size, bSave);
			}
			#ifdef _UNICODE
				Variable = pBuf;
			#else
				wchar_t *pWide = (wchar_t*)_alloca(Size*2);
				mbstowcs(pWide, pBuf, Size);
				Variable = pWide;
			#endif
			return bResult;
		}
	}
#endif

	//@}
	//------------------------------------------------------------------------------------
	/// @name Collections Templates
	///	These are templates that handle collections of any CSettings subclass.
	//@{

	/// ATL CSimpleArray<T>, MFC CArray<T>
	template <class TArray, class TArrayItem> 
	bool SaveLoadItemCArray(LPCTSTR szName, TArray& Variable, TArrayItem& dummy, bool bSave)
	{
		if(!bSave)
			Variable.RemoveAll();

		bool bResult;
		UINT ItemCount = Variable.GetSize();
		if (bResult=OnBeginCollection(szName, ItemCount, bSave))
		{
			if(bSave)
			{
				for(UINT i = 0; i < ItemCount; i++)
				{
					if (0 == OnItemCollection(i, Variable[i], bSave))
					{
						bResult = false;
						if (!ContinueOnError())
							break;
					}
				}
			}
			else
			{
				// ItemCount can be 0xFFFFFFFF, which is fine for us because loop will be broken by the return code of OnItemCollection()
				for(UINT i = 0; i < ItemCount; i++)
				{
					TArrayItem item;
					int RetVal = OnItemCollection(i, item, bSave);

					// If no more items, break loop
					if (RetVal < 0)
						break;

					if (RetVal == 0)
					{
						bResult = false;
						if (!ContinueOnError())
							break;
					}
					else
						Variable.Add(item);
				}
			}

		}
		bResult &= OnEndCollection(bSave);
		return bResult;
	}
	
	/// std::vector<T>, std::list<T>, std::deque<T>
	template <class TSTLArray> 
	bool SaveLoadItemSTL(LPCTSTR szName, TSTLArray& Variable, bool bSave)
	{
		if(!bSave)
			Variable.clear();

		bool bResult;
		UINT ItemCount = Variable.size();
		if (bResult=OnBeginCollection(szName, ItemCount, bSave))
		{
			if(bSave)
			{
				UINT i = 0;
				for(TSTLArray::iterator it = Variable.begin(); it != Variable.end(); it++)
				{
					if (0 == OnItemCollection(i, *it, bSave))
					{
						bResult = false;
						if (!ContinueOnError())
							break;
					}
					i++;
				}
			}
			else
			{
				// ItemCount can be 0xFFFFFFFF, which is fine for us because loop will be broken by the return code of OnItemCollection()
				for(UINT i = 0; i < ItemCount; i++)
				{
					TSTLArray::value_type item;
					int RetVal = OnItemCollection(i, item, bSave);

					// If no more items, break loop
					if (RetVal < 0)
						break;

					if (0 == RetVal)
					{
						bResult = false;
						if (!ContinueOnError())
							break;
					}
					else
						Variable.push_back(item);
				}
			}

		}
		bResult &= OnEndCollection(bSave);
		return bResult;
	}

	/// Generic Collection, must adhere to the ICollection interface (not necessarely derive because of the template instanciation)
	template <class TCollection> 
	bool SaveLoadItemCollection(LPCTSTR szName, TCollection& Variable, bool bSave)
	{
		if(!bSave)
			Variable.CollectionRemoveAll();

		bool bResult;
		UINT ItemCount = Variable.CollectionGetSize();
		if (bResult=OnBeginCollection(szName, ItemCount, bSave))
		{
			if(bSave)
			{
				for(UINT i = 0; i < ItemCount; i++)
				{
					if (0 == OnItemCollection(i, Variable.CollectionGetAt(i), bSave))
					{
						bResult = false;
						if (!ContinueOnError())
							break;
					}
				}
			}
			else
			{
				// ItemCount can be 0xFFFFFFFF, which is fine for us because loop will be broken by the return code of OnItemCollection()
				for(UINT i = 0; i < ItemCount; i++)
				{
					TCollection::TCollectionItem item;
					int RetVal = OnItemCollection(i, item, bSave);

					// If no more items, break loop
					if (RetVal < 0)
						break;

					if (RetVal == 0)
					{
						bResult = false;
						if (!ContinueOnError())
							break;
					}
					else
						Variable.CollectionAdd(item);
				}
			}

		}
		bResult &= OnEndCollection(bSave);
		return bResult;
	}

	/// ATL CSimpleArray<T>, MFC CArray<T>  for standalone types
	template <class TArray, class TArrayItem> 
	bool SaveLoadItemCArrayAlone(LPCTSTR szName, TArray& Variable, TArrayItem& dummy, bool bSave)
	{
		if(!bSave)
			Variable.RemoveAll();

		bool bResult = true;
		UINT ItemCount = Variable.GetSize();

		if (!OnBeginAloneCollection(szName, ItemCount, bSave))
			return false;

		LPCTSTR ItemName;
		if(bSave)
		{
			for(UINT i = 0; i < ItemCount; i++)
			{
				int RetVal = OnItemAloneCollection(i, bSave, ItemName);
				if (RetVal < 0)
					break;

				if ((RetVal==0) || !m_pCollectionStorage->SaveLoadItem(ItemName, Variable[i], bSave))
				{
					bResult = false;
					if (!ContinueOnError())
						break;
				}
			}
		}
		else
		{
			TArrayItem item;
			for(UINT i = 0; i < ItemCount; i++)
			{
				int RetVal = OnItemAloneCollection(i, bSave, ItemName);
				if (RetVal < 0)
					break;

				if ((RetVal==0) || !m_pCollectionStorage->SaveLoadItem(ItemName, item, bSave))
				{
					bResult = false;
					if (!ContinueOnError())
						break;
				}
				else
					Variable.Add(item);
			}
		}

		bResult &= OnEndAloneCollection(bSave);
		return bResult;
	}

	/// std::vector<T>, std::list<T>, std::deque<T> for standalone types
	template <class TSTLArray> 
	bool SaveLoadItemSTLAlone(LPCTSTR szName, TSTLArray& Variable, bool bSave)
	{
		if(!bSave)
			Variable.clear();

		bool bResult = true;
		UINT ItemCount = Variable.size();

		if (!OnBeginAloneCollection(szName, ItemCount, bSave))
			return false;

		LPCTSTR ItemName;
		if(bSave)
		{
			UINT i = 0;
			for(TSTLArray::iterator it = Variable.begin(); it != Variable.end(); it++)
			{
				int RetVal = OnItemAloneCollection(i, bSave, ItemName);
				if (RetVal < 0)
					break;

				if ((RetVal==0) || !m_pCollectionStorage->SaveLoadItem(ItemName, *it, bSave))
				{
					bResult = false;
					if (!ContinueOnError())
						break;
				}
				i++;
			}
		}
		else
		{
			TSTLArray::value_type item;
			for(UINT i = 0; i < ItemCount; i++)
			{
				int RetVal = OnItemAloneCollection(i, bSave, ItemName);
				if (RetVal < 0)
					break;

				if ((RetVal==0) || !m_pCollectionStorage->SaveLoadItem(ItemName, item, bSave))
				{
					bResult = false;
					if (!ContinueOnError())
						break;
				}
				else
					Variable.push_back(item);
			}
		}

		bResult &= OnEndAloneCollection(bSave);
		return bResult;
	}

	//@}

protected:
	CSettingsStorage *m_pCollectionStorage;
	TCHAR m_ItemName[16];
};

//========================================================================================

inline bool CSettings::Load(CSettingsStorage &Storage)
{
	bool Result;

	OnBeforeLoad(Result = Storage.OnBeforeLoad());

	if (Result)
		Result = InternalSaveLoad(Storage.ContinueOnError(), &Storage, false, false);

	Result &= Storage.OnAfterLoad();
	OnAfterLoad(Result);

	return Result;
}

/// Save the settings to \a Storage.
/// Note that it's up to the caller to init the \a Storage and to move to the right position.
/// @param Storage Any initialized storage object. (Can be different from the one passed in Load()).
/// @return Success status.
inline bool CSettings::Save(CSettingsStorage &Storage)
{
	bool Result;

	OnBeforeSave(Result = Storage.OnBeforeSave());

	if (Result)
		Result = InternalSaveLoad(Storage.ContinueOnError(), &Storage, true, false);

	Result &= Storage.OnAfterSave();
	OnAfterSave(Result);

	return Result;
}


}; // namespace Mortimer

//========================================================================================
// MAP macros

#define BEGIN_SETTING_MAP(CLASS_NAME) \
	CLASS_NAME() \
	{ \
		InternalSaveLoad(true, NULL, false, true); \
	} \
	virtual bool InternalSaveLoad(bool Continue, Mortimer::CSettingsStorage *pStorage, bool bSave, bool bOnlyInit) { \
	bool bResult = true;

#define END_SETTING_MAP() return bResult; }

#define SETTING_ITEM_MEMBER(member) \
			if (bOnlyInit) member.InternalSaveLoad(Continue, NULL, false, true); else { \
				if (bSave) bResult &= member.Save(*pStorage); \
				else bResult &= member.Load(*pStorage); \
				if (!bResult && !Continue) return false; }

#define SETTING_ITEM(variable) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItem(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}

#define SETTING_ITEM_DEFAULT(variable, DEFAULT_VALUE) \
			if (bOnlyInit) variable = DEFAULT_VALUE; else \
			if (!pStorage->SaveLoadItem(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) variable = DEFAULT_VALUE; \
				bResult &= !bSave;}

#define SETTING_ITEM_REQUIRE(variable) \
			if(!bOnlyInit) \
			if (!pStorage->SaveLoadItem(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}

#define SETTING_ITEM_SUBITEM(variable) \
			if (bOnlyInit) variable.InternalSaveLoad(Continue, NULL, false, true); else \
			if (!pStorage->SaveLoadItem(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult &= false;}

#define SETTING_ITEM_ARRAY(variable, itemtype) \
			if (!bOnlyInit) \
			{ itemtype _dummy; \
			if (!pStorage->SaveLoadItemCArray(_T(#variable), variable, _dummy, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}}

#define SETTING_ITEM_ARRAY_DEFAULT(variable, itemtype, DefaultValue) \
			if (bOnlyInit) { variable.RemoveAll(); for (int i=0; i<DefaultValue.GetSize(); variable.Add(DefaultValue[i++])); } else \
			{ itemtype _dummy; \
			if (!pStorage->SaveLoadItemCArray(_T(#variable), variable, _dummy, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) { variable.RemoveAll(); for (int i=0; i<DefaultValue.GetSize(); variable.Add(DefaultValue[i++])); } \
			bResult &= !bSave;}}

#define SETTING_ITEM_ARRAY_REQUIRE(variable, itemtype) \
			if (!bOnlyInit) \
			{ itemtype _dummy; \
			if (!pStorage->SaveLoadItemCArray(_T(#variable), variable, _dummy, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}}

#define SETTING_ITEM_STL(variable) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItemSTL(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}

#define SETTING_ITEM_STL_DEFAULT(variable, DefaultValue) \
			if (bOnlyInit) variable.assign(DefaultValue.begin(), DefaultValue.end()); else \
			if (!pStorage->SaveLoadItemSTL(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) variable.assign(DefaultValue.begin(), DefaultValue.end()); \
				bResult &= !bSave;}

#define SETTING_ITEM_STL_REQUIRE(variable) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItemSTL(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}

#define SETTING_ITEM_COLLECTION(variable) \
			if (!bOnlyInit) { \
			if (!pStorage->SaveLoadItemCollection(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}}

#define SETTING_ITEM_COLLECTION_DEFAULT(variable, DefaultValue) \
			if (bOnlyInit) { variable.CollectionRemoveAll(); Variable.CollectionCopy(DefaultValue); } else { \
			if (!pStorage->SaveLoadItemCollection(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) { Variable.CollectionCopy(DefaultValue); } \
			bResult &= !bSave;}}

#define SETTING_ITEM_COLLECTION_REQUIRE(variable) \
			if (!bOnlyInit) { \
			if (!pStorage->SaveLoadItemCollection(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}}

#define SETTING_ITEM_ALONE_ARRAY(variable, itemtype) \
			if (!bOnlyInit) \
			{ itemtype _dummy; \
			if (!pStorage->SaveLoadItemCArrayAlone(_T(#variable), variable, _dummy, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}}

#define SETTING_ITEM_ALONE_ARRAY_DEFAULT(variable, itemtype, DefaultValue) \
			if (bOnlyInit) { variable.RemoveAll(); for (int i=0; i<DefaultValue.GetSize(); variable.Add(DefaultValue[i++])); } else \
			{ itemtype _dummy; \
			if (!pStorage->SaveLoadItemCArrayAlone(_T(#variable), variable, _dummy, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) { variable.RemoveAll(); for (int i=0; i<DefaultValue.GetSize(); variable.Add(DefaultValue[i++])); } \
			bResult &= !bSave;}}

#define SETTING_ITEM_ALONE_ARRAY_REQUIRE(variable, itemtype) \
			if (!bOnlyInit) \
			{ itemtype _dummy; \
			if (!pStorage->SaveLoadItemCArrayAlone(_T(#variable), variable, _dummy, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}}

#define SETTING_ITEM_ALONE_STL(variable) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItemSTLAlone(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}

#define SETTING_ITEM_ALONE_STL_DEFAULT(variable, DefaultValue) \
			if (bOnlyInit) variable.assign(DefaultValue.begin(), DefaultValue.end()); else \
			if (!pStorage->SaveLoadItemSTLAlone(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) variable.assign(DefaultValue.begin(), DefaultValue.end()); \
				bResult &= !bSave;}

#define SETTING_ITEM_ALONE_STL_REQUIRE(variable) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItemSTLAlone(_T(#variable), variable, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}

#define SETTING_ITEM_SZ(variable, variable_len) \
			if (!bOnlyInit) \
			{ ULONG size = variable_len; if (!pStorage->SaveLoadItem(_T(#variable), variable, size, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}}

#define SETTING_ITEM_SZ_DEFAULT(variable, variable_len, DEFAULT_VALUE) \
			if (bOnlyInit) _tcsncpy(variable, DEFAULT_VALUE, variable_len); else \
			{ ULONG size = variable_len; if (!pStorage->SaveLoadItem(_T(#variable), variable, size, bSave)) { \
				if (!Continue) return false; \
				if (!bSave) _tcsncpy(variable, DEFAULT_VALUE, variable_len); \
				bResult &= !bSave;}}

#define SETTING_ITEM_SZ_REQUIRE(variable, variable_len) \
			if (!bOnlyInit) \
			{ ULONG size = variable_len; if (!pStorage->SaveLoadItem(_T(#variable), variable, size, bSave)) { \
				if (!Continue) return false; \
				bResult = false;}}

#define SETTING_ITEM_BINARY(variable, variable_size) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItem(_T(#variable), (void*)&variable, variable_size, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}

#define SETTING_ITEM_BINARY_DEFAULT(variable, variable_size, pDefaultValue) \
			if (bOnlyInit) { if (pDefaultValue) memcpy((void*)&variable, (void*)pDefaultValue, variable_size); } else \
			if (!pStorage->SaveLoadItem(_T(#variable), (void*)&variable, variable_size, bSave)) { \
				if (!Continue) return false; \
				if (!bSave && pDefaultValue) memcpy((void*)&variable, (void*)pDefaultValue, variable_size); \
				bResult &= !bSave;}

#define SETTING_ITEM_BINARY_REQUIRE(variable, variable_size) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItem(_T(#variable), (void*)&variable, variable_size, bSave)) } \
				if (!Continue) return false; \
				bResult = false;}

#define SETTING_ITEM_BINARYPTR(variable, variable_size) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItemSTL(_T(#variable), (void*)variable, variable_size, bSave)) { \
				if (!Continue) return false; \
				bResult &= !bSave;}

#define SETTING_ITEM_BINARYPTR_DEFAULT(variable, variable_size, pDefaultValue) \
			if (bOnlyInit) { if (pDefaultValue) memcpy((void*)variable, (void*)pDefaultValue, variable_size); } else \
			if (!pStorage->SaveLoadItem(_T(#variable), (void*)variable, variable_size, bSave)) { \
				if (!Continue) return false; \
				if (!bSave && pDefaultValue) memcpy((void*)variable, (void*)pDefaultValue, variable_size); \
				bResult &= !bSave;}

#define SETTING_ITEM_BINARYPTR_REQUIRE(variable, variable_size) \
			if (!bOnlyInit) \
			if (!pStorage->SaveLoadItem(_T(#variable), (void*)variable, variable_size, bSave)) } \
				if (!Continue) return false; \
				bResult = false;}


#endif // __MORTIMER_SETTINGSSTORAGE_H__
