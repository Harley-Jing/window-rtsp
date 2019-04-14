#include "StdAfx.h"
#include "MediaStreamH263.h"

MediaStreamH263::MediaStreamH263(LPCSTR mediaStreamName) 
	:MediaStream(mediaStreamName, MEDIA_STREAM_TYPE_H263)
{
	m_nWidth = 0;
	m_nHeight = 0;
}

MediaStreamH263::~MediaStreamH263()
{
	m_Packet.FreeBuffer();
}

BOOL MediaStreamH263::Init(UINT nStreamBitrate, UINT nMTU, UINT nWidth, UINT nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	return MediaStream::Init(nStreamBitrate, nMTU);
}

string MediaStreamH263::GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP)
{
	string	mediaSdp;
	
	string	port;
	string	payloadType;
	string	bs;
	string	fmtp;
	string	streamid;
	string	esid;
	string	cliprect;

	char	temp[500];
	
	m_nRtpPayloadType = nRtpPayloadType;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType);
	payloadType	= temp;

	_snprintf(temp, 500, "%u", m_nRtpPort);
	port = temp;

	_snprintf(temp, 500, "%u", m_nBandWidth);
	bs = temp;
	
	_snprintf(temp, 500, "0,0,%u,%u", m_nHeight, m_nWidth);
	cliprect = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 1);
	streamid = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 201);
	esid = temp;

	// Éú³ÉsdpÄÚÈÝ
	mediaSdp += "m=video "+port+" RTP/AVP "+payloadType+"\r\n";						//m
	mediaSdp += "b=AS:"+bs+"\r\n";													//b																						
	mediaSdp += "a=rtpmap:"+payloadType+" H263-2000/90000\r\n";						//a=rtpmap
	mediaSdp += "a=fmtp:"+payloadType+" profile=0; level=10\r\n";					//a=fmtp
	mediaSdp += "a=cliprect:"+cliprect+"\r\n";										//a=cliprect
	mediaSdp += "a=mpeg4-esid:"+esid+"\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:trackID="+streamid+"\r\n";
	
	return mediaSdp;
}

UINT MediaStreamH263::TransportData(PBYTE pData, UINT dataSize, int pts)
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return 0;

	PBYTE	p_data	= pData;
    int     i_data  = dataSize;
    int     i;
	UINT	mtu		= m_nMTU;
    int     i_max   = mtu - RTP_HEADER_SIZE - 2; // payload max in one packet
    int     i_count;
    int     b_p_bit;
    int     b_v_bit = 0;	// no pesky error resilience
    int     i_plen = 0;		// normally plen=0 for PSC packet
    int     i_pebit = 0;	// because plen=0
    unsigned short h;

    if( i_data < 2 )
    {
        return -1;
    }
    if( p_data[0] || p_data[1] )
    {
        return -1;
    }
    /* remove 2 leading 0 bytes */
    p_data += 2;
    i_data -= 2;
    i_count = ( i_data + i_max - 1 ) / i_max;

	UINT writeSize = 0;

    for( i = 0; i < i_count; i++ )
    {
        int   i_payload =  i_max < i_data  ? i_max : i_data;

		m_Packet.ExtendBuffer(2 + i_payload);

        b_p_bit = (i == 0) ? 1 : 0;
        h = ( b_p_bit << 10 )|
            ( b_v_bit << 9  )|
            ( i_plen  << 3  )|
              i_pebit;

        /* h263 header */
        m_Packet.m_pData[0] = ( h >>  8 )&0xff;
        m_Packet.m_pData[1]  = ( h       )&0xff;
        memcpy( &m_Packet.m_pData[2], p_data, i_payload );

		m_Packet.m_nDataSize = 2 + i_payload;

        /* rtp common header */
        //b_m_bit = 1; // always contains end of frame
        //int iWrite = m_pRtpTransport->SetRtpData(m_Packet.m_pData, m_Packet.m_DataSize, pts, (i == i_count - 1)?TRUE:FALSE);
		int iWrite = m_pRtpTransport->Write(m_Packet.m_pData, m_Packet.m_nDataSize, m_nRtpPayloadType, pts, 0, (i == i_count - 1)?TRUE:FALSE);
		if (iWrite > 0)
			writeSize += iWrite;

        p_data += i_payload;
        i_data -= i_payload;
    }

    return writeSize;
}