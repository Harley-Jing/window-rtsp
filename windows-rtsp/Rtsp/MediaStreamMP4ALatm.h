#ifndef __MEDIA_STREAM_MP4A_LATM_H__
#define __MEDIA_STREAM_MP4A_LATM_H__

#include "MediaStream.h"

class MediaStreamMP4ALatm :	public MediaStream
{
public:
	MediaStreamMP4ALatm(PCSTR mediaStreamName = "MP4ALatm Stream");

	virtual ~MediaStreamMP4ALatm();

	virtual BOOL	Init(UINT nStreamBitrate, UINT nMTU, INT channel, INT sampleFreq);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP);

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, INT32 pts);

protected:
	Buffer m_Packet;

	int m_Channel;
	int m_SampleFreq;

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
};

#endif //__MEDIA_STREAM_MP4A_LATM_H__