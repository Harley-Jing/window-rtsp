
#include "stdafx.h"
#include "RtspSession.h"

#include <process.h>

RtspSession::RtspSession(RtspResponse* rtspResponse, MediaSessionList* mediaSessionList)
{
	m_pRtspResponse = rtspResponse;
	m_pMediaSessionList = mediaSessionList;

	m_isStopProcessThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hProcessThread = (HANDLE)_beginthreadex(NULL, 0, StartRtspSessionThread, this, 0, NULL);
}

RtspSession::RtspSession(SOCKET s, SOCKADDR_IN bindAddr, SOCKADDR_IN connectAddr, MediaSessionList* mediaSessionList)
{
	m_pRtspResponse = new RtspResponse(s, bindAddr, connectAddr);
	m_pMediaSessionList = mediaSessionList;

	m_isStopProcessThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hProcessThread = (HANDLE)_beginthreadex(NULL, 0, StartRtspSessionThread, this, 0, NULL);
}

RtspSession::~RtspSession()
{
	SetEvent(m_isStopProcessThread);
	WaitForSingleObject(m_hProcessThread, INFINITE);
	CloseHandle(m_hProcessThread);
	CloseHandle(m_isStopProcessThread);

	delete m_pRtspResponse;
}

UINT RtspSession::StartRtspSessionThread(PVOID pParam)
{
	RtspSession* pThis = (RtspSession*)pParam;
	pThis->RtspSessionThread();

	return 0;
}

void RtspSession::RtspSessionThread()
{
	int requestType;
	int waite;

	while(TRUE)
	{
		waite = WaitForSingleObject(m_isStopProcessThread, 0);
		if (WAIT_OBJECT_0 == waite)
			break;

		if ( !m_pRtspResponse->GetRequests() )
			break;

		if ( !m_pRtspResponse->GetRequestType(&requestType) )
			continue;

		switch(requestType)
		{
		case requestOptions:
			ResponseOptions();
			break;
		case requestDescribe:
			ResponseDescribe();
			break;
		case requestSetup:
			ResponseSetup();
			break;
		case requestPlay:
			ResponsePlay();
			break;
		case requestPause:
			ResponsePause();
			break;
		case requestTeardown:
			ResponseTeardown();
			SetEvent(m_isStopProcessThread);
			break;
		case requestGetParameter:
			ResponseGetParameter();
			break;
		default:
			break;
		}//switch end
	}
}

BOOL RtspSession::RtspSessionIsClose()
{
	int wait = WaitForSingleObject(m_hProcessThread, 0);
	if (wait == WAIT_OBJECT_0)
		return TRUE;
	return FALSE;
}

void RtspSession::ResponseOptions()
{
	m_pRtspResponse->ResponseOptions();
}

void RtspSession::ResponseDescribe()
{
	string sdp;

	if (!TransportDescribe(&sdp) )
		return;

	m_pRtspResponse->ResponseDescribe(sdp.c_str(), sdp.length());
}

void RtspSession::ResponseSetup()
{
	string	serverIp;
	INT		serverPort;
	string	targetIp;
	INT		targetPort;
	INT32	ssrc;

	if (!TransportSetup(&serverIp, &serverPort, &targetIp, &targetPort, &ssrc) )
		return;

	m_pRtspResponse->ResponseSetup(serverIp.c_str(), serverPort, targetIp.c_str(), targetPort, ssrc);
}

void RtspSession::ResponsePlay()
{
	if ( !TransportPlay() )
		return;

	m_pRtspResponse->ResponsePlay(m_SetupUrl.c_str());
}

void RtspSession::ResponsePause()
{
	if ( !TransportPause() )
		return;

	m_pRtspResponse->ResponsePause();

}

void RtspSession::ResponseTeardown()
{
	if ( !TransportTeardown() )
		return;

	m_pRtspResponse->ResponseTeardown();
}

void RtspSession::ResponseGetParameter()
{

}

BOOL RtspSession::TransportDescribe(string* describe)
{
	// 查询返回何种描述类型 目前只支持sdp描述类型
	//string accept;
	//if ( !m_pRtspControl->SearchRequests(&accept, "Accept") )
	//	printf("RtspSession	:Describe type only application/sdp\n");

	//if (accept.find("rtsl") != string::npos )		//rtsl
	//	return;

	//if (accept.find("mheg") != string::npos )		//mheg
	//	return;

	//if (accept.find("sdp") != string::npos )		//sdp
	//{	//return;
	//}	
	string sessionName;
	m_pRtspResponse->GetRequestSessionName(&sessionName);

	MediaSession* pMediaSession = m_pMediaSessionList->FindMediaSession(sessionName.c_str());
	if (!pMediaSession)
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 404 File Url Not Found, Or In Incorrect Format");
		return FALSE;
	}
	m_SessionName = sessionName;	//QuitTime SETUP时的路径不包含session name 所以预先保存下来

	string localIpAddr = inet_ntoa(m_pRtspResponse->GetBindAddr().sin_addr);
	string targetIpAddr = inet_ntoa(m_pRtspResponse->GetConnectAddr().sin_addr);

	string sdp = pMediaSession->GenerateMediaSdp(localIpAddr.c_str(), targetIpAddr.c_str(), TRUE);
	if ( !sdp.length() )
		return FALSE;

	// 清空m_SetupUrl 准备在SETUP时保存url地址
	m_SetupUrl = "";

	if (describe)
		*describe = sdp;

	return TRUE;
}

BOOL RtspSession::TransportSetup(	string* pBindIp, INT* pBindPort, 
									string* pTargetIp, INT* pTargetPort, 
									INT32* pSsrc)
{
	string			sessionName;
	string			streamName;
	MediaSession*	pMediaSession;
	MediaStream*	pMediaStream;
	INT32			ssrc;

	ssrc = (INT32)GenerateOneNumber();
	if (pSsrc)
		*pSsrc = ssrc;

	// 获取session
	if ( !m_pRtspResponse->GetRequestSessionName(&sessionName) )
		sessionName = m_SessionName; // QuickTime SETUP时mrl没有包含session name 需要提前获取
	
	pMediaSession = m_pMediaSessionList->FindMediaSession(sessionName.c_str());
	if (!pMediaSession)
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 404 Session Not Found");
		return FALSE;
	}

	// 获取stream
	if ( !m_pRtspResponse->GetRequestStreamName(&streamName) )
	{
		//m_pRtspResponse->ResponseError("RTSP/1.0 400 Bad Request");
		//return FALSE;
	}

	//使用trackID设置stream
	string::size_type iFind = streamName.find("trackID=");
	if (iFind != string::npos)
	{
		streamName.erase(iFind, 8);
		int streamIdx = atoi(streamName.c_str()) - 1;

		pMediaStream = pMediaSession->GetMediaStream( streamIdx );
		if (!pMediaStream)
		{
			m_pRtspResponse->ResponseError("RTSP/1.0 404 Stream Not Found");
			return FALSE;
		}

		return StreamSetup(pMediaStream, pBindIp, pBindPort, pTargetIp, pTargetPort);
	}

	//使用StreamName设置stream
	if (streamName.length() >0 )
	{
		pMediaStream = pMediaSession->FindMediaStream(streamName.c_str());
		if (!pMediaStream)
		{
			m_pRtspResponse->ResponseError("RTSP/1.0 404 Stream Not Found");
			return FALSE;
		}
		
		return StreamSetup(pMediaStream, pBindIp, pBindPort, pTargetIp, pTargetPort);
	}

	//使用idx遍历设置stream
	if (streamName.length() == 0)
	{
		UINT nStreamCount = pMediaSession->GetMediaStreamCount();
		if (!nStreamCount)
		{
			m_pRtspResponse->ResponseError("RTSP/1.0 404 Stream Not Found");
			return FALSE;
		}

		for (UINT iStream = 0; iStream < nStreamCount; iStream++)
		{
			pMediaStream = pMediaSession->GetMediaStream( iStream );
			
			if (!StreamSetup(pMediaStream, pBindIp, pBindPort, pTargetIp, pTargetPort))
				return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

BOOL RtspSession::StreamSetup(MediaStream* pMediaStream, string* pBindIp, INT* pBindPort, string* pTargetIp, INT* pTargetPort)
{
	SOCKADDR_IN addr;
	string		bindIp;
	INT			bindPort;
	string		targetIp;
	INT			targetPort;

	if (pMediaStream == NULL)
		return FALSE;

	addr		= m_pRtspResponse->GetBindAddr();
	bindIp		= inet_ntoa(addr.sin_addr);

	addr		= m_pRtspResponse->GetConnectAddr();
	targetIp	= inet_ntoa(addr.sin_addr);

	if ( !m_pRtspResponse->GetRequestTransportInfo(&targetPort, NULL) )
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 400 Bad Request");
		return FALSE;
	}

	// 从8000端口开始 尝试绑定偶数端口
	bindPort = 8000;
	while( !pMediaStream->TransportSetup(bindIp.c_str(), bindPort, targetIp.c_str(), targetPort) )
	{
		bindPort += 2;

		if (bindPort >= 65530)
			return FALSE;
	}

	// 保存SETUP的url地址 用于PLAY时生成RTP-info
	string mrl;
	m_pRtspResponse->GetRequestMrl(&mrl);

	if (m_SetupUrl.length())
		m_SetupUrl += ',';
	m_SetupUrl += ("url=" + mrl);

	if (pBindIp)
		*pBindIp = bindIp;
	
	if (pBindPort)
		*pBindPort = bindPort;

	if (pTargetIp)
		*pTargetIp = targetIp;
	
	if (pTargetPort)
		*pTargetPort = targetPort;

	return TRUE;
}

BOOL RtspSession::TransportPlay()
{
	string			sessionName;
	MediaSession*	pMediaSession;

	if ( !m_pRtspResponse->GetRequestSessionName(&sessionName) )
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 400 Bad Request");
		return FALSE;
	}
	
	pMediaSession = m_pMediaSessionList->FindMediaSession(sessionName.c_str());
	if (!pMediaSession)
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 404 Session Not Found");
		return FALSE;
	}
	
	UINT streamCount = pMediaSession->GetMediaStreamCount();

	for (UINT iStream = 0; iStream < streamCount; iStream++)
	{
		MediaStream* pStream = pMediaSession->GetMediaStream(iStream);
		if (!pStream)
			continue;
		pStream->TransportStart();
	}

	return TRUE;
}

BOOL RtspSession::TransportPause()
{
	string			sessionName;
	MediaSession*	pMediaSession;

	if ( !m_pRtspResponse->GetRequestSessionName(&sessionName) )
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 400 Bad Request");
		return FALSE;
	}
	
	pMediaSession = m_pMediaSessionList->FindMediaSession(sessionName.c_str());
	if (!pMediaSession)
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 404 Session Not Found");
		return FALSE;
	}
	
	UINT streamCount = pMediaSession->GetMediaStreamCount();

	for (UINT iStream = 0; iStream < streamCount; iStream++)
	{
		MediaStream* pStream = pMediaSession->GetMediaStream(iStream);
		if (!pStream)
			continue;
		pStream->TransportStop();
	}

	return TRUE;
}

BOOL RtspSession::TransportTeardown()
{
	string			sessionName;
	MediaSession*	pMediaSession;

	if ( !m_pRtspResponse->GetRequestSessionName(&sessionName) )
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 400 Bad Request");
		return FALSE;
	}
	
	pMediaSession = m_pMediaSessionList->FindMediaSession(sessionName.c_str());
	if (!pMediaSession)
	{
		m_pRtspResponse->ResponseError("RTSP/1.0 404 Session Not Found");
		return FALSE;
	}
	
	UINT streamCount = pMediaSession->GetMediaStreamCount();

	for (UINT iStream = 0; iStream < streamCount; iStream++)
	{
		MediaStream* pStream = pMediaSession->GetMediaStream(iStream);
		if (!pStream)
			continue;
		pStream->TransportTeardown();
	}
	
	// 清空保存SETUP的url地址 在SETUP时再重新设置
	m_SetupUrl = "";

	return TRUE;
}

LONGLONG RtspSession::GenerateOneNumber() 
{
	static LARGE_INTEGER	tickFrequency;
	static BOOL				tickFrequencySet = FALSE;

	LARGE_INTEGER	tickNow;

	if (tickFrequencySet == FALSE) 
	{
		QueryPerformanceFrequency(&tickFrequency);
		tickFrequencySet = TRUE;
	}
	QueryPerformanceCounter(&tickNow);

	//return (INT32)(tickNow.QuadPart / tickFrequency.QuadPart);
	return tickNow.QuadPart;
}