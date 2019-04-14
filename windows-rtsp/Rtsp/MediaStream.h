#ifndef __MEDIA_STREAM_H__
#define __MEDIA_STREAM_H__

#include "RtpTransport.h"

enum MediaStreamType
{
	MEDIA_STREAM_TYPE_AUDIO = 100, 	//audio
	MEDIA_STREAM_TYPE_MP4A,
	MEDIA_STREAM_TYPE_MP4A_LATM,
	MEDIA_STREAM_TYPE_AMR,

	MEDIA_STREAM_TYPE_VIDEO = 200,	//video
	MEDIA_STREAM_TYPE_H264,
	MEDIA_STREAM_TYPE_H263,
	MEDIA_STREAM_TYPE_MP4V,
	
	MEDIA_STREAM_TYPE_DATA	= 300,	//mux
	MEDIA_STREAM_TYPE_MPEG2TS,
};

class MediaStream
{
public:
	MediaStream(PCSTR name = "Media Stream", INT type = 0);
	
	virtual ~MediaStream();

	virtual BOOL	Init(UINT nStreamBitrate, UINT nMTU);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP = FALSE) = 0;

	virtual BOOL	TransportSetup(PCSTR psBindIp, UINT nBindPort, PCSTR psTargetIp, UINT nTargetPort);

	virtual BOOL	TransportStart();

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, INT32 pts) = 0;

	virtual BOOL	TransportStop();

	virtual BOOL	TransportTeardown();

	virtual string	GetName();

	virtual INT		GetType();

	virtual INT		GetRtpPayloadType();

	virtual UINT	GetBandWidth();

protected:
	string	m_Name;
	INT		m_Type;

	UINT	m_nMTU;
	INT		m_nRtpPayloadType;
	UINT	m_nRtpPort;
	BOOL	m_bUseRTSP;

	UINT	m_nBandWidth;

	BOOL	m_bInit;
	BOOL	m_bSetup;
	BOOL	m_bRun;
	
	//RtpTransport* m_pRtpTransport;
	Rtp*	m_pRtpTransport;
	TLock	m_tlockRun;
};

#endif __MEDIA_STREAM_H__