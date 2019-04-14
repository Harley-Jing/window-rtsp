#include "StdAfx.h"
#include "MediaStream.h"

MediaStream:: MediaStream(PCSTR name, int type)	
	:m_Name(name),m_Type(type)
{
	m_nBandWidth = 0;

	m_nMTU = 0;
	m_nRtpPayloadType = 0;
	m_nRtpPort = 0;
	
	m_bInit = FALSE;
	m_bSetup = FALSE;
	m_bRun = FALSE;
	
	m_pRtpTransport = NULL;
}

MediaStream::~MediaStream()
{
	TransportTeardown();
}

BOOL MediaStream::Init(UINT nStreamBitrate, UINT nMTU)
{
	UINT nRtpPacketCount = nStreamBitrate/((nMTU-RTP_HEADER_SIZE)*8) + (nStreamBitrate%((nMTU-RTP_HEADER_SIZE)*8)) ? 1:0;
	UINT nRtpHeaderBitSize = nRtpPacketCount * RTP_HEADER_SIZE;

	m_nBandWidth = (nStreamBitrate*1.2+nRtpHeaderBitSize+999) / 1000;

	m_nMTU = nMTU;

	m_bInit = TRUE;
	return TRUE;
}

BOOL MediaStream::TransportSetup(PCSTR psBindIp, UINT nBindPort, PCSTR psTargetIp, UINT nTargetPort)
{
	if (!m_bInit)
		return FALSE;

	m_bSetup = FALSE;

	if (m_pRtpTransport != NULL)
		delete m_pRtpTransport;
	//m_pRtpTransport = new RtpTransport(m_nBandWidth, nMTU, m_nRtpPayloadType, m_nRtpPayloadType);
	m_pRtpTransport = new Rtp(m_nMTU);
	
	//if (m_pRtpTransport->Setup(psBindIp, nBindPort, psTargetIp, nTargetPort) == FALSE)
	//	return FALSE;
	if ( m_pRtpTransport->Open(psBindIp, nBindPort) == FALSE)
		return FALSE;

	if ( m_pRtpTransport->Connect(psTargetIp, nTargetPort) == FALSE)
		return FALSE;

	m_nRtpPort = nTargetPort;

	m_bSetup = TRUE;
	
	return TRUE;
}

BOOL MediaStream::TransportStart()
{
	if (m_bSetup == FALSE)
		return FALSE;

	ATLock atlock(&m_tlockRun);
	if (m_bRun == TRUE)
		return TRUE;

	//if (m_pRtpTransport == NULL || m_pRtpTransport->Run() == FALSE)
	//	return FALSE;

	m_bRun = TRUE;

	return TRUE;
}

BOOL MediaStream::TransportStop()
{
	ATLock atlock(&m_tlockRun);

	if (m_bRun == FALSE)
		return TRUE;

	//if (m_pRtpTransport == NULL)
	//	return FALSE;
	
	//if (m_pRtpTransport == NULL || m_pRtpTransport->Stop() == FALSE)
	//	return FALSE;

	m_bRun = FALSE;

	return TRUE;
}

BOOL MediaStream::TransportTeardown()
{
	TransportStop();

	if (m_pRtpTransport)
		delete m_pRtpTransport;
	m_pRtpTransport = NULL;

	return TRUE;
}

string MediaStream::GetName()
{
	return m_Name;
}

INT	MediaStream::GetType()
{
	return m_Type;
}

INT	MediaStream::GetRtpPayloadType()
{
	return m_nRtpPayloadType;
}

UINT MediaStream::GetBandWidth()
{
	return m_nBandWidth;
}

