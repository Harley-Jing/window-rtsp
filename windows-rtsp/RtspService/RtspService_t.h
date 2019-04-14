#pragma once

#include "RtspTransport.h"

struct RtspService_t
{
	RtspTransport*	m_pRtspTransport;
	
	MediaSession*	m_pMediaSession;
	MediaStream*	m_pMediaStream_Video;
	MediaStream*	m_pMediaStream_Audio;

	HANDLE			m_hVideoFile;
	HANDLE			m_hAudioFile;
	
	HANDLE			m_hVideoFile_ReadThread;
	BOOL			m_bVideoFile_ReadThread_Stop;

	HANDLE			m_hAudioFile_ReadThread;
	BOOL			m_bAudioFile_ReadThread_Stop;

	BOOL			m_bOpen;

	RtspService_t();
	~RtspService_t();
	
	void Release();
	BOOL Open(LPCTSTR textVideoFilePath, LPCTSTR textAudioFilePath);
};
