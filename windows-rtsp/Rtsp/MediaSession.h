#ifndef __MEDIA_SESSION_H__
#define __MEDIA_SESSION_H__

#include "MediaStream.h"

//==========================================
// class MediaSession

class MediaSession
{
public:
	MediaSession(PCSTR name);

	virtual ~MediaSession(void);

	virtual MediaStream*	FindMediaStream(PCSTR streamName);

	virtual MediaStream*	GetMediaStream(UINT streamIdx);
	
	virtual UINT	GetMediaStreamCount();

	virtual BOOL	AddMediaStream(MediaStream* pMediaStream);

	virtual string	GenerateMediaSdp(PCSTR localIpAddr, PCSTR targetIpAddr, BOOL bUseRTSP = FALSE);

	virtual string	GetName();
	
protected:
	LONGLONG GenerateOneNumber();

	string					m_Name;
	
	vector<MediaStream*>	m_MediaStreams;
};

//==========================================
// class MediaSessionList

class MediaSessionList
{
public:
	MediaSessionList();
	virtual ~MediaSessionList(void);

	virtual BOOL	AddMediaSession(MediaSession* pSession);
	virtual BOOL	RemoveMediaSession(LPCSTR sessionName);
	virtual MediaSession*	FindMediaSession(LPCSTR sessionName);

protected:
	vector<MediaSession*> m_MediaSessions;

	TLock	m_Lock;
};

#endif // __MEDIA_SESSION_H__