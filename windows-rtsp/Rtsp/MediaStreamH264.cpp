#include "StdAfx.h"
#include "MediaStreamH264.h"
#include "BaseEncoder.h"
MediaStreamH264::MediaStreamH264(LPCSTR mediaStreamName)
		:MediaStream(mediaStreamName, MEDIA_STREAM_TYPE_H264)
{
	m_nWidth = 0;
	m_nHeight = 0;
}

MediaStreamH264::~MediaStreamH264()
{
	m_Packet.FreeBuffer();
}

BOOL MediaStreamH264::Init(UINT nStreamBitrate, UINT nMTU, UINT nWidth, UINT nHeight, int profileLevelId, const string& spropParameterSets)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_ProfileLevelId = profileLevelId;
	m_SpropParameterSets = spropParameterSets;
	return MediaStream::Init(nStreamBitrate, nMTU);
}

string MediaStreamH264::GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP)
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


	// 生成各字段的内容
	_snprintf(temp, 500, "%u", m_nRtpPort);
	port			= temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType);
	payloadType		= temp;

	_snprintf(temp, 500, "profile-level-id=%06X; sprop-parameter-sets=%s; packetization-mode=1;", 
																m_ProfileLevelId, m_SpropParameterSets.c_str());
	fmtp			= temp;

	_snprintf(temp, 500, "0,0,%u,%u", m_nHeight, m_nWidth);
	cliprect = temp;

	_snprintf(temp, 500, "%u", m_nBandWidth);
	bs = temp;
	
	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 1);
	streamid = temp;

	_snprintf(temp, 500, "%u", m_nRtpPayloadType - 96 + 201);
	esid = temp;

	// 生成sdp内容
	mediaSdp += "m=video "+port+" RTP/AVP "+payloadType+"\r\n";	//m
	mediaSdp += "b=AS:"+bs+"\r\n";													//b																						
	mediaSdp += "a=rtpmap:"+payloadType+" H264/90000\r\n";							//a=rtpmap
	mediaSdp += "a=fmtp:"+payloadType+" "+fmtp+"\r\n";								//a=fmtp
	mediaSdp += "a=cliprect:"+cliprect+"\r\n";										//a=cliprect
	mediaSdp += "a=mpeg4-esid:"+esid+"\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:trackID="+streamid+"\r\n";
	
	return mediaSdp;
}

UINT MediaStreamH264::TransportData(PBYTE pData, UINT dataSize, int pts)
{
	PBYTE p_buffer = pData;
	int	i_buffer = dataSize;

	UINT writeSize = 0;

	while( i_buffer > 4 && ( p_buffer[0] != 0 || p_buffer[1] != 0 || p_buffer[2] != 1 ) )
	{
		i_buffer--;
		p_buffer++;
	}

	/* Split nal units */
	while( i_buffer > 4 )
	{
		int i_offset;
		int i_size = i_buffer;
		int i_skip = i_buffer;

		/* search nal end */
		for( i_offset = 4; i_offset+2 < i_buffer ; i_offset++)
		{
			if( p_buffer[i_offset] == 0 && p_buffer[i_offset+1] == 0 && p_buffer[i_offset+2] == 1 )
			{
				/* we found another startcode */
				i_size = i_offset - ( p_buffer[i_offset-1] == 0 ? 1 : 0);
				i_skip = i_offset;
				break;
			} 
		}
		/* TODO add STAP-A to remove a lot of overhead with small slice/sei/... */
		UINT iWrite = TransportH264Nal(p_buffer, i_size, pts, (i_size >= i_buffer) );
		if (iWrite > 0 )
			writeSize += iWrite;

		i_buffer -= i_skip;
		p_buffer += i_skip;
	}
	return writeSize;
}

UINT MediaStreamH264::TransportH264Nal(const PBYTE pNal, UINT nalSize, INT32 pts, BOOL isLast)
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return 0;

	if( nalSize < 5 )
		return 0;

	UINT	mtu = m_nMTU;

	const int i_max = mtu - RTP_HEADER_SIZE; /* payload max in one packet */
	int i_nal_hdr;
	int i_nal_type;

	i_nal_hdr = pNal[3];
	i_nal_type = i_nal_hdr&0x1f;
	
	string sps;
	string pps;

	if( i_nal_type == 7 || i_nal_type == 8 )
	{
		/* XXX Why do you want to remove them ? It will break streaming with 
		* SPS/PPS change (broadcast) ? */
		return 0;
	}

	/* Skip start code */
	PBYTE p_data = pNal;
	int	i_data = nalSize;

	p_data += 3;
	i_data -= 3;

	int writeSize = 0;

	if( i_data <= i_max )
	{
		/* Single NAL unit packet */
		//writeSize = m_pRtpTransport->SetRtpData(p_data, i_data, pts, isLast);
		writeSize = m_pRtpTransport->Write(p_data, i_data, m_nRtpPayloadType, pts, 0, isLast);
		if (writeSize <= 0)
			return 0;
		return writeSize;
	}
	else
	{
		/* FU-A Fragmentation Unit without interleaving */
		const int i_count = ( i_data-1 + i_max-2 - 1 ) / (i_max-2);
		int i;

		p_data++;
		i_data--;

		for( i = 0; i < i_count; i++ )
		{
			const int i_payload =  (i_data < (i_max-2)) ? i_data : (i_max-2);
			const int nalSize = 2 + i_payload;

			m_Packet.ExtendBuffer(nalSize);

			/* FU indicator */
			m_Packet.m_pData[0] = 0x00 | (i_nal_hdr & 0x60) | 28;
			/* FU header */
			m_Packet.m_pData[1] = ( i == 0 ? 0x80 : 0x00 ) | ( (i == i_count-1) ? 0x40 : 0x00 )  | i_nal_type;

			/* FU payload */
			memcpy( &m_Packet.m_pData[2], p_data, i_payload );

			m_Packet.m_nDataSize = nalSize;

			//int iWrite = m_pRtpTransport->SetRtpData(m_Packet.m_pData, m_Packet.m_DataSize, pts, isLast && (i == i_count-1));
			int iWrite = m_pRtpTransport->Write(m_Packet.m_pData, m_Packet.m_nDataSize, m_nRtpPayloadType, pts, 0, isLast && (i == i_count-1));
			if (iWrite > 0)
				writeSize += iWrite;

			i_data -= i_payload;
			p_data += i_payload;
		}
	}
	return writeSize;
}
