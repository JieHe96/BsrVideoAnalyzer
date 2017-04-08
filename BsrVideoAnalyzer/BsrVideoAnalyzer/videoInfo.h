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
	float max_framerate;  // 由SPS计算得到的帧率，为0表示SPS中没有相应的字段计算
	int chroma_format_idc;  // YUV颜色空间 0: monochrome 1:420 2:422 3:444
	int encoding_type;  // 为1表示CABAC 0表示CAVLC
	int bit_depth_luma;
	int bit_depth_chroma;
} videoinfo_t;



#endif