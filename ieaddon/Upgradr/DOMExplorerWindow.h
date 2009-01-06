#pragma once
#include "TreeListView.h"

typedef CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE> TDOMExplorerWinTraits;

#define IDC_DOMEXPLORER_TREE							1

class CImportantElementAttributes {
public:
	CComQIPtr<IHTMLDOMAttribute>						m_Id;
	CComQIPtr<IHTMLDOMAttribute>						m_Class;
};

//////////////////////////////////////////////////////////////////////////

// treeview class 
class CDOMTree : public CContainedWindowT<CTreeViewCtrl> {
public:
	DECLARE_WND_CLASS(DOMTREE_CLASS_NAME)

	CDOMTree(CMessageMap* pObject, DWORD dwMsgMapID = 0) : CContainedWindowT<CTreeViewCtrl>(pObject, dwMsgMapID) {}

	// CDOMTree message handlers
	LRESULT													OnTreeDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT													OnExpandAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT													OnCollapseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT													OnTreeItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT													OnTreeItemExpanded(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT													OnTreeGetDispInfo(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT													OnTreeContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT													OnTreeItemDeleted(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	// CDOMTree helper methods
	LRESULT													FindImportantAttributes(CComQIPtr<IHTMLDOMNode> spNode, CImportantElementAttributes& res);

	// type of the CDOMTree method that will be called recursively for each node and all of its children
	typedef void (CDOMTree::*FOREACHITEM)(HTREEITEM);

	// flags for calling method on parent node and all of its children
	enum
	{
		// call method on the parent node first, then on its children
		// othwerwise, method is called on children first
		DOPARENTFIRST = 1,
		// don't call a method on topomost parent
		SKIPTOPPARENT = 2
	};

	void														UpdateDOM(IStream* pStream); 
	void														SetupImageList(bool bSet = true); 
	IHTMLDocument2*										GetDocPtr(void) { return (IHTMLDocument2*)m_Document; }

private:
	bool														PrepareDOMTree(void);
	HTREEITEM												InsertDOMNode(IHTMLDOMNode* pINode, HTREEITEM hparent, HTREEITEM hinsertAfter);
	void														InsertDOMNodeChildren(IHTMLDOMNode* pINode, HTREEITEM hparent);
	HTREEITEM												InsertAttributeCollection(IHTMLAttributeCollection* pIAttrColl, HTREEITEM hparent);
	void														InsertAttributes(IHTMLAttributeCollection* pIAttrColl, HTREEITEM hparent);
	HTREEITEM												InsertAttribute(IHTMLDOMAttribute* pIAttribute, HTREEITEM hparent, HTREEITEM hinsertAfter);
	bool														IsAnyAttributeSet(IHTMLAttributeCollection* pIAttrColl); 

	// recursive procedure that will call fp for the item and for all of its children
	void														DoForItemAndChildren(HTREEITEM hParent, FOREACHITEM fp, unsigned int flags = 0);

	// ExpandItem method wih FOREACHITEM signature so it can be parameter to DoForItemAndChildren
	void														ExpandItem(HTREEITEM hItem);
	void														ExpandAll(void);
	void														CollapseAll(void);
	void														FormatNameAndValue(const CComBSTR& name, const CComVariant& value, CImportantElementAttributes& attributes, TCHAR* pTextResult);

	// members
	CImageList												m_ImageList;
	CComPtr<IHTMLDocument2>								m_Document;
}; // class CDOMTree

//////////////////////////////////////////////////////////////////////////

class CDOMExplorerTreeWindow : public CTreeListViewImpl<CDOMExplorerTreeWindow> {
	typedef CTreeListViewImpl<CDOMExplorerTreeWindow> baseClass;
public:
	CDOMExplorerTreeWindow();
	virtual ~CDOMExplorerTreeWindow();

	DECLARE_WND_CLASS(DOMEXPLORERTREE_CLASS_NAME);

	BEGIN_MSG_MAP(CDOMExplorerTreeWindow)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(1) // Tree
		CHAIN_MSG_MAP_ALT(baseClass, 1)
	ALT_MSG_MAP(2) // Header
		CHAIN_MSG_MAP_ALT(baseClass, 2)
	END_MSG_MAP()

	void														SetupImageList(bool bSet);

	CImageList												m_ImageList;
};

//////////////////////////////////////////////////////////////////////////

class CDOMExplorerWindow : public CWindowImpl<CDOMExplorerWindow, CWindow, TDOMExplorerWinTraits> {
public:
	CDOMExplorerWindow();
	virtual ~CDOMExplorerWindow();

	DECLARE_WND_CLASS(DOMEXPLORER_CLASS_NAME)

	BEGIN_MSG_MAP(CDOMExplorerWindow)
		//NOTIFY_HANDLER(IDC_DOMEXPLORER_TREE, TVN_GETDISPINFO, m_DOMTree.OnTreeGetDispInfo)
		//NOTIFY_HANDLER(IDC_DOMEXPLORER_TREE, TVN_ITEMEXPANDING, m_DOMTree.OnTreeItemExpanding) 
		//NOTIFY_HANDLER(IDC_DOMEXPLORER_TREE, TVN_ITEMEXPANDED, m_DOMTree.OnTreeItemExpanded) 
		//NOTIFY_HANDLER(IDC_DOMEXPLORER_TREE, TVN_DELETEITEM, m_DOMTree.OnTreeItemDeleted) 
		//MESSAGE_HANDLER(WM_CONTEXTMENU, m_DOMTree.OnTreeContextMenu)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(WM_UPDATEDOM, OnUpdateDOM)

		//ALT_MSG_MAP(1)   // Handlers for the subclassed tree view control.
		//MESSAGE_HANDLER(WM_DESTROY, m_DOMTree.OnTreeDestroy)
		//COMMAND_ID_HANDLER(IDM_EXPANDALL, m_DOMTree.OnExpandAll)
		//COMMAND_ID_HANDLER(IDM_COLLAPSEALL, m_DOMTree.OnCollapseAll)
	END_MSG_MAP()

	virtual LRESULT										OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT										OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT										OnUpdateDOM(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	bool                                         SetBrowserId(TBrowserId browserId);
	bool                                         RequestDOM();

protected:
	virtual void											UpdateLayout();

	// members
private:
	TCHAR														m_Title[MAX_PATH];
	CDOMExplorerTreeWindow								m_TreeWindow;
	TBrowserId                                   m_BrowserId;
};