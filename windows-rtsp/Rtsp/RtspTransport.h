#ifndef __RTSP_TRANSPORT_H__
#define __RTSP_TRANSPORT_H__

#include "RtspSession.h"
#include "MediaSession.h"

class RtspTransport : public MediaSessionList
{
public:
	RtspTransport();
	
	virtual ~RtspTransport(void);

	virtual BOOL Open(PCSTR bindIp = "127.0.0.1", UINT bindPort = 554, UINT maxConnects = 10);

	virtual BOOL Close();

protected:
	static UINT WINAPI StartListenThread(PVOID param);
	void ListenThread();

	string		m_BindIp;
	UINT		m_BindPort;
	UINT		m_MaxConnects;
	SOCKET		m_Socket;
	SOCKADDR_IN m_BindAddr;

	BOOL		m_isOpen;

	HANDLE		m_hListenThread;
	HANDLE		m_isStopListenThread;

	RtspSessionList		m_RtspSessionList;
};

#endif //__RTSP_TRANSPORT_H__