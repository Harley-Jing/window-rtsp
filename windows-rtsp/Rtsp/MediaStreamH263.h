#ifndef __MEDIA_STREAM_H263_H__
#define __MEDIA_STREAM_H263_H__

#include "MediaStream.h"

class MediaStreamH263 :	public MediaStream
{
public:
	MediaStreamH263(LPCSTR mediaStreamName = "H263 Stream");

	virtual ~MediaStreamH263();

	virtual	BOOL	Init(UINT nStreamBitrate, UINT nMTU, UINT nWidth, UINT nHeight);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP);

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, int pts);

protected:
	UINT	m_nWidth;
	UINT	m_nHeight;

	Buffer	m_Packet;
};

#endif //__MEDIA_STREAM_H263_H__