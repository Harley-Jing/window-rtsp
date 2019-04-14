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
	*		Rtsp 客户端	
	*/	  
	string url = "rtsp://10.2.1.60:5510/chID=4&date=2019-04-03&time=16:32:59&timelen=64";
	string setupName;	//	string setupName = "trackID=";
	
	//client port
	INT rtpPort = 8080;				   
	INT rtcpPort = rtpPort + 2;
	string sdp;
	//Session头字段标识了一个RTSP会话。Session ID 是由服务器在SETUP的回应中选择的，
	//客户端一当得到Session ID后，在以后的对Session 的操作请求消息中都要包含Session ID.
	INT64 sess;	

	// 1.创建一个tcp socket，链接到URL
	g_RtspRequest.Open(url.c_str(), "127.0.0.1", 20);  //TCP常用端口:20 (数据连接)
	// 2.发送OPTIONS
	g_RtspRequest.RequestOptions();
	// 3.发送DESCRIBE and Receiving SDP informatio
	//It's not necessary to parse the SDP information entirely. 
	//It will suffice to extract the lines containing "a=control:trackID=x" 
	//and keep hold of these control strings. 
	g_RtspRequest.RequestDescribe(&sdp);  
	g_RtspRequest.SearchSdpInfo(&setupName, &sdp, "trackID=");	//date: 2019.4.11	   add function
	// 4.发送SETUP
	g_RtspRequest.RequestSetup(setupName.c_str(), transportModeRtpUdp, rtpPort , rtcpPort , &sess);
	// 5.发送PLAY
	g_RtspRequest.RequestPlay();
	

	/*
	*		Rtp 客户端
	*/

	// 1.创建一个udp socket,绑定到固定的端口上
	Rtp* pRtp = new Rtp();
	pRtp->Open("0.0.0.0", rtpPort);  

	Buffer buff;
	int iRead;

	INT8 nPayloadType;
	WORD nSequenceNumber;  //序列号
	INT32 nTimeStamp;	   //时间戳
	INT32 nSsrc;		   //同步信源
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
			fseek(fp, 0, SEEK_END);//定位文件指针到文件结束位置
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

