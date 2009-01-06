#include "StdAfx.h"
#include "DOMExplorerWindow.h"
#include "BrowserManager.h"

//////////////////////////////////////////////////////////////////////////
// CDOMTree

void 
CDOMTree::UpdateDOM(IStream* pStream)
{
	SetRedraw(FALSE);
	DeleteAllItems();
	m_Document = NULL;

	IHTMLDocument2* pDoc = NULL;
	CHECK(CoGetInterfaceAndReleaseStream(pStream, IID_IHTMLDocument2, (void**)&pDoc));
	m_Document = pDoc;
	pDoc->Release();
	PrepareDOMTree();
	SetRedraw(TRUE);
}

bool 
CDOMTree::PrepareDOMTree()
{
	CComQIPtr<IHTMLDocument3, &IID_IHTMLDocument3> spDoc3 = m_Document;
	bool bRet = spDoc3 != NULL;

	if ( bRet)
	{
		CComPtr<IHTMLElement> spRootElement;
		bRet = SUCCEEDED(spDoc3->get_documentElement(&spRootElement));
		if (bRet)
		{
			CComQIPtr<IHTMLDOMNode> spRootNode = spRootElement;
			bRet = spRootNode != NULL;
			if (bRet) InsertDOMNode(spRootNode, NULL, NULL);
		}
	}
	else
	{
		ATLASSERT(0); // IE 4 does not support IHTMLDocument3
	}
	return bRet;
}

void 
CDOMTree::SetupImageList(bool bSet) 
{
	if (bSet)
	{
		if (m_ImageList.CreateFromImage(IDB_DOMICONS, 24, 0, CLR_DEFAULT, IMAGE_BITMAP))
			SetImageList(m_ImageList, TVSIL_NORMAL);
	}
	else
	{
		m_ImageList.Destroy(); 
		m_ImageList = NULL;
	}
}	

// Find which node are we expanding by checking for particular interface pointer
// stored as node data. Then insert its children. Creating children only on demand,
// like in this case, saves a bit of memory plus we don't need to call all interfaces
// and construct entire tree if it is not going to be entirely expanded.
LRESULT 
CDOMTree::OnTreeItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);
	TVITEM* pItem = &pnmtv->itemNew;
	RECT rc;

	if (!pItem || !pItem->hItem) return 1;

	if (pnmtv->action == TVE_EXPAND)
	{
		// lParam is documented as valid during this notification
		IUnknown* pIface = reinterpret_cast<IUnknown*>(pItem->lParam); 

		CComQIPtr<IHTMLDOMNode> spNode = pIface;
		if (!!spNode) InsertDOMNodeChildren(spNode, pItem->hItem);
		//else
		//{
		//	CComQIPtr<IHTMLAttributeCollection> spAttrColl = pIface;
		//	// if expanding attribute collection this is the time to add attributes
		//	if (!!spAttrColl)  
		//		InsertAttributes(spAttrColl, pItem->hItem);
		//	else
		//		ATLASSERT(0); // something strange here
		//}
	}

	// Cause a call to OnTreeGetDispInfo which will take
	// care of displaying correct, i.e. expanded or collapsed node bitmap.
	// It is neccessary to handle this message since double-click,
	// which will expand or collapse the item, will not necesarily 
	// redisplay the item by calling OnTreeGetDispInfo

	if (GetItemRect(pItem->hItem, &rc, FALSE)) InvalidateRect( &rc);
	return 0;
}

// Here we delete collapsing node's children
LRESULT 
CDOMTree::OnTreeItemExpanded(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);

	if (pnmtv->action & TVE_COLLAPSE)
	{
		TVITEM* pItem = &pnmtv->itemNew;
		if (!pItem || !pItem->hItem)	return 0;

		Expand(pItem->hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
		// TVE_COLLAPSERESET should clear TVIS_EXPANDEDONCE if it was set
		ATLASSERT((GetItemState(pItem->hItem, 0xFF) & TVIS_EXPANDEDONCE) == 0);
	}
	return 0;
}

// DOM treeview display procedure. Finds the interface stored under tree item data and figures
// out what text and bitmap indices should be used for that interface
LRESULT 
CDOMTree::OnTreeGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMTVDISPINFO lptvdi = reinterpret_cast<LPNMTVDISPINFO>(pnmh); 
	DWORD_PTR data = GetItemData( lptvdi->item.hItem);
	IUnknown* pIface = reinterpret_cast<IUnknown*>(data);
	static TCHAR itemtext[4096]; // not so good - should check for node type and call method to get text length

	if (pIface == NULL) return 0;

	CComQIPtr<IHTMLDOMNode> spNode = pIface;
	if (!spNode) // not a node, may be attribute collection
	{
		CComQIPtr<IHTMLAttributeCollection> spAttrColl = pIface;
		if (!spAttrColl) // may be attribute itself
		{
			CComQIPtr<IHTMLDOMAttribute> spAttr = pIface;
			if (!!spAttr)
			{
				if ((lptvdi->item.mask & TVIF_IMAGE) || (lptvdi->item.mask & TVIF_SELECTEDIMAGE))
				{
					int imageidx = 8;
					lptvdi->item.iImage = lptvdi->item.iSelectedImage = imageidx;
				}

				if (lptvdi->item.mask & TVIF_TEXT)
				{
					CComBSTR name;
					CComVariant value;

					spAttr->get_nodeName(&name);
					spAttr->get_nodeValue(&value); 
					CImportantElementAttributes kImportantAttributes;
					FindImportantAttributes(spNode, kImportantAttributes);
					FormatNameAndValue(name, value, kImportantAttributes, itemtext);
					lptvdi->item.pszText = itemtext;
					lptvdi->item.cchTextMax = lstrlen(itemtext);
				}
			}
		}
		else
		{
			if ((lptvdi->item.mask & TVIF_IMAGE) || (lptvdi->item.mask & TVIF_SELECTEDIMAGE))
			{
				bool bExpanded = (GetItemState(lptvdi->item.hItem, 0xFF) & TVIS_EXPANDED) != 0;
				int imageidx = 6;
				lptvdi->item.iImage = bExpanded ? (imageidx + 1) : imageidx;
				lptvdi->item.iSelectedImage = lptvdi->item.iImage;
			}
			if (lptvdi->item.mask & TVIF_TEXT)
			{
				static LPTSTR strAttributeCollectionTitle = _T("Attributes");
				lptvdi->item.pszText = strAttributeCollectionTitle;
				lptvdi->item.cchTextMax = lstrlen( strAttributeCollectionTitle);
			}
		}
	}
	else // this is a text, comment or element node
	{
		long type; //  1 == ELEMENT - no enum for node types in MSHTML headers ( 3 is text and 8 is comment)

		if (FAILED(spNode->get_nodeType(&type))) return 0;
		ATLASSERT(type == 1 || type == 3 || type == 8);

		if ((lptvdi->item.mask & TVIF_IMAGE) || (lptvdi->item.mask & TVIF_SELECTEDIMAGE))
		{
			bool bExpanded =  (GetItemState(lptvdi->item.hItem, 0xFF) & TVIS_EXPANDED) != 0;
			int imageidx = type == 1 ? 0 : (type == 8 ? 4 : 2);
			lptvdi->item.iImage = bExpanded ? (imageidx + 1) : imageidx;
			lptvdi->item.iSelectedImage = lptvdi->item.iImage;
		}

		if (lptvdi->item.mask & TVIF_TEXT)
		{
			CComBSTR name;
			CComVariant value;

			spNode->get_nodeName(&name);
			if (type != 1)	spNode->get_nodeValue(&value); // elements never have a value

			CImportantElementAttributes kImportantAttributes;
			FindImportantAttributes(spNode, kImportantAttributes);
			FormatNameAndValue(name, value, kImportantAttributes, itemtext);
			lptvdi->item.pszText = itemtext;
			lptvdi->item.cchTextMax = lstrlen(itemtext);
		}
	}
	return 0;
}

LRESULT 
CDOMTree::FindImportantAttributes(CComQIPtr<IHTMLDOMNode> spNode, CImportantElementAttributes& res)
{
	CComPtr<IDispatch> spCollectionDispatch;
	if (SUCCEEDED(spNode->get_attributes(&spCollectionDispatch)))
	{
		CComQIPtr<IHTMLAttributeCollection> spCollection = spCollectionDispatch;
		if (!!spCollection)
		{
			long numChildren = 0;
			spCollection->get_length(&numChildren);

			for (long i = 0; i < numChildren; i++)
			{
				CComVariant index(i);
				CComPtr<IDispatch> spItemDispatch;
				spCollection->item(&index, &spItemDispatch);
				if (spItemDispatch != NULL)
				{
					CComQIPtr<IHTMLDOMAttribute> spAttribute = spItemDispatch;
					if (spAttribute != NULL)
					{
						VARIANT_BOOL bspecified = VARIANT_FALSE;
						spAttribute->get_specified(&bspecified);

						if (bspecified == VARIANT_TRUE) 
						{
							CComBSTR name;
							spAttribute->get_nodeName(&name);
							COLE2T aname(name);
							if (lstrcmpi(aname, _T("id"))==0) res.m_Id = spAttribute;
							if (lstrcmpi(aname, _T("class"))==0) res.m_Class = spAttribute;
						}
					}
				}
			}
		}
	}
	return 0;
}

// context menu allows us to expand or collapse entire tree (all its children, children's children tec.)
// for selected item,
LRESULT 
CDOMTree::OnTreeContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (HWND(wParam) != m_hWnd) // handle it only in this control
		return 0;

	int x = GET_X_LPARAM(lParam); 
	int y = GET_Y_LPARAM(lParam); 
	//CMenu popup, context;
	//popup.LoadMenu( IDR_TREECONTEXTMENU);
	//context = popup.GetSubMenu(0);
	//context.TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, HWND(wParam));
	//bHandled = TRUE;
	return 0;
}

// release interface pointer when tree item is deleted
LRESULT 
CDOMTree::OnTreeItemDeleted(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);
	TVITEM* pItem = &pnmtv->itemOld;

	if (pItem && pItem->lParam)
	{
		IUnknown* pNodeUnknown = reinterpret_cast<IUnknown*>(pItem->lParam);

#if defined(_DEBUG)
		long relcnt = pNodeUnknown->Release(); // for AddRef when added to the tree item data
		ATLASSERT(relcnt == 0);
#else
		pNodeUnknown->Release(); 
#endif
	}
	return 0;
}

// some DOM elements have a lot of attributes but few that are set. 
bool 
CDOMTree::IsAnyAttributeSet(IHTMLAttributeCollection* pIAttrColl) 
{
	bool bRet = false;
	long numChildren = 0;
	pIAttrColl->get_length( &numChildren);
	if (numChildren > 0)
	{
		for (long i = 0; i < numChildren; i++)
		{
			CComVariant index( i);
			CComPtr<IDispatch> spItemDispatch;
			pIAttrColl->item(&index, &spItemDispatch);
			if (spItemDispatch != NULL)
			{
				CComQIPtr<IHTMLDOMAttribute> spAttribute = spItemDispatch;
				if (spAttribute != NULL)
				{
					VARIANT_BOOL bspecified = VARIANT_FALSE;
					spAttribute->get_specified(&bspecified);

					if (bspecified == VARIANT_TRUE)
					{
						bRet = true;
						break; // one attribute is set, get out
					}
				}
			}
		}
	}
	return bRet;
}

// Except for inserting the node, this method figures out if it is a leaf node or it has children.
// Children are not inserted here but setting cChildren in TVITEM to 1 signals to treeview that
// node may be expanded so that children may be added at expansion time.
HTREEITEM 
CDOMTree::InsertDOMNode(IHTMLDOMNode* pINode, HTREEITEM hparent, HTREEITEM hinsertAfter)
{
	CComPtr<IDispatch> spCollectionDispatch;
	CComPtr<IHTMLDOMNode> spNode(pINode);
	TV_INSERTSTRUCT tvis;

	tvis.hParent = hparent;
	tvis.hInsertAfter = hinsertAfter;
	tvis.item.mask =  TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_PARAM; 
	tvis.item.pszText = LPSTR_TEXTCALLBACK;
	tvis.item.iImage = I_IMAGECALLBACK; 
	tvis.item.iSelectedImage = I_IMAGECALLBACK;
	tvis.item.cChildren = 0;

	if (SUCCEEDED(spNode->get_childNodes(&spCollectionDispatch)))
	{
		CComQIPtr<IHTMLDOMChildrenCollection> spCollection = spCollectionDispatch;
		if (!!spCollection)
		{
			long numChildren = 0;
			spCollection->get_length(&numChildren);
			if (numChildren > 0)	tvis.item.cChildren = 1;
		}
	}
	// see if we have attributes
	//if (tvis.item.cChildren == 0)
	//{
	//	// release for use in the the next call
	//	spCollectionDispatch = NULL;

	//	if (SUCCEEDED(spNode->get_attributes(&spCollectionDispatch)))
	//	{
	//		CComQIPtr<IHTMLAttributeCollection> spCollection = spCollectionDispatch;
	//		if (!!spCollection)
	//		{
	//			if (IsAnyAttributeSet(spCollection)) tvis.item.cChildren = 1;
	//		}
	//	}
	//}

	// Need to AddRef because we'll be keeping interface pointer as treeview item data
	// and use it in display phase
	pINode->AddRef();

	tvis.item.lParam = reinterpret_cast<LPARAM>(pINode);
	HTREEITEM hthisItem = InsertItem(&tvis);
	if (hthisItem == NULL)
	{
		pINode->Release();
		return NULL;
	}
	return hthisItem;
}

// This is HTMLDOMNode children isertion method. Only one level of children are inserted.
// Both attributes and "real" HTMLDOMNode children are added.
void 
CDOMTree::InsertDOMNodeChildren(IHTMLDOMNode* pINode, HTREEITEM hparent)
{
	CComPtr<IDispatch> spCollectionDispatch;
	CComPtr<IHTMLDOMNode> spNode(pINode);

	HTREEITEM hlastInsertedChild = NULL;

	//if (SUCCEEDED(spNode->get_attributes( &spCollectionDispatch)))
	//{
	//	CComQIPtr<IHTMLAttributeCollection> spCollection = spCollectionDispatch;
	//	if (!!spCollection && IsAnyAttributeSet(spCollection))
	//		hlastInsertedChild = InsertAttributeCollection(spCollection, hparent);		
	//}

	// release for use in the the next call
	spCollectionDispatch = NULL;

	if (SUCCEEDED(spNode->get_childNodes(&spCollectionDispatch)))
	{
		long numChildren = 0;
		CComQIPtr<IHTMLDOMChildrenCollection> spCollection = spCollectionDispatch;
		if (!!spCollection)
		{
			spCollection->get_length(&numChildren);

			for (long i = 0; i < numChildren; i++)
			{
				CComPtr<IDispatch> spItemDispatch;
				spCollection->item(i, &spItemDispatch);
				if (!!spItemDispatch)
				{
					CComQIPtr<IHTMLDOMNode> spItemNode = spItemDispatch;
					if (!!spItemNode) hlastInsertedChild = InsertDOMNode(spItemNode, hparent, hlastInsertedChild);
				}
			}
		}
	}
}

// We will put all IHTMLDOMNode attributes under the separate treeview node and each attribute
// will be a child of that special node. We are limiting ourselves to IE 5 assuming
// that each attribute is childless treeview node. Newer IE 6 IHTMLDOMAttribute2 interface
// treats attribute as DOMNode so attribute can have it own IHTMLDOMNode children.
HTREEITEM 
CDOMTree::InsertAttributeCollection(IHTMLAttributeCollection* pIAttrColl, HTREEITEM hparent)
{
	TV_INSERTSTRUCT tvis;
	tvis.hParent = hparent;
	tvis.hInsertAfter = NULL; // attribute collection is always the first child of each DOM node
	tvis.item.mask =  TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_PARAM; 
	tvis.item.pszText = LPSTR_TEXTCALLBACK;
	tvis.item.iImage = I_IMAGECALLBACK; 
	tvis.item.iSelectedImage = I_IMAGECALLBACK;
	tvis.item.cChildren  = 1;

	// we'll be keeping interface pointer as treeview item data so that we can call it during display phase
	pIAttrColl->AddRef();
	tvis.item.lParam = reinterpret_cast<LPARAM>(pIAttrColl);
	HTREEITEM hthisItem = InsertItem(&tvis);
	if (!hthisItem) pIAttrColl->Release();
	return hthisItem;
}

// Attribute collection children are inserted when collection node is expanding
void CDOMTree::InsertAttributes(IHTMLAttributeCollection* pIAttrColl, HTREEITEM hparent)
{
	long numChildren = 0;
	HTREEITEM hlastInsertedChild = NULL;
	pIAttrColl->get_length(&numChildren);

	for (long i = 0; i < numChildren; i++)
	{
		CComVariant index( i);
		CComPtr<IDispatch> spItemDispatch;
		pIAttrColl->item( &index, &spItemDispatch);
		if (spItemDispatch != NULL)
		{
			CComQIPtr<IHTMLDOMAttribute> spAttribute = spItemDispatch;
			if (spAttribute != NULL)
			{
				VARIANT_BOOL bspecified = VARIANT_FALSE;
				spAttribute->get_specified(&bspecified);

				if (bspecified == VARIANT_TRUE) 
					hlastInsertedChild = InsertAttribute(spAttribute, hparent, hlastInsertedChild);
			}
		}
	}
}

// single attribute leaf node insertion method
HTREEITEM 
CDOMTree::InsertAttribute(IHTMLDOMAttribute* pIAttribute, HTREEITEM hparent, HTREEITEM hinsertAfter)
{
	TV_INSERTSTRUCT tvis;
	tvis.hParent = hparent;
	tvis.hInsertAfter = hinsertAfter;
	tvis.item.mask =  TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
	tvis.item.pszText = LPSTR_TEXTCALLBACK;
	tvis.item.iImage = I_IMAGECALLBACK; 
	tvis.item.iSelectedImage = I_IMAGECALLBACK;

	// we'll be keeping interface pointer as treeview item data so that we can call it during display phase
	pIAttribute->AddRef();
	tvis.item.lParam = reinterpret_cast<LPARAM>(pIAttribute);
	HTREEITEM hthisItem = InsertItem( &tvis);
	if (!hthisItem) pIAttribute->Release();
	return hthisItem;
}

// HTMLDOMNode and HTMLDOMAttribute can have null, string and integer values
void 
CDOMTree::FormatNameAndValue(const CComBSTR& name, const CComVariant& value, CImportantElementAttributes& attributes, TCHAR* pTextResult)
{
	if (value.vt != VT_EMPTY && value.vt != VT_NULL)
	{
		if (value.vt == VT_BSTR)
		{
			if (value.bstrVal != NULL)
				lstrcpy(pTextResult, COLE2T(value.bstrVal));
		}
		else if (value.vt == VT_I4)
		{
			TCHAR number[64];
			wsprintf(number, _T("%d"), value.intVal);
			lstrcpy(pTextResult, number);
		}
	}
	else
	{
		lstrcpy(pTextResult, _T("<"));
		lstrcat(pTextResult, COLE2T(name));

		if (!!attributes.m_Id)
		{
			CComVariant attrValue;
			attributes.m_Id->get_nodeValue(&attrValue);
			if (attrValue.vt == VT_BSTR && attrValue.bstrVal != NULL)
			{
				lstrcat(pTextResult, _T(" id="));
				lstrcat(pTextResult, COLE2T(attrValue.bstrVal));
			}
		}

		if (!!attributes.m_Class)
		{
			CComVariant attrValue;
			attributes.m_Class->get_nodeValue(&attrValue);
			if (attrValue.vt == VT_BSTR && attrValue.bstrVal != NULL)
			{
				lstrcat(pTextResult, _T(" class="));
				lstrcat(pTextResult, COLE2T(attrValue.bstrVal));
			}
		}

		lstrcat(pTextResult, _T(">"));
	}
}

// Recursive helper that calls a method, that param fp points to, for a hParent node
// and all of its children, its children's children, etc.
// This is one of few cases where I used ->* notation for method call through a pointer
void 
CDOMTree::DoForItemAndChildren(HTREEITEM hParent, FOREACHITEM fp, unsigned int flags)
{
	// DOPARENTFIRST flag tells us if we want fp to be called on parent node first, then children, or viceversa
	if (flags & DOPARENTFIRST)
	{
		if (!(flags & SKIPTOPPARENT))	(this->*fp)( hParent);
	}

	HTREEITEM hChild = GetChildItem(hParent);
	while (hChild)
	{
		DoForItemAndChildren(hChild, fp, (flags & DOPARENTFIRST) ? DOPARENTFIRST : 0);
		hChild = GetNextSiblingItem(hChild);
	}
	if (!(flags & DOPARENTFIRST))
	{
		if (!(flags & SKIPTOPPARENT)) (this->*fp)(hParent);
	}
}

// Expand entire tree under currently selected node. Mouse click expands only
// one level so it may take quite some time to expand a large tree. 
void 
CDOMTree::ExpandAll()
{
	HTREEITEM hCurrent = GetSelectedItem();
	if (!hCurrent) hCurrent = GetRootItem();
	if (hCurrent)
	{
		SetRedraw(FALSE);
		// When expanding we must expand parent before its children
		// because expanding notification is what inserts children in
		// the first place.
		unsigned int flags = DOPARENTFIRST;
		DoForItemAndChildren(hCurrent, &CDOMTree::ExpandItem, flags);
		EnsureVisible(hCurrent);
		SetRedraw(TRUE);
	}
}

void 
CDOMTree::CollapseAll()
{
	HTREEITEM hCurrent = GetSelectedItem();
	if (!hCurrent)	hCurrent = GetRootItem();
	if (hCurrent)
	{
		SetRedraw(FALSE);
		Expand(hCurrent, TVE_COLLAPSE | TVE_COLLAPSERESET);
		SetRedraw(TRUE);
	}
}

LRESULT 
CDOMTree::OnTreeDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
	// We call SetupImageList method here because we are not destroying CDOMTree object
	// even when dialog and its child treeview window are destroyed. It is a member object.
	// So we can not destroy image list in destructor. 
	// DeleteAllItems() makes sure that OnTreeItemDeleted() is invoked for all items
	// which in turn releases interface pointer stored as item data

	SetupImageList(false); // destroys image list - it is not destroyed when window is
	DeleteAllItems();
	m_Document = NULL;
	return 0;
}

LRESULT 
CDOMTree::OnExpandAll( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	ExpandAll();
	return 0;
}

LRESULT 
CDOMTree::OnCollapseAll( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	CollapseAll();
	return 0;
}

void 
CDOMTree::ExpandItem( HTREEITEM hItem )
{
	// The following two calls are required to cover one strange bug in treeview. Namely,
	// TVE_COLLAPSE | TVE_COLLAPSERESET flags do not always clear TVIS_EXPANDEDONCE flag
	// as claimed in the documentation. Without SetItemState call, for example, tree root
	// item will always have TVIS_EXPANDEDONCE set, even if Expand( root, TVE_COLLAPSE | TVE_COLLAPSERESET)
	// was called on it right before expanding it.

	if ((GetItemState( hItem, 0xFF) & TVIS_EXPANDEDONCE))
	{
		Expand(hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
		SetItemState(hItem, 0, TVIS_EXPANDED | TVIS_EXPANDEDONCE);
	}

	ATLASSERT((GetItemState(hItem, 0xFF) & TVIS_EXPANDEDONCE) == 0);
	// this is the only call we would make if documentation was not misleading and/or incomplete
	Expand(hItem);
}

//////////////////////////////////////////////////////////////////////////
// CDOMExplorerTreeWindow

CDOMExplorerTreeWindow::CDOMExplorerTreeWindow()
{

}

CDOMExplorerTreeWindow::~CDOMExplorerTreeWindow()
{

}

void 
CDOMExplorerTreeWindow::SetupImageList(bool bSet) 
{
	if (bSet)
	{
		if (m_ImageList.CreateFromImage(IDB_DOMICONS, 24, 0, CLR_DEFAULT, IMAGE_BITMAP))
			GetTreeControl()->SetImageList(m_ImageList, TVSIL_NORMAL);
	}
	else
	{
		m_ImageList.Destroy(); 
		m_ImageList = NULL;
	}
}	


//////////////////////////////////////////////////////////////////////////
// CDOMExplorerWindow

CDOMExplorerWindow::CDOMExplorerWindow()
{
}

CDOMExplorerWindow::~CDOMExplorerWindow()
{
}

LRESULT 
CDOMExplorerWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD style = WS_TABSTOP | 
		WS_CHILD | 
		WS_VISIBLE |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN |
		TVS_HASLINES | 
		TVS_HASBUTTONS | 
		TVS_NOTOOLTIPS |
		TVS_SHOWSELALWAYS |
		TVS_NOHSCROLL |
		TVS_DISABLEDRAGDROP |
		TVS_LINESATROOT;

	m_TreeWindow.Create(*this, rcDefault, DOMEXPLORERTREE_WINDOW_NAME);
	m_TreeWindow.SetupImageList(true);

	CHeaderCtrl& header = *m_TreeWindow.GetHeaderControl();

	HDITEM col = { 0 };
	col.mask = HDI_FORMAT | HDI_TEXT | HDI_WIDTH;
	col.fmt = HDF_LEFT;
	col.cxy = 250;
	col.pszText = _T("DOM");
	header.InsertItem(0, &col);
	col.cxy = 150;
	col.pszText = _T("CSS");
	header.InsertItem(1, &col);
	
	CTreeCtrl& tree = *m_TreeWindow.GetTreeControl();

	HTREEITEM hParent = tree.InsertItem( _T( "Top Level 1.1" ), 0, 1, TVI_ROOT, TVI_LAST );
	m_TreeWindow.SetSubItemText(hParent, 	1, _T("Sub item test"));
	tree.InsertItem( _T( "Second Level 2.1" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Second Level 2.2" ), 0, 1, hParent, TVI_LAST );
	tree.Expand( hParent, TVE_EXPAND );
	tree.SelectItem( hParent );
	hParent = tree.InsertItem( _T( "Second Level 2.3" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Third Level 3.1" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Third Level 3.2" ), 0, 1, hParent, TVI_LAST );
	tree.Expand( hParent, TVE_EXPAND );
	hParent = tree.InsertItem( _T( "Third Level 3.3" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Fourth Level 4.1" ), 0, 1, hParent, TVI_LAST );
	tree.Expand( hParent, TVE_EXPAND );
	hParent = tree.InsertItem( _T( "Top Level 1.2" ), 0, 1, TVI_ROOT, TVI_LAST );
	tree.InsertItem( _T( "Second Level 2.1" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Second Level 2.2" ), 0, 1, hParent, TVI_LAST );
	tree.Expand( hParent, TVE_EXPAND );
	hParent = tree.InsertItem( _T( "Second Level 2.3" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Third Level 3.1" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Third Level 3.2" ), 0, 1, hParent, TVI_LAST );
	tree.Expand( hParent, TVE_EXPAND );
	hParent = tree.InsertItem( _T( "Third Level 3.3" ), 0, 1, hParent, TVI_LAST );
	tree.InsertItem( _T( "Fourth Level 4.1" ), 0, 1, hParent, TVI_LAST );
	tree.Expand( hParent, TVE_EXPAND );		

	tree.SetMultipleSelect(TRUE);
	tree.SetFocus();		
	tree.ShowThemed(TRUE);

	return TRUE;
}

bool                                         
CDOMExplorerWindow::RequestDOM()
{
	// request DOM document from BHO
	ATLASSERT(m_BrowserId!=NULL_BROWSER);

	BrowserManagerLock browserManager;
	return browserManager->RequestDOM(m_BrowserId);
}

LRESULT 
CDOMExplorerWindow::OnUpdateDOM(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_TreeWindow.UpdateDOM((IStream*)lParam);
	return 0;
}

void 
CDOMExplorerWindow::UpdateLayout()
{
	RECT  rc;
	GetClientRect(&rc);

	HDWP hdwp = BeginDeferWindowPos(1);
	hdwp = m_TreeWindow.DeferWindowPos(hdwp, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
}

LRESULT 
CDOMExplorerWindow::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS * lpwndpos = (WINDOWPOS *)lParam;
	if (!(lpwndpos->flags & SWP_NOSIZE))
	{
		UpdateLayout();
		bHandled = TRUE;
	}
	return 0;
}

bool                                         
CDOMExplorerWindow::SetBrowserId(TBrowserId browserId)
{
	m_BrowserId = browserId;
	return true;
}

//////////////////////////////////////////////////////////////////////////