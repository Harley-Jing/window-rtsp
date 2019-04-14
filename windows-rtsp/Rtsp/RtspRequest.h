#ifndef __RTSP_REQUEST_H__
#define __RTSP_REQUEST_H__

#include "Rtsp.h"

class RtspRequest : public Rtsp
{
public:
	RtspRequest();
	virtual ~RtspRequest();

	virtual BOOL Open(PCSTR mrl, PCSTR bindIp = "", INT bindPort = 0);

	virtual BOOL RequestOptions();
	virtual BOOL RequestDescribe(string* pDescribe);
	virtual BOOL RequestSetup(PCSTR setupName, INT transportMode, INT clientPort, INT clientRtcpPort, INT64* pSession);
	virtual BOOL RequestPlay();
	virtual BOOL RequestPause();
	//add function 修改日期：2019.4.11  	author : Jing AoLei
	virtual BOOL RequestGetParameter();	
	virtual BOOL RequestTeardown();

	virtual void Close();								   
	//add function 修改日期：2019.4.11  	author : Jing AoLei
	virtual BOOL SearchSdpInfo(string* pStrDst, string* pStrsrc, string field); 
protected:
	BOOL GetDescribe(string* pDescribe);
	BOOL GetResponses();
	BOOL SearchResponses(string* pStr, string field);

	void SendRequest(string requestType);

	BOOL ParseTransportField(string transport, int* pStreamingMode, int* pClientRtpPort, int* pClientRtcpPort);
	BOOL GenerateTransportField(string *pTransport, int streamingMode, int clientRtpPort, int clientRtcpPort);

	string  m_RequestsMrl;
	string  m_SetupName;
	vector<string> m_Responses;
};

#endif //__RTSP_REQUEST_H__