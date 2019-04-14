#include "StdAfx.h"
#include "MediaStreamMP4A.h"

MediaStreamMP4A::MediaStreamMP4A(LPCSTR name, INT nISMA)
:MediaStream(name, MEDIA_STREAM_TYPE_MP4A),
m_nISMA(nISMA)
{
	m_Channel = 0;
	m_SampleFreq = 0;
}

MediaStreamMP4A::~MediaStreamMP4A()
{
	m_Packet.FreeBuffer();
}

BOOL MediaStreamMP4A::Init(UINT nStreamBitrate, UINT nMTU, int channel, int sampleFreq)
{
	m_Channel = channel;
	m_SampleFreq = sampleFreq;

	return MediaStream::Init(nStreamBitrate, nMTU);
}

string MediaStreamMP4A::GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP)
{
	string	mediaSdp;
	
	string	port;
	string	payloadType;
	string	bs;
	string	fmtp;
	string	streamid;
	string	esid;
	
	char	temp[500];
	
	m_nRtpPayloadType = nRtpPayloadType;

	// Construct the 'specificConfig', and from it, the corresponding ASCII string:
	int profile = 1;
	int srate_idx = 15;
	
	const int aac_sampling_freq[16] = {96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025,  8000,
		0, 0, 0, 0}; 
	
	for (int i = 0; i < 16; i++)
	{
		if (m_SampleFreq >= (aac_sampling_freq[i] - 1000)) 
		{
			srate_idx = i;
			break;
		}
	}
	
	BYTE specificConfig[2];
	int	 objectType = profile + 1;
	specificConfig[0] = (objectType<<3) | (srate_idx>>1);
	specificConfig[1] = (srate_idx<<7) | (m_Channel<<3);
	
	char config[5];
	sprintf_hexa(config, &specificConfig[0], 2);

	// 生成各字段的内容
	_snprintf(temp, 500, "%u", m_nRtpPort);
	port			= temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType);
	payloadType		= temp;

	_snprintf(temp, 500, "%u", m_nBandWidth);
	bs = temp;
	
	_snprintf(temp, 500, "streamtype=5; profile-level-id=15; mode=AAC-hbr; config=%s; SizeLength=13; IndexLength=3; IndexDeltaLength=3;", 
		config);
	fmtp			= temp;

	_snprintf(temp, 500, "MPEG4-GENERIC/%u/%u", m_SampleFreq, m_Channel);
	string rtpmap	= temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 1);
	streamid = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 101);
	esid = temp;

	// 生成sdp内容
	mediaSdp += "m=audio "+port+" RTP/AVP "+payloadType+"\r\n";						//m
	mediaSdp += "b=AS:"+bs+"\r\n";													//b																						
	mediaSdp += "a=rtpmap:"+payloadType+" "+rtpmap+"\r\n";							//a=rtpmap
	mediaSdp += "a=fmtp:"+payloadType+" "+fmtp+"\r\n";								//a=fmtp
	mediaSdp += "a=mpeg4-esid:"+esid+"\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:trackID="+streamid+"\r\n";
	
	return mediaSdp;
}

UINT MediaStreamMP4A::TransportData(PBYTE pData, UINT dataSize, int pts)
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return 0;

	UINT	mtu		= m_nMTU;
	
	int     i_max   = mtu - RTP_HEADER_SIZE - 4 ; // payload max in one packet 
	int     i_count = ( dataSize + i_max - 1 ) / i_max;

	PBYTE	p_data = pData;
	int     i_data  = dataSize;
	int     i;

	UINT	writeSize = 0;
	
	//=====================================================
	BOOL	protection_absent = pData[1]&0x01;
	UINT16	frame_length	= ((pData[3]&0x03)<<11) | (pData[4]<<3) | ((pData[5]&0xE0)>>5);

	UINT	numBytesToRead	= frame_length > 7  ? frame_length - 7 : 0;

	// If there's a 'crc_check' field, skip it:
	if (!protection_absent) 
	{
		numBytesToRead = numBytesToRead > 2 ? numBytesToRead - 2 : 0;
	}

	dataSize = numBytesToRead;
	i_data = dataSize;
	p_data = pData+7;

	//=====================================================

	for( i = 0; i < i_count; i++ )
	{
		int	i_payload = (i_max < i_data) ? i_max : i_data;
		
		m_Packet.ExtendBuffer(4 + i_payload);

		/* AU headers */
		m_Packet.m_pData[0] = 0;
		m_Packet.m_pData[1] = 16;						// AU-headers-length (bits)
		m_Packet.m_pData[2] = ( dataSize >> 5 )&0xff;	// for each AU length 13 bits + idx 3bits
		m_Packet.m_pData[3] = ( (dataSize&0xff)<<3 )|0;

		memcpy( &m_Packet.m_pData[4], p_data, i_payload );
		m_Packet.m_nDataSize = 4 + i_payload;
		
		//int iWrite = m_pRtpTransport->SetRtpData(m_Packet.m_pData, m_Packet.m_DataSize, pts, ((i == i_count - 1)?TRUE:FALSE) );
		int iWrite = m_pRtpTransport->Write(m_Packet.m_pData, m_Packet.m_nDataSize, m_nRtpPayloadType, pts, 0, ((i == i_count - 1)?TRUE:FALSE) );
		if (iWrite > 0)
			writeSize += iWrite;

		p_data += i_payload;
		i_data -= i_payload;
	}

	return writeSize;
}
