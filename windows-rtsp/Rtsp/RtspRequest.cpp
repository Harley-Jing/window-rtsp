#include "stdafx.h"
#include "RtspRequest.h"

#include <algorithm>

RtspRequest::RtspRequest()
{
	m_State = stateInit;
}

RtspRequest::~RtspRequest()
{
	Close();
}

BOOL RtspRequest::Open(PCSTR mrl, PCSTR bindIp, INT bindPort)
{
	if (m_State > stateInit)
		return FALSE;

	string preSuffix;
	string suffix;
	INT	   port;

	m_RequestsMrl = mrl;

	if ( !Rtsp::ParseMrl(m_RequestsMrl, &preSuffix, &suffix, &port) ) 
		return FALSE;

	if ( !Tcp::Open(bindIp, bindPort) )
		return FALSE;

	if ( !Tcp::Connect(preSuffix.c_str(), port) )
		return FALSE;

	printf("\nConnect server: %s port:%i addr:%s\n\n", preSuffix.c_str(), port, suffix.c_str());

	m_State = stateConnected;

	//if ( !RequestOptions() )
	//	return FALSE;

	return TRUE;
}

BOOL RtspRequest::RequestOptions()
{
	if (m_State < stateConnected)
		return FALSE;

	SendRequest("OPTIONS");

	printf("\n");

	if ( !GetResponses() )
		return FALSE;

	return TRUE;
}


BOOL RtspRequest::RequestDescribe(string* pDescribe)
{
	if (m_State < stateConnected)
		return FALSE;

	SendRequest("DESCRIBE");

	printf("\n");

	if ( !GetResponses() )
		return FALSE;

	if ( !GetDescribe(pDescribe) )
		return FALSE;
	
	return TRUE;
}

BOOL RtspRequest::RequestSetup(PCSTR setupName, INT transportMode, INT clientPort, INT clientRtcpPort, INT64* pSession)
{
	if (m_State < stateConnected)
		return FALSE;

	string transportField;

	if (setupName == NULL)
		m_SetupName = "";
	else
		m_SetupName = setupName;

	if ( !GenerateTransportField(&transportField, transportMode, clientPort, clientRtcpPort) )
		return FALSE;

	AddField(transportField);
	SendRequest("SETUP");

	printf("\n");

	if ( !GetResponses() )
		return FALSE;

	m_State = stateReady;

	if (pSession)
		*pSession = m_Session;

	return TRUE;
}

//Range头可能包含一个时间参数。该参数以UTC格式指定了播放开始的时间。
//如果在这个指定时间后收到消息，那么播放立即开始。
//时间参数可能用来帮助同步从不同数据源获取的数据流。

//不含Range头的PLAY请求也是合法的。它从媒体流开头开始播放，直到媒体流被暂停。
//如果媒体流通过PAUSE暂停，媒体流传输将在暂停点（the pause point）重新开始
BOOL RtspRequest::RequestPlay()
{
	if (m_State < stateReady)
		return FALSE;

	SendRequest("PLAY");

	printf("\n");

	if ( !GetResponses() )
		return FALSE;
	
	m_State = statePlaying;

	return TRUE;
}

BOOL RtspRequest::RequestPause()
{
	if ( m_State < statePlaying)
		return FALSE;

	SendRequest("PAUSE");

	printf("\n");

	if ( !GetResponses() )
		return FALSE;

	m_State = statePause;

	return TRUE;
}

BOOL RtspRequest::RequestGetParameter()
{
	if (m_State < stateReady)
		return FALSE;

	SendRequest("GET_PARAMETER");

	printf("\n");

	if (!GetResponses())
		return FALSE;

	m_State = statePlaying;
}

BOOL RtspRequest::RequestTeardown()
{
	if (m_State < stateConnected)
		return FALSE;

	SendRequest("TEARDOWN");

	printf("\n");

	if ( !GetResponses() )
		return FALSE;

	m_State = stateInit;

	return TRUE;
}

void RtspRequest::Close()
{
	Tcp::Close();

	m_State = stateInit;

	printf("\nDisconnection server!\n");
}


BOOL RtspRequest::GetDescribe(string* pDescribe)
{
	BYTE* pDescribeBuffer = NULL;
	int describeSize;
	string describe;
	string searchField;

	if ( !SearchResponses(&searchField, "Content-Length") )
		return FALSE;

	describeSize = atoi( searchField.c_str() );
	pDescribeBuffer = new BYTE[describeSize + 1];
	if (!pDescribeBuffer)
		return FALSE;
	memset(pDescribeBuffer, 0, describeSize);

	int iResult = Tcp::Read(pDescribeBuffer, describeSize);
	if (iResult != describeSize)
	{
		delete []pDescribeBuffer;
		return FALSE;
	}
	pDescribeBuffer[describeSize] = '\0';

	*pDescribe = (char*)pDescribeBuffer;

	delete []pDescribeBuffer;

	printf("%s\n\n", pDescribe->c_str());

	return TRUE;
}

void RtspRequest::SendRequest(string requestType) 
{
	string requestCmd;
	char cseq[256];
	char session[256];

	m_CSeq++;

	if ( m_SetupName.length() )
	{
		requestCmd = requestType;
		requestCmd += " ";
		requestCmd += m_RequestsMrl;
		requestCmd += "/";
		requestCmd += m_SetupName;
		requestCmd += " ";
		requestCmd += "RTSP/1.0";

		m_SetupName = "";
	}
	else
	{
		requestCmd = requestType;
		requestCmd += " ";
		requestCmd += m_RequestsMrl;
		requestCmd += " ";
		requestCmd += "RTSP/1.0";
	}

	_snprintf(cseq, 256, "CSeq: %u", m_CSeq);

	if (requestType.compare("TEARDOWN") == 0)
		m_Session = 0;
	_snprintf(session, 256, "Session: %I64u", m_Session);

	Write(requestCmd.c_str());
	Write(cseq);
	/*
	*	User-Agent
	*/
	WriteFields("User-Agent: MyRtspClient");

	if (m_Session > 0)
		Write(session);


	if ((requestType.compare("DESCRIBE") == 0))
	{
		WriteFields("Accept: application/sdp");
	}
	Write("");
}

BOOL RtspRequest::GetResponses()
{	
	string str;
	string::size_type iFind;
	int cseq;
	INT64 session;
	int iRead = 1;

	m_Responses.clear();

	while(iRead > 0)
	{
		iRead = Read(str);
		if (iRead > 0)
		{
			iFind = str.find("CSeq:");
			if ( iFind != -1 )
			{
				cseq = atoi( str.substr(iFind+5).c_str() );
				if ( m_CSeq != cseq)
				{
					printf("warning: CSeq mismatch. got %u, assumed %u\n", cseq, m_CSeq);
					m_CSeq = cseq;
				}
			}

			iFind = str.find("Session:");
			if ( iFind != -1 )
			{
				session = _atoi64( str.substr(iFind+8).c_str() );
				if ( m_Session != session)
				{
					m_Session = session;
					//printf("setting session id to: %I64u\n", m_Session);
				}
			}

			m_Responses.push_back(str);
		}	
	}

	if ( !m_Responses.size() )
		return FALSE;

	if ( m_Responses[0].find("RTSP/1.0 200 OK") == -1)
		return FALSE;

	return TRUE;
}

BOOL RtspRequest::SearchResponses(string* pStr, string field)
{
	UINT iResponse;
	string::size_type iFind;
	string search;

	transform(field.begin(), field.end(), field.begin(), tolower);

	for (iResponse = 0; iResponse < m_Responses.size(); iResponse++)
	{
		search = m_Responses[iResponse];
		transform(search.begin(), search.end(), search.begin(), tolower);

		iFind = search.find( field );
		if (iFind != string::npos)
		{
			*pStr = m_Responses[iResponse];

			//去除头部多余字符
			pStr->erase(0, iFind + field.size());

			iFind = pStr->find_first_not_of(':');
			if (iFind >= 0)
				pStr->erase(0, iFind);

			iFind = pStr->find_first_not_of(' ');
			if (iFind >= 0)
				pStr->erase(0, iFind);			

			return TRUE;
		}
	}
	return FALSE;
}

BOOL RtspRequest::SearchSdpInfo(string* pStrDst, string* pStrsrc, string field)
{
	char* pSearchBuffer = NULL;
	string::size_type iFind;
	int iSize;

	pSearchBuffer = new char[pStrsrc->size() + 1];
	if (!pSearchBuffer)
		return FALSE;
	memset(pSearchBuffer, 0, pStrsrc->size());

	strcpy(pSearchBuffer, pStrsrc->c_str());
	pSearchBuffer[pStrsrc->size()] = '\0';
	*pStrDst = (char*)pSearchBuffer;

	delete[]pSearchBuffer;

	iFind = pStrDst->find(field);
	if (iFind != string::npos)
	{
		//去除头部多余字符
		pStrDst->erase(0, iFind);

		iFind = pStrDst->find_first_not_of(':');
		if (iFind >= 0)
			pStrDst->erase(0, iFind);

		iFind = pStrDst->find_first_not_of(' ');
		if (iFind >= 0)
			pStrDst->erase(0, iFind);

		//去除尾部多余字符
		iFind = pStrDst->find_first_of('\r\n');
		if (iFind > 0)
			pStrDst->erase(iFind, pStrDst->size());

		return TRUE;
	}

	return TRUE;
}

BOOL RtspRequest::GenerateTransportField(string *pTransport, int streamingMode, int clientRtpPort, int clientRtcpPort)
{
	char temp[10];

	if ( !pTransport )
		return FALSE;

	*pTransport = "Transport: ";

	switch( streamingMode )
	{
	case transportModeRtpUdp:
		*pTransport += ("RTP/AVP;");
		break;
	case transportModeRtpTcp:
		*pTransport += "RTP/AVP/TCP;";
		break;
	case transportModeRawUdp:
		*pTransport += "RAW/RAW/UDP;";
		break;
	default:
		return FALSE;
	}
	
	*pTransport += "unicast;";
	*pTransport += "client_port=";

	_snprintf(temp, 10, "%u", clientRtpPort);
	*pTransport += temp;

	if (clientRtcpPort)
	{
		_snprintf(temp, 10, "%u", clientRtcpPort);
		
		*pTransport += "-";
		*pTransport += temp;
	}
	
	return TRUE;
}