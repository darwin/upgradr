#include "StdAfx.h"
#include "Server.h"

//#include "Debug.h"

CServer::CServer():
INIT_TRACE(CServer)
{
	DTI(TRACE_LI(FS(_T("Constructor()"))));
}

CServer::~CServer()
{
	DTI(TRACE_LI(FS(_T("Destructor()"))));
}

bool 
CServer::Init()
{
	CHECK_THREAD_OWNERSHIP;
	if (m_Inited) return true;

	DTI(TRACE_LI(_T("Init()")));
	m_Inited = true;
	return true;
}

bool 
CServer::Done()
{
	CHECK_THREAD_OWNERSHIP;
	if (!m_Inited) return true;
	DTI(TRACE_LI(_T("Done()")));
	m_Inited = false;
	return true;
}

bool														
CServer::Start()
{
	CHECK_THREAD_OWNERSHIP;
	DTI(TRACE_LI(_T("Start()")));

	return true;
}

bool														
CServer::Stop()
{
	CHECK_THREAD_OWNERSHIP;
	DTI(TRACE_LI(_T("Stop()")));

	return true;
}
