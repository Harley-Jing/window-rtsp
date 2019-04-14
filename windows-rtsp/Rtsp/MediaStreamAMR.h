#ifndef __MEDIA_STREAM_AMR_H__
#define __MEDIA_STREAM_AMR_H__

#include "MediaStream.h"

class MediaStreamAMR :	public MediaStream
{
public:
	MediaStreamAMR(LPCSTR name = "AMR Stream");

	virtual ~MediaStreamAMR();

	virtual	BOOL	Init(UINT nStreamBitrate, UINT nMTU, BOOL isWB, UINT sampleFreq, UINT channels);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP);

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, INT32 pts);

protected:
	Buffer		m_Packet;

	BOOL		m_isWB;
	UINT		m_SampleFreq;
	UINT		m_Channels;
};

#endif //__MEDIA_STREAM_AMR_H__