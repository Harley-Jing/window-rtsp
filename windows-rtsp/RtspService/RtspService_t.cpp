#include "StdAfx.h"
#include "RtspService_t.h"

#include "RtspService.h"
#include <process.h>

RtspService_t::RtspService_t()
{
	m_pRtspTransport = NULL;
	
	m_pMediaSession = NULL;
	m_pMediaStream_Video = NULL;
	m_pMediaStream_Audio = NULL;
	
	m_hVideoFile = NULL;
	m_hAudioFile = NULL;

	m_hVideoFile_ReadThread = NULL;
	m_bVideoFile_ReadThread_Stop = FALSE;

	m_hAudioFile_ReadThread = NULL;
	m_bAudioFile_ReadThread_Stop = FALSE;

	m_bOpen = FALSE;
}

RtspService_t::~RtspService_t()
{
	Release();
}

void RtspService_t::Release()
{
	if (m_hVideoFile_ReadThread)
	{
		m_bVideoFile_ReadThread_Stop = TRUE;
		WaitForSingleObject(m_hVideoFile_ReadThread, INFINITE);
	}
	
	if (m_hAudioFile_ReadThread)
	{
		m_bAudioFile_ReadThread_Stop = TRUE;
		WaitForSingleObject(m_hAudioFile_ReadThread, INFINITE);
	}

	if (m_hVideoFile && m_hVideoFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hVideoFile);
	m_hVideoFile = NULL;
	
	if (m_hAudioFile && m_hAudioFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hAudioFile);
	m_hAudioFile = NULL;

	m_bOpen = FALSE;
}

BOOL RtspService_t::Open(LPCTSTR textVideoFilePath, LPCTSTR textAudioFilePath)
{
	if (!textVideoFilePath || !textAudioFilePath)
		return FALSE;

	if (m_bOpen)
		return TRUE;
	
	// 打开音视频文件
	m_hVideoFile = CreateFile(textVideoFilePath, 
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);	
    if (m_hVideoFile == INVALID_HANDLE_VALUE) 
    {
		Release();
        printf("Could not open file (error %d)\n", GetLastError());
        return FALSE; 
    }
	
	//m_hAudioFile = CreateFile(textAudioFilePath, 
	//	GENERIC_READ,
	//	FILE_SHARE_READ,
	//	NULL,
	//	OPEN_EXISTING,
	//	FILE_ATTRIBUTE_NORMAL,
	//	NULL);
	//if (m_hAudioFile == INVALID_HANDLE_VALUE) 
	//   {
	//	Release();
	//       printf("Could not open file (error %d)\n", GetLastError());
	//       return FALSE; 
	//   }

	// 创建音视频文件的读取线程
	m_hVideoFile_ReadThread = (HANDLE)_beginthreadex(NULL, 0, H264File_Read2SendThread, this, 0, NULL);
	//m_hAudioFile_ReadThread = (HANDLE)_beginthreadex(NULL, 0, AACFile_Read2SendThread, this, 0, NULL);
	
	m_bOpen = TRUE;

	return TRUE;
}
