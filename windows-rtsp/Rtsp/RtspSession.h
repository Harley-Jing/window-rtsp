#ifndef __RTSP_SESSION_H__
#define __RTSP_SESSION_H__

#include "MediaStreamTransport.h"
#include "RtspResponse.h"
#include "Rtp.h"

class RtspSession
{
public:
	RtspSession(RtspResponse* rtspResponse, MediaSessionList* mediaSessionList);
	
	RtspSession(SOCKET s, SOCKADDR_IN bindAddr, SOCKADDR_IN connectAddr, MediaSessionList* mediaSessionList);

	virtual ~RtspSession();

	virtual BOOL RtspSessionIsClose();

protected:
	static UINT WINAPI StartRtspSessionThread(PVOID pParam);
	void RtspSessionThread();

	void ResponseGetParameter();
	void ResponseOptions();
	void ResponseDescribe();
	void ResponseSetup();
	void ResponsePlay();
	void ResponsePause();
	void ResponseTeardown();

	BOOL TransportDescribe(string* describe);
	BOOL TransportSetup(string* pBindIp, INT* pBindPort, 
						string* pTargetIp, INT* pTargetPort, 
						INT32* pSsrc);
	BOOL TransportPlay();
	BOOL TransportPause();
	BOOL TransportTeardown();

	BOOL StreamSetup(MediaStream* pMediaStream, string* pBindIp, INT* pBindPort, string* pTargetIp, INT* pTargetPort);

	LONGLONG GenerateOneNumber();
	void GetServerIp(PSTR ip);

	HANDLE m_hProcessThread;
	HANDLE m_isStopProcessThread;

	RtspResponse*		m_pRtspResponse;
	MediaSessionList*	m_pMediaSessionList;

	string	m_SessionName;
	string	m_SetupUrl;
};

typedef vector<RtspSession*> RtspSessionList;

#endif //__RTSP_SESSION_H__