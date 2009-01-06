#include "stdafx.h"
#include "ManagedWindow.h"
#include "WorkspaceWindow.h"

HWND
CManagedWindowHandle::GetPaneWorkspace(HWND& topMostBox)
{
	HWND hWnd = GetPane().GetParent();
	HINSTANCE hInstance = GetBaseModule().GetModuleInstance();

	WINDOWINFO info;
	ZeroMemory(&info, sizeof(WINDOWINFO));
	info.cbSize = sizeof(WINDOWINFO);
	topMostBox = NULL;
	HWND level1 = NULL;
	while (hWnd)
	{
		if (!GetWindowInfo(hWnd, &info)) break;;
		if (CWorkspaceWindow::GetWndClassInfo().m_atom==info.atomWindowType) return hWnd;
		topMostBox = level1;
		level1 = hWnd;
		hWnd = GetParent(hWnd);
	}

	return NULL;
}
