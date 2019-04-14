#ifndef __RTP_TRANSPORT_H__
#define __RTP_TRANSPORT_H__

#include "Rtp.h"
#include "VBufferT.h"

class RtpTransport
{
public:

	RtpTransport(UINT nBandWidth, UINT nMTU, UINT8 nPayloadType, INT32 nSSRC);

	virtual ~RtpTransport();

	virtual BOOL Setup(PCSTR psBindIp, UINT nBindPort, PCSTR psTargetIp, UINT nTargetPort);

	virtual BOOL Run();

	virtual BOOL Stop();

	virtual UINT SetRtpData(PBYTE pRtpData, UINT pRtpDataSize, INT32 nTimeStamp, BOOL nMarker = FALSE);
	
protected:
	void TransportThread();
	static UINT WINAPI RunTransportThread(PVOID pParam)
	{
		RtpTransport* pThis = (RtpTransport*)pParam;
		pThis->TransportThread();
		return 0;
	};
	
	struct RtpBuffer : public Buffer
	{
		UINT8	m_nPayloadType;
		INT32	m_nTimeStamp;
		INT32	m_nSSRC;
		BOOL	m_nMarker;
		
		RtpBuffer()
		{
			m_nPayloadType = 0;
			m_nTimeStamp = 0;
			m_nSSRC = 0;
			m_nMarker = FALSE;
		};
		
		virtual ~RtpBuffer()
		{
			
		};
		
		virtual BOOL FillData(PBYTE pData, UINT nDataSize, 
			UINT8 nPayloadType, INT32 nTimeStamp, INT32 nSSRC,  BOOL nMarker = FALSE)
		{
			m_nPayloadType = nPayloadType;
			m_nTimeStamp = nTimeStamp;
			m_nSSRC = nSSRC;
			m_nMarker = nMarker;
			
			return Buffer::FillData(pData, nDataSize);
		};
	};

	BOOL		m_bSetup;
	BOOL		m_bRun;

	Rtp*		m_pRtp;

	UINT		m_nMTU;
	UINT8		m_nPayloadType;
	INT32		m_nSSRC;

	UINT		m_nBandWidth;
	VBufferT	m_vBuffer;
	HANDLE		m_hTransport;
	HANDLE		m_eStopTransport;
};




#endif //__RTP_TRANSPORT_H__