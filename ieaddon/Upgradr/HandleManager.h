#pragma once

// singleton: preklada GUIDy na HANDLEs
// handle je u32 pouzivany v ramci behu aplikace

typedef hash_map<CGuid, THandle, CGuidHasher> TGUIDHandleMap;
typedef hash_map<THandle, CGuid> THandleGUIDMap;

class CHandleManager : public TGUIDHandleMap, public CResourceInit<SR_HANDLEMANAGER> {
public:
	typedef stack<THandle> THandleStack;

	CHandleManager();
	~CHandleManager();

	THandle                                        GetHandle(CGuid guid);
	bool                                           FreeHandle(CGuid guid);
	CGuid                                          LookupGUID(THandle handle) const;

private:
	THandleStack                                   m_FreeHandles;
	THandle                                        m_LastHandle;
	THandleGUIDMap                                 m_ReverseMap;
};
