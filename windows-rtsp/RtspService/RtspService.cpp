// RtspService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RtspService.h"
#include "RtspService_t.h"

#include "MediaStreamH264.h"
#include "MediaStreamMP4ALatm.h"

#include "cmd.h"

RtspService_t* g_pRtspService;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	string	textBindIP;
	UINT	nBindPort;
	string	textVideoFile;
	string	textAudioFile;

	if ( !ParseArg(argc, argv, textBindIP, nBindPort, textVideoFile, textAudioFile) )
	{
		PrintfHelp();
		return 0;
	}

	// 创建Rtsp Transport对象
	g_pRtspService = new RtspService_t();

	// RtspTransport->StreamSession->MediaStream 绑定
	g_pRtspService->m_pMediaStream_Video = new MediaStreamH264( "h264" );
	g_pRtspService->m_pMediaStream_Audio = new MediaStreamMP4ALatm( "aac" );

	g_pRtspService->m_pMediaSession = new MediaSession( "TestSession" );
	g_pRtspService->m_pMediaSession->AddMediaStream( g_pRtspService->m_pMediaStream_Video );
	//g_pRtspService->m_pMediaSession->AddMediaStream( g_pRtspService->m_pMediaStream_Audio );

	g_pRtspService->m_pRtspTransport = new RtspTransport();
	g_pRtspService->m_pRtspTransport->AddMediaSession( g_pRtspService->m_pMediaSession );

	// 打开数据服务线程
	if ( !g_pRtspService->Open(textVideoFile.c_str(), textAudioFile.c_str()) )
	{
		printf("Rtsp Service open failed.\n");
		delete g_pRtspService->m_pRtspTransport;
		delete g_pRtspService;
		return 0;
	}

	// 打开Rtsp服务
	if ( !g_pRtspService->m_pRtspTransport->Open(textBindIP.c_str(), nBindPort, 0xFFFFFFFF) )
	{
		printf("Rtsp Transport open failed.\n");
		delete g_pRtspService->m_pRtspTransport;
		delete g_pRtspService;
	}

	printf( "Rtsp Service is working.\n");
	printf( "Input any char will break the RTSP Service.\n");
	getchar();

	delete g_pRtspService->m_pRtspTransport;
	delete g_pRtspService;

	return 0;
}
