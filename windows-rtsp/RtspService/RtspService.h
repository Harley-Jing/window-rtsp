#pragma once

#include "RtspService_t.h"
#include "H264.h"
#include "BaseEncoder.h"
#include "MediaStreamH264.h"

#define NETWORK_MTU 1482
#define FILE_READ_SIZE 1024*1024*2

static UINT WINAPI H264File_Read2SendThread(LPVOID pUser)
{
	RtspService_t* pRtspService = (RtspService_t*)pUser;
	
	Buffer		read_buff;			// 文件读取数据缓冲区
	Buffer		h264_buff;			// H264数据缓冲区
	VBuffer		h264_nal_buff_list;	// H264 nal数据缓冲链
	Buffer		h264_frame_buff;	// H264每帧包含的nal数据

	string		sps_base64;
	string		pps_base64;

	bool_t		b_result;

	bool_t		b_find_sps = FALSE;
	bool_t		b_find_pps = FALSE;
	bool_t		found_frame_start = FALSE;
	uint32_t	i_begin_time = 0;
	uint32_t	i_current_time = 0;
	int64_t		frame_count = 0;
	int64_t		iframe_offset = 0;

	h264_sps_t	sps;
	
	read_buff.AllocateBuffer(FILE_READ_SIZE);
	h264_buff.AllocateBuffer(FILE_READ_SIZE*2);
	memset(&sps, 0, sizeof(h264_sps_t));

	while( !pRtspService->m_bVideoFile_ReadThread_Stop )
	{
		// 当H264 nal数据不足时，从文件中读取补充
		if (h264_nal_buff_list.GetDataSize() < FILE_READ_SIZE)
		{
			read_buff.ClearData();
			b_result = ReadFile(pRtspService->m_hVideoFile, 
				read_buff.m_pBuffer, 
				read_buff.m_nBufferSize, 
				(DWORD*)&read_buff.m_nDataSize, 
				NULL);

			if (b_result && !read_buff.m_nDataSize)
			{
				SetFilePointer(pRtspService->m_hVideoFile, 0, NULL, FILE_BEGIN);
				continue;
			}
			h264_buff.AppendData(read_buff.m_pData, read_buff.m_nDataSize);

			// 将读取的H264数据流分割为nal，保存在数据缓冲数据链表中
			h264_split_nal(&h264_buff, &h264_nal_buff_list);
		}

		while(TRUE)
		{
			if (b_find_sps && b_find_pps)
			{
				// 根据帧率，控制发送速度
				i_current_time = GetTickCount();
				if (i_current_time < i_begin_time)
				{
					// 这句... 不想解释...
					// 曾听说有某网游公司的某离职coder写的服务每隔近2个月就挂，作为RPWT处理...
					i_begin_time = i_current_time;
					frame_count = 0;
				}
				
				if (i_current_time - i_begin_time < frame_count * 1000 * 2 * sps.num_units_in_tick / sps.time_scale )
				{
					Sleep(10);
					break;
				}
			}
			
			// 获取一个nal
			Buffer* p_h264_nal = h264_nal_buff_list.GetFullBuffer();
			if (!p_h264_nal)
				break;

			// 获取nal type
			int i_nal_type;
			h264_get_nal_type(&i_nal_type, p_h264_nal->m_pData);

			// 将nal按帧重组，注意：我忽略掉了SEI、PPS和SPS
			if (i_nal_type != 6 && i_nal_type != 7 && i_nal_type != 8)
				h264_frame_buff.AppendData(p_h264_nal->m_pData, p_h264_nal->m_nDataSize);

			// 查找sps nal，以获取帧率，控制数据发送速度
			if ( !b_find_sps )
			{
				// 找到sps nal
				if ( i_nal_type == 7)
				{
					Buffer sps_buff;
					sps_buff.AllocateBuffer(p_h264_nal->m_nDataSize);
					
					h264_decode_annexb( sps_buff.m_pBuffer, (INT *)&sps_buff.m_nDataSize, 
						p_h264_nal->m_pData+4, p_h264_nal->m_nDataSize-4);
					
					h264_decode_seq_parameter_set(sps_buff.m_pData, sps_buff.m_nDataSize, &sps);
					if (sps.time_scale)	// 预防sps数据错误，导致sps.time_scale为0作为除数
					{
						BaseEncoder::Base64Encode(p_h264_nal->m_pData, p_h264_nal->m_nDataSize, &sps_base64);
						b_find_sps = TRUE;
					}
					else
					{
						// SPS没有帧率信息 (这些伪劣编码器太可恶了，应该扯出来批斗！:( )
						printf("The SPS's time_scale field info is error!\n");
					}
				}
			}

			// 查找pps，并初始化视频流信息
			if ( !b_find_pps )
			{
				// 找到pps nal
				if ( i_nal_type == 8 )
				{
					BaseEncoder::Base64Encode(p_h264_nal->m_pData, p_h264_nal->m_nDataSize, &pps_base64);
					b_find_pps = TRUE;
				}

				// 初始化视频流信息
				if (b_find_sps && b_find_pps)
				{
					string sprop_parameter_sets;
					sprop_parameter_sets = sps_base64;
					sprop_parameter_sets += ',';
					sprop_parameter_sets += pps_base64;
					
					((MediaStreamH264*)(pRtspService->m_pMediaStream_Video))->Init(0,
						NETWORK_MTU, sps.mb_width, sps.mb_height, sps.profile_idc, sprop_parameter_sets);

					i_begin_time = GetTickCount();
				}
			}
			
			// 帧缓存已经填满一帧数据
			if ( h264_find_frame_end(&found_frame_start, p_h264_nal->m_pData, p_h264_nal->m_nDataSize, i_nal_type) )
			{
				frame_count++;
				
				// 播出数据，注意这里的时钟频率是按90k计算
				if (b_find_sps && b_find_pps)
				{
					int64_t i_pts = h264_get_pts_rtp(&iframe_offset, frame_count, &sps, p_h264_nal->m_pData, p_h264_nal->m_nDataSize, i_nal_type);
					pRtspService->m_pMediaStream_Video->TransportData(h264_frame_buff.m_pData, h264_frame_buff.m_nBufferSize, i_pts);
					h264_frame_buff.ClearData();
				}
			}

			h264_nal_buff_list.AddEmptyBuffer(p_h264_nal);
		}
	}
	
	return 0;
}

static UINT WINAPI AACFile_Read2SendThread(LPVOID pUser)
{
	RtspService_t* pRtspService = (RtspService_t*)pUser;
	
	Buffer		read_buff;			// 文件读取数据缓冲区

	read_buff.AllocateBuffer(FILE_READ_SIZE);
	
	while( !pRtspService->m_bAudioFile_ReadThread_Stop )
	{
		Sleep(10);



		//pRtspService->m_pMediaStream_Audio->TransportData(pData, nDataSize, nPts);
	}
	
	return 0;
}
