// Rtsp.cpp: implementation of the libRtsp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdAfx.h"
#include "Rtsp.h"

#include "string_t.h"

Rtsp::Rtsp()
{
	m_CSeq = 0;
	m_State = 0;
	m_Session = 0;
}

Rtsp::~Rtsp()
{
}

int Rtsp::Read(string& str)
{
	char c;
	int iRead;

	str = "";

	// Receive until the peer closes the connection
	do 
	{
		iRead = Socket::Read((BYTE*)&c, 1);
		if (iRead == 1)
		{
			if ( c == '\r' || c == '\n')
				break;

			str.append(1, c);
		}
	} while (iRead == 1); //while (iRead > 0);  

	if (c == '\r')
		iRead = Socket::Read((BYTE*)&c, 1);

	// 套接字读取错误
	if (iRead == -1)
		return -1;

	// 空行代表回应结束
	if (str.size() == 0 && (c == '\r' || c == '\n') )
	{
		//printf("<< ''\n\n");
		printf("\n\n");
		return 0;
	}
	
	//if (!iRead || !pStr->size())
	//	return -1;
	if (str.size())
	{
		//printf("<< '%s'\n", str.c_str());
		printf("<< %s\n", str.c_str());
	}
		

	wstring wide_char = a2w_cp(str, CP_UTF8);
	string multi_char = w2a(wide_char);

	str = multi_char;
	
	return (UINT)str.size();
}

int Rtsp::Write(string str) 
{
	int iWrite;

	//printf(">> '%s'", str.c_str());
	if (str.size())
	{
		printf(">> %s", str.c_str());
	}
	

	wstring wide_char = a2w(str);
	string multi_utf8 = w2a_cp(wide_char, CP_UTF8);
	
	iWrite = Tcp::Write((PBYTE)multi_utf8.c_str(), multi_utf8.length());
	Tcp::Write((PBYTE)"\r\n", 2);

	//printf("done.\n");
	printf("\n");
	return iWrite;
}

void Rtsp::AddField(string field)
{
	m_Fields.push_back(field);
}

void Rtsp::WriteFields(string str)
{
	m_Fields.push_back(str);
	for (UINT iField = 0; iField < m_Fields.size(); iField++)
	{
		Write(m_Fields[iField]);
	}
	m_Fields.clear();
}

BOOL Rtsp::ParseMrl(string mrl, string* pPreSuffix, string* pSuffix, int* pPort)
{
	int port;
	string preSuffix;
	string suffix;
	string::size_type iFind, iFindEnd;

	iFind = mrl.find("rtsp://");
	if (iFind == string::npos)
	{
		printf("rtsp: bad url: %s\n", mrl.c_str());
		return FALSE;
	}
	
	mrl.erase(0, iFind + 7);	//remove "rtsp://"

	port = RTSP_PROTOCOL_PORT;	//标准rtsp协议端口
	iFind = mrl.find(':');
	if (iFind != string::npos)
	{
		iFindEnd = mrl.find('/', iFind);
		if (iFindEnd != string::npos)
		{
			port = atoi( mrl.substr(iFind + 1, iFindEnd - iFind).c_str() );
			mrl.erase(iFind, iFindEnd - iFind);
		}
	}

	iFind = mrl.find('/');
	if (iFind != string::npos)
	{
		preSuffix = mrl.substr(0, iFind - 0);
		mrl.erase(0, iFind + 1);
		suffix = mrl;
	}
	else
	{
		preSuffix = mrl;
	}

	if (pPreSuffix)
		*pPreSuffix = preSuffix;

	if (pSuffix)
		*pSuffix = suffix; 

	if (pPort)
		*pPort = port;

	return TRUE;
}

