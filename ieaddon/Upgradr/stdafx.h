// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#define SIDEBYSIDE_COMMONCONTROLS 1
#define ISOLATION_AWARE_ENABLED 1

// Windows 95												WINVER=0x0400	and _WIN32_WINDOWS=0x0400
// Windows 98												WINVER=0x0400	and _WIN32_WINDOWS=0x0410
// Windows Me												WINVER=0x0400	and _WIN32_WINDOWS=0x0490

// Windows NT 4.0											WINVER=0x0400	and _WIN32_WINNT=0x0400
// Windows 2000											WINVER=0x0500	and _WIN32_WINNT=0x0500
// Windows XP												WINVER=0x0501	and _WIN32_WINNT=0x0501 
// Windows Server 2003 family							WINVER=0x0501	and _WIN32_WINNT=0x0502
// Windows Vista											WINVER=0x0600	and _WIN32_WINNT=0x0600

// Target Windows NT 4.0 and Windows 98
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0600	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

// Internet Explorer 3.0, 3.01, 3.02				_WIN32_IE=0x0300 
// Internet Explorer 4.0								_WIN32_IE=0x0400 
// Internet Explorer 4.01								_WIN32_IE=0x0401 
// Internet Explorer 5.0, 5.0a, 5.0b				_WIN32_IE=0x0500 
// Internet Explorer 5.01, 5.5						_WIN32_IE=0x0501 
// Internet Explorer 6.0								_WIN32_IE=0x0560 or _WIN32_IE=0x0600

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
//#define _WTL_NO_CSTRING

#include "resource.h"

#include "atldebug.h"

// ATL
#include <atlbase.h>
#include <atlmem.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlmisc.h>
#include <atlcom.h>
#include <atlsplit.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlctrlw.h>
#include <dispex.h>
#include <atltime.h>
#include <atlframe.h>

#include <atlcrack.h>
#include <atlddx.h>
#include <atlcoll.h>
//#include <atlstr.h> //?

#define END_MSG_MAP_EX	END_MSG_MAP

// Shell
#include <commctrl.h>

#include <shlguid.h>     // IID_IWebBrowser2, DIID_DWebBrowserEvents2, etc.
#include <exdispid.h> // DISPID_DOCUMENTCOMPLETE, etc.
#include <mshtml.h>         // DOM interfaces
#include <shlwapi.h>
#include <olectl.h>
#include <shlobj.h>
#include <exdisp.h>
#include <exdispid.h>
#include <activscp.h>
#include <activdbg.h>

// Remove
#include <string>
#include <stdexcept>
#include <sstream>
#include <cassert>
#include <gdiplus.h>

// STL
#include <hash_map>
#include <hash_set>
#include <functional>
#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <utility>
#include <vector>
#include <string>
#include <strstream>
#include <bitset>

#include <string>
#include <sstream>
#include <fstream>

using namespace ATL;

#define INTRESOURCE(_r) ((WORD)((ULONG_PTR)((_r)) & 0x0000FFFF))

LPWSTR ALLOCINTRESOURCE_A2W(LPCSTR);
void FREEINTRESOURCE(LPCWSTR);

#include "msgcrack.h"

inline CAtlBaseModule& GetBaseModule() { return _AtlBaseModule; } 
inline CAtlWinModule& GetWinModule() { return _AtlWinModule; } 
//inline CAtlDebugInterfacesModule& GetDebugInterfacesModule() { return _AtlDebugInterfacesModule; }

#include <DWAutoHide.h>
#include "atlgdix.h"

//#include <DockingFocus.h>
#include "CustomTabCtrl.h"
#include "DotNetTabCtrl.h"
#include "SimpleTabCtrls.h"
#include "TabbedFrame.h"
#include "ListViewNoFlicker.h"

#define _TABBEDMDI_MESSAGES_EXTERN_REGISTER
#define _TABBEDMDI_MESSAGES_NO_WARN_ATL_MIN_CRT
#include "TabbedMDI.h"

#include <dbstate.h>
#include <DockingFrame.h>
#include <VC7LikeCaption.h>
#include <TabbedDockingWindow.h>

using namespace std;
using namespace stdext;

typedef basic_ios<TCHAR> tios;
typedef basic_streambuf<TCHAR> tstreambuf;
typedef basic_istream<TCHAR> tistream;
typedef basic_ostream<TCHAR> tostream;
typedef basic_iostream<TCHAR> tiostream;
typedef basic_stringbuf<TCHAR> tstringbuf;
typedef basic_istringstream<TCHAR> tistringstream;
typedef basic_ostringstream<TCHAR> tostringstream;
typedef basic_stringstream<TCHAR> tstringstream;
typedef basic_filebuf<TCHAR> tfilebuf;
typedef basic_ifstream<TCHAR> tifstream;
typedef basic_ofstream<TCHAR> tofstream;
typedef basic_fstream<TCHAR> tfstream;

// Debug
#include "tracetool.h"

// crash reporting
#include "bugtrap.h"

// Internal
extern "C" { 
#include "fnv.h" 
};

#include "DebugSupport.h" // DT macro
#include "ListCtrl.h"
#include "Upgradr.h"
#include "Guid.h"
#include "Globals.h"
#include "Utils.h"

#include "Module.h"

#include "DPIHelper.h"
#include "DbSqlite.h"
#include "Registry.h"

#include "wtlhelpers.h"