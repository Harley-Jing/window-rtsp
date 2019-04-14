#ifndef __MEDIA_STREAM_TRANSPORT_H__
#define __MEDIA_STREAM_TRANSPORT_H__

#include "MediaSession.h"
#include "MediaStream.h"
#include <list>
#include <string>

class MediaStreamTransport
{
public:
	MediaStreamTransport(void);
	virtual ~MediaStreamTransport(void);

	virtual BOOL AddMediaSession(MediaSession* pSession);
	//virtual BOOL AddMediaStream(LPCSTR sessionName, MediaStream* pMediaStream);

	virtual MediaSession* FindMediaSession(LPCSTR sessionName);

protected:
		
	list<MediaSession*> m_MediaSessions;
};

#endif