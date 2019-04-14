#ifndef __MEDIA_STREAM_H264_H__
#define __MEDIA_STREAM_H264_H__

#include "MediaStream.h"

class MediaStreamH264 :	public MediaStream
{
public:
	MediaStreamH264(LPCSTR mediaStreamName = "H264 Stream");

	virtual ~MediaStreamH264();

	virtual BOOL	Init(UINT nStreamBitrate, UINT nMTU, UINT nWidth, UINT nHeight, int profileLevelId, const string& spropParameterSets);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP);

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, int pts);

protected:
	UINT TransportH264Nal(const PBYTE pNal, UINT nalSize, INT32 pts, BOOL isLast);

	UINT	m_nWidth;
	UINT	m_nHeight;
	int		m_ProfileLevelId;
	string	m_SpropParameterSets;

	Buffer	m_Packet;
};

#endif //__MEDIA_STREAM_H264_H__