#include "stdafx.h"
#include "MediaSession.h"

//==========================================
// class MediaSession

MediaSession::MediaSession(PCSTR name)	
	:m_Name(name)
{

}

MediaSession::~MediaSession(void)
{
	for (UINT iStream = 0; iStream < m_MediaStreams.size(); iStream++)
	{
		delete m_MediaStreams[iStream];
	}
	m_MediaStreams.clear();
}

MediaStream* MediaSession::GetMediaStream(UINT streamIdx)
{
	if (streamIdx >= m_MediaStreams.size() )
		return NULL;

	return m_MediaStreams[streamIdx];
}

UINT MediaSession::GetMediaStreamCount()
{
	return m_MediaStreams.size();
}

BOOL MediaSession::AddMediaStream(MediaStream* pMediaStream)
{
	if (!pMediaStream)
		return FALSE;

	if ( FindMediaStream(pMediaStream->GetName().c_str()) )
		return FALSE;

	m_MediaStreams.push_back(pMediaStream);

	return TRUE;
}

MediaStream* MediaSession::FindMediaStream(PCSTR streamName)
{
	if (!streamName)
		return NULL;
	
	for (UINT iStream = 0; iStream < m_MediaStreams.size(); iStream++)
	{
		if ( m_MediaStreams[iStream]->GetName().compare( streamName ) == 0)
		{
			return m_MediaStreams[iStream];
		}
	}	

	return NULL;
}

string MediaSession::GetName()
{
	return m_Name;
}

string MediaSession::GenerateMediaSdp(PCSTR localIpAddr, PCSTR targetIpAddr, BOOL bUseRTSP)
{
	string	userName, sessionId, sessionVersion;
	string	startTime, stopTime;
	string	sessionName;

	char	numberStr[100];
	_snprintf(numberStr, 100, "%I64d", GenerateOneNumber());

	string	mediaSdp;
	int		rtpPayloadType = 96;

	sessionId		= numberStr;
	sessionVersion	= sessionId;

	sessionName		= m_Name;
	
	startTime		= "0";
	stopTime		= "0";

	mediaSdp += "v=0\r\n";		// v=0 sdp版本
	mediaSdp += "o=- "+sessionId+" "+sessionVersion+" "+"IN IP4 "+localIpAddr+"\r\n"; 	//- 无用户 IN Internet IP4 ip地址类型
	mediaSdp += "s="+sessionName+"\r\n";
	//mediaSdp += "b=AS:50\r\n";
	mediaSdp += "c=IN IP4 "+string(targetIpAddr)+"\r\n";
	mediaSdp += "t="+startTime+" "+stopTime+"\r\n";
	//mediaSdp += "a=x-broadcastcontrol:RTSP\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:*\r\n";

#if 0 //test
	mediaSdp += "a=isma-compliance:2,2.0,2\r\n";
#endif 

	for (UINT iStream = 0; iStream < m_MediaStreams.size(); iStream++)
	{
		mediaSdp += m_MediaStreams[iStream]->GenerateMediaSdp(rtpPayloadType++, bUseRTSP);
	}	
	return mediaSdp;
}

LONGLONG MediaSession::GenerateOneNumber() 
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

//============================================================
// class MediaSessionList

MediaSessionList::MediaSessionList()
{
	
}

MediaSessionList::~MediaSessionList(void)
{
	for (UINT iSession = 0; iSession < m_MediaSessions.size(); iSession++)
	{
		delete m_MediaSessions[iSession];
	}
	m_MediaSessions.clear();
}

BOOL MediaSessionList::AddMediaSession(MediaSession* pSession)
{
	if (!pSession)
		return FALSE;

	ATLock autolock(m_Lock);

	for (UINT iSession = 0; iSession < m_MediaSessions.size(); iSession++)
	{
		if ( string(m_MediaSessions[iSession]->GetName() ).compare( pSession->GetName() ) == 0)
			return FALSE;
	}

	m_MediaSessions.push_back( pSession );

	return TRUE;
}

BOOL MediaSessionList::RemoveMediaSession(LPCSTR sessionName)
{
	if (!sessionName)
		return FALSE;

	ATLock autolock(m_Lock);

	for (UINT iSession = 0; iSession < m_MediaSessions.size(); iSession++)
	{
		if ( string(m_MediaSessions[iSession]->GetName() ).compare( sessionName ) == 0)
		{
			delete m_MediaSessions[iSession];
			m_MediaSessions.erase(m_MediaSessions.begin()+iSession);
			return TRUE;
		}
	}

	return FALSE;
}

MediaSession* MediaSessionList::FindMediaSession(LPCSTR sessionName)
{
	if (!sessionName)
		return FALSE;

	ATLock autolock(m_Lock);

	for (UINT iSession = 0; iSession < m_MediaSessions.size(); iSession++)
	{
		if ( string(m_MediaSessions[iSession]->GetName() ).compare( sessionName ) == 0)
		{
			return m_MediaSessions[iSession];
		}
	}
	return NULL;
}
