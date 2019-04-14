#ifndef __RTSP_RESPONSE_H__
#define __RTSP_RESPONSE_H__

#include "Rtsp.h"

typedef enum RequestType
{
	requestOptions		= 0x001,
	requestDescribe		= 0x002,
	requestAnnounce		= 0x004,
	requestSetup		= 0x008,
	requestGetParameter = 0x010,
	requestSetParameter = 0x020,
	requestTeardown		= 0x040,
	requestPlay			= 0x080,
	requestPause		= 0x100,
	requestRecord       = 0x200
}requestType;

class RtspResponse : public Rtsp
{
public:
	RtspResponse(SOCKET s, SOCKADDR_IN bindAddr, SOCKADDR_IN connectAddr);
	virtual ~RtspResponse();

	virtual void ResponseOptions();
	virtual void ResponseDescribe(PCSTR sdp, UINT sdpLength);
	virtual void ResponseSetup(	PCSTR serverIp, INT serverRtpPort,
								PCSTR targetIp, INT targetRtpPort,
								INT32 ssrc);
	virtual void ResponseTeardown();
	virtual void ResponsePlay(PCSTR setupUrl);
	virtual void ResponsePause();
	virtual void ResponseGetParameter();
	virtual void ResponseError(PCSTR error);
	
	virtual BOOL SendResponse(PCSTR responseType);
	virtual BOOL SearchRequests(string* pStr, string field);

	virtual BOOL GetRequests();
	virtual BOOL GetRequestType(INT* pRequestType);
	virtual BOOL GetRequestMrl(string* pMrl);
	virtual BOOL GetRequestServerUrl(string* pUrl);
	virtual BOOL GetRequestSessionName(string* pSessionName);
	virtual BOOL GetRequestStreamName(string* pStreamName);
	virtual BOOL GetRequestTransportInfo(INT* pRtpPort, INT* pRtcpPort);

protected:
	LONGLONG GenerateOneNumber();

	vector<string> m_Requests;
};

#endif //__RTSP_RESPONSE_H__