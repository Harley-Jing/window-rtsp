#include "StdAfx.h"
#include "MediaStreamTransport.h"

MediaStreamTransport::MediaStreamTransport(void)
{
}

MediaStreamTransport::~MediaStreamTransport(void)
{
	list<MediaSession*>::iterator i;
	for (i = m_MediaSessions.begin(); i != m_MediaSessions.end(); i++)
	{
		delete (*i);
	}
	m_MediaSessions.clear();
}

BOOL MediaStreamTransport::AddMediaSession(MediaSession* pSession)
{
	if ( !pSession )
		return FALSE;
	
	if ( FindMediaSession( pSession->GetName().c_str() ) )
		return FALSE;

	m_MediaSessions.push_back(pSession);

	return TRUE;
}
// 
// BOOL MediaStreamTransport::AddMediaStream(LPCSTR sessionName, MediaStream* pMediaStream)
// {
// 	MediaSession* pMediaSession = NULL;
// 
// 	pMediaSession = FindMediaSession(sessionName);
// 	if (!pMediaSession)
// 		return FALSE;
// 
// 	if ( !pMediaSession->AddMediaStream(pMediaStream) )
// 		return FALSE;
// 
// 	return TRUE;
// }

MediaSession* MediaStreamTransport::FindMediaSession(LPCSTR sessionName)
{
	list<MediaSession*>::iterator i;
	for (i = m_MediaSessions.begin(); i != m_MediaSessions.end(); i++)
	{
		if ( (*i)->GetName() == sessionName)
		{
			return *i;
		}
	}	

	return NULL;
}
