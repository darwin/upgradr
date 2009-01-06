#include "StdAfx.h"
#include "FrameTable.h"
#include "FrameManager.h"

//#include "Debug.h"

CFrameTable::CFrameTable()
{

}

CFrameTable::~CFrameTable()
{
	Clear();
}

void
CFrameTable::Clear()
{
	iterator i = begin();
	while (i!=end())
	{
		delete i->second;
		++i;
	}
	clear();
}

CFrame*                                       
CFrameTable::Create(CComPtr<IHTMLDocument2> document)
{
	DT(TRACE_I(FS(_T("C_FrameTable::Create(doc=%08X)"), document)));
	if (GetFrameId(document)!=NULL_FRAME) return Find(document);

	CFrame* pkFrame = new CFrame(document, *this);
	// !!! don't dispatch mesages here
	CFrame* res = Insert(pkFrame);
	ATLASSERT(res);
	if (!res) delete pkFrame;
	DT(Dump());
	// ok free to dispatch messages
	return res;
}

void
CFrameTable::Destroy(CComPtr<IHTMLDocument2> document)
{
	DT(TRACE_I(FS(_T("C_FrameTable::Destroy(doc=%08X)"), document)));
	ATLASSERT(!!document);
	ATLASSERT(GetFrameId(document)!=NULL_FRAME);
	ATLASSERT(Find(GetFrameId(document)));

	vector<TFrameId> kFrames;
	GetListRecursive(document, kFrames);

	vector<TFrameId>::iterator i = kFrames.begin();
	while (i!=kFrames.end())
	{
		Remove(*i);
		++i;
	}

	DT(Dump());
}

CFrame*                                        
CFrameTable::Find(TFrameId frameId)
{
	ATLASSERT(frameId!=NULL_FRAME);
	iterator f = find(frameId);
	if (f!=end()) return f->second;
	return NULL;
}

CFrame*                                        
CFrameTable::Find(CComPtr<IHTMLDocument2> document)
{
	// document pointers may change from time to time, we need to lookup frame using frameId
	// this is always safe method

	ATLASSERT(document);
	ATLASSERT(GetFrameId(document)!=NULL_FRAME);
	return Find(GetFrameId(document));
}

TFrameId                                       
CFrameTable::GenerateFrameId()
{
	FrameManagerLock frameManager;
	return frameManager->GenerateFrameId();	
}

CFrame*                                           
CFrameTable::Insert(CFrame* pkFrame)
{
	ATLASSERT(pkFrame);
	TFrameId id = GetFrameId(pkFrame->m_Document);
	ATLASSERT(id!=NULL_FRAME);
	ATLASSERT(find(id)==end());
	std::pair<iterator, bool> res = insert(make_pair(id, pkFrame));
	ATLASSERT(res.second==true);
	return res.first->second;
}

bool                                           
CFrameTable::Remove(CComPtr<IHTMLDocument2> document)
{
	iterator i = begin();
	while (i!=end())
	{
		if (i->second->m_Document==document) 
		{
			Remove(i->first);
			return true;
		}
		++i;
	}
	ATLASSERT(0);
	return false;
}

bool                                           
CFrameTable::Remove(TFrameId frameId)
{
	ATLASSERT(frameId!=NULL_FRAME);

	iterator f = find(frameId);
	if (f==end()) return false;

	// remove document
	delete f->second;
	erase(f);

	// clear all links to this document
	iterator i = begin();
	while (i!=end())
	{
		i->second->RemoveLink(frameId);
		++i;
	}
	return true;
}

void                                           
CFrameTable::GetListRecursive(CComPtr<IHTMLDocument2> document, vector<TFrameId>& res)
{
	TFrameId id = GetFrameId(document);
	GetListRecursive(id, res);
}

void                                           
CFrameTable::GetListRecursive(TFrameId frameId, vector<TFrameId>& res)
{
	res.push_back(frameId);
	CFrame* pkFrame = Find(frameId);
	if (!pkFrame) return;
	TFrameLinkContainer::iterator i = pkFrame->m_ChildFrames.begin();
	while (i!=pkFrame->m_ChildFrames.end())
	{
		GetListRecursive(*i, res);
		++i;
	}
}

void                                           
CFrameTable::Dump()
{
	TRACE_I(FS(_T("CFrameTable[%08X]: size=%d"), this, size()));
	iterator i = begin();
	while (i!=end())
	{
		IHTMLDocument2* pDoc = i->second->m_Document;
		ULONG refs = pDoc->AddRef()-1;
		pDoc->Release();
		TRACE_I(FS(_T("  ID=%d: doc=%08X, childs=%d, #refs=%d, url=%s"), i->first, i->second->m_Document, i->second->m_ChildFrames.size(), refs, i->second->m_URL));
		TFrameLinkContainer::iterator j  = i->second->m_ChildFrames.begin();
		while (j!=i->second->m_ChildFrames.end())
		{
			TRACE_I(FS(_T("      %d"), *j));
			++j;
		}
		++i;
	}
}

bool                                           
CFrameTable::IsManaged(CComPtr<IHTMLDocument2> document, bool bImportant)
{
	return GetFrameId(document, bImportant)!=NULL_FRAME;
}

void                                    
CFrameTable::SetFrameId(CComPtr<IHTMLDocument2> document, TFrameId frameId, bool bImportant)
{
	// retrieve document's window object
	CComPtr<IHTMLWindow2> window;
	CHECK_COM(document->get_parentWindow(&window), _T("Cannot retrieve parent window"));
	if (!window) 
	{
		TRACE_E(_T("Cannot retrieve parent window for document"));
		ATLASSERT(0);
		return;
	}

	// cast window object to IDispatchEx
	CComQIPtr<IDispatchEx> windowDispEx = window;
	if (!windowDispEx)
	{
		TRACE_E(_T("Cannot retrieve IDispatchEx on parent window of document"));
		ATLASSERT(0);
		return;
	}

	// add FrameId property to window object
	DISPID dispid;
	DISPPARAMS dispparams;
	HRESULT hr = windowDispEx->GetDispID(CComBSTR(JAVASCRIPT_FRAMEID_VARIABLE), fdexNameEnsure|fdexNameCaseSensitive, &dispid);
	if (!bImportant && hr==E_ACCESSDENIED) return;
	CHECK_COM(hr, FS(_T("GetDispID for window.%s has failed"), JAVASCRIPT_FRAMEID_VARIABLE));
	DISPID putid = DISPATCH_PROPERTYPUT;
	CComVariant var(frameId);
	dispparams.rgvarg = &var;
	dispparams.rgdispidNamedArgs = &putid;
	dispparams.cArgs = 1;
	dispparams.cNamedArgs = 1;
	VARIANT temp;
	CHECK_COM(windowDispEx->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, &temp, NULL, NULL), _T("Failed to add property to window object")); 

	ATLASSERT(GetFrameId(document)==frameId);
	DT(TRACE_I(FS(_T("CFrameTable::SetFrameId(doc=%08X, id=%d) OK"), document, frameId)));
}

TFrameId                             
CFrameTable::GetFrameId(CComPtr<IHTMLDocument2> document, bool bImportant)
{
	// get frame id from window.<JAVASCRIPT_FRAMEID_VARIABLE>

	// retrieve document's window object
	CComPtr<IHTMLWindow2> window;
	CHECK_COM(document->get_parentWindow(&window), _T("Cannot retrieve parent window"));
	if (!window) 
	{
		TRACE_E(_T("Cannot retrieve parent window for document"));
		ATLASSERT(0);
		return NULL_FRAME;
	}

	// cast window object to IDispatchEx
	CComQIPtr<IDispatchEx> windowDispEx = window;
	if (!windowDispEx)
	{
		TRACE_E(_T("Cannot retrieve IDispatchEx on parent window of document"));
		ATLASSERT(0);
		return NULL_FRAME;
	}

	// read FrameId property to window object
	DISPID dispid;
	DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
	HRESULT hr = windowDispEx->GetDispID(CComBSTR(JAVASCRIPT_FRAMEID_VARIABLE), fdexNameCaseSensitive, &dispid);
	if (hr==DISP_E_UNKNOWNNAME) return NULL_FRAME;
	if (!bImportant && hr==E_ACCESSDENIED) return NULL_FRAME;
	CHECK_COM(hr, FS(_T("GetDispID for window.%s has failed"), JAVASCRIPT_FRAMEID_VARIABLE));
	CComVariant res;
	CHECK_COM(windowDispEx->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispparams, &res, NULL, NULL), _T("Failed to read property from window object")); 
	ATLASSERT(res.vt==VT_UI4);
	return res.ulVal;
}

void                                           
CFrameTable::RemoveFrameId(CComPtr<IHTMLDocument2> document)
{
	// we have to remove member property window.<JAVASCRIPT_FRAMEID_VARIABLE> from document's window
	// but implmentation of IDispatchEx of IHTMLWindow2 does not support removing properties
	//
	// so, we just set frame id in window.<JAVASCRIPT_FRAMEID_VARIABLE> to NULL_FRAME
	// this efectively marks document as not tracked by our system
	// this is not important call (there may be ACCESSDENIED respons, because document is being destroyed)
	SetFrameId(document, NULL_FRAME, false); 

	ATLASSERT(GetFrameId(document)==NULL_FRAME);
	DT(TRACE_I(FS(_T("CFrameTable::RemoveFrameId(doc=%08X) OK"), document)));
}

void                                           
CFrameTable::UpdateLinks()
{
	iterator i = begin();
	while (i!=end())
	{
		i->second->UpdateLinks();
		++i;
	}
	DT(Dump());
}


//////////////////////////////////////////////////////////////////////////

CFrame::CFrame(CComPtr<IHTMLDocument2> document, CFrameTable& rkFrameTable):
m_FrameTable(rkFrameTable),
m_Attached(false)
{
	// create script engine
	CComObject<CUpgradrScriptEngine>::CreateInstance(&m_ScriptEngine);
	ATLASSERT(!!m_ScriptEngine);
	m_ScriptEngine->AddRef();

	// it is important to do frame registration here
	// there must be no message dispatching between setting of frame id and inserting frame into frame table
	TFrameId newId = m_FrameTable.GenerateFrameId();
	// we must set it at this point before creation of document, so scripts can ask for it during creation
	m_ScriptEngine->SetFrameId(newId);

	// create document
	Create(document);

	DT(TRACE_I(FS(_T("CFrame::Create(%s) id=%d"), m_URL, newId)));
	m_FrameTable.SetFrameId(m_Document, newId, true); // this important call
	ATLASSERT(m_FrameTable.GetFrameId(m_Document)!=NULL_FRAME);
}

CFrame::~CFrame()
{
	Destroy();

	CHECK_REFCOUNT_BEFORE_DELETE(m_ScriptEngine);
	m_ScriptEngine->Release();
	m_ScriptEngine = NULL;
}

void                                           
CFrame::UpdateLinks()
{
	// start from scratch
	m_ChildFrames.clear();

	// detect all child frames
	CComQIPtr<IOleContainer> spContainer = m_Document;
	if (!!spContainer)
	{
		// get an enumerator for the frames
		IEnumUnknown* pEnumerator;
		HRESULT hr = spContainer->EnumObjects(OLECONTF_EMBEDDINGS, &pEnumerator);

		if (SUCCEEDED(hr))
		{
			IUnknown* pUnknown;
			ULONG uFetched;

			// enumerate all the frames
			for (UINT i = 0; S_OK == pEnumerator->Next(1, &pUnknown, &uFetched); i++)
			{
				CComQIPtr<IWebBrowser2> spChildBrowser = pUnknown;
				pUnknown->Release();
				if (!!spChildBrowser)
				{
					// well done, store child frame
					CComPtr<IDispatch> spChildDispDoc;
					CHECK_COM(spChildBrowser->get_Document(&spChildDispDoc), FS(_T("Cannot retrieve document object from %08X"), spChildBrowser));
					CComQIPtr<IHTMLDocument2> spChildDoc = spChildDispDoc;

					// sometimes IE can return NULL spChildDispDoc 
					// 
					// (observed on pages with file download blocking)
					// test for valid doc
					if (!!spChildDoc)
					{
						CComBSTR url2;
						CHECK_COM(spChildDoc->get_URL(&url2), FS(_T("Cannot retrieve location URL from %08X"), spChildDoc));
						CString URL2 = url2;

						// it wouldn't be so easy to simply add child document into childs
						// but IE (maybe) does some reallocation of this document, so we need to do lookup via frameId

						TFrameId id = m_FrameTable.GetFrameId(spChildDoc, false);

						// wait for document to load, 
						// there is RISK OF CALLS to other functions !!!
						//if (id==NULL_FRAME) id = m_FrameTable.GetFrameId(spChildDoc, true); // blocking call
						if (id==NULL_FRAME)
						{
							// TODO: tohle se muze nekdy stat, kdyz se frame jeste nestihne naloadovat pri F5, napriklad gmail.com to dela
							TRACE_W(FS(_T("Child doc not found for %s !!"), URL2));
						}
						else
						{
							m_ChildFrames.push_back(id);
						}
					}
				}
			}
		}
		pEnumerator->Release();
	}
}

bool														
CFrame::IsScriptableDocument()
{
	if (m_MIME.IsEmpty()) return true; // documents withou MIME type are probably HTML Documents, see remarks below
	if (m_MIME==_T("HTML Document")) return true;
	return false;
}

HRESULT
CFrame::Create(CComPtr<IHTMLDocument2> document)
{
	// cache document
	m_Document = document;
	CComBSTR url;
	CHECK_COM(document->get_URL(&url), FS(_T("Cannot retrieve location URL from %08X"), document));
	m_URL = url;

	CComBSTR mime;
	document->get_mimeType(&mime);
	DT(TRACE_I(FS(_T("MIME=%s, URL=%s"), CString(mime), m_URL)));
	m_MIME = mime;

	//////////////////////////////////////////////////////////////////////////
	// look for child frames

	// attach scripting engine (if applicable)
	// ---------------------------------------
	// here we need to test what kind of document we have here
	// we need to apply scrpting engine only to HTML documents
	// applying scripting engine to PDF, DOC, ... documents may cause problems and even throw exceptions
	// see: http://groups.google.cz/group/microsoft.public.inetsdk.programming.html_objmodel/browse_thread/thread/a72010c809ced9d1/00f299ba42346dff?lnk=st&q=ihtmldocument2+pdf&rnum=3#00f299ba42346dff
	// probably we need to use http://home.nyc.rr.com/itandetnik/PassthruAPP.zip to implement this test correctly
	// TODO: use PasstrhuAPP to detect document type and attach script engine only onto HTML documents

	if (IsScriptableDocument())
	{
		m_Attached = m_ScriptEngine->Attach(document, m_URL);
	}

	// update my links
	// different approach: do not update when created frame, child documents may be not prepared
	// we will fire links update when loading of all relevant documents is finished
	// UpdateLinks();

	return S_OK;
}

void                                           
CFrame::Destroy()
{
	if (m_Attached)
	{
		m_ScriptEngine->Detach();
	}

	m_ChildFrames.clear();
	m_FrameTable.RemoveFrameId(m_Document);
	m_Document = NULL;
}

void                                           
CFrame::RemoveLink(TFrameId frameId)
{
	m_ChildFrames.erase(remove(m_ChildFrames.begin(), m_ChildFrames.end(), frameId), m_ChildFrames.end());
}