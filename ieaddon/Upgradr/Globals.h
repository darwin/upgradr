#pragma once

class CUpgradrScriptSite;
class CUpgradrScriptEngine;
class CScript;
class CLogger;
class CConsoleWindow;
class CUpgradrBHO;
class CUpgradrHelperbar;
class CBrowserManager;

typedef unsigned int										TFrameId;
const TFrameId												NULL_FRAME = 0;
typedef unsigned int										TBrowserId;
const TBrowserId											NULL_BROWSER = 0;
typedef LONG									         THandle;
const THandle												NULL_HANDLE = 0;
typedef THandle											TScriptId;
const TScriptId											NULL_SCRIPT = 0;
typedef THandle                                 TScriptInstanceId;
const TScriptInstanceId									NULL_SCRIPT_INSTANCE = 0;
typedef unsigned long TWindowId;
const TWindowId                                 NULL_WINDOW = 0;

#define WM_UPDATEDOM                            ( WM_APP + 100) // custom message sent to dialog to update DOM tree and its title

// messages
const int IDC_TOOLBUTTON                        = 1;
const int IDC_DISABLEBUTTON                     = 2;

#define PRODUCT_ID_TEXT                         _T("44386A7B-7093-4FDC-8B52-5F7E8B968964")

// system script IDs
#define SCRIPT_GUID_XPATH                       _T("{31EF75F2-B105-15BB-0D8A-10CEFA4E}")
#define SCRIPT_GUID_IE2W3C                      _T("{E67E3604-469B-70A4-C9FB-8F5BB964}")
#define SCRIPT_GUID_MONKEYAPI                   _T("{11EF79F1-210A-16A1-0D8A-11CDFDDD}")

#define REGISTRY_ROOT_KEY                       _T("Software\\Upgradr\\")
#define REGISTRY_SETTINGS_KEY                   _T("Settings")
#define REGISTRY_SETTINGS_DATABASEPATH          _T("DatabasePath")
#define REGISTRY_SETTINGS_USERSCRIPTSDIR        _T("UserScriptsDir")
#define REGISTRY_SETTINGS_SYSTEMSCRIPTSDIR      _T("SystemScriptsDir")
#define REGISTRY_SETTINGS_WORKSPACEDIR          _T("WorkspaceDir")
#define REGISTRY_OPTIONS_KEY                    _T("Options")

#define DOMEXPLORERTREE_CLASS_NAME					_T("CUpgradrDOMExplorerTree")
#define DOMEXPLORERTREE_WINDOW_NAME					_T("UpgradrDOMExplorerTreeWindow")

#define DOMEXPLORER_CLASS_NAME                  _T("CUpgradrDOMExplorer")
#define DOMEXPLORER_WINDOW_NAME                 _T("UpgradrDOMExplorerWindow")

#define CONSOLE_WINDOW_NAME							_T("ConsoleWindow")

#define MAIN_TOOLBAR_CLASS_NAME						_T("CUpgradrToolbar")
#define MAIN_TOOLBAR_WINDOW_NAME						_T("UpgradrToolbarWindow")

#define HELPERBAR_WORKSPACE_CLASS_NAME				_T("CUpgradrHelperbarWorkspace")
#define HELPERBAR_WORKSPACE_WINDOW_NAME			_T("UpgradrHelperbarWorkspaceWindow")

#define HELPERBAR_MAIN_CLASS_NAME					_T("CUpgradrHelperbarMain")
#define HELPERBAR_MAIN_WINDOW_NAME					_T("UpgradrHelperbarMainWindow")

#define HELPERBAR_CLASS_NAME							_T("CUpgradrHelperbar")
#define HELPERBAR_WINDOW_NAME							_T("UpgradrHelperbarWindow")

#define DOMTREE_WINDOW_NAME							_T("UpgradrDOMTreeWindow")
#define DOMTREE_CLASS_NAME								_T("CUpgradrDOMTree")

#define CONSOLE_LIST_WINDOW_NAME						_T("UpgradrConsoleListWindow")
#define CONSOLE_LIST_CLASS_NAME						_T("CUpgradrConsoleList")

#define CONSOLE_WINDOW_CLASS_NAME					_T("CUpgradrConsoleWindow")

#define MESSAGE_WINDOW_WINDOW_NAME					_T("UpgradrMessageWindow")
#define MESSAGE_WINDOW_CLASS_NAME					_T("CUpgradrMessageWindow")

#define BROWSER_MESSAGE_WINDOW_NAME					_T("UpgradrBrowserMessageWindow")
#define BROWSER_MESSAGE_WINDOW_CLASS_NAME			_T("CUpgradrBrowserMessageWindow")

#define DISPID_DOCUMENTRELOAD                  	282

#define WAIT_GRANULARITY                       	20 // in ms

// window.UpgradrShell
#define JAVASCRIPT_UPGRADRSHELL_OBJECT    	_T("UpgradrShell")
#define JAVASCRIPT_FRAMEID_VARIABLE    			_T("UpgradrFrameId")
