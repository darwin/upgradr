//========================================================================================
//
// Module:			SettingsStorage
// Author:          Pascal Hurni
// Creation Date:	25.05.2004
//
// Copyright 2003 Mortimer Systems
// This software is free. I grant you a non-exclusive license to use it.
//
// Modifications:
//
//
//========================================================================================

#ifndef __MORTIMER_SETTINGSSTORAGEMSXML_H__
#define __MORTIMER_SETTINGSSTORAGEMSXML_H__

#include "MSettingsStorage.h"

#import "msxml3.dll" raw_interfaces_only
using namespace MSXML2;


#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Mortimer
{
#endif

/// CSettingsStorage implementation for XML using MSXML.
/// This class can't be used alone, it's a base class that handles DOMDocument.
/// Use subclasses to actually use a persistant XML storage.
class CSettingsStorageMSXML : public CSettingsStorage
{
protected:
	IXMLDOMDocumentPtr m_DocumentPtr;
	IXMLDOMNodePtr m_NodePtr;				// The root node from where to get/put children elements
	IXMLDOMNodePtr m_ParentNodePtr;			// The parent of the root node.
	TCHAR m_NodeName[MAX_PATH];				// The m_NodePtr related node name.
	bool m_UseParentAsNode;					// Use the given parent as the node.

	// Used for collections
	IXMLDOMNodeListPtr m_CollectionNodeListPtr;
	IXMLDOMNodePtr m_CollectionNodePtr;
	IXMLDOMNodePtr m_CollectionCurrentItemNodePtr;
	TCHAR m_CollectionItemName[MAX_PATH];
	bool m_ProcessingAloneCollection;

public:
	CSettingsStorageMSXML() : m_ProcessingAloneCollection(false)
	{
	}

	CSettingsStorageMSXML(IXMLDOMDocumentPtr DocPtr, IXMLDOMNodePtr NodePtr, LPCTSTR SubName, bool UseParentAsNode)
		: m_DocumentPtr(DocPtr), m_ParentNodePtr(NodePtr), m_UseParentAsNode(UseParentAsNode), m_ProcessingAloneCollection(false)
	{
		_tcscpy(m_NodeName, SubName);
		m_ProcessingAloneCollection = false;
	}

	CSettingsStorage *CreateSubStorage(LPCTSTR SubName)
	{
		return new CSettingsStorageMSXML(m_DocumentPtr, m_NodePtr, SubName, false);
	}

	bool OnBeforeLoad()
	{
		if (m_UseParentAsNode)
			m_NodePtr = m_ParentNodePtr;
		else
			m_NodePtr = GetChildNodeByName(m_ParentNodePtr, m_NodeName);

		return m_NodePtr;
	}

	bool OnAfterLoad()
	{
		return true;
	}

	bool OnBeforeSave()
	{
		if (m_UseParentAsNode)
			m_NodePtr = m_ParentNodePtr;
		else
			m_NodePtr = CreateChildNode(m_ParentNodePtr, m_NodeName);

		return m_NodePtr;
	}

	bool OnAfterSave()
	{
		return true;
	}

	bool ContinueOnError()
	{
		// We want the system to read/write the most possible items, even if one fails
		return true;
	}

public:
	//------------------------------------------------------------------------------------
	// Here are the type specific SaveLoad functions

	// Handy macro used for all simple types
	#define SSXF_SAVELOADITEM(type, token) inline bool SaveLoadItem(LPCTSTR szName, type& Variable, bool bSave) \
	{	TCHAR Value[MAX_PATH]; \
		ULONG Size = MAX_PATH; \
		if (bSave) { \
			_stprintf(Value, _T(token), Variable); \
			Size = _tcslen(Value); \
			return SaveLoadItem(szName, Value, Size, bSave); \
		} else { \
			if (!SaveLoadItem(szName, Value, Size, bSave)) return false; \
			return 1 == _stscanf(Value, _T(token), &Variable); \
	}	}

	SSXF_SAVELOADITEM(long, "%d")
	SSXF_SAVELOADITEM(unsigned long, "%u")

	SSXF_SAVELOADITEM(float, "%f")
	SSXF_SAVELOADITEM(double, "%lf")

	// bool
	inline bool SaveLoadItem(LPCTSTR szName, bool& Variable, bool bSave)
	{
		if (bSave)
			return SaveLoadItem(szName, Variable ? _T("TRUE") : _T("FALSE"), 6, bSave);
		else
		{
			TCHAR Value[10];
			if (!SaveLoadItem(szName, Value, sizeof(Value), bSave)) return false;
			Variable = _tcscmp(Value, _T("TRUE")) == 0;
			return true;
		}
	}

	// TCHAR*
	inline bool SaveLoadItem(LPCTSTR szName, TCHAR* Variable, ULONG& Size, bool bSave)
	{
		HRESULT hr;

		if (bSave)
		{
			IXMLDOMElementPtr ElementPtr;
			IXMLDOMTextPtr ValuePtr;
			IXMLDOMNodePtr DummyNodePtr;

			hr = m_DocumentPtr->createElement(_bstr_t(szName), &ElementPtr);
			if (FAILED(hr))
				return false;

			if (_tcspbrk(Variable, _T("<>&")) != NULL)
				hr = m_DocumentPtr->createCDATASection(_bstr_t(Variable), (MSXML2::IXMLDOMCDATASection**)&ValuePtr);
			else
				hr = m_DocumentPtr->createTextNode(_bstr_t(Variable), &ValuePtr);
			if (FAILED(hr))
				return false;

			hr = ElementPtr->appendChild(ValuePtr, &DummyNodePtr);
			if (FAILED(hr))
				return false;

			if (m_ProcessingAloneCollection)
				hr = m_CollectionNodePtr->appendChild(ElementPtr, &DummyNodePtr);
			else
				hr = m_NodePtr->appendChild(ElementPtr, &DummyNodePtr);
			if (FAILED(hr))
				return false;

			return true;
		}
		else
		{
			IXMLDOMNodePtr NodePtr;

			if (m_ProcessingAloneCollection)
				NodePtr = m_CollectionCurrentItemNodePtr;
			else
				NodePtr = GetChildNodeByName(m_NodePtr, szName);

			if (NodePtr == NULL)
			{
				Size = 0;
				return false;
			}

			IXMLDOMNodePtr ValuePtr;
			hr = NodePtr->get_firstChild(&ValuePtr);
			if (FAILED(hr))
			{
				Size = 0;
				return false;
			}

			_variant_t ValueVariant;
			_bstr_t ValueText;

			// When no child, don't get it. Will result in an empty string
			if (hr == S_OK)
			{
				hr = ValuePtr->get_nodeValue(&ValueVariant);
				if (hr != S_OK)
				{
					Size = 0;
					return false;
				}
				ValueText = ValueVariant;
			}

			if (Variable == NULL)
			{
				Size = ValueText.length()+1;
				return true;
			}
			if (ValueText.length() >= Size)
			{
				Size = ValueText.length()+1;
				return false;
			}

			if ((TCHAR*)ValueText)
				_tcsncpy(Variable, (TCHAR*)ValueText, Size);
			else
				*Variable = 0;
			return true;
		}
	}

	// void*
	inline bool SaveLoadItem(LPCTSTR szName, void* Variable, ULONG Size, bool bSave)
	{
		if (bSave)
		{
			// allocate memory for the hex string
			ULONG StringSize = Size*3;
			TCHAR *pHexString = new TCHAR[StringSize+1];
			if (!pHexString)
				return false;

			BYTE *pVar = (BYTE*)Variable;
			for (ULONG i=0; i<Size; i++)
			{
				_stprintf(pHexString+i*3, _T("%02X "), *pVar++);
			}
			pHexString[StringSize] = 0;

			bool Result = SaveLoadItem(szName, pHexString, StringSize, bSave);
			delete pHexString;
			return Result;
		}
		else
		{
			ULONG StringSize;
			if (!SaveLoadItem(szName, (TCHAR*)NULL, StringSize, bSave))
				return false;

			// allocate memory for the hex string
			TCHAR *pHexString = new TCHAR[StringSize];
			if (!pHexString)
				return false;

			if (!SaveLoadItem(szName, pHexString, StringSize, bSave))
				return false;

			BYTE *pVar = (BYTE*)Variable;
			ULONG Val;
			for (ULONG i=0; i<Size; i++)
			{
				_stscanf(pHexString+i*3, _T("%2x "), &Val);
				*pVar++ = (BYTE)Val;
			}

			delete pHexString;
			return true;
		}
	}

protected:
	bool OnBeginCollection(LPCTSTR szName, UINT& ItemCount, bool bSave)
	{
		if (bSave)
			m_CollectionNodePtr = CreateChildNode(m_NodePtr, szName);
		else
			m_CollectionNodePtr = GetChildNodeByName(m_NodePtr, szName);

		if (m_CollectionNodePtr == NULL)
			return false;

		// Generate the collection item name
		_stprintf(m_CollectionItemName, _T("%s.Item"), szName);

		if (!bSave)
		{
			HRESULT hr;

			hr = m_CollectionNodePtr->get_childNodes(&m_CollectionNodeListPtr);
			if (FAILED(hr))
				return false;

			LONG Count;
			hr = m_CollectionNodeListPtr->get_length(&Count);
			if (FAILED(hr))
				return false;

			hr = m_CollectionNodeListPtr->reset();
			if (FAILED(hr))
				return false;

			ItemCount = Count;
		}

		return true;
	}

	int OnItemCollection(UINT Index, CSettings &variable, bool bSave)
	{
		HRESULT hr;
		IXMLDOMNodePtr ItemNodePtr;

		if (bSave)
		{
			CSettingsStorageMSXML ItemStorage(m_DocumentPtr, m_CollectionNodePtr, m_CollectionItemName, false);
			return variable.Save(ItemStorage) ? 1 : 0;
		}
		else
		{
			hr = m_CollectionNodeListPtr->get_item(Index, &ItemNodePtr);
			if (FAILED(hr) || !(bool)ItemNodePtr)
				return 0;

			// Ensure it is a valid Item element
			BSTR bstrNode;
			if (S_OK != ItemNodePtr->get_nodeName(&bstrNode))
				return 0;

			_bstr_t NodeName(bstrNode, false);
			if (_tcscmp(m_CollectionItemName, (TCHAR*)NodeName) != 0)
				return 0;

			CSettingsStorageMSXML ItemStorage(m_DocumentPtr, ItemNodePtr, m_CollectionItemName, true);
			return variable.Load(ItemStorage) ? 1 : 0;
		}
	}

	bool OnEndCollection(bool bSave)
	{
		m_CollectionNodeListPtr = NULL;
		m_CollectionNodePtr = NULL;
		m_CollectionCurrentItemNodePtr = NULL;

		return true;
	}

	bool OnBeginAloneCollection(LPCTSTR szName, UINT& ItemCount, bool bSave)
	{
		if (!OnBeginCollection(szName, ItemCount, bSave))
			return false;

		m_ProcessingAloneCollection = true;
		m_pCollectionStorage = this;
		return true;
	}

	int OnItemAloneCollection(UINT Index, bool bSave, LPCTSTR &ItemName)
	{
		ItemName = m_CollectionItemName;

		if (bSave)
			return 1;
		else
		{
			HRESULT hr = m_CollectionNodeListPtr->get_item(Index, &m_CollectionCurrentItemNodePtr);
			if (FAILED(hr) || !(bool)m_CollectionCurrentItemNodePtr)
				return 0;

			// Ensure it is a valid Item element
			BSTR bstrNode;
			if (S_OK != m_CollectionCurrentItemNodePtr->get_nodeName(&bstrNode))
				return 0;

			_bstr_t NodeName(bstrNode, false);
			return _tcscmp(m_CollectionItemName, (TCHAR*)NodeName) == 0 ? 1 : 0;
		}
	}

	bool OnEndAloneCollection(bool bSave)
	{
		if (!OnEndCollection(bSave))
			return false;

		m_ProcessingAloneCollection = false;
		m_pCollectionStorage = NULL;
		return true;
	}

protected:
	IXMLDOMNodePtr GetChildNodeByName(IXMLDOMNodePtr ParentNodePtr, LPCTSTR Name)
	{
		HRESULT hr;
		IXMLDOMNodeListPtr NodeListPtr;
		IXMLDOMNodePtr NodePtr;

		hr = ParentNodePtr->get_childNodes(&NodeListPtr);
		if (FAILED(hr))
			return NULL;

		LONG Count;
		hr = NodeListPtr->get_length(&Count);
		if (FAILED(hr))
			return NULL;

		hr = NodeListPtr->reset();
		if (FAILED(hr))
			return NULL;

		for(int i = 0; i < Count; i++)
		{
			hr = NodeListPtr->get_item(i, &NodePtr);
			if (SUCCEEDED(hr) && (bool)NodePtr)
			{
				BSTR bstrItem;
				hr = NodePtr->get_nodeName(&bstrItem);
				_bstr_t ItemText(bstrItem, false);
				if ((hr==S_OK) && (_tcscmp((TCHAR*)ItemText, Name) == 0))
				{
					return NodePtr;
				}
			}
		}

		return NULL;
	}

	IXMLDOMNodePtr CreateChildNode(IXMLDOMNodePtr ParentNodePtr, LPCTSTR Name)
	{
		HRESULT hr;
		IXMLDOMElementPtr NodePtr;

		hr = m_DocumentPtr->createElement(_bstr_t(Name), &NodePtr);
		if (FAILED(hr))
			return NULL;

		IXMLDOMNodePtr InsertedNodePtr;
		hr = ParentNodePtr->appendChild(NodePtr, &InsertedNodePtr);
		if (FAILED(hr))
			return NULL;

		return InsertedNodePtr;
	}

}; // class CSettingsStorageMSXML

/// CSettingsStorage implementation for XML files using MSXML.
class CSettingsStorageMSXMLFile : public CSettingsStorageMSXML
{
public:
	CSettingsStorageMSXMLFile()
	{
		m_XMLFileName[0] = 0;
		m_RootElementName[0] = 0;
	}

	/// Constructor. You can already define which XML file to use. See SetXMLFileName() for the parameters.
	CSettingsStorageMSXMLFile(LPCTSTR FileName, LPCTSTR RootElementName)
	{
		m_XMLFileName[0] = 0;
		m_RootElementName[0] = 0;
		SetXMLFileName(FileName, RootElementName);
	}

	/// Defines the XML Filename related to this storage object.
	/// Call it before any calls to CSettings::Load() or CSettings::Save().
	/// @param FileName The XML file pathname.
	/// @param RootElementName The name for the root element.
	void SetXMLFileName(LPCTSTR FileName, LPCTSTR RootElementName)
	{
		if (FileName) _tcsncpy(m_XMLFileName, FileName, MAX_PATH);
		if (RootElementName) _tcsncpy(m_RootElementName, RootElementName, MAX_PATH);
	}

	bool OnBeforeLoad()
	{
		HRESULT hr;

		// Create DOMDocument and load it
		hr = m_DocumentPtr.CreateInstance(_T("Msxml2.DOMDocument"));
		if (FAILED(hr))
			return false;

		// Prepare the filename in case of relative path
		_bstr_t URLName;		
		if (m_XMLFileName[1] != ':')
		{
			TCHAR CurDir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, CurDir);
			URLName += CurDir;
			URLName += "\\";
		}
		URLName += m_XMLFileName;

		// Note that if load returns S_FALSE to indicate it can't, this is
		// ok for us, because the SETTING_ITEM_ might all be non _REQUIRE ones.
		VARIANT_BOOL LoadOk;
		hr = m_DocumentPtr->load(_variant_t(URLName), &LoadOk);
		if (FAILED(hr))
			return false;

		if (LoadOk)
		{
			IXMLDOMElementPtr RootNodePtr;
			hr = m_DocumentPtr->get_documentElement(&RootNodePtr);
			if (FAILED(hr))
				return false;

			m_NodePtr = RootNodePtr;
			return true;
		}
		else
		{
			// The current Document is empty, so create the root node so that any subitem behaves correctly
			m_NodePtr = CreateChildNode(m_DocumentPtr, m_RootElementName);
			return m_NodePtr;
		}
	}

	bool OnAfterLoad()
	{
		m_NodePtr = NULL;

		// Destroy the DOMDocument
		m_DocumentPtr = NULL;

		return true;
	}

	bool OnBeforeSave()
	{
		HRESULT hr;

		// Create DOMDocument
		hr = m_DocumentPtr.CreateInstance(_T("Msxml2.DOMDocument"));
		if (FAILED(hr))
			return false;

		// create the root element
		m_NodePtr = CreateChildNode(m_DocumentPtr, m_RootElementName);
		return m_NodePtr;
	}

	bool OnAfterSave()
	{
		HRESULT hr;

		// Save the DOMDocument to file
		hr = m_DocumentPtr->save(_variant_t(m_XMLFileName));

		// Destroy the DOMDocument
		m_DocumentPtr = NULL;

		return SUCCEEDED(hr);
	}

protected:
	TCHAR m_XMLFileName[MAX_PATH];
	TCHAR m_RootElementName[MAX_PATH];
};


}; // namespace Mortimer

#endif // __MORTIMER_SETTINGSSTORAGERMSXML_H__
