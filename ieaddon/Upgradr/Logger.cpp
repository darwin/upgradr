#include "StdAfx.h"
#include "Logger.h"

CLoggerConsole::CLoggerConsole()
{
	QueryPerformanceCounter(&m_StartTime);
}

BOOL 
CLoggerConsole::Initialise()
{
	CListImpl<CLoggerConsole>::Initialise();

	AddColumn(_T("Time"), 100);
	AddColumn(_T("Message"), 500);
	AddColumn(_T("Script"), 100);
	AddColumn(_T("Frame ID"), 50);
	AddColumn(_T("Source"), 400);

	return TRUE;
}

int 
CLoggerConsole::GetItemCount() // required by CListImpl
{
	LoggerLock logger;
	return logger->m_Messages.GetSize();
}

BOOL 
CLoggerConsole::GetLogMessage(int nItem, CLogMessage& logMessage)
{
	if (nItem < 0 || nItem >= GetItemCount()) return FALSE;
	LoggerLock logger;
	logMessage = logger->m_Messages[nItem];
	return TRUE;
}

CString 
CLoggerConsole::GetItemText(int nItem, int nSubItem) // required by CListImpl
{
	CLogMessage logMessage;
	if (!GetLogMessage(nItem, logMessage))	return _T("");
	switch ((UserColumns)nSubItem) {
		case E_TIME:	   
		{
			CString s;
			LARGE_INTEGER& t = logMessage.m_Time;
			LONGLONG relative = t.QuadPart - m_StartTime.QuadPart;
			s.Format(_T("%06d.%03d"), (int)(relative / 1000000000), (int)((relative / 1000000) % 1000));
			return s;
		}
		case E_MESSAGE:	return logMessage.m_Message;
		case E_FRAMEID:	return logMessage.m_FrameId;
		case E_URL:	      return logMessage.m_URL;
		case E_SCRIPT:	   return logMessage.m_Script;
	}
	return _T("");
}

int 
CLoggerConsole::GetItemImage(int nItem, int nSubItem) // overrides CListImpl::GetItemImage
{
	if ((UserColumns)nSubItem != E_TIME) return -1;
	return 0;
}

void 
CLoggerConsole::SortItems(int nColumn, BOOL bAscending) // overrides CListImpl::SortItems
{
	LoggerLock logger;
	logger->m_Messages.Sort(CompareItem((UserColumns)nColumn));
}

void 
CLoggerConsole::ReverseItems() // overrides CListImpl::ReverseItems
{
	LoggerLock logger;
	logger->m_Messages.Reverse();
}

//////////////////////////////////////////////////////////////////////////

bool                                           
CLogger::AddMessage(LARGE_INTEGER time, CString message, CString script, CString frameId, CString url)
{
	CHECK_THREAD_OWNERSHIP;
	return m_Messages.Add(CLogMessage(time, message, script, frameId, url))?true:false;
}
