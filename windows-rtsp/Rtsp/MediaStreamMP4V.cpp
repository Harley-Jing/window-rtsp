#include "StdAfx.h"
#include "MediaStreamMP4V.h"

#include "Bitstream.h"

#define MP4AV_MPEG4_SYNC		0x000001
#define MP4AV_MPEG4_VOL_START	0x20
#define MP4AV_MPEG4_VOSH_START	0xB0
#define MP4AV_MPEG4_VOSH_END    0xB1
#define MP4AV_MPEG4_USER_DATA_START 0xB2
#define MP4AV_MPEG4_GOV_START	0xB3
#define MP4AV_MPEG4_VO_START	0xB5
#define MP4AV_MPEG4_VOP_START	0xB6

MediaStreamMP4V::MediaStreamMP4V(LPCSTR mediaStreamName, BOOL bISMAStream) 
	:MediaStream(mediaStreamName, MEDIA_STREAM_TYPE_MP4V)
{
	m_bISMAStream = bISMAStream;

	m_bframes = FALSE;
	m_fps = 0.0; 
	m_nWidth = 0; 
	m_nHeight = 0;
	
}

MediaStreamMP4V::~MediaStreamMP4V()
{
	m_Packet.FreeBuffer();
}

BOOL MediaStreamMP4V::Init(UINT nStreamBitrate, UINT nMTU, 
						   UINT	nWidth, 
						   UINT	nHeight,
						   BOOL	bframes, 
						   double	fps)
{
	m_nWidth = nWidth; 
	m_nHeight = nHeight;
	m_bframes = bframes;
	m_fps = fps; 
	return MediaStream::Init(nStreamBitrate, nMTU);
}

string MediaStreamMP4V::GenerateMediaSdp(UINT nRtpPayloadType, BOOL bUseRTSP)
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

	PBYTE	pConfig;
	UINT	lConfig;
	UINT8	nTimeIncrBits;

	GenerateConfig(
		&pConfig, 
		&lConfig,
		&nTimeIncrBits, 
		false, 
		m_fps, 
		true, 
		m_nWidth, 
		m_nHeight);

	// profile_level_id, default is 3, Simple Profile @ Level 3
	UINT8 iProfileLevelId = 0xFE;

	// attempt to get a valid profile-level
	static UINT8 voshStartCode[4] = { 0x00, 0x00, 0x01, /*MP4AV_MPEG4_VOSH_START*/0xB0 };
	if (lConfig >= 5 && !memcmp(pConfig, voshStartCode, 4)) 
	{
		iProfileLevelId = pConfig[4];
	} 
	if (iProfileLevelId == 0xFE) 
	{
		//UINT8 iodProfileLevel = MP4GetVideoProfileLevel(mp4File);
		//if (iodProfileLevel > 0 && iodProfileLevel < 0xFE) 
		//{
		//	iProfileLevel = iodProfileLevel;
		//} 
		//else 
		//{
			iProfileLevelId = 1;
		//}
	} 

	UINT	lConfigHexa = lConfig*2+1;
    PCHAR	pConfigHexa = new char[lConfigHexa];

	sprintf_hexa( pConfigHexa, pConfig, lConfig);

	_snprintf(temp, 500, "profile-level-id=%u; config=%s", iProfileLevelId, pConfigHexa);
	fmtp = temp;

	delete []pConfig;
	delete []pConfigHexa;

	// Éú³ÉsdpÄÚÈÝ
	mediaSdp += "m=video "+port+" RTP/AVP "+payloadType+"\r\n";						//m
	mediaSdp += "b=AS:"+bs+"\r\n";													//b																						
	mediaSdp += "a=rtpmap:"+payloadType+" MP4V-ES/90000\r\n";						//a=rtpmap
	mediaSdp += "a=fmtp:"+payloadType+" "+fmtp+"\r\n";								//a=fmtp
	mediaSdp += "a=cliprect:"+cliprect+"\r\n";										//a=cliprect
	mediaSdp += "a=mpeg4-esid:"+esid+"\r\n";
	if (bUseRTSP)
		mediaSdp += "a=control:trackID="+streamid+"\r\n";
	//mediaSdp += "a=cliprect:0,0,144,176\r\n";
	//mediaSdp += "a=framesize:96 176-144\r\n";
	
	return mediaSdp;
}


void MediaStreamMP4V::GenerateConfig(PBYTE*	ppConfig, 
									 UINT*	pConfigLength, 
									 UINT8*	pTimeIncrBits,							  
									 bool	bframes, 
									 double	fps, 
									 bool	shortTime, 
									 UINT	width, 
									 UINT	height)
{
	PBYTE	pMpeg4Config = new BYTE[256];
	UINT	mpeg4ConfigLength = 0;
	UINT8	timeBits = 1;

	// profile_level_id, default is 3, Simple Profile @ Level 3
	CreateVosh(&pMpeg4Config, &mpeg4ConfigLength, 1);

	CreateVo(&pMpeg4Config, &mpeg4ConfigLength, 1);

	UINT vot = bframes ?  17 : 1;

	CreateVol(
		&pMpeg4Config,
		&mpeg4ConfigLength,
		vot,//vot, 
		fps,
		shortTime,	// short time - true if we haven't set the # of bits
		false,		// variableRate
		width,
		height,
		0,			// quantType, H.263
		&timeBits);

	*ppConfig = pMpeg4Config;
	*pConfigLength = mpeg4ConfigLength;
	*pTimeIncrBits = timeBits;
}

bool MediaStreamMP4V::CreateVosh(PBYTE* ppBytes, UINT32* pNumBytes, UINT8 profileLevel)
{
	Bitstream vosh;

	try {
		if (*ppBytes) {
			// caller must guarantee buffer against overrun
			memset((*ppBytes) + (*pNumBytes), 0, 5);
			vosh.SetBytes(*ppBytes, (*pNumBytes) + 5);
			vosh.SetBitPosition((*pNumBytes) << 3);
		} else {
			vosh.AllocBytes(5);
		}

		vosh.PutBits(MP4AV_MPEG4_SYNC, 24);
		vosh.PutBits(MP4AV_MPEG4_VOSH_START, 8);
		vosh.PutBits(profileLevel, 8);

		*ppBytes = vosh.GetBuffer();
		*pNumBytes = vosh.GetNumberOfBytes();
	}
	catch (int e) {
		return false;
	}

	return true;
}

bool MediaStreamMP4V::CreateVo(PBYTE* ppBytes, UINT32* pNumBytes, UINT8 objectId)
{
	Bitstream vo;

	try {
		if (*ppBytes) {
			// caller must guarantee buffer against overrun
			memset((*ppBytes) + (*pNumBytes), 0, 9);
			vo.SetBytes(*ppBytes, *pNumBytes + 9);
			vo.SetBitPosition((*pNumBytes) << 3);
		} else {
			vo.AllocBytes(9);
		}

		vo.PutBits(MP4AV_MPEG4_SYNC, 24);
		vo.PutBits(MP4AV_MPEG4_VO_START, 8);
		vo.PutBits(0x08, 8);	// no verid, priority, or signal type
		vo.PutBits(MP4AV_MPEG4_SYNC, 24);
		vo.PutBits(objectId - 1, 8);

		*ppBytes = vo.GetBuffer();
		*pNumBytes = vo.GetNumberOfBytes();
	}
	catch (int e) {
		return false;
	}

	return true;
}

bool MediaStreamMP4V::CreateVol(PBYTE*	ppBytes, 
								UINT*	pNumBytes,
								UINT8	profile, 
								double	frameRate, 
								bool	shortTime, 
								bool	variableRate,
								UINT16	width, 
								UINT16	height, 
								UINT8	quantType, 
								UINT8*	pTimeBits)
{
	Bitstream vol;

	try 
	{
		if (*ppBytes) 
		{
			// caller must guarantee buffer against overrun
			memset((*ppBytes) + (*pNumBytes), 0, 20);
			vol.SetBytes(*ppBytes, *pNumBytes + 20);
			vol.SetBitPosition((*pNumBytes) << 3);
		}
		else
		{
			vol.AllocBytes(20);
		}

		/* VOL - Video Object Layer */
		vol.PutBits(MP4AV_MPEG4_SYNC, 24);
		vol.PutBits(MP4AV_MPEG4_VOL_START, 8);

		/* 1 bit - random access = 0 (1 only if every VOP is an I frame) */
		vol.PutBits(0, 1);
		/*
		 * 8 bits - type indication 
		 * 		= 1 (simple profile)
		 * 		= 4 (main profile)
		 */
		vol.PutBits(profile, 8);
		/* 1 bit - is object layer id = 1 */
		vol.PutBits(1, 1);
		/* 4 bits - visual object layer ver id = 1 */
		vol.PutBits(1, 4); 
		/* 3 bits - visual object layer priority = 1 */
		vol.PutBits(1, 3); 

		/* 4 bits - aspect ratio info = 1 (square pixels) */
		vol.PutBits(1, 4);
		/* 1 bit - VOL control params = 0 */
		vol.PutBits(0, 1);
		/* 2 bits - VOL shape = 0 (rectangular) */
		vol.PutBits(0, 2);
		/* 1 bit - marker = 1 */
		vol.PutBits(1, 1);

		UINT16 ticks;
		if (shortTime /* && frameRate == (float)((int)frameRate) */) {
			ticks = (UINT16)(frameRate + 0.5);
		} else {
			ticks = 30000;
		}
		/* 16 bits - VOP time increment resolution */
		vol.PutBits(ticks, 16);
		/* 1 bit - marker = 1 */
		vol.PutBits(1, 1);

		UINT8 rangeBits = 1;
		while (ticks > (1 << rangeBits)) {
			rangeBits++;
		}
		if (pTimeBits) {
			*pTimeBits = rangeBits;
		}

		/* 1 bit - fixed vop rate = 0 or 1 */
		if (variableRate) {
			vol.PutBits(0, 1);
		} else {
			vol.PutBits(1, 1);

			UINT16 frameDuration = 
				(UINT16)((double)ticks / frameRate);

			/* 1-16 bits - fixed vop time increment in ticks */
			vol.PutBits(frameDuration, rangeBits);
		}
		/* 1 bit - marker = 1 */
		vol.PutBits(1, 1);
		/* 13 bits - VOL width */
		vol.PutBits(width, 13);
		/* 1 bit - marker = 1 */
		vol.PutBits(1, 1);
		/* 13 bits - VOL height */
		vol.PutBits(height, 13);
		/* 1 bit - marker = 1 */
		vol.PutBits(1, 1);
		/* 1 bit - interlaced = 0 */
		vol.PutBits(0, 1);

		/* 1 bit - overlapped block motion compensation disable = 1 */
		vol.PutBits(1, 1);
#if 0
		/* 2 bits - sprite usage = 0 */
		vol.PutBits(0, 2);
#else
		vol.PutBits(0, 1);
#endif
		/* 1 bit - not 8 bit pixels = 0 */
		vol.PutBits(0, 1);
		/* 1 bit - quant type = 0 */
		vol.PutBits(quantType, 1);
		if (quantType) {
			/* 1 bit - load intra quant mat = 0 */
			vol.PutBits(0, 1);
			/* 1 bit - load inter quant mat = 0 */
			vol.PutBits(0, 1);
		}
#if 0
		/* 1 bit - quarter pixel = 0 */
		vol.PutBits(0, 1);
#endif
		/* 1 bit - complexity estimation disable = 1 */
		vol.PutBits(1, 1);
		/* 1 bit - resync marker disable = 1 */
		vol.PutBits(1, 1);
		/* 1 bit - data partitioned = 0 */
		vol.PutBits(0, 1);
#if 0
		/* 1 bit - newpred = 0 */
		vol.PutBits(0, 1);
		/* 1 bit - reduced resolution vop = 0 */
		vol.PutBits(0, 1);
#endif
		/* 1 bit - scalability = 0 */
		vol.PutBits(0, 1);

		/* pad to byte boundary with 0 then as many 1's as needed */
		vol.PutBits(0, 1);
		if ((vol.GetBitPosition() & 7) != 0) {
			vol.PutBits(0xFF, 8 - (vol.GetBitPosition() & 7));
		}

		*ppBytes = vol.GetBuffer();
		*pNumBytes = vol.GetBitPosition() >> 3;
	}
	catch (int e) {
		return false;
	}

	return true;
}

UINT MediaStreamMP4V::TransportData(PBYTE pData, UINT dataSize, int pts)
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return 0;

	UINT	mtu		= m_nMTU;
    int     i_max   = mtu - RTP_HEADER_SIZE; // payload max in one packet 
    int     i_count = ( dataSize + i_max - 1 ) / i_max;

    PBYTE	p_data = pData;
    int     i_data  = dataSize;
    int     i;

	UINT writeSize = 0;

    for( i = 0; i < i_count; i++ )
    {
        int i_payload = i_max < i_data ? i_max : i_data;

		//int iWrite = m_pRtpTransport->SetRtpData(p_data, i_payload, pts, ((i == i_count - 1)?TRUE:FALSE));
		int iWrite = m_pRtpTransport->Write(p_data, i_payload, m_nRtpPayloadType, pts, 0, ((i == i_count - 1)?TRUE:FALSE));
		if (iWrite > 0)
			writeSize += iWrite;

        p_data += i_payload;
        i_data -= i_payload;
    }

    return writeSize;
}