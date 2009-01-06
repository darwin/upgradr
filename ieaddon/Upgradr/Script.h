#pragma once

#include "FileSystem.h"

class CStringList : public list<CString> {
public:
};

class CIncludeList : public CStringList {
public:
};

class CExcludeList : public CStringList {
public:
};

class CDisableList : public CStringList {
public:
};

// nikde krome manageru se nesmi pamatovat pointery na skripty !!!
// pres manager mam ohlidany multithreading 
// jinym slovy: do skriptu muze jen vlastnik manageru
class CScript {
public:
	enum ECategory {
		EC_USER,
		EC_SYSTEM,
		EC_ALL
	};

	CScript();
	~CScript();

	// meta data
	CString&                                     Code() { return m_Code; }
	CGuid&                                       Guid() { return m_Guid; }
	CString&                                     Name() { return m_Name; }
	CString&                                     Namespace() { return m_Namespace; }
	CString&                                     Description() { return m_Description; }
	CIncludeList&                                Includes() { return m_Includes; }
	CExcludeList&                                Excludes() { return m_Excludes; }
	CString&                                     Comment() { return m_Comment; }
	CDisableList&                                Disables() { return m_Disables; }
	CScript::ECategory                           Category() const { return m_Category; }
	void                                         Category(CScript::ECategory val) { m_Category = val; }
	CString&                                     FileName() { return m_FileName; }

	bool                                         AddProperty(LPCTSTR name, LPCTSTR value);
	bool                                         GuessGuid();

	bool														SetDirectoryHandle(CDirectoryHandle directoryHandle);

	CString                                      FormTraceName();
	void                                         DetectCategory();


private:
	// meta data
	ECategory                                    m_Category;
	CString                                      m_Name;
	CString                                      m_Namespace;
	CString                                      m_Description;
	CString                                      m_Comment;
	CIncludeList                                 m_Includes;
	CExcludeList                                 m_Excludes;
	CGuid                                        m_Guid;
	CDirectoryHandle										m_DirectoryHandle;
	
	// injected source code
	CString                                      m_Code;

	// for debug purposes
	CString                                      m_FileName;

	// runtime settings
	CDisableList                                 m_Disables; //!< list of disabled sites
};
