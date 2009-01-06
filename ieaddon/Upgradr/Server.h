#pragma once

// CServer implements interface to embedded web server
// we use Simple HTTPD server
class CServer : public CResourceInit<SR_SERVER>,
					 public WinTrace
{
public:
	CServer();
	~CServer();

	bool														Init();
	bool														Done();

	bool														Start();
	bool														Stop();

private:
};