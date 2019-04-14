#include "stdafx.h"
#include "RtspResponse.h"

#include <algorithm>

RtspResponse::RtspResponse(SOCKET s, SOCKADDR_IN bindAddr, SOCKADDR_IN connectAddr)
{
	m_Socket = s;
	m_BindAddr = bindAddr;
	m_ConnectAddr = connectAddr;

	m_isOpen = TRUE;
	m_isConnect = TRUE;
}

RtspResponse::~RtspResponse()
{

}

void RtspResponse::ResponseOptions()
{
	AddField("Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE");
	SendResponse("");

	printf("\n");
}

void RtspResponse::ResponseDescribe(PCSTR sdp, UINT sdpLength)
{
	string contentBase;
	string contentType;
	string contentLength;
	string server;

	char temp[20];
	string requestMrl;

	server = "Server: RTSP Service";
	
	contentType = "Content-Type: application/sdp";

	_snprintf(temp, 20, "%lu", sdpLength);
	contentLength = "Content-Length: ";
	contentLength += temp;

	GetRequestMrl(&requestMrl);
	contentBase = "Content-Base: ";
	contentBase += requestMrl;

	AddField(server);
	AddField(contentBase);
	AddField(contentType);
	AddField(contentLength);
	SendResponse("");

	printf("\n");

	Tcp::Write((PBYTE)sdp, sdpLength);

	printf("Content:\n");
	printf(sdp);
	printf("\n\n");
}

void RtspResponse::ResponseSetup(	PCSTR serverIp, INT serverRtpPort,
									PCSTR targetIp, INT targetRtpPort,
									INT32 ssrc)
{
	string	transport;
	string	client_port;
	string	server_port;
	string	ssrc_;
	char	temp[100];

	if (!m_Session)
		m_Session = GenerateOneNumber();

	_snprintf(temp, 100, "server_port=%u-%u", serverRtpPort, serverRtpPort+1);
	server_port = temp;

	_snprintf(temp, 100, "client_port=%u-%u", targetRtpPort, targetRtpPort+1);
	client_port = temp;

	_snprintf(temp, 100, "ssrc=%u", ssrc);
	ssrc_ = temp;

	transport += "Transport: RTP/AVP;unicast;";
	transport += "source=";
	transport += serverIp;
	transport += ';';
	transport += server_port;
	transport += ';';
	transport += client_port;
	transport += ';';
	transport += ssrc_;
		
	AddField(transport);
	SendResponse("");

	printf("\n");
}

void RtspResponse::ResponsePlay(PCSTR setupUrl)
{
	string rtpinfo = "RTP-Info: ";
	rtpinfo += setupUrl;

	//string range = "Range: npt=now-";
	//AddField(range);
	AddField(rtpinfo);
	SendResponse("");

	printf("\n");
}

void RtspResponse::ResponsePause()
{
	SendResponse("");

	printf("\n");
}

void RtspResponse::ResponseTeardown()
{
	AddField("Connection: Close");
	SendResponse("");

	printf("\n");
}

void RtspResponse::ResponseGetParameter()
{

}

void RtspResponse::ResponseError(PCSTR error)
{
	SendResponse(error);
	printf("\n");
}

BOOL RtspResponse::GetRequests()
{
	string read;
	int iRead = 1;

	m_Requests.clear();

	while (iRead > 0)
	{
		iRead = Read(read);
		if (iRead > 0)
		{
			m_Requests.push_back(read);
		}
	}

	if (iRead < 0)
		return FALSE;
	
	string cseq;
	if ( SearchRequests(&cseq, "CSeq") )
		m_CSeq = atoi(cseq.c_str());

	return 	TRUE;
}

BOOL RtspResponse::GetRequestType(INT* pRequestType)
{
	string search;
	int requestType = 0;

	if ( !m_Requests.size() )
		return FALSE;

	if ( m_Requests[0].find("OPTIONS") != string::npos)
		requestType = requestOptions;
	else if( m_Requests[0].find("DESCRIBE") != string::npos)
		requestType = requestDescribe;
	else if( m_Requests[0].find("SETUP") != string::npos)
		requestType = requestSetup;
	else if( m_Requests[0].find("TEARDOWN") != string::npos)
		requestType = requestTeardown;
	else if( m_Requests[0].find("PLAY") != string::npos)
		requestType = requestPlay;
	else if( m_Requests[0].find("PAUSE") != string::npos)
		requestType = requestPause;
	else if( m_Requests[0].find("GET_PARAMETER") != string::npos)
		requestType = requestGetParameter;

	if (!requestType)
		return FALSE;

	if (pRequestType)
		*pRequestType = requestType;

	return TRUE;
}

BOOL RtspResponse::SearchRequests(string* pStr, string field)
{
	UINT iRequest;
	string search;
	string::size_type iFind;

	transform(field.begin(), field.end(), field.begin(), tolower);

	for (iRequest = 0; iRequest < m_Requests.size(); iRequest++)
	{
		search = m_Requests[iRequest];
		transform(search.begin(), search.end(), search.begin(), tolower);

		iFind = search.find( field );
		if (iFind != string::npos)
		{
			*pStr = m_Requests[iRequest];

			//去除头部多余字符
			pStr->erase(0, iFind + field.size());

			iFind = pStr->find_first_not_of(':');
			if (iFind != string::npos)
				pStr->erase(0, iFind);

			iFind = pStr->find_first_not_of(' ');
			if (iFind != string::npos)
				pStr->erase(0, iFind);			

			return TRUE;
		}
	}
	return TRUE;
}

BOOL RtspResponse::SendResponse(PCSTR responseType)
{
	string responseCmd;
	char cseq[256];
	char session[256];

	if (responseType)
		responseCmd = responseType;

	if (!responseCmd.length())
		responseCmd = "RTSP/1.0 200 OK";
		
	_snprintf(cseq, 256, "CSeq: %u", m_CSeq);
	_snprintf(session, 256, "Session: %I64u", m_Session);

	Write(responseCmd.c_str());
	Write(cseq);
	if (m_Session > 0)
		Write(session);
	WriteFields();
	Write("");

	return TRUE;
}

BOOL RtspResponse::GetRequestMrl(string* pMrl)
{
	string	mrl;
	string::size_type iFind;

	if ( !m_Requests.size() )
		return FALSE;

	mrl = m_Requests[0];
	transform(mrl.begin(), mrl.end(), mrl.begin(), tolower);

	//查找"rtsp:/"起始位置
	iFind = mrl.find("rtsp:/");
	if ( iFind == string::npos)
		return FALSE;

	mrl = m_Requests[0];
	mrl.erase(0, iFind);

	// 移除"RTSP/1.0"字符
	iFind = mrl.find("RTSP/1.0");
	if (iFind == string::npos)
		return FALSE;
	mrl.erase(iFind, 8);

	// 移除后面多余空格
	iFind = mrl.find_last_not_of(' ');
	if (iFind != string::npos)
		mrl.erase(iFind+1, mrl.size()-iFind);

	if (pMrl)
		*pMrl = mrl;

	return TRUE;
}

BOOL RtspResponse::GetRequestServerUrl(string* pUrl)
{
	string url;
	string::size_type iFind;

	if ( !GetRequestMrl(&url) )
		return FALSE;

	// 移除"rtsp:/"
	url.erase(0, 6);
	iFind = url.find_first_not_of('/');
	if (iFind == string::npos)
		return FALSE;
	url.erase(0, iFind); //"rtsp://

	iFind = url.find('/');
	if (iFind != string::npos)
	{
		url.erase(iFind, url.size()-iFind);
	}
	
	if (pUrl)
		*pUrl = url;

	return TRUE;
}

BOOL RtspResponse::GetRequestSessionName(string* pSessionName)
{
	string sessionName;
	string::size_type iFind;

	if ( !GetRequestMrl(&sessionName) )
		return FALSE;

	// 移除"rtsp:/"
	sessionName.erase(0, 6);
	iFind = sessionName.find_first_not_of('/');
	if (iFind == 1)
		sessionName.erase(0, iFind); //"rtsp://

	iFind = sessionName.find('/');
	if (iFind == string::npos)	//地址字符
		return FALSE;			//如果没有发现'/' 则没有session name
	
	sessionName.erase(0, iFind+1);

	//移除stream name
	iFind = sessionName.find('/');
	if (iFind != string::npos)
		sessionName.erase(iFind, sessionName.size()-iFind);

	//sessionName里含有trackID 说明没有sessionName
	iFind = sessionName.find("trackID=");
	if (iFind != string::npos)
		return FALSE;

	if (pSessionName)
		*pSessionName = sessionName;

	return TRUE;
}

BOOL RtspResponse::GetRequestStreamName(string* pStreamName)
{
	string streamName;
	string::size_type iFind;

	if ( !GetRequestMrl(&streamName) )
		return FALSE;

	// 移除"rtsp:/"
	streamName.erase(0, 6);
	iFind = streamName.find_first_not_of('/');
	if (iFind == 1)
		streamName.erase(0, iFind); //"rtsp://

	iFind = streamName.find('/');
	if (iFind != string::npos)	//地址字符
		streamName.erase(0, iFind+1);
	else
	{
		streamName = "";		//如果没有发现'/' 则没有session name
		return FALSE;
	}

	//查找stream name
	iFind = streamName.find('/');
	if (iFind == string::npos)
	{
		//sessionName里面出现trackID 说明是个streamName 没有sessionName
		iFind = streamName.find("trackID=");
		if (iFind == string::npos)
			return FALSE;
	}
	else
	{
		streamName.erase(0, iFind+1);
	}
	
	if (pStreamName)
		*pStreamName = streamName;
	return TRUE;
}

BOOL RtspResponse::GetRequestTransportInfo(INT* pRtpPort, INT* pRtcpPort)
{
	string	transport;
	int		rtpPort = 0;
	int		rtcpPort = 0;
	string::size_type iFind;

	if ( !SearchRequests(&transport, "Transport") )
		return FALSE;
	
	iFind = transport.find("client_port=");
	if (iFind == string::npos)
		return FALSE;
	transport.erase(0, iFind+12);

	iFind = transport.find(';');
	if (iFind != string::npos)
		transport.erase(iFind, transport.size()-iFind);

	rtpPort = atoi(transport.c_str());

	iFind = transport.find('-');
	if (iFind != string::npos)
		transport.erase(0, iFind+1);

	rtcpPort = atoi(transport.c_str());

	if (pRtpPort)
		*pRtpPort = rtpPort;
	if (pRtcpPort)
		*pRtcpPort = rtcpPort;

	return TRUE;
}

LONGLONG RtspResponse::GenerateOneNumber() 
{
	static LARGE_INTEGER	tickFrequency;
	static BOOL				tickFrequencySet = FALSE;

	LARGE_INTEGER	tickNow;

	if (tickFrequencySet == FALSE) 
	{
		QueryPerformanceFrequency(&tickFrequency);
		tickFrequencySet = TRUE;
	}
	QueryPerformanceCounter(&tickNow);

	//return (INT32)(tickNow.QuadPart / tickFrequency.QuadPart);
	return tickNow.QuadPart;
}