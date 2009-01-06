/////////////////////////////////////////////////////////////////////////////
//
// ATL Active Script Host Wrapper
// (C) Copyright 2001 VisionTech Limited. All rights reserved.
// http://www.visiontech.ltd.uk/
// bateman@acm.org
//
// VisionTech Limited makes no warranties, either express or implied,
// with respect to this source code and any accompanying materials.
//
// In no event shall VisionTech Limited or its suppliers be liable for
// any damages whatsoever (including, without limitation, damages for
// loss of business profits, business interruption, loss of business
// information, or other percuniary loss) arising out of the use or
// inability to use this software.
//
// This source code may be used for any purpose, including commercial
// applications, and may be modified or redistributed subject to the
// following conditions:
//
// a) This notice may not be removed or changed in any source distribution.
//
// b) Altered source versions must be include a note to that effect,
//    and must not be misrepresented as the original.
//
// c) The origin of this software may not be misrepresented - you may
//    not claim to have written the original version. If you use this
//    source in a product, an acknowledgement in the documentation
//    would be appreciated, but is not required.
//
/////////////////////////////////////////////////////////////////////////////

// modified by Antonin Hildebrand for purposes of Upgradr

#pragma once

/////////////////////////////////////////////////////////////////////////////
// IDebugDocumentHostImpl

class ATL_NO_VTABLE IDebugDocumentHostImpl : public IDebugDocumentHost {
public:

	STDMETHOD(GetDeferredText)( 
		DWORD dwTextStartCookie, 
		// Specifies a character text buffer. NULL means do not return characters. 
		WCHAR *pcharText, 
		// Specifies a character attribute buffer. NULL means do not return attributes. 
		SOURCE_TEXT_ATTR *pstaTextAttr, 
		// Indicates the actual number of characters/attributes returned. Must be set to zero 
		// before the call. 
		ULONG *pcNumChars, 
		// Specifies the number maximum number of character desired. 
		ULONG cMaxChars) 
	{
		return E_NOTIMPL;
	}

	// Return the text attributes for an arbitrary block of document text. 
	// It is acceptable for hosts to return E_NOTIMPL, in which case the 
	// default attributes are used. 
	STDMETHOD(GetScriptTextAttributes)( 
		// The script block text. This string need not be null terminated. 
		LPCOLESTR pstrCode, 
		// The number of characters in the script block text. 
		ULONG uNumCodeChars, 
		// See IActiveScriptParse::ParseScriptText for a description of this argument. 
		LPCOLESTR pstrDelimiter, 
		// See IActiveScriptParse::ParseScriptText for a description of this argument. 
		DWORD dwFlags, 
		// Buffer to contain the returned attributes. 
		SOURCE_TEXT_ATTR *pattr) 
	{
		return E_NOTIMPL;
	}

	// Notify the host that a new document context is being created 
	// and allow the host to optionally return a controlling unknown 
	// for the new context. 
	// 
	// This allows the host to add new functionality to the helper-provided 
	// document contexts. It is acceptable for the host to return E_NOTIMPL 
	// or a null outer unknown for this method, in which case the context is 
	// used "as is". 
	STDMETHOD(OnCreateDocumentContext)(IUnknown** ppunkOuter) 
	{
		return E_NOTIMPL;
	}

	// Return the full path (including file name) to the document's source file. 
	//*pfIsOriginalPath is TRUE if the path refers to the original file for the document. 
	//*pfIsOriginalPath is FALSE if the path refers to a newly created temporary file 
	//Returns E_FAIL if no source file can be created/determined. 
	STDMETHOD(GetPathName)(BSTR *pbstrLongName, BOOL *pfIsOriginalFile)
	{
		m_FileName.CopyTo(pbstrLongName);
		*pfIsOriginalFile = TRUE;
		return S_OK;
	}

	// Return just the name of the document, with no path information. 
	// (Used for "Save As...") 
	STDMETHOD(GetFileName)(BSTR *pbstrShortName) 
	{
		m_FileName.CopyTo(pbstrShortName);
		return S_OK;
	}

	// Notify the host that the document's source file has been saved and 
	// that its contents should be refreshed. 
	STDMETHOD(NotifyChanged)()
	{
		return S_OK;
	}

	CComBSTR                                       m_FileName;
};

/////////////////////////////////////////////////////////////////////////////
// IDebugDocumentHostImpl

class ATL_NO_VTABLE IDebugDocumentTextImpl : public IDebugDocumentText {
public:
	STDMETHOD(GetName)( 
		/* [in] */ DOCUMENTNAMETYPE dnt,
		/* [out] */ BSTR __RPC_FAR *pbstrName)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetDocumentClassId)( 
		/* [out] */ CLSID __RPC_FAR *pclsidDocument)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetDocumentAttributes)( 
		/* [out] */ TEXT_DOC_ATTR __RPC_FAR *ptextdocattr) 
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetSize)( 
		/* [out] */ ULONG __RPC_FAR *pcNumLines,
		/* [out] */ ULONG __RPC_FAR *pcNumChars)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetPositionOfLine)( 
		/* [in] */ ULONG cLineNumber,
		/* [out] */ ULONG __RPC_FAR *pcCharacterPosition)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetLineOfPosition)( 
		/* [in] */ ULONG cCharacterPosition,
		/* [out] */ ULONG __RPC_FAR *pcLineNumber,
		/* [out] */ ULONG __RPC_FAR *pcCharacterOffsetInLine)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetText)( 
		/* [in] */ ULONG cCharacterPosition,
		/* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
		/* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
		/* [out][in] */ ULONG __RPC_FAR *pcNumChars,
		/* [in] */ ULONG cMaxChars)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetPositionOfContext)( 
		/* [in] */ IDebugDocumentContext __RPC_FAR *psc,
		/* [out] */ ULONG __RPC_FAR *pcCharacterPosition,
		/* [out] */ ULONG __RPC_FAR *cNumChars)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

	STDMETHOD(GetContextOfPosition)( 
		/* [in] */ ULONG cCharacterPosition,
		/* [in] */ ULONG cNumChars,
		/* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc)
	{
		WarningBeep();
		ATLASSERT(0);
		return E_NOTIMPL;
	}

};
