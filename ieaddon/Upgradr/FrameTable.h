#pragma once

#include "UpgradrScriptEngine.h"

class CFrame;

typedef hash_map<TFrameId, CFrame*>             TFrameContainer;
typedef vector<TFrameId>                        TFrameLinkContainer;
typedef vector<CAdapt<CComPtr<IDispatch> > >		TDocumentContainer;

class CFrameTable : TFrameContainer {
public:
	CFrameTable();
	~CFrameTable();

	TFrameId                                     GenerateFrameId();

	void                                         SetFrameId(CComPtr<IHTMLDocument2> document, TFrameId frameId, bool bImportant = false);
	void                                         RemoveFrameId(CComPtr<IHTMLDocument2> document);
	TFrameId                                     GetFrameId(CComPtr<IHTMLDocument2> document, bool bImportant = false);

	bool                                         IsManaged(CComPtr<IHTMLDocument2> document, bool bImportant = false);
	CFrame*                                      Create(CComPtr<IHTMLDocument2> document);
	CFrame*                                      Find(CComPtr<IHTMLDocument2> document);
	void                                         Destroy(CComPtr<IHTMLDocument2> document);
	void		                                    Clear();

	void                                         UpdateLinks();

private:
	CFrame*                                      Insert(CFrame* frame);
	bool                                         Remove(TFrameId frameId);
	bool                                         Remove(CComPtr<IHTMLDocument2> document);
	CFrame*                                      Find(TFrameId frameId);
	void                                         GetListRecursive(TFrameId frameId, vector<TFrameId>& res);
	void                                         GetListRecursive(CComPtr<IHTMLDocument2> document, vector<TFrameId>& res);
	void                                         Dump();

private:
};

class CFrame {
public:
	friend CFrameTable;

	CFrame(CComPtr<IHTMLDocument2> document, CFrameTable& rkFrameTable);
	~CFrame();

	void                                         UpdateLinks();

private:
	bool														IsScriptableDocument();
	HRESULT                                      Create(CComPtr<IHTMLDocument2> document);
	void                                         Destroy();
	void                                         RemoveLink(TFrameId frameId);

	CFrameTable&                                 m_FrameTable;		///< reference to owner
	CComPtr<IHTMLDocument2>                      m_Document;			///< reference to wrapped document
	CComObject<CUpgradrScriptEngine>*       m_ScriptEngine;	///< associated script engine
	CString                                      m_URL;				///< document URL
	CString													m_MIME;				///< mime type of document
	TFrameLinkContainer                          m_ChildFrames;		///< ids of child frames
	bool														m_Attached;			///< is script engine attached ?
};
