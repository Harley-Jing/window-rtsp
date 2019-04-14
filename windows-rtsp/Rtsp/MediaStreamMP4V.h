#ifndef __MEDIA_STREAM_MP4V_H__
#define __MEDIA_STREAM_MP4V_H__

#include "MediaStream.h"

class MediaStreamMP4V :	public MediaStream
{
public:
	MediaStreamMP4V(LPCSTR mediaStreamName = "MP4V Stream", BOOL bISMAStream = FALSE);

	virtual ~MediaStreamMP4V();

	virtual	BOOL Init(UINT nStreamBitrate, UINT nMTU, 
		UINT	nWidth, 
		UINT	nHeight,
		BOOL	bframes, 
		double	fps);

	virtual string	GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP);

	virtual UINT	TransportData(PBYTE pData, UINT dataSize, int pts);

protected:
	void GenerateConfig(
		PBYTE*	ppConfig, 
		UINT*	pConfigLength, 
		UINT8*	pTimeIncrBits,							  
		bool	bframes, 
		double	fps,
		bool	shortTime, 
		UINT	width, 
		UINT	height);

	bool CreateVosh(PBYTE* ppBytes,	UINT32* pNumBytes, UINT8 profileLevel);

	bool CreateVo(PBYTE*	ppBytes, UINT32* pNumBytes,	UINT8 objectId);

	bool CreateVol(
		PBYTE*	ppBytes, 
		UINT*	pNumBytes,
		UINT8	profile, 
		double	frameRate, 
		bool	shortTime, 
		bool	variableRate,
		UINT16	width, 
		UINT16	height, 
		UINT8	quantType, 
		UINT8*	pTimeBits);

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

	Buffer	m_Packet;

	BOOL	m_bframes;
	double	m_fps; 
	UINT	m_nWidth; 
	UINT	m_nHeight;

	BOOL	m_bISMAStream;
};

#endif //__MEDIA_STREAM_MP4V_H__