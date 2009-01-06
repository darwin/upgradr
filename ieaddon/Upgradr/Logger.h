#pragma once

class CLogMessage {
public:
	CLogMessage()
	{
	}

	CLogMessage(LARGE_INTEGER time, CString message, CString script, CString frameId, CString url)
	{
		m_Time = time;
		m_Message = message;
		m_FrameId = frameId;
		m_URL = url;
		m_Script = script;
	}

	LARGE_INTEGER                                  m_Time;
	CString                                        m_Message;
	CString                                        m_FrameId;
	CString                                        m_URL;
	CString                                        m_Script;
};

//////////////////////////////////////////////////////////////////////////

class CLogger : public CResourceInit<SR_LOGGER> {
public:
	bool                                           AddMessage(LARGE_INTEGER time, CString message, CString script, CString frameId, CString url);

	CListArray<CLogMessage>                        m_Messages;
};

//////////////////////////////////////////////////////////////////////////

class CLoggerConsole : public CListImpl<CLoggerConsole> {
public:
	DECLARE_WND_CLASS(CONSOLE_LIST_CLASS_NAME)

	enum UserColumns 
	{
		E_TIME, 
		E_MESSAGE, 
		E_SCRIPT, 
		E_FRAMEID, 
		E_URL,
		E_LAST
	};

	class CompareItem {
	public:
		CompareItem(UserColumns colColumn) : m_Columns(colColumn) {}
		inline bool operator() (const CLogMessage& logMessage1, const CLogMessage& logMessage2)
		{
			switch (m_Columns) {
				case E_TIME:	   return ( logMessage1.m_Time.QuadPart < logMessage2.m_Time.QuadPart );
				case E_MESSAGE:	return ( logMessage1.m_Message.Compare(logMessage2.m_Message) < 0);
				case E_FRAMEID:	return ( logMessage1.m_FrameId.Compare(logMessage2.m_FrameId) < 0);
				case E_URL:	      return ( logMessage1.m_URL.Compare(logMessage2.m_URL) < 0);
				case E_SCRIPT:	   return ( logMessage1.m_Script.Compare(logMessage2.m_Script) < 0);
			}
			return false;
		}

	protected:
		UserColumns m_Columns;
	};

	CLoggerConsole();

	BOOL                                           Initialise();
	int                                            GetItemCount(); // required by CListImpl
	BOOL                                           GetLogMessage(int nItem, CLogMessage& logMessage);
	CString                                        GetItemText(int nItem, int nSubItem); // required by CListImpl
	int                                            GetItemImage(int nItem, int nSubItem); // overrides CListImpl::GetItemImage
	void                                           ReverseItems(); // overrides CListImpl::ReverseItems
	void                                           SortItems(int nColumn, BOOL bAscending); // overrides CListImpl::SortItems

protected:
	LARGE_INTEGER                                  m_StartTime;
};

//////////////////////////////////////////////////////////////////////////
