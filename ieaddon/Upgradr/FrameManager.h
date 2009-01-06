#pragma once

class CFrameManager : public CResourceInit<SR_FRAMEMANAGER> {
public:
	CFrameManager();
	~CFrameManager();

	TFrameId                                       GenerateFrameId();

protected:
	TFrameId                                       m_NextId;
};
