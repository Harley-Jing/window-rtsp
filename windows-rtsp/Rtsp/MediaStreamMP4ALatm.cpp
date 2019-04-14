#include "StdAfx.h"
#include "MediaStreamMP4ALatm.h"

MediaStreamMP4ALatm::MediaStreamMP4ALatm(PCSTR mediaStreamName)
		:MediaStream(mediaStreamName, MEDIA_STREAM_TYPE_H264)
{

}

MediaStreamMP4ALatm::~MediaStreamMP4ALatm()
{
	m_Packet.FreeBuffer();
}


// BOOL MediaStreamMP4ALatm::Init(UINT profileLevelId, const string& config)
// {
// 	m_ProfileLevelId = profileLevelId;
// 	m_Config = config;
// 	return TRUE;
// }

BOOL MediaStreamMP4ALatm::Init(UINT nStreamBitrate, UINT nMTU, INT channel, INT sampleFreq)
{
	m_Channel = channel;
	m_SampleFreq = sampleFreq;
	return MediaStream::Init(nStreamBitrate, nMTU);
}

string MediaStreamMP4ALatm::GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP)
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

	// 生成各字段的内容
	const int aac_sampling_freq[15] = {
		96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
        16000, 12000, 11025, 8000, 7350, 0, 0 };

	int aac_sampling_freq_Idx = 15;
	
	for(int i = 0; i < 15; i++ )
	{
		if(  m_SampleFreq == aac_sampling_freq[i] )
		{
			aac_sampling_freq_Idx = i;
			break;
		}
	}

	unsigned char config[6];

	config[0]=0x40;
	config[1]=0;
	config[2]=0x20|aac_sampling_freq_Idx;
	config[3]=m_Channel<<4;
	config[4]=0x3f;
	config[5]=0xc0;

	_snprintf(temp, 500, " MP4A-LATM/%d/%d", m_SampleFreq, m_Channel);
	string rtpmap	= temp;
	
    char	configHexa[13];
	sprintf_hexa( configHexa, config, 6 );

	_snprintf(temp, 500, "%u", m_nBandWidth);
	bs = temp;
	
	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 1);
	streamid = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 101);
	esid = temp;

	_snprintf(temp, 500, "%u", m_nRtpPort);
	port			= temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType);
	payloadType = temp;

	_snprintf(temp, 500, "profile-level-id=15; object=2; cpresent=0; config=%s", configHexa);
	fmtp	= temp; 

	// 生成sdp内容
	mediaSdp += "m=audio "+port+" RTP/AVP "+payloadType+"\r\n";						//m
	mediaSdp += "b=AS:"+bs+"\r\n";													//b																						
	mediaSdp += "a=rtpmap:"+payloadType+rtpmap+"\r\n";								//a=rtpmap
	mediaSdp += "a=fmtp:"+payloadType+" "+fmtp+"\r\n";								//a=fmtp
	mediaSdp += "a=mpeg4-esid:"+esid+"\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:trackID="+streamid+"\r\n";
	
	return mediaSdp;
}

UINT MediaStreamMP4ALatm::TransportData(PBYTE pData, UINT dataSize, INT32 pts)
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return 0;

	UINT	mtu = m_nMTU;

	int     i_max   = mtu - RTP_HEADER_SIZE - 2;  // payload max in one packet 

    int     latmhdrsize = dataSize / 0xff + 1;
    int     i_count = ( dataSize + i_max - 1 ) / i_max;
	
    PBYTE	p_data = pData, p_header = NULL;
    int     i_data  = dataSize;
    int     i;
	
	UINT	writeSize = 0;

    for( i = 0; i < i_count; i++ )
    {
        int     i_payload = (i_max < i_data ) ? i_max : i_data;
        
        if( i != 0 )
            latmhdrsize = 0;

		m_Packet.ExtendBuffer(latmhdrsize + i_payload);
		
        if( i == 0 )
        {
            int tmp = dataSize;
			
            p_header = m_Packet.m_pData;
            while( tmp > 0xfe )
            {
                *p_header = 0xff;
                p_header++;
                tmp -= 0xff;
            }
            *p_header = tmp;
        }
		
        memcpy( &m_Packet.m_pData[latmhdrsize], p_data, i_payload );
		m_Packet.m_nDataSize = latmhdrsize + i_payload;
		
		//int iWrite = m_pRtpTransport->SetRtpData(m_Packet.m_pData, m_Packet.m_DataSize, pts, ((i == i_count - 1) ? TRUE : FALSE) );
		int iWrite = m_pRtpTransport->Write(m_Packet.m_pData, m_Packet.m_nDataSize, m_nRtpPayloadType, pts, 0, ((i == i_count - 1) ? TRUE : FALSE) );
		if (iWrite > 0)
			writeSize += iWrite;

        p_data += i_payload;
        i_data -= i_payload;
    }
	
	return writeSize;
}
