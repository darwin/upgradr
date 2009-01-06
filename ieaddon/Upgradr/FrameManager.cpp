#include "StdAfx.h"
#include "FrameManager.h"

CFrameManager::CFrameManager():
m_NextId(0)
{
}

CFrameManager::~CFrameManager()
{
}

TFrameId 
CFrameManager::GenerateFrameId()
{
	CHECK_THREAD_OWNERSHIP;
	return ++m_NextId;	
}