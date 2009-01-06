#pragma once

class CDatabase : public CDbSQLite, 
						public CResourceInit<SR_DATABASE> 
{
public:
	CDatabase();
	~CDatabase();

	sqlite3* GetRaw() { return m_db; }

	bool                                           Init();
	bool                                           Done();

	bool                                           CheckDBSchema();

private:
};