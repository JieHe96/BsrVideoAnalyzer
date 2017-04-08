#pragma once
#ifndef _COMMON_INFO_H_
#define _COMMON_INFO_H_

#include <stdint.h>
#include <stdio.h>
#include <assert.h>


typedef struct
{
	int type;           // 0:h264 1:h265
	int init;
	int profile_idc;
	int level_idc;
	int tier_idc;
	int width;
	int height;
	int crop_left;
	int crop_right;
	int crop_top;
	int crop_bottom;
	float max_framerate;  // ��SPS����õ���֡�ʣ�Ϊ0��ʾSPS��û����Ӧ���ֶμ���
	int chroma_format_idc;  // YUV��ɫ�ռ� 0: monochrome 1:420 2:422 3:444
	int encoding_type;  // Ϊ1��ʾCABAC 0��ʾCAVLC
	int bit_depth_luma;
	int bit_depth_chroma;
} videoinfo_t;



#endif