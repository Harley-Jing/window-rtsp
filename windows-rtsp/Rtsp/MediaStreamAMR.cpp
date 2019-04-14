#include "StdAfx.h"
#include "MediaStreamAMR.h"

MediaStreamAMR::MediaStreamAMR(LPCSTR name)	
:MediaStream(name, MEDIA_STREAM_TYPE_AMR)
{
	
}

MediaStreamAMR::~MediaStreamAMR()
{
	m_Packet.FreeBuffer();
}

BOOL MediaStreamAMR::Init(UINT nStreamBitrate, UINT nMTU, BOOL isWB, UINT sampleFreq, UINT channels)
{
	m_isWB = isWB;
	m_SampleFreq = sampleFreq;
	m_Channels = channels;
	return MediaStream::Init(nStreamBitrate, nMTU);
}

string MediaStreamAMR::GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP)
{
	string	mediaSdp;
	
	string	port;
	string	payloadType;
	string	bs;
	string	fmtp;
	string	streamid;
	string	esid;
	string	rtpmap;
	char	temp[500];
	
	m_nRtpPayloadType = nRtpPayloadType;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType);
	payloadType	= temp;

	_snprintf(temp, 500, "%u", m_nRtpPort);
	port = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 1);
	streamid = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 101);
	esid = temp;

	_snprintf(temp, 500, "%u", m_nBandWidth);
	bs = temp;

	rtpmap = "AMR";
	if (m_isWB)
		rtpmap += "-WB";
	_snprintf(temp, 500, "/%u/%u", m_SampleFreq, m_Channels);
	rtpmap += temp;

	// Éú³ÉsdpÄÚÈÝ
	mediaSdp += "m=audio "+port+" RTP/AVP "+payloadType+"\r\n";						//m
	mediaSdp += "b=AS:"+bs+"\r\n";												//b																						
	mediaSdp += "a=rtpmap:"+payloadType+" "+rtpmap+"\r\n";							//a=rtpmap
	mediaSdp += "a=fmtp:"+payloadType+" octet-align=1;\r\n";						//a=fmtp
	mediaSdp += "a=mpeg4-esid:"+esid+"\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:trackID="+streamid+"\r\n";

	return mediaSdp;
}

UINT MediaStreamAMR::TransportData(PBYTE pData, UINT dataSize, INT32 pts)
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return 0;

	UINT	mtu		= m_nMTU;
	
	int     i_max   = mtu - RTP_HEADER_SIZE - 2; // payload max in one packet 
    int     i_count = ( dataSize + i_max - 1 ) / i_max;

    PBYTE	p_data	= pData;
    int     i_data  = dataSize;
    int     i;

	UINT writeSize = 0;

    /* Only supports octet-aligned mode */
    for( i = 0; i < i_count; i++ )
    {
        int  i_payload = i_max < i_data ? i_max : i_data;
		
		m_Packet.ExtendBuffer( 2 + i_payload);

		/* Payload header */
        m_Packet.m_pData[0] = 0xF0; /* CMR */
        m_Packet.m_pData[1] = p_data[0]&0x7C; /* ToC */ /* FIXME: frame type */

        /* FIXME: are we fed multiple frames ? */
        memcpy( &m_Packet.m_pData[2], p_data+1, i_payload-1);

		m_Packet.m_nDataSize = 2 + i_payload - 1;

		//int iWrite = m_pRtpTransport->SetRtpData(m_Packet.m_pData, m_Packet.m_DataSize, pts, ((i == i_count - 1)?TRUE:FALSE) );
		int iWrite = m_pRtpTransport->Write(m_Packet.m_pData, m_Packet.m_nDataSize, m_nRtpPayloadType, pts, 0, ((i == i_count - 1)?TRUE:FALSE) );
		if (iWrite > 0)
			writeSize += iWrite;

        p_data += i_payload;
        i_data -= i_payload;
    }

    return writeSize;
}
