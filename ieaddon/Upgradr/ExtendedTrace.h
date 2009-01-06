//////////////////////////////////////////////////////////////////////////////////////
//
// Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
// The source is free, but if you want to use it, mention my name and e-mail address
//
// History:
//    1.0      Initial version                  Zoltan Csizmadia
//
//////////////////////////////////////////////////////////////////////////////////////
//
// ExtendedTrace.h
//

#ifndef EXTENDEDTRACE_H_INCLUDED
#define EXTENDEDTRACE_H_INCLUDED

#if defined(_DEBUG) && defined(WIN32)

#include <windows.h>
#include <tchar.h>

#pragma comment( lib, "imagehlp.lib" )

#if defined(_AFX) || defined(_AFXDLL)
#define TRACE_F									         TRACE
#else
#define TRACE_F									         OutputDebugStringFormat
void OutputDebugStringFormat( LPCTSTR, ... );
#endif

#define TRACE_INITIALIZE( IniSymbolPath )	InitSymInfo( IniSymbolPath )
#define TRACE_UNINITIALIZE()			         UninitSymInfo()
#define TRACE_SRC_EX( Msg, File, Line)       SrcLinkTrace( Msg, File, Line )
#define TRACE_SRC( Msg )                        SrcLinkTrace( Msg, __FILE__, __LINE__ )
#define TRACE_FN()							         FunctionParameterInfo()
#define TRACE_STACK_MSG( Msg )					         StackTrace( Msg )
#define TRACE_STACK()							            StackTrace( GetCurrentThread(), _T("") )
#define TRACE_THREAD_STACK_MSG( hThread, Msg )		   StackTrace( hThread, Msg )
#define TRACE_THREAD_STACK( hThread )				      StackTrace( hThread, _T("") )

BOOL InitSymInfo( PCSTR );
BOOL UninitSymInfo();
void SrcLinkTrace( LPCTSTR, LPCTSTR, ULONG );
void StackTrace( HANDLE, LPCTSTR );
void FunctionParameterInfo();
void FunctionParameterInfo2(LPCTSTR cinfo);

#else

#define TRACE_INITIALIZE( IniSymbolPath )   ((void)0)
#define TRACE_UNINITIALIZE()			         ((void)0)
#define TRACE_F						            ((void)0)
#define TRACE_SRC_EX( Msg, File, Line)	      ((void)0)
#define TRACE_SRC( Msg )						      ((void)0)
#define TRACE_FN()							         ((void)0)
#define TRACE_STACK_MSG( Msg )					         ((void)0)
#define TRACE_STACK()						         	   ((void)0)
#define TRACE_THREAD_STACK_MSG( hThread, Msg )		   ((void)0)
#define TRACE_THREAD_STACK( hThread )				      ((void)0)
#define DECLARE_CLASS_SIGNATURE(name) 

#endif

#endif