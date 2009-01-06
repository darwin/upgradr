// stdafx.cpp : source file that includes just the standard includes
// Upgradr.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "DockImpl.cpp"

LPWSTR ALLOCINTRESOURCE_A2W(LPCSTR _r)
{
	if (IS_INTRESOURCE(_r))
		return MAKEINTRESOURCEW(INTRESOURCE(_r));

	if (_r == 0)
		return 0;

	int len = ::lstrlenA(_r);
	LPWSTR r = new WCHAR[len + 1];
	::lstrcpyn(r, CA2W(_r), len + 1);

	return r;
}

void FREEINTRESOURCE(LPCWSTR _r)
{
	if (IS_INTRESOURCE(_r))
		return ;

	if (_r == 0)
		return ;

	delete [] _r;
}
