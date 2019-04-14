#include "stdafx.h"
#include "RtspTransport.h"

#include <process.h>

RtspTransport::RtspTransport()
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup(wVersionRequested, &wsaData);

	m_isOpen = FALSE;
	m_hListenThread = NULL;
	m_isStopListenThread = NULL;

	m_BindPort = 0;
	m_MaxConnects = 0;
}

RtspTransport::~RtspTransport(void)
{
	Close();

	WSACleanup();
}

BOOL RtspTransport::Open(PCSTR bindIp, UINT bindPort, UINT maxConnects)
{
	if (m_isOpen)
		return TRUE;

	m_BindIp = bindIp;
	m_BindPort = bindPort;
	m_MaxConnects = maxConnects;

	if (m_Socket)
		closesocket(m_Socket);
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if (m_Socket == -1) 
	{
		printf("failed to create socket");
		Close();
		return FALSE;
	}

	ULONG nonBlock = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &nonBlock)) 
	{
		printf("can't put socket in non-blocking mode");
		Close();
		return FALSE;
	}

	m_BindAddr.sin_family = AF_INET;
	m_BindAddr.sin_port = htons(m_BindPort);
	m_BindAddr.sin_addr.s_addr = inet_addr(m_BindIp.c_str());

	if (bind(m_Socket, (SOCKADDR*)&m_BindAddr, sizeof(m_BindAddr)) < 0)
	{
		printf("bind error. WSAGetLastError() = %d\n", WSAGetLastError());
		Close();
		return FALSE;
	}

	if (listen(m_Socket, SOMAXCONN) < 0)
	{
		printf("listen error. WSAGetLastError() = %d\n", WSAGetLastError());
		Close();
		return FALSE;
	}

	m_isStopListenThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, StartListenThread, this, 0, NULL);

	printf("\nRtspTransport:	Begin listen connect. ip address:%s  port:%d\n", m_BindIp.c_str(), m_BindPort);

	m_isOpen = TRUE;
	return TRUE;
}

BOOL RtspTransport::Close()
{
	if (!m_isOpen)
		return TRUE;

	SetEvent(m_isStopListenThread);
	WaitForSingleObject(m_hListenThread, INFINITE);
	CloseHandle(m_hListenThread);
	CloseHandle(m_isStopListenThread);
	m_hListenThread = NULL;
	m_isStopListenThread = NULL;

	for(UINT32 _iPtr = 0; _iPtr < m_RtspSessionList.size(); _iPtr++) {
		delete m_RtspSessionList[_iPtr]; 
	}
	m_RtspSessionList.clear();

	if (m_Socket)
		closesocket(m_Socket);
	m_Socket = NULL;

	m_isOpen = FALSE;

	printf("\nRtspTransport:	listen thread is closed!\n");

	return TRUE;
}

UINT RtspTransport::StartListenThread(PVOID param)
{
	RtspTransport* pThis = (RtspTransport*)param;
	pThis->ListenThread();
	return 0;
}

void RtspTransport::ListenThread()
{
	SOCKET		connect;
	SOCKADDR_IN connectAddr;
	int			addrSize = sizeof(connectAddr);
	int			wait;
	UINT		i = 0;

	while(TRUE)
	{
		wait = WaitForSingleObject(m_isStopListenThread, 100);
		if (WAIT_OBJECT_0 == wait)
		{
			break;
		}
		
		// 删除已经断开的会话
		for( i = 0; i < m_RtspSessionList.size(); i ++)
		{
			if (m_RtspSessionList[i]->RtspSessionIsClose() )
			{
				delete m_RtspSessionList[i];
				m_RtspSessionList.erase(m_RtspSessionList.begin()+i);
				i--;
				printf("\nRtspTransport:	Connect session is closed!\n");
			}
		}		
		
		// 建立新的会话
		connect = accept(m_Socket, (struct sockaddr*)&connectAddr, &addrSize);

		if(connect == SOCKET_ERROR || m_RtspSessionList.size() >= m_MaxConnects)
			continue;

		RtspSession* rtspSession = new RtspSession(	connect, m_BindAddr, connectAddr, (MediaSessionList*)this);
		m_RtspSessionList.push_back( rtspSession );
		
		printf("\nRtspTransport:	New connect session add.\n");
	}
}
