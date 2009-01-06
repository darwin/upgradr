#include "StdAfx.h"
#include "FileSystem.h"
#include "ScriptManager.h"
#include "unzip.h"

//#include "Debug.h"

#define META_FILE_NAME									_T("META.yaml")
#define USERSCRIPT_TOKEN "==UserScript=="
#define MAX_FILE_PATH									(64*1024)  // 64kb UNICODE filename should be enough for everyone (japanese freaks including)

#define CU2P(x) ((const PHYSFS_uint16*)(x))
#define CU2C(x) ((const char*)(x))

//////////////////////////////////////////////////////////////////////////

CPath::CPath(TCHAR* path)
{
	ATLASSERT(sizeof(TCHAR)==2);
	ATLASSERT(sizeof(Fnv32_t)==4);

	size_t len = _tcslen(path);
	m_Data = new TCHAR[len+3];
	_tcscpy_s(m_Data+2, len+1, path);
	*(Fnv32_t*)m_Data = fnv_32_buf(path, 2*len, FNV1_32_INIT);
}

CPath::~CPath()
{
	delete[] m_Data;
}

size_t 
CPath::GetHash() const
{
	ATLASSERT(sizeof(size_t)==4);
	return *(Fnv32_t*)m_Data;
}

const TCHAR* 
CPath::GetPath() const
{
	return m_Data+2;
}

CPath::operator const TCHAR* () const
{
	return GetPath();
}


//////////////////////////////////////////////////////////////////////////

CDirectory::CDirectory(CPathSharedPtr path, EType type):
m_Path(path),
m_Type(type)
{

}

CDirectory::~CDirectory()
{

}

size_t 
CDirectory::GetHash() const
{
	return m_Path->GetHash();
}

//////////////////////////////////////////////////////////////////////////

CDirectoryList::CDirectoryList()
{

}

CDirectoryList::~CDirectoryList()
{
	iterator i = begin();
	while (i!=end())
	{
		delete *i;
		++i;
	}
	clear();
}

bool														
CDirectoryList::Remove(CDirectory* directory)
{
	iterator lookup = find(directory);
	if (lookup==end()) return false;
	delete *lookup;
	erase(lookup);
	return true;
};

CDirectoryHandle										
CDirectoryList::Find(CPathSharedPtr path)
{
	CDirectory directory(path, CDirectory::E_UNKNOWN);
	iterator i = find(&directory);
	if (i==end()) return NULL_DIRECTORY;
	return *i;
}

CDirectoryHandle										
CDirectoryList::Add(CPathSharedPtr path, CDirectory::EType type)
{
	CDirectoryHandle directory = Find(path);
	if (directory!=NULL_DIRECTORY) return directory;

	directory = new CDirectory(path, type);
	insert(directory);
	return directory;
}

//////////////////////////////////////////////////////////////////////////

CFileSystem::CFileSystem():
INIT_TRACE(CFileSystem)
{
	DTI(TRACE_LI(FS(_T("Constructor()"))));
}

CFileSystem::~CFileSystem()
{
	DTI(TRACE_LI(FS(_T("Destructor()"))));
	ATLASSERT(m_Directories.size()==0);
}

bool 
CFileSystem::Init()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_Inited) return true;
	DTI(TRACE_LI(_T("Init()")));

	TCHAR path[MAX_PATH];
	DWORD size;

	// load system scripts
	size = MAX_PATH;
	if (GetStringValueFromRegistry(REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_SYSTEMSCRIPTSDIR, path, &size))
	{
		TRACE_E(FS(_T("Cannot read system scripts location from HKCU\\%s%s\\%s"), REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_SYSTEMSCRIPTSDIR));
		// FATAL
		return false;
	}
	LoadFromDirectory(path);
	
	// mark scripts as system scripts
	{
		ScriptManagerLock scriptManager;
		TScriptContainer::iterator s = scriptManager->begin();
		while (s!=scriptManager->end())
		{
			s->second->Category(CScript::EC_SYSTEM);
			++s;
		}
	}

	// load user scripts
	size = MAX_PATH;
	if (GetStringValueFromRegistry(REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_USERSCRIPTSDIR, path, &size))
	{
		TRACE_E(FS(_T("Cannot read user scripts location from HKCU\\%s%s\\%s"), REGISTRY_ROOT_KEY, REGISTRY_SETTINGS_KEY, REGISTRY_SETTINGS_USERSCRIPTSDIR));
		// FATAL
		return false;
	}
	LoadFromDirectory(path);

	m_Inited = true;
	return true;
}

bool 
CFileSystem::Done()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return true;
	DTI(TRACE_LI(_T("Done()")));
	m_Directories.clear();
	m_Inited = false;
	return true;
}

bool                                           
CFileSystem::LoadFromDirectory(LPCTSTR path)
{
	DTI(TRACE_LI(FS(_T("LoadFromDirectory(path='%s')"), path)));
	CHECK_THREAD_OWNERSHIP;

	TCHAR workingBuffer[MAX_FILE_PATH];
	_tcscpy_s(workingBuffer, path);

	// recursively scan directory
	return ScanDirectory(workingBuffer);
}

bool
CFileSystem::ScanDirectory(LPTSTR path)
{
	DTI(TRACE_LI(FS(_T("ScanDirectory(path='%s')"), path)));
	CHECK_THREAD_OWNERSHIP;
	WIN32_FIND_DATA kFindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError;

	TCHAR* separator = path+_tcslen(path);
	_tcscat_s(path, MAX_FILE_PATH, _T("\\*"));

	hFind = FindFirstFile(path, &kFindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		TRACE_E(FS(_T("Invalid file handle. Error is %u"), GetLastError()));
		return false;
	} 

	// strip filespec part from the buffer to be reused
	*separator = 0;
	ProcessFileData(path, &kFindFileData);
	while (FindNextFile(hFind, &kFindFileData) != 0) 
	{
		// strip filespec part from the buffer to be reused
		*separator = 0;
		ProcessFileData(path, &kFindFileData);
	}

	dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		TRACE_E(FS(_T("FindNextFile error. Error is %u\n"), dwError));
		return false;
	}
	return true;
}

bool 
TestExtension(LPCTSTR file, LPCTSTR ext)
{
	const TCHAR* last = _tcsrchr(file, '.');
	if (!last) return false;
	if (_tcslen(last+1)!=_tcslen(ext)) return false;
	const TCHAR* f = last+1;
	const TCHAR* e = ext;
	while (*f)
	{
		if (_totlower(*f)!=_totlower(*e)) return false;
		f++;
		e++;
	}
	return true;
}

bool 
CFileSystem::ProcessFileData(LPTSTR path, WIN32_FIND_DATA* pFindFileData)
{
	DTI(TRACE_LI(FS(_T("ProcessFileData(path='%s', file='%s')"), path, pFindFileData->cFileName)));
	CHECK_THREAD_OWNERSHIP;
	// first, reject files we don't want at all
	if (pFindFileData->dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_TEMPORARY|FILE_ATTRIBUTE_OFFLINE))
		return false;

	// skip empty, '.' and '..' entries
	if (!pFindFileData->cFileName[0] || 
		(pFindFileData->cFileName[0]==_T('.') && !pFindFileData->cFileName[1]) ||
		(pFindFileData->cFileName[0]==_T('.') && pFindFileData->cFileName[1]==_T('.') && !pFindFileData->cFileName[2]))
		return false;

	// ignore anything which starts with underscore
	if (pFindFileData->cFileName[0]=='_') 
	{
		DT(TRACE_LI(FS(_T(" ignoring '%s' because of underscore"), pFindFileData->cFileName)));
		return false;
	}

	// prepare new fileName (the buffer was provided by ScanDirectory)
	TCHAR* separator = path+_tcslen(path);
	_tcscat_s(path, MAX_FILE_PATH, FS(_T("\\%s"), pFindFileData->cFileName));

	// is it directory ?
	if (pFindFileData->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
	{
		// traverse recursively into directory
		return ScanDirectory(path);
	}

	// we have a sigle file here
	
	// detect if it is archive
	if (TestExtension(path, _T("upg")) || TestExtension(path, _T("zip")))
	{
		return ProcessArchive(path);
	}

	// detect if it is standalone script
	if (TestExtension(path, _T("js")))
	{
		return ProcessFile(path);
	}

	return false;
}

CFileSystem::EUpgradeType
CFileSystem::DetectScriptType(char* block, size_t len, CFileSystem::EExpectedFormat format)
{
	CHECK_THREAD_OWNERSHIP;

	// taken from subversion

	// Right now, this function is going to be really stupid.  It's
	// going to examine the first block of data, and make sure that 85%
	// of the bytes are such that their value is in the ranges 0x07-0x0D
	// or 0x20-0x7F, and that 100% of those bytes is not 0x00.

	// If those criteria are not met, we're calling it binary.
	if (len > 0)
	{
		size_t binary_count = 0;

		// Run through the data we've read, counting the 'binary-ish'
		// bytes.  HINT: If we see a 0x00 byte, we'll set our count to its
		// max and stop reading the file.
		for (size_t i = 0; i < len; i++)
		{
			if (block[i] == 0)
			{
				binary_count = len;
				break;
			}
			if ((block[i] < 0x07) || 
				((block[i] > 0x0D) && (block[i] < 0x20)) || 
				(block[i] > 0x7F))
			{
				binary_count++;
			}
		}

		if (((binary_count * 1000) / len) > 850)
		{
			// binary file
			return UT_UNKNOWN;
		}
	}

	// well we have a text file here

	if (format==EEF_GREASEMONKEY)
	{
		// test for GM script
		// try to find "==UserScript=="
		// TODO: unicode test ?
		if (strstr(block, USERSCRIPT_TOKEN))
		{
			return UT_GREASEMONKEY;
		}
	}

	if (format==EEF_YAML)
	{
		return UT_META;
	}

	// unknown type
	return UT_UNKNOWN;
}

// caller is responsible for deleting the string
LPTSTR ExtractDirectory(LPCTSTR path)
{
	const TCHAR* lastSlash = _tcsrchr(path, '\\');
	ATLASSERT(lastSlash); // it must be there!

	size_t copyLen = lastSlash-path;
	TCHAR* copy = new TCHAR[copyLen+1];
	_tcsncpy_s(copy, copyLen+1, path, copyLen);

	return copy;
}

bool														
CFileSystem::ProcessArchive(LPCTSTR path)
{

	return true;
}

bool														
CFileSystem::ProcessFile(LPCTSTR path)
{	
	ATLASSERT(path);

	// read first 1024 byte as binary file
	char block[1024];
	FILE* file;
	_tfopen_s(&file, path, _T("rb"));
	if (!file) return false;
	size_t amt_read = fread(block, 1, 1024, file);
	fclose(file);

	// detect script type
	EUpgradeType type = DetectScriptType(block, amt_read, EEF_GREASEMONKEY);
	if (type==UT_UNKNOWN) return true;

	if (type==UT_GREASEMONKEY)
	{
		// well, open file in text mode
		tifstream stream(path);
		if (!stream) return false;
		CScript* script = ParsePlainScript(stream);
		if (!script)
		{
			// something went wrong
			TRACE_W(FS(_T("Cannot parse script '%s'"), path));
			return false;
		}

		script->FileName() = path;
		script->Category(CScript::EC_USER);

		// extract directory from filename
		LPTSTR directoryPath = ExtractDirectory(path);
		CPathSharedPtr dirPath(new CPath(directoryPath));
		CDirectoryHandle directory = AddDirectory(dirPath, CDirectory::E_FILESYSTEM);
		delete[] directoryPath;

		script->SetDirectoryHandle(directory);

		ScriptManagerLock scriptManager;
		scriptManager->AddScript(script);
	}

	return true;
}

#define INITIAL_COMMENT_BUFFER_COUNT (64*1024) // 64kb
#define INITIAL_CODE_BUFFER_COUNT (100*1024) // 100kb
#define PROPNAME_BUFFER_COUNT (1*1024) // 1kb
#define PROPVALUE_BUFFER_COUNT (4*1024) // 4kb

CScript*                                       
CFileSystem::ParsePlainScript(tistream& file)
{
	CHECK_THREAD_OWNERSHIP;

	// 0 = begin, 1 = userscript section, 2 = code
	int state = 0; 
	size_t lineCounter = 0;
	CScript* pScript = new CScript();

	pScript->Code().GetBuffer(INITIAL_CODE_BUFFER_COUNT);
	pScript->Code().ReleaseBuffer(0);

	pScript->Comment().GetBuffer(INITIAL_COMMENT_BUFFER_COUNT);
	pScript->Comment().ReleaseBuffer(0);

	TRACE_I(_T("Code"));
	TTrace::Debug()->Indent();
	while (!file.eof() && file.good())
	{
		TCHAR line[64*1024];
		lineCounter++;

		//file >> ws; // skip whitespace
		if (file.eof()) break;
		file.getline(line, 64*1024);

		TRACE_I(line);

		// gather code
		if (pScript->Code().GetLength()) pScript->Code()+=_T('\n');
		pScript->Code() += line;

		TCHAR* sws = line;
		while (*sws && *sws==_T(' ') || *sws==_T('\t')) sws++;
		TCHAR* linews = sws;

		if (state!=2 && linews[0]==0) continue; // skip empty lines
		if (linews[0]==_T('/') && linews[1]==_T('/'))
		{
			// we have javascript comment here
			if (_tcsstr(sws, _T(USERSCRIPT_TOKEN)))
			{
				state = 1;
				continue;
			}
		}
		else
		{
			// js code here
			if (!state) 
			{
				delete pScript;
				return NULL;
			}
			state = 2;
		}

		switch (state) {
		case 0:
			if (pScript->Comment().GetLength()) pScript->Comment()+=_T('\n');
			pScript->Comment()+=line;
			break;
		case 1:
			// look for userscript parameters
			{
				TCHAR* pos = linews+2;
				while (*pos && *pos==_T(' ') || *pos==_T('\t')) pos++;
				if (*pos)
				{
					if (*pos==_T('@'))
					{
						pos++;

						// read property name and value
						TCHAR propName[PROPNAME_BUFFER_COUNT];
						TCHAR* propNamePos = propName;
						while (*pos && !(*pos==_T(' ') || *pos==_T('\t'))) 
						{
							*propNamePos = *pos;
							propNamePos++;
							if (propNamePos==propName+PROPNAME_BUFFER_COUNT-1)
							{
								TRACE_E(FS(_T("PropName greater than %d characters at line %d -- skipping script"), PROPNAME_BUFFER_COUNT, lineCounter));
								delete pScript;
								return NULL;
							}
							pos++;
						}
						*propNamePos = 0;

						// skip whitespace
						while (*pos && *pos==_T(' ') || *pos==_T('\t')) pos++;

						TCHAR propValue[PROPVALUE_BUFFER_COUNT];
						TCHAR* propValuePos = propValue;
						while (*pos) 
						{
							*propValuePos = *pos;
							propValuePos++;
							if (propValuePos==propValue+PROPVALUE_BUFFER_COUNT-1)
							{
								TRACE_E(FS(_T("PropValue greater than %d characters at line %d -- skipping script"), PROPVALUE_BUFFER_COUNT, lineCounter));
								delete pScript;
								return NULL;
							}
							pos++;
						}
						*propValuePos = 0;

						pScript->AddProperty(propName, propValue);
					}
				}
			}
			break;
		case 2:
			// code is taken always
			break;
		}
	}

	TTrace::Debug()->UnIndent();

	// compact string buffers
	pScript->Code().FreeExtra();
	pScript->Comment().FreeExtra();

	// prepare guid if there is no specified
	if (pScript->Guid()==CGuid::m_Null)
	{
		pScript->GuessGuid();
	}
	return pScript;
}

CDirectoryHandle												
CFileSystem::FindDirectory(CPathSharedPtr path)
{
	return m_Directories.Find(path);
}

CDirectoryHandle
CFileSystem::AddDirectory(CPathSharedPtr path, CDirectory::EType type)
{
	return m_Directories.Add(path, type);
}

bool														
CFileSystem::RemoveDirectory(CDirectoryHandle directory)
{
	return m_Directories.Remove(directory);
}