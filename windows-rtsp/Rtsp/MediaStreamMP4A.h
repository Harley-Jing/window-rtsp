#ifndef __MEDIA_STREAM_MP4A_H__
#define __MEDIA_STREAM_MP4A_H__

#include "MediaStream.h"

class MediaStreamMP4A :	public MediaStream
{
public:
	MediaStreamMP4A(LPCSTR name = "MP4A Stream", INT nISMA = -1);

	virtual ~MediaStreamMP4A();

	virtual BOOL	Init(UINT nStreamBitrate, UINT nMTU, int channel, int sampleFreq);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP);

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, int pts);

protected:
	inline void sprintf_hexa( char *s, PBYTE p_data, int i_data )
	{
		const char* hex = "0123456789abcdef";
		int i;
		
		for( i = 0; i < i_data; i++ )
		{
			s[2*i+0] = hex[(p_data[i]>>4)&0xf];
			s[2*i+1] = hex[(p_data[i]   )&0xf];
		}
		s[2*i_data] = '\0';
	};

	Buffer m_Packet;

	int m_Channel;
	int m_SampleFreq;

	INT m_nISMA;
};

#endif //__MEDIA_STREAM_MP4A_H__