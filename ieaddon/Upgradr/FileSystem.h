#pragma once
#include "boost/shared_ptr.hpp"
#include "RootSettings.h"

class CPath {
public:
	CPath(TCHAR* path);
	~CPath();
	operator const TCHAR* () const;

	size_t													GetHash() const; 
	const TCHAR*											GetPath() const;

private:
	TCHAR*													m_Data;   ///< layout of this memory: [HASH32][wide char null terminated string]
};

typedef boost::shared_ptr<CPath>						CPathSharedPtr;

class CDirectory {
public:
	enum EType {
		E_NONE,
		E_FILESYSTEM,
		E_ZIP,
		E_UNKNOWN
	};

	CDirectory(CPathSharedPtr path, EType type);
	~CDirectory();

	size_t													GetHash() const;
	
private:
	EType														m_Type;
	CPathSharedPtr											m_Path;

	// cache
};

typedef CDirectory*										CDirectoryHandle;
const CDirectoryHandle									NULL_DIRECTORY = 0;

class CDirectoryListHasher : public hash_compare<CDirectoryHandle> {
public:
	size_t operator() (const CDirectoryHandle& key) const { return key->GetHash(); }
	bool operator() (const CDirectoryHandle& left, const CDirectoryHandle& right) const { return left->GetHash()<right->GetHash(); }
};

class CDirectoryList : public hash_set<CDirectoryHandle, CDirectoryListHasher> {
public:
	CDirectoryList();
	~CDirectoryList();

	CDirectoryHandle										Find(CPathSharedPtr path);
	CDirectoryHandle										Add(CPathSharedPtr path, CDirectory::EType type);
	bool														Remove(CDirectoryHandle directory);
};

// CFileSystem implements filesystem abstraction
class CFileSystem : public CResourceInit<SR_FILESYSTEM>,
						  public WinTrace
{
public:
	enum EExpectedFormat {
		EEF_GREASEMONKEY,
		EEF_YAML
	};

	enum EUpgradeType {
		UT_GREASEMONKEY,
		UT_PLAIN,
		UT_META,
		UT_META_ZIP,
		UT_UNKNOWN
	};

	CFileSystem();
	~CFileSystem();

	bool														Init();
	bool														Done();

	CDirectoryHandle										FindDirectory(CPathSharedPtr path);
	CDirectoryHandle										AddDirectory(CPathSharedPtr path, CDirectory::EType type);
	bool														RemoveDirectory(CDirectoryHandle directory);

private:
	bool														SetupMountPoints();

	bool														LoadFromDirectory(LPCTSTR path);

	bool 														ProcessFileData(LPTSTR path, WIN32_FIND_DATA* pFindFileData);
	bool														ScanDirectory(LPTSTR path);
	EUpgradeType											DetectScriptType(char* block, size_t len, CFileSystem::EExpectedFormat format);
	bool														ProcessArchive(LPCTSTR path);
	bool														ProcessFile(LPCTSTR path);
	CScript*                                     ParsePlainScript(tistream& file);

	CDirectoryList											m_Directories;
};
