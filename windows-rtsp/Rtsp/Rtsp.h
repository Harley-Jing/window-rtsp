// Rtsp.h: interface for the Rtsp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTSP_H__8C4DD607_2EBB_4D85_B489_E739ACD1D93F__INCLUDED_)
#define AFX_RTSP_H__8C4DD607_2EBB_4D85_B489_E739ACD1D93F__INCLUDED_

#include "Tcp.h"

#define RTSP_PROTOCOL_PORT 554

//typedef enum serverState
enum serverState
{
	stateInit = 0,
	stateConnected,
	stateReady,
	statePlaying,
	statePause,
	stateRecording
};

//typedef enum streamingTransportMode
enum streamingTransportMode
{
	transportModeRtpUdp = 1,
	transportModeRtpTcp,
	transportModeRawUdp
};

static const int MAX_FIELDS_COUNT = 256;

class Rtsp  : public Tcp
{
public:

public:
	Rtsp();
	virtual ~Rtsp();

	virtual int  Write(string str);
	virtual int  Read(string& str);

	virtual void AddField(string field);
	virtual void WriteFields(string str);

	virtual BOOL ParseMrl(string mrl, string* pPreSuffix, string* pSuffix, int* pPort);

protected:
	int		m_State;
	int		m_CSeq;
	INT64	m_Session;

	vector<string> m_Fields;
};

#endif // !defined(AFX_RTSP_H__8C4DD607_2EBB_4D85_B489_E739ACD1D93F__INCLUDED_)
