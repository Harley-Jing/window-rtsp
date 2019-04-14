#include "StdAfx.h"
#include "RtpTransport.h"

#include <process.h>

RtpTransport::RtpTransport(UINT nBandWidth, UINT nMTU, UINT8 nPayloadType, INT32 nSSRC)
{
	m_nBandWidth = nBandWidth;
	m_nMTU = nMTU;
	m_nPayloadType = nPayloadType;
	m_nSSRC = nSSRC;

	m_bSetup = FALSE;
	m_bRun = FALSE;
	m_hTransport = NULL;
	m_eStopTransport = NULL;

	m_pRtp = NULL;
}

RtpTransport::~RtpTransport()
{
	Stop();

	if (m_pRtp != NULL)
		delete m_pRtp;
}

BOOL RtpTransport::Setup(PCSTR psBindIp, UINT nBindPort, PCSTR psTargetIp, UINT nTargetPort)
{
	if (m_bRun == TRUE)
		Stop();

	m_bSetup = FALSE;

	if (m_pRtp != NULL)
		delete m_pRtp;
	m_pRtp = new Rtp(m_nMTU);
	
	if ( m_pRtp->Open(psBindIp, nBindPort) == FALSE)
		return FALSE;

	if ( m_pRtp->Connect(psTargetIp, nTargetPort) == FALSE)
		return FALSE;
	
	m_bSetup = TRUE;

	return TRUE;
}

BOOL RtpTransport::Run()
{
	if (m_bSetup == FALSE)
		return FALSE;

	if (m_bRun == TRUE)
		return TRUE;

	m_vBuffer.FreeBuffer();

	m_eStopTransport = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hTransport = (HANDLE)_beginthreadex(NULL, 0, RunTransportThread, this, 0, NULL);

	m_bRun = TRUE;

	return TRUE;
}

BOOL RtpTransport::Stop()
{
	if (m_bRun == FALSE)
		return TRUE;

	SetEvent(m_eStopTransport);
	WaitForSingleObject(m_hTransport, INFINITE);

	CloseHandle(m_eStopTransport);
	CloseHandle(m_hTransport);

	m_eStopTransport = NULL;
	m_hTransport = NULL;

	m_bRun = FALSE;

	return TRUE;
}

void RtpTransport::TransportThread()
{
	DWORD nWait;

	DWORD nBeginTick;
	DWORD nCurrTick;

	UINT64 nSendSize;

	DWORD nTransportSpeed;
	DWORD nBandWidth;

	nSendSize = 0;
	nTransportSpeed = 0;
	nBandWidth = m_nBandWidth * 1000;
	nBeginTick = GetTickCount();

	while(TRUE)
	{
		nWait = WaitForSingleObject(m_eStopTransport, 0);
		if (nWait == WAIT_OBJECT_0)
			break;

		// 控制发送码率
		nCurrTick = GetTickCount();
		if ( nCurrTick < nBeginTick)
		{
			nBeginTick = nCurrTick;		// 49天后归零
			nSendSize = 0;
		}
	
		if ( nCurrTick - nBeginTick == 0) 
			nTransportSpeed = 0;
		else 
			nTransportSpeed = (DWORD)(nSendSize / ( nCurrTick-nBeginTick));
	
		if ( nTransportSpeed > nBandWidth) 
		{
			Sleep(10);
			continue;
		}	

		RtpBuffer* pBuffer = (RtpBuffer*)m_vBuffer.GetFullBuffer();
		if ( pBuffer == NULL)
		{
			Sleep(10);
			continue;
		}

		if ( m_pRtp->Write(pBuffer->m_pData, pBuffer->m_nDataSize, 
			pBuffer->m_nPayloadType, pBuffer->m_nTimeStamp, pBuffer->m_nSSRC, pBuffer->m_nMarker) != pBuffer->m_nDataSize)
			printf("\nRtp write error.\n");

		nSendSize += pBuffer->m_nDataSize;

		m_vBuffer.AddEmptyBuffer(pBuffer);
	}
}

UINT RtpTransport::SetRtpData(PBYTE pRtpData, UINT nRtpDataSize, INT32 nTimeStamp, BOOL nMarker)
{
	if (pRtpData == NULL || nRtpDataSize > m_nMTU)
		return 0;

	if (m_vBuffer.GetDataSize() > m_nBandWidth*10) // 只缓存5秒数据
	{
		printf("\nRtpTransport	:Buffer too large, drop the data.\n");
		return 0;
	}

	RtpBuffer* pBuffer = (RtpBuffer*)m_vBuffer.GetEmptyBuffer();
	if (pBuffer == NULL)
		pBuffer = new RtpBuffer();

	pBuffer->FillData(pRtpData, nRtpDataSize, m_nPayloadType, nTimeStamp, m_nSSRC, nMarker);

	m_vBuffer.AddFullBuffer(pBuffer);

	return nRtpDataSize;
}
