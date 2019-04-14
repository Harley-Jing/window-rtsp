// RtspClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "RtspRequest.h"   
#include "Rtp.h"

#include "VBuffer.h"

RtspRequest g_RtspRequest; 

int main(int argc, char* argv[])
{

	/*
	*		Rtsp �ͻ���	
	*/	  
	string url = "rtsp://10.2.1.60:5510/chID=4&date=2019-04-03&time=16:32:59&timelen=64";
	string setupName;	//	string setupName = "trackID=";
	
	//client port
	INT rtpPort = 8080;				   
	INT rtcpPort = rtpPort + 2;
	string sdp;
	//Sessionͷ�ֶα�ʶ��һ��RTSP�Ự��Session ID ���ɷ�������SETUP�Ļ�Ӧ��ѡ��ģ�
	//�ͻ���һ���õ�Session ID�����Ժ�Ķ�Session �Ĳ���������Ϣ�ж�Ҫ����Session ID.
	INT64 sess;	

	// 1.����һ��tcp socket�����ӵ�URL
	g_RtspRequest.Open(url.c_str(), "127.0.0.1", 20);  //TCP���ö˿�:20 (��������)
	// 2.����OPTIONS
	g_RtspRequest.RequestOptions();
	// 3.����DESCRIBE and Receiving SDP informatio
	//It's not necessary to parse the SDP information entirely. 
	//It will suffice to extract the lines containing "a=control:trackID=x" 
	//and keep hold of these control strings. 
	g_RtspRequest.RequestDescribe(&sdp);  
	g_RtspRequest.SearchSdpInfo(&setupName, &sdp, "trackID=");	//date: 2019.4.11	   add function
	// 4.����SETUP
	g_RtspRequest.RequestSetup(setupName.c_str(), transportModeRtpUdp, rtpPort , rtcpPort , &sess);
	// 5.����PLAY
	g_RtspRequest.RequestPlay();
	

	/*
	*		Rtp �ͻ���
	*/

	// 1.����һ��udp socket,�󶨵��̶��Ķ˿���
	Rtp* pRtp = new Rtp();
	pRtp->Open("0.0.0.0", rtpPort);  

	Buffer buff;
	int iRead;

	INT8 nPayloadType;
	WORD nSequenceNumber;  //���к�
	INT32 nTimeStamp;	   //ʱ���
	INT32 nSsrc;		   //ͬ����Դ
	UINT32 unHead = 0x01000000;

	buff.AllocateBuffer(3000);

	FILE* fp = NULL;
	fp = fopen("D:/cpp/windows-rtsp/bufTest.txt", "w+");
	if (NULL == fp)
	{
		printf("The file doesn't exist!\n");
		return -1;
	}

	while(TRUE)
	{
		//g_RtspRequest.RequestGetParameter();	//ping
		iRead = pRtp->Read(buff.m_pBuffer, 3000, &nPayloadType, &nSequenceNumber, &nTimeStamp, &nSsrc);

		if (iRead > 0)
		{
			fseek(fp, 0, SEEK_END);//��λ�ļ�ָ�뵽�ļ�����λ��
			fwrite(&unHead, sizeof(UINT32), 1, fp);
			// save buff 
			fwrite(buff.m_pBuffer, iRead, 1, fp);
			//printf("%s", buff.m_pBuffer);
		}
		else
		{
			break;
		}
	}
	fclose(fp);

	delete pRtp;

    g_RtspRequest.RequestPause();
	g_RtspRequest.RequestTeardown();
	g_RtspRequest.Close();
	
	return 0;
}

