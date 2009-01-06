#pragma once

#define DT(x) 
#define DTI(x) 

#if defined(_DEBUG) && defined(WIN32)

#include "StackWalker.h"
#include "ExtendedTrace.h"

extern StackWalker sw;

#define DECLARE_CLASS_SIGNATURE(name) inline CString GetClassSignature() const { CString s; s.Format(_T("%s [%08X]"), _T(#name), this); return s; }
#define TRACE_MFN()							         //sw.ShowCallstack();

#else

#define DECLARE_CLASS_SIGNATURE(name) 

#endif
