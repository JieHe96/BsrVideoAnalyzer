#include "stdafx.h" // for MFC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "VideoDecode.h"

#include "bmp_utils.h"


#define MAKE_Y(r,g,b)  ( (unsigned char) (((int)(r)*299+(int)(g)*587+(int)(b)*114)/1000) )
#define MAKE_Cr(r,g,b) ( (unsigned char) ((128000-(int)(r)*169-(int)(g)*332+(int)(b)*500)/1000) )
#define MAKE_Cb(r,g,b) ( (unsigned char) ((1280000+(int)(r)*5000-(int)(g)*4190-(int)(b)*813)/10000) )

#define MAKE_RED_Cr(x) ( (unsigned char) ((128000-(int)(x)*169)/1000) )
#define MAKE_RED_Cb(x) ( (unsigned char) (((int)(x)*500+128000)/1000) )

#define MAKE_BLUE_Cr(x) ( (unsigned char) (((int)(x)*500+128000)/1000) )
#define MAKE_BLUE_Cb(x) ( (unsigned char) ((1280000-(int)(x)*813)/10000) )

#define MAKE_GREEN_Cr(x) ( (unsigned char) ((128000-(int)(x)*332)/1000) )
#define MAKE_GREEN_Cb(x) ( (unsigned char) ((128000-(int)(x)*419)/1000) )
/*
#define MB_TYPE_INTRA4x4   (1 <<  0)
#define MB_TYPE_INTRA16x16 (1 <<  1) // FIXME H.264-specific
#define MB_TYPE_INTRA_PCM  (1 <<  2) // FIXME H.264-specific
#define MB_TYPE_16x16      (1 <<  3)
#define MB_TYPE_16x8       (1 <<  4)
#define MB_TYPE_8x16       (1 <<  5)
#define MB_TYPE_8x8        (1 <<  6)
#define MB_TYPE_INTERLACED (1 <<  7)
#define MB_TYPE_DIRECT2    (1 <<  8) // FIXME
#define MB_TYPE_ACPRED     (1 <<  9)
#define MB_TYPE_GMC        (1 << 10)
#define MB_TYPE_SKIP       (1 << 11)
#define MB_TYPE_P0L0       (1 << 12)
#define MB_TYPE_P1L0       (1 << 13)
#define MB_TYPE_P0L1       (1 << 14)
#define MB_TYPE_P1L1       (1 << 15)
#define MB_TYPE_L0         (MB_TYPE_P0L0 | MB_TYPE_P1L0)
#define MB_TYPE_L1         (MB_TYPE_P0L1 | MB_TYPE_P1L1)
#define MB_TYPE_L0L1       (MB_TYPE_L0   | MB_TYPE_L1)
#define MB_TYPE_QUANT      (1 << 16)
#define MB_TYPE_CBP        (1 << 17)
#define MB_TYPE_8x8DCT     0x01000000
*/

unsigned char FRAME_0[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8
};
unsigned char FRAME_1[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8
};
unsigned char FRAME_2[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8
};
unsigned char FRAME_3[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8
};
unsigned char FRAME_4[] = {
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8
};

unsigned char FRAME_4x4[] = {
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0x00, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x00, 0xD8,
	0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8
};

unsigned char N9_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0x3A, 0xD8, 0xCA, 0x2B, 0xAD, 0x48, 0x3A, 0x90, 0xAD, 0x3A, 0x56, 0xBB,
	0x56, 0xD8, 0xD8, 0xAD, 0x08, 0x08, 0x65, 0x90, 0x65, 0xD8, 0xCA, 0x2B, 0x08, 0x08, 0x08, 0x08,
};
unsigned char N8_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0x3A, 0xD8, 0xD8, 0x3A, 0x82, 0x65, 0x56, 0x82, 0x48, 0xD8, 0xD8, 0x48,
	0xAD, 0x3A, 0x2B, 0xAD, 0xAD, 0x3A, 0x2B, 0xBB, 0x3A, 0xD8, 0xD8, 0x56, 0x08, 0x08, 0x08, 0x08,
};
unsigned char N7_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0xBB, 0xD8, 0xD8, 0xAD, 0x08, 0x08, 0x65, 0x3A, 0x08, 0x1D, 0x9E, 0x08,
	0x08, 0x65, 0x48, 0x08, 0x08, 0xAD, 0x0E, 0x08, 0x08, 0xCA, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};
unsigned char N6_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0x1D, 0xBB, 0xD8, 0x65, 0x73, 0x65, 0x08, 0x08, 0x9E, 0xD8, 0xD8, 0x56,
	0x9E, 0x56, 0x2B, 0xBB, 0x90, 0x48, 0x2B, 0xAD, 0x2B, 0xCA, 0xD8, 0x48, 0x08, 0x08, 0x08, 0x08,
};
unsigned char N5_4x8[] = {
	0x08, 0x08, 0x08, 0x08,
	0x48, 0xD8, 0xD8, 0x82,
	0x65, 0x65, 0x08, 0x08,
	0x90, 0xD8, 0xD8, 0x56,
	0x08, 0x08, 0x2B, 0xBB,
	0xAD, 0x48, 0x3A, 0xAD,
	0x3A, 0xD8, 0xD8, 0x3A,
	0x08, 0x08, 0x08, 0x08,
};
unsigned char N4_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0x08, 0x0E, 0xCA, 0x1D, 0x08, 0x73, 0xCA, 0x1D, 0x1D, 0x9E, 0x9E, 0x1D,
	0x9E, 0x1D, 0x9E, 0x1D, 0xD8, 0xD8, 0xD8, 0xBB, 0x08, 0x08, 0x9E, 0x1D, 0x08, 0x08, 0x08, 0x08,
};
unsigned char N3_4x8[] = {
	0x00, 0x41, 0x3A, 0x00, 
	0x69, 0xF1, 0xD5, 0x4B, 
	0x6B, 0x26, 0x86, 0xAD, 
	0x00, 0x38, 0xFE, 0x2C, 
	0x00, 0x16, 0xA9, 0x84, 
	0x63, 0x07, 0x2F, 0xE5, 
	0x88, 0xEE, 0xDB, 0x6A, 
	0x00, 0x62, 0x5F, 0x00	
};
unsigned char N2_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0x3A, 0xD8, 0xD8, 0x56,
	0x90, 0x48, 0x3A, 0xAD, 0x08, 0x08, 0x48, 0x82,
	0x08, 0x56, 0xAD, 0x0E, 0x48, 0x9E, 0x08, 0x08,
	0xAD, 0xD8, 0xD8, 0xBB, 0x08, 0x08, 0x08, 0x08
};
unsigned char N1_4x8[] = {
	0x08, 0x08, 0x44, 0x08, 
	0x08, 0x51, 0xD8, 0x08,
	0x29, 0x6C, 0xD8, 0x08,
	0x08, 0x08, 0xD8, 0x08,
	0x08, 0x08, 0xD8, 0x08,
	0x08, 0x08, 0xD8, 0x08,
	0x08, 0x08, 0xD8, 0x08,
	0x08, 0x08, 0x29, 0x08,
};
unsigned char N0_4x8[] = {
	0x08, 0x08, 0x08, 0x08, 0x2B, 0xD8, 0xD8, 0x2B,
	0x90, 0x56, 0x56, 0x90, 0xAD, 0x1D, 0x1D, 0xBB,
	0xBB, 0x1D, 0x1D, 0xBB, 0x90, 0x56, 0x56, 0x90,
	0x2B, 0xD8, 0xD8, 0x2B, 0x08, 0x08, 0x08, 0x08
};

uint8_t *NX_4x8[] = {
	N0_4x8,
	N1_4x8,
	N2_4x8,
	N3_4x8,
	N4_4x8,
	N5_4x8,
	N6_4x8,
	N7_4x8,
	N8_4x8,
	N9_4x8
};

#ifdef _DEBUG_
int debug(const char* fmt, ...)
{
    char buffer[512] = {0};
    va_list ap;
    int ret;
    va_start(ap, fmt);
    ret = vsprintf(buffer, fmt, ap);
    va_end(ap);
    printf("%s", buffer);
    return ret;
}
#else
int debug(const char* fmt, ...)
{
    return 0;
}
#endif
static void pic_arrays_free(HEVCContext *s)
{

	av_freep(&s->skip_flag);
	av_freep(&s->tab_ct_depth);

	av_freep(&s->tab_ipm);

	av_freep(&s->qp_y_tab);

	av_freep(&s->ps.sps);
}

/* allocate arrays that depend on frame dimensions */
static int pic_arrays_init(HEVCContext *s, const HEVCSPS *sps)
{
	int log2_min_cb_size = sps->log2_min_cb_size;
	int width = sps->width;
	int height = sps->height;
	int pic_size_in_ctb = ((width >> log2_min_cb_size) + 1) *
		((height >> log2_min_cb_size) + 1);
	int ctb_count = sps->ctb_width * sps->ctb_height;
	int min_pu_size = sps->min_pu_width * sps->min_pu_height;

	s->bs_width = (width >> 2) + 1;
	s->bs_height = (height >> 2) + 1;

	memset(s, 0, sizeof(HEVCContext));
	s->ps.sps = (HEVCSPS *)av_mallocz(sizeof(HEVCSPS));
	memcpy((void *)(s->ps.sps), sps, sizeof(HEVCSPS));

	s->skip_flag = (uint8_t *)av_malloc_array(sps->min_cb_height, sps->min_cb_width);
	s->tab_ct_depth = (uint8_t *)av_malloc_array(sps->min_cb_height, sps->min_cb_width);
	if (!s->skip_flag || !s->tab_ct_depth)
		goto fail;

	s->tab_ipm = (uint8_t *)av_mallocz(min_pu_size);
	if (!s->tab_ipm)
		goto fail;

	s->qp_y_tab = (int8_t *)av_malloc_array(pic_size_in_ctb,
		sizeof(*s->qp_y_tab));
	if (!s->qp_y_tab)
		goto fail;

	return 0;

fail:
	pic_arrays_free(s);
	return AVERROR(ENOMEM);
}

static void pic_arrays_copy(const HEVCContext *src, HEVCContext *dst)
{
	int min_pu_size = src->ps.sps->min_pu_width * src->ps.sps->min_pu_height;
	int min_cb_size = src->ps.sps->min_cb_width * src->ps.sps->min_cb_height;


	//memset(, 0, sizeof(HEVCContext));
	if (!dst->skip_flag)
	{
		int ret = pic_arrays_init(dst, src->ps.sps);
		if (ret) return;
	}

	memcpy(dst->skip_flag, src->skip_flag, min_cb_size);
	memcpy(dst->tab_ct_depth, src->tab_ct_depth, min_cb_size);
	memcpy(dst->qp_y_tab, src->qp_y_tab, min_cb_size);
	memcpy(dst->tab_ipm, src->tab_ipm, min_pu_size);

	dst->poc = src->poc;

	return;
}


#define MAX_HEVC_GOP	500
BsrVideoDecoder::BsrVideoDecoder()
    :m_skippedFrame(0),
    m_picWidth(0),
    m_picHeight(0),
    m_videoStream(-1),
    m_picBuffer(NULL),
    m_fmtctx(NULL),
    m_avctx(NULL),
    m_picture(NULL),
    m_frameRGB(NULL),
    m_bufferYUV(NULL),
    m_frameYUV(NULL),
    m_imgctx(NULL),
    m_imgctxyuv(NULL),
	m_bDisplayQP(FALSE),
	m_bDisplayQC(FALSE),
	m_bDisplayMB(FALSE)
{
	m_drawBuffer[0] = NULL;

	hevcCTX = (HEVCContext *)malloc(MAX_HEVC_GOP * sizeof(HEVCContext));
	memset(hevcCTX, 0, MAX_HEVC_GOP * sizeof(HEVCContext));

	display_poc = -1;
	m_skipStep = 0;
	m_errFrames = 0;
	m_cCodecInfo[0] = '\0';
}

BsrVideoDecoder::~BsrVideoDecoder()
{
    closeVideoFile();

	for (int i = 0; i < MAX_HEVC_GOP; i++)
	{
		pic_arrays_free(&hevcCTX[i]);
	}

	free(hevcCTX);
}

int BsrVideoDecoder::openVideoFile(const char* avifile)
{
    int ret = 0;
    int size = 0;
    AVCodec* codec = NULL;

    if (avifile == NULL)
    {
        debug("file invalid.\n");
        return -1;
    }
    av_register_all();

    // 打开视频文件
    ret = avformat_open_input(&m_fmtctx, avifile, NULL, NULL);
    if (ret != 0)
    {
        debug("open input file failed. ret: %d\n", ret);
        return -1;
    }
    ret = avformat_find_stream_info(m_fmtctx, NULL);
    if (ret < 0)
    {
        debug("find stream info failed. ret: %d\n", ret);
        return -1;
    }

#ifdef _DEBUG_
    // 打印视频信息
    av_dump_format(m_fmtctx, 0, avifile, 0);
#endif

    // 找视频流
    for (unsigned int i = 0; i < m_fmtctx->nb_streams; i++)
    {
        if (m_fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoStream = i;
            break;
        }
    }
    if (m_videoStream == -1)
    {
        debug("no video stream.\n");
        return -1;
    }

    // 打开解码器
    m_avctx = m_fmtctx->streams[m_videoStream]->codec;
    codec = avcodec_find_decoder(m_avctx->codec_id);
    if (codec == NULL)
    {
        debug("unsupported codec\n");
        return -1;
    }

	AVDictionary *opts = NULL;
	/* Init the decoders, with or without reference counting */
	//ret = avcodec_open2(m_avctx, codec, NULL);
	av_dict_set(&opts, "refcounted_frames", "1", 0);
	//m_avctx->thread_count = 1;
	m_avctx->thread_type = FF_THREAD_SLICE;
	ret = avcodec_open2(m_avctx, codec, &opts);
    if (ret < 0)
    {
        debug("open codec failed.\n");
        return -1;
    }

	//display mb type using different color
	//ret = av_opt_set(m_avctx, "debug", "vis_mb_type", 0);
	//ret = av_opt_set(m_avctx, "debug", "vis_qp", 0);

    // 分配frame
    m_picture = av_frame_alloc();
    if (!m_picture)
    {
        return -1;
    }
	// 分配转换成RGB后的frame
    m_frameRGB = av_frame_alloc();
    if (!m_frameRGB)
    {
        av_free(m_picture);
        return -1;
    }

	size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, m_avctx->width, m_avctx->height, 1);// m_avctx->width);
	// m_picBuffer要到最后释放
	// we allocated more 1024 bytes for copying bmp head info
    m_picBuffer = (unsigned char *)av_malloc(size+1024);
    if (!m_picBuffer)
    {
        av_free(m_picture);
        av_free(m_frameRGB);
        return -1;
    }

	av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize, m_picBuffer+1024, AV_PIX_FMT_BGR24, m_avctx->width, m_avctx->height, 1);

    // 创建转换上下文
    m_imgctx = sws_getContext(m_avctx->width, m_avctx->height, m_avctx->pix_fmt, m_avctx->width, m_avctx->height, 
        AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
    if (m_imgctx == NULL)
    {
        av_free(m_picture);
        av_free(m_frameRGB);
        av_free(m_picBuffer);
        return -1;
    }

    return 0;
}

void BsrVideoDecoder::closeVideoFile(void)
{
    m_skippedFrame = 0;
	m_errFrames = 0;
	if (m_picture)
    {
		av_frame_unref(m_picture);
        av_free(m_picture);
        m_picture = NULL;
    }
    if (m_avctx)
    {
        avcodec_close(m_avctx);
        m_avctx = NULL;
    }
    if (m_fmtctx)
    {
        avformat_close_input(&m_fmtctx);
        m_fmtctx = NULL;
    }
    if (m_picBuffer)
    {
        av_free(m_picBuffer);
        m_picBuffer = NULL;
    }
    if (m_frameRGB)
    {
        av_free(m_frameRGB);
        m_frameRGB = NULL;
    }
    if (m_imgctx)
    {
        sws_freeContext(m_imgctx);
        m_imgctx = NULL;
    }
    if (m_bufferYUV)
    {
        av_free(m_bufferYUV);
        m_bufferYUV = NULL;
    }
    if (m_frameYUV)
    {
        av_free(m_frameYUV);
        m_frameYUV = NULL;
    }
    if (m_imgctxyuv)
    {
        sws_freeContext(m_imgctxyuv);
        m_imgctxyuv = NULL;
    }

	if (m_drawBuffer[0])
	{
		//for SDL
		//free(m_drawBuffer[0]);
		m_drawBuffer[0] = NULL;
	}

	for (int i = 0; i < MAX_HEVC_GOP; i++)
	{
		pic_arrays_free(&hevcCTX[i]);
	}

	display_poc = -1;
	m_skipStep = 0;
	m_cCodecInfo[0] = '\0';
}

int BsrVideoDecoder::jumpToTime(int64_t time)
{
    int64_t seekTarget = 0;
    AVRational AV_TIME = {1, AV_TIME_BASE};
    int ret = 0;

    // 跳到指定时间
    seekTarget = time * AV_TIME_BASE;
    if (seekTarget > m_fmtctx->duration || seekTarget < 0)
    {
        debug("time beyond the video or less than zero.\n");
        return -1;
    }
    seekTarget = av_rescale_q(seekTarget, AV_TIME, m_fmtctx->streams[m_videoStream]->time_base);
    ret = av_seek_frame(m_fmtctx, m_videoStream, seekTarget,0);   //  AVSEEK_FLAG_ANY AVSEEK_FLAG_BACKWARD
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}

int BsrVideoDecoder::seek(int64_t pos)
{
	int ret = 0;

	if (m_fmtctx == NULL)
		return -1;

	ret = av_seek_frame(m_fmtctx, m_videoStream, pos, AVSEEK_FLAG_BYTE);   //  AVSEEK_FLAG_ANY AVSEEK_FLAG_BACKWARD
	if (ret < 0)
	{
		return -1;
	}
	avcodec_flush_buffers(m_avctx);

	m_skippedFrame = 0;
	m_errFrames = 0;
	return 0;
}

void BsrVideoDecoder::Calc_Info_H264(H264Context *h)
{
	if (!h || !h->next_output_pic) return;

	int mb_y;
	int mb_height = h->mb_height;
	int mb_width = h->mb_width;
	int mb_stride = h->mb_stride;
	int block_height = 8;
	int block_height_y = 16;

	int cb_size = mb_width*mb_height;

	int8_t *qscale_table = h->next_output_pic->qscale_table;
	uint32_t *mbtype_table = h->next_output_pic->mb_type;
	if (m_skippedFrame)
	{
		qscale_table = h->cur_pic.qscale_table;
		mbtype_table = h->cur_pic.mb_type;
	}

	int min_qp = 127, max_qp = 0, sum_qp = 0;
	int cnt_skip = 0, cnt_intra_pm = 0, cnt_inter_pm = 0;
	int isIntra = 0, isInter = 0;

	for (mb_y = 0; mb_y < mb_height; mb_y++)
	{
		int mb_x;
		for (mb_x = 0; mb_x < mb_width; mb_x++)
		{
			const int mb_index = mb_x + mb_y * mb_stride;

			uint32_t x = mbtype_table[mb_index];

			sum_qp += qscale_table[mb_index];
			max_qp = (max_qp < qscale_table[mb_index] ? qscale_table[mb_index] : max_qp);
			min_qp = (min_qp > qscale_table[mb_index] ? qscale_table[mb_index] : min_qp);

			cnt_skip += ((x & MB_TYPE_SKIP) ? 1 : 0);
			
			isInter = ((x & MB_TYPE_16x16) | (x & MB_TYPE_16x8) | (x & MB_TYPE_8x16) | (x & MB_TYPE_8x8));
			isIntra = ((x & MB_TYPE_INTRA16x16) | ((x & MB_TYPE_INTRA4x4) && (x & MB_TYPE_8x8DCT)) | ((x & MB_TYPE_INTRA4x4) && !(x & MB_TYPE_8x8DCT)));
			cnt_inter_pm += (isInter>0 ? 1 : 0);
			cnt_intra_pm += (isIntra>0 ? 1 : 0);
		}
	}

	sprintf(m_cCodecInfo, "block 16x16: %d -- [qp: min %d|max %d|avg %d]\r\n[skip: %d(.%02d)][intra: %d(.%02d)][inter: %d(.%02d)]",
		cb_size,
		min_qp, max_qp, sum_qp / cb_size,
		cnt_skip, (cnt_skip * 100) / cb_size,
		cnt_intra_pm, (cnt_intra_pm * 100) / cb_size,
		cnt_inter_pm, (cnt_inter_pm * 100) / cb_size
		);
}

typedef struct YCrBr
{
	unsigned char Y;
	unsigned char Cr;
	unsigned char Cb;
	unsigned char qp;
}YCrBr;

void MakeColor(YCrBr *yc)
{
	if (yc->qp < 10) //cyan
	{
		int val = 240 - yc->qp * 15;
		yc->Y = MAKE_Y(0, val, val);
		yc->Cr = MAKE_Cr(0, val, val);
		yc->Cb = MAKE_Cb(0, val, val);
	}
	else if (yc->qp < 20) //red
	{
		int val = 240 - (yc->qp-10) * 12;
		yc->Y = MAKE_Y(val, 0, 0);
		yc->Cr = MAKE_Cr(val, 0, 0);
		yc->Cb = MAKE_Cb(val, 0, 0);
	}
	else if (yc->qp < 30) //green
	{
		int val = 192 - (yc->qp - 20) * 10;
		yc->Y =   MAKE_Y(0, val, 0);
		yc->Cr = MAKE_Cr(0, val, 0);
		yc->Cb = MAKE_Cb(0, val, 0);
	}
	else if (yc->qp < 40) //blue
	{
		int val = 240 - (yc->qp - 30) * 12;
		yc->Y = MAKE_Y(0, 0, val);
		yc->Cr = MAKE_Cr(0, 0, val);
		yc->Cb = MAKE_Cb(0, 0, val);
	}
	else if (yc->qp < 48) //yellow
	{
		int val = 240 - (yc->qp - 40) * 12;
		yc->Y = MAKE_Y(val, val, 0);
		yc->Cr = MAKE_Cr(val, val, 0);
		yc->Cb = MAKE_Cb(val, val, 0);
	}
	else if (yc->qp >= 48) //purple
	{
		int val = 200 - (yc->qp - 48) * 20;
		yc->Y = MAKE_Y(val, 0, val);
		yc->Cr = MAKE_Cr(val, 0, val);
		yc->Cb = MAKE_Cb(val, 0, val);
	}

}

void BsrVideoDecoder::drawH264UI()
{
	H264Context *h = (H264Context *)m_avctx->priv_data;
	AVFrame *pict = m_picture;

	if (!h || !h->next_output_pic) return;

	int mb_y;
	int mb_height = h->mb_height;
	int mb_width = h->mb_width;
	int mb_stride = h->mb_stride;
	int block_height = 8;
	int block_height_y = 16;

	int8_t *qscale_table = h->next_output_pic->qscale_table;
	uint32_t *mbtype_table = h->next_output_pic->mb_type;

	if (m_skippedFrame)
	{
		qscale_table = h->cur_pic.qscale_table;
		mbtype_table = h->cur_pic.mb_type;
	}

	if (m_drawBuffer[0] == NULL)	return;

	if (m_bDisplayQP || m_bDisplayMB || m_bDisplayQC)
	{
		for (mb_y = 0; mb_y < mb_height; mb_y++)
		{
			int mb_x;
			for (mb_x = 0; mb_x < mb_width; mb_x++)
			{
				const int mb_index = mb_x + mb_y * mb_stride;
				uint8_t *overlayFrame = FRAME_0;

				uint8_t ovCr = 0, ovCb = 0, ovY = 0;
				uint32_t x = mbtype_table[mb_index];

				if (x & MB_TYPE_SKIP) //GREEN
				{
					overlayFrame = FRAME_0;
					ovCr = MAKE_GREEN_Cr(255);
					ovCb = MAKE_GREEN_Cb(255);
					ovY = MAKE_Y(0, 255, 0);
				}
				else
				{
					if (x & MB_TYPE_16x16) //f0
					{
						overlayFrame = FRAME_0;
					}
					//if (x & MB_TYPE_16x8) //f1
					//{
					//	overlayFrame = FRAME_1;
					//}
					//if (x & MB_TYPE_8x16) //f2
					//{
					//	overlayFrame = FRAME_2;
					//}

					if ((x & MB_TYPE_INTRA4x4) && (x & MB_TYPE_8x8DCT)) //f3 RED
					{
						overlayFrame = FRAME_3;
						ovCr = MAKE_Cr(255, 0, 0);
						ovCb = MAKE_Cb(255, 0, 0);
						ovY = MAKE_Y(255, 0, 0);
					}
					if (x & MB_TYPE_INTRA16x16) //f0 RED
					{
						overlayFrame = FRAME_0;
						ovCr = MAKE_Cr(255, 0, 0);
						ovCb = MAKE_Cb(255, 0, 0);
						ovY = MAKE_Y(255, 0, 0);
					}
					if ((x & MB_TYPE_INTRA4x4) && !(x & MB_TYPE_8x8DCT)) //f4 RED
					{
						overlayFrame = FRAME_4;
						ovCr = MAKE_Cr(255, 0, 0);
						ovCb = MAKE_Cb(255, 0, 0);
						ovY = MAKE_Y(255, 0, 0);
					}
					if (x & MB_TYPE_16x8) //f1
					{
						overlayFrame = FRAME_1;
					}
					if (x & MB_TYPE_8x16) //f2
					{
						overlayFrame = FRAME_2;
					}
					if (x & MB_TYPE_8x8) //f3
					{
						overlayFrame = FRAME_3;
					}
				}


				int y;

				for (y = 0; y < block_height; y++)
				{
					if (block_height * mb_y + y >= m_avctx->height / 2) break;

					if (m_bDisplayQC)
					{
						YCrBr yc; yc.qp = qscale_table[mb_index]; MakeColor(&yc);
						*(uint32_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1]) = 0x01010101UL * (m_bDisplayQP && y < 4 ? 0x80 : yc.Cr);
						*(uint32_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1] + 4) = 0x01010101UL * yc.Cr;

						*(uint32_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2]) = 0x01010101UL * (m_bDisplayQP && y < 4 ? 0x80 : yc.Cb);
						*(uint32_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2] + 4) = 0x01010101UL * yc.Cb;

						continue;
					}

					*(uint64_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1]) =
						*(uint64_t *)(pict->data[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1]);
					*(uint64_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2]) =
						*(uint64_t *)(pict->data[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2]);

					//flag color for MB_Type
					if ((ovCr > 0 || ovCb > 0) && (y == 3 || y == 4) && m_bDisplayMB)
					{
						*(uint8_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1] + 3) = ovCr;
						*(uint8_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1] + 4) = ovCr;
						*(uint8_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2] + 3) = ovCb;
						*(uint8_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2] + 4) = ovCb;
					}

					//copy Cr Cb
					if (y < 4 && m_bDisplayQP && !m_bDisplayMB)
					{
						*(uint32_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1]) = 0x80808080UL;// 0x61616161UL;
						*(uint32_t *)(m_drawBuffer[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1] + 4) =
							*(uint32_t *)(pict->data[1] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[1] + 4);
						
						*(uint32_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2]) = 0x80808080UL;// 0x84848484UL;
						*(uint32_t *)(m_drawBuffer[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2] + 4) =
							*(uint32_t *)(pict->data[2] + 8 * mb_x + (block_height * mb_y + y) * pict->linesize[2] + 4);
					}

				}

				for (y = 0; y < block_height_y; y++)
				{
					if (block_height_y * mb_y + y >= m_avctx->height) break;

					if (m_bDisplayQC)
					{
						YCrBr yc; yc.qp = qscale_table[mb_index]; MakeColor(&yc);
						uint64_t ovQP1 = (y >= 8 || !m_bDisplayQP) ?
							//0x0101010101010101ULL * yc.Y :
							(*(uint64_t *)(pict->data[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 0)) :
							((uint64_t)(*(uint32_t *)(NX_4x8[qscale_table[mb_index] % 10] + y * 4)) << 32) | (*(uint32_t *)(NX_4x8[qscale_table[mb_index] / 10] + y * 4));
						uint64_t ovQP2 =
							(*(uint64_t *)(pict->data[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 8));
							//0x0101010101010101ULL * yc.Y;

						uint64_t ovFrame0 = *(uint64_t *)(overlayFrame + y * 16 + 0);
						uint64_t ovFrame1 = *(uint64_t *)(overlayFrame + y * 16 + 8);


						*(uint64_t *)(m_drawBuffer[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 0) = ovQP1 | ovFrame0;
						*(uint64_t *)(m_drawBuffer[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 8) = ovQP2 | ovFrame1;


						continue;
					}

					uint64_t ovQP = (y >= 8 || !m_bDisplayQP) ?
						(*(uint64_t *)(pict->data[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 0)) :
						((uint64_t)(*(uint32_t *)(NX_4x8[qscale_table[mb_index] % 10] + y * 4)) << 32) |
						(*(uint32_t *)(NX_4x8[qscale_table[mb_index] / 10] + y * 4));

					uint64_t ovFrame0 = (m_bDisplayMB) ? *(uint64_t *)(overlayFrame + y * 16 + 0) : *(uint64_t *)(FRAME_0 + y * 16 + 0);
					uint64_t ovFrame1 = (m_bDisplayMB) ? *(uint64_t *)(overlayFrame + y * 16 + 8) : *(uint64_t *)(FRAME_0 + y * 16 + 8);

					*(uint64_t *)(m_drawBuffer[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 0) = ovQP | ovFrame0;
					*(uint64_t *)(m_drawBuffer[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 8) =
						*(uint64_t *)(pict->data[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 8) | ovFrame1;

					if ((ovCr > 0 || ovCb > 0) && (y >= 6 && y <= 9) && m_bDisplayMB)
					{
						*(uint32_t *)(m_drawBuffer[0] + 16 * mb_x + (block_height_y * mb_y + y) * pict->linesize[0] + 6) = 0x01010101UL * ovY;
					}
				}

			}
		}
	}

}

#define FILL_COLOR(Y, U, V, x0, y0, x1, y1, ovY, ovCr, ovCb) \
	if(m_bDisplayMB)\
	{\
		*(uint32_t *)((Y) + (y0) * 64 + (x0)) = 0x01010101UL * (ovY); \
		*(uint32_t *)((Y) + ((y0) +1) * 64 + (x0)) = 0x01010101UL * (ovY); \
		*(uint32_t *)((Y) + ((y0) +2) * 64 + (x0)) = 0x01010101UL * (ovY); \
		*(uint32_t *)((Y) + ((y0) +3) * 64 + (x0)) = 0x01010101UL * (ovY); \
		*(U + (y1)* 32 + (x1)) = ovCr; \
		*(U + (y1)* 32 + (x1) + 1) = ovCr; \
		*(U + ((y1)+1)* 32 + (x1)) = ovCr; \
		*(U + ((y1)+1)* 32 + (x1) + 1) = ovCr; \
		*(V + (y1)* 32 + (x1)) = ovCb; \
		*(V + (y1)* 32 + (x1) + 1) = ovCb; \
		*(V + ((y1)+1)* 32 + (x1)) = ovCb; \
		*(V + ((y1)+1)* 32 + (x1) + 1) = ovCb; \
	}

#define MAKE_OV_VAL(nn) (uint64_t)(\
	((nn)[0] ? 0 : ((unsigned long long)0xff)<<8*0) | \
	((nn)[1] ? 0 : ((unsigned long long)0xff)<<8*1) | \
	((nn)[2] ? 0 : ((unsigned long long)0xff)<<8*2) | \
	((nn)[3] ? 0 : ((unsigned long long)0xff)<<8*3) | \
	((nn)[4] ? 0 : ((unsigned long long)0xff)<<8*4) | \
	((nn)[5] ? 0 : ((unsigned long long)0xff)<<8*5) | \
	((nn)[6] ? 0 : ((unsigned long long)0xff)<<8*6) | \
	((nn)[7] ? 0 : ((unsigned long long)0xff)<<8*7) ) 

#define OVERLAY_QP(Y, U, V, xx, yy, x, y) \
	{\
		int n1 = tab_qp[y*min_cb_size + x] % 10; \
		int n2 = tab_qp[y*min_cb_size + x] / 10; \
		*(uint64_t *)(Y + (yy + 0) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 0 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 0 * 4)); \
		*(uint64_t *)(Y + (yy + 1) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 1 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 1 * 4)); \
		*(uint64_t *)(Y + (yy + 2) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 2 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 2 * 4)); \
		*(uint64_t *)(Y + (yy + 3) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 3 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 3 * 4)); \
		*(uint64_t *)(Y + (yy + 4) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 4 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 4 * 4)); \
		*(uint64_t *)(Y + (yy + 5) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 5 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 5 * 4)); \
		*(uint64_t *)(Y + (yy + 6) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 6 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 6 * 4)); \
		*(uint64_t *)(Y + (yy + 7) * perCtbHeight + xx) = ((uint64_t)(*(uint32_t *)(NX_4x8[n1] + 7 * 4)) << 32) | (*(uint32_t *)(NX_4x8[n2] + 7 * 4)); \
		*(uint32_t *)(U + (yy / 2 + 0) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x61616161; \
		*(uint32_t *)(U + (yy / 2 + 1) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x61616161; \
		*(uint32_t *)(U + (yy / 2 + 2) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x61616161; \
		*(uint32_t *)(U + (yy / 2 + 3) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x61616161; \
		*(uint32_t *)(V + (yy / 2 + 0) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x84848484; \
		*(uint32_t *)(V + (yy / 2 + 1) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x84848484; \
		*(uint32_t *)(V + (yy / 2 + 2) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x84848484; \
		*(uint32_t *)(V + (yy / 2 + 3) * perCtbHeight / 2 + xx / 2) = (uint32_t)0x84848484; \
	}

#define  OVERLAY_QC(Y, U, V, xx, yy, x, y, depth) \
	{ \
		int ix, iy; \
		YCrBr yc; yc.qp = tab_qp[y*min_cb_size + x]; MakeColor(&yc); \
		ix = iy = 1 << (3 - (depth)); \
		for (int i = 0; i < iy * 8; i++) \
		{ \
			if (iCtbH*perCtbHeight + i == pict->height) break; \
			for (int j = 0; j < ix; j++) \
			{ \
				uint64_t tt64 = *(uint64_t *)(Y + (yy + i) * perCtbHeight + xx + j * 8); \
				*(uint64_t *)(Y + (yy + i) * perCtbHeight + xx + j * 8) |= \
					(MAKE_OV_VAL(Y + (yy + i) * perCtbHeight + xx + j * 8)) & tt64; \
			} \
		} \
		for (int i = 0; i < iy * 4; i++) \
		{ \
			if (iCtbH*perCtbHeight/2 + i == pict->height/2) break; \
			for (int j = 0; j < ix; j++) \
			{ \
				YCrBr yc; yc.qp = tab_qp[y*min_cb_size + x]; MakeColor(&yc); \
				*(uint32_t *)(U + (yy / 2 + i) * perCtbHeight/2 + xx / 2 + j * 4) = 0x01010101UL * yc.Cr; \
				*(uint32_t *)(V + (yy / 2 + i) * perCtbHeight/2 + xx / 2 + j * 4) = 0x01010101UL * yc.Cb; \
			} \
		} \
	}

void BsrVideoDecoder::Calc_Info_H265(HEVCContext *h)
{
	if (!h || !h->ps.sps) return;

	int cb_height = h->ps.sps->min_cb_height;
	int cb_width = h->ps.sps->min_cb_width;
	int pu_height = h->ps.sps->min_pu_height;
	int pu_width = h->ps.sps->min_pu_width;

	int cb_size = cb_width*cb_height;
	int pu_size = pu_width*pu_height;

	int min_qp = 127, max_qp = 0, sum_qp=0;

	int cnt_skip = 0, cnt_intra_pm = 0, cnt_inter_pm = 0;

	int8_t *p_qp = h->qp_y_tab;
	uint8_t *p_ipm = h->tab_ipm;
	uint8_t *p_skip = h->skip_flag;

	for (int i = 0; i < cb_size; i++)
	{
		sum_qp += *(p_qp + i);
		max_qp = (max_qp < *(p_qp + i) ? *(p_qp + i) : max_qp);
		min_qp = (min_qp > *(p_qp + i) ? *(p_qp + i) : min_qp);

		cnt_skip += *(p_skip+i);
	}

	for (int i = 0; i < pu_size; i++)
	{
		cnt_intra_pm += (*(p_ipm + i) & 0x80) == 0 ? 1 : 0;
	}

	cnt_intra_pm /= 4;

	cnt_inter_pm = cb_size - cnt_intra_pm - cnt_skip;

	sprintf(m_cCodecInfo, "block 8x8: %d -- [qp: min %d|max %d|avg %d]\r\n[skip: %d(.%02d)][intra: %d(.%02d)][inter: %d(.%02d)]",
		cb_size,
		min_qp, max_qp, sum_qp/cb_size,
		cnt_skip, (cnt_skip * 100) / cb_size,
		cnt_intra_pm, (cnt_intra_pm * 100) / cb_size,
		cnt_inter_pm, (cnt_inter_pm * 100) / cb_size
		);
}

void BsrVideoDecoder::drawH265UI()
{
	if (display_poc == -1) return;

	HEVCContext *h = (HEVCContext *)m_avctx->priv_data;
	HEVCContext *h_poc = &hevcCTX[display_poc];
	AVFrame *pict = m_picture;

	if (!h_poc->qp_y_tab || !h_poc->tab_ipm || !h_poc->skip_flag || !h_poc->tab_ct_depth)
		return;

	int32_t perCtbHeight = (1 << h->ps.sps->log2_ctb_size );
	int32_t min_cb_size = (1 << h->ps.sps->log2_min_cb_size );
	int32_t min_pu_size = (1 << h->ps.sps->log2_min_pu_size);

	uint8_t tmpBuffer000[8 * 8];
	uint8_t tmpBuffer001[8 * 8];
	uint8_t tmpBuffer002[8 * 8];
	uint8_t tmpBuffer003[8 * 8];
	uint8_t tmpBuffer004[16 * 16];
	uint8_t *tmpBufferY0 = (uint8_t *)malloc(64 * 64 + 32 * 32 + 32 * 32);
	uint8_t *tmpBufferU0 = tmpBufferY0 + 64 * 64;
	uint8_t *tmpBufferV0 = tmpBufferU0 + 32 * 32;
	uint8_t *tmpBufferY1 = (uint8_t *)malloc(64 * 64 + 32 * 32 + 32 * 32);
	uint8_t *tmpBufferU1 = tmpBufferY1 + 64 * 64;
	uint8_t *tmpBufferV1 = tmpBufferU1 + 32 * 32;
	uint8_t *tmpBufferY2 = (uint8_t *)malloc(64 * 64 + 32 * 32 + 32 * 32);
	uint8_t *tmpBufferU2 = tmpBufferY2 + 64 * 64;
	uint8_t *tmpBufferV2 = tmpBufferU2 + 32 * 32;
	uint8_t *tmpBufferY3 = (uint8_t *)malloc(64 * 64 + 32 * 32 + 32 * 32);
	uint8_t *tmpBufferU3 = tmpBufferY3 + 64 * 64;
	uint8_t *tmpBufferV3 = tmpBufferU3 + 32 * 32;

	int cb_height = h->ps.sps->min_cb_height;
	int cb_width = h->ps.sps->min_cb_width;
	int pu_height = h->ps.sps->min_pu_height;
	int pu_width = h->ps.sps->min_pu_width;


	int8_t *p_qp = h_poc->qp_y_tab;
	uint8_t *p_ipm = h_poc->tab_ipm;
	uint8_t *p_skip = h_poc->skip_flag;
	uint8_t *p_depth = h_poc->tab_ct_depth;

	if (m_drawBuffer[0] == NULL) return;
	
	if (m_bDisplayQP || m_bDisplayMB || m_bDisplayQC)
	{

		uint8_t *status8x8 = tmpBuffer000;

		uint8_t *tab_ipm = tmpBuffer004; //16x16
		uint8_t *tab_qp = tmpBuffer001;
		uint8_t *tab_skip = tmpBuffer002;
		uint8_t *tab_ct_depth = tmpBuffer003;

		memset(tmpBufferY1, 0, 64 * 64 * 3 / 2);
		memset(tmpBufferY2, 0, 64 * 64 * 3 / 2);
		memset(tmpBufferY3, 0, 64 * 64 * 3 / 2);

		memset(tmpBufferY1, 0xD8, 64);
		memset(tmpBufferY1+63*64, 0xD8, 64);

		memset(tmpBufferY2, 0xD8, 64);
		memset(tmpBufferY2 + 63 * 64, 0xD8, 64);
		memset(tmpBufferY2 + 31 * 64, 0xD8, 64);

		memset(tmpBufferY3, 0xD8, 64);
		memset(tmpBufferY3 + 63 * 64, 0xD8, 64);

		for (int iHH = 0; iHH < 64; iHH++)
		{
			*(tmpBufferY1 + iHH * 64 + 0) = (uint8_t)0xD8;
			*(tmpBufferY1 + iHH * 64 + 63) = (uint8_t)0xD8;

			*(tmpBufferY2 + iHH * 64 + 0) = (uint8_t)0xD8;
			*(tmpBufferY2 + iHH * 64 + 31) = (uint8_t)0xD8;
			*(tmpBufferY2 + iHH * 64 + 63) = (uint8_t)0xD8;

			*(tmpBufferY3 + iHH * 64 + 0) = (uint8_t)0xD8;
			*(tmpBufferY3 + iHH * 64 + 31) = ((iHH + 1) % 3) ? ((uint8_t)0xD8) : 0;
			*(tmpBufferY3 + iHH * 64 + 63) = (uint8_t)0xD8;
			*(tmpBufferY3 + 31 * 64 + iHH) = ((iHH + 1) % 3) ? ((uint8_t)0xD8) : 0;
		}


		uint8_t ovRedCr = 0, ovRedCb = 0, ovRedY = 0;
		uint8_t ovGreenCr = 0, ovGreenCb = 0, ovGreenY = 0;
		ovGreenCr = MAKE_GREEN_Cr(255);
		ovGreenCb = MAKE_GREEN_Cb(255);
		ovGreenY = MAKE_Y(0, 255, 0);
		ovRedCr = MAKE_RED_Cr(255);
		ovRedCb = MAKE_RED_Cb(255);
		ovRedY = MAKE_Y(255, 0, 0);

		for (int32_t iCtbH = 0; iCtbH < h->ps.sps->ctb_height; iCtbH++)
		{
			for (int32_t iCtbW = 0; iCtbW < h->ps.sps->ctb_width; iCtbW++)
			{
				for (int32_t iCb = 0; iCb < perCtbHeight/min_cb_size; iCb++)
				{
					*(uint64_t *)(tab_qp + iCb * 8) = 
						*(uint64_t *)(p_qp + (iCtbH*(perCtbHeight / min_cb_size) + iCb)*cb_width + iCtbW*(perCtbHeight / min_cb_size));
					*(uint64_t *)(tab_skip + iCb * 8) = 
						*(uint64_t *)(p_skip + (iCtbH*(perCtbHeight / min_cb_size) + iCb)*cb_width + iCtbW*(perCtbHeight / min_cb_size));
					*(uint64_t *)(tab_ct_depth + iCb * 8) = 
						*(uint64_t *)(p_depth + (iCtbH*(perCtbHeight / min_cb_size) + iCb)*cb_width + iCtbW*(perCtbHeight / min_cb_size));
				}

				for (int iPu = 0; iPu < perCtbHeight/min_pu_size; iPu++)
				{
					*(uint64_t *)(tab_ipm + iPu * perCtbHeight / min_pu_size) = 
						*(uint64_t *)(p_ipm + (iCtbH*(perCtbHeight / min_pu_size) + iPu)*pu_width + iCtbW*(perCtbHeight / min_pu_size));
					
					if (perCtbHeight / min_pu_size > 8)
					{
						*(uint64_t *)(tab_ipm + iPu * perCtbHeight / min_pu_size + 8) =
							*(uint64_t *)(p_ipm + (iCtbH*(perCtbHeight / min_pu_size) + iPu)*pu_width + iCtbW*(perCtbHeight / min_pu_size) + 8);
					}
				}

				memset(status8x8, 0, 8 * 8);

				uint8_t *Y = tmpBufferY0;
				uint8_t *U = tmpBufferU0;
				uint8_t *V = tmpBufferV0;

				int initBuffer = 0;

				for (int y = 0; y < min_cb_size; y++)
				{
					if (iCtbH*perCtbHeight + y * 8 == pict->height) break;

					for (int x = 0; x < min_cb_size;)
					{
						if (status8x8[y*min_cb_size + x]) 
						{
							x++; 
							continue;
						}
						if (!initBuffer)
						{
							if (tab_ct_depth[y*min_cb_size + x] == 0 && tab_skip[y*min_cb_size + x] == 1)
								memcpy(Y, tmpBufferY1, 64 * 64 * 3 / 2);
							else
								memcpy(Y, tmpBufferY2, 64 * 64 * 3 / 2);
							initBuffer = 1;
						}

						if (tab_ct_depth[0] == 0)
						{
							int pX = x, pY = y;

							if (tab_skip[0] == 1)
							{
								FILL_COLOR(Y, U, V, 30, 30, 15, 15, ovGreenY, ovGreenCr, ovGreenCb);
							}
							else
							{
								memcpy(Y, tmpBufferY3, 64 * 64 * 3 / 2);
							}

							if (m_bDisplayQC)
							{
								OVERLAY_QC(Y, U, V, pX, pY, x, y, tab_ct_depth[0]);
							}

							if (m_bDisplayQP)
							{
								OVERLAY_QP(Y, U, V, pX, pY, x, y);
							}

							y = x = min_cb_size;

							break;
						}

						if (tab_ct_depth[y*min_cb_size + x] == 1)
						{
							int pX = x*min_cb_size;
							int pY = y*min_cb_size;

							if (tab_skip[y*min_cb_size + x] == 1)
							{
								FILL_COLOR(Y, U, V, (pX + 14), (pY + 14), (pX / 2 + 7), (pY / 2 + 7), ovGreenY, ovGreenCr, ovGreenCb);
							}

							if ((tab_ipm[y * 2 * (perCtbHeight / min_pu_size) + x * 2] & 0x80) == 0)
							{
								FILL_COLOR(Y, U, V, (pX + 14), (pY + 14), (pX / 2 + 7), (pY / 2 + 7), ovRedY, ovRedCr, ovRedCb);
							}

							if (m_bDisplayQC)
							{
								OVERLAY_QC(Y, U, V, pX, pY, x, y, tab_ct_depth[y*min_cb_size + x]);
							}

							if (m_bDisplayQP)
							{
								OVERLAY_QP(Y, U, V, pX, pY, x, y);
							}

							for (int i = 0; i < 4; i++)
								for (int j = 0; j < 4; j++)
									status8x8[(y + i)*min_cb_size + x + j] = 1;

							x += 4;
						}
						else
						if (tab_ct_depth[y*min_cb_size + x] == 2) //16x16
						{
							int pX = x*min_cb_size;
							int pY = y*min_cb_size;

							if (tab_skip[y*min_cb_size + x] == 1)
							{
								FILL_COLOR(Y, U, V, (pX + 6), (pY + 6), (pX / 2 + 3), (pY / 2 + 3), ovGreenY, ovGreenCr, ovGreenCb);
							}

							if ((tab_ipm[y * 2 * (perCtbHeight / min_pu_size) + x * 2] & 0x80) == 0)
							{
								FILL_COLOR(Y, U, V, (pX + 6), (pY + 6), (pX / 2 + 3), (pY / 2 + 3), ovRedY, ovRedCr, ovRedCb);
							}

							if (m_bDisplayQC)
							{
								OVERLAY_QC(Y, U, V, pX, pY, x, y, tab_ct_depth[y*min_cb_size + x]);
							}

							if (m_bDisplayQP)
							{
								OVERLAY_QP(Y, U, V, pX, pY, x, y);
							}

							for (int i = 0; i < 2; i++)
								for (int j = 0; j < 2; j++)
									status8x8[(y + i)*min_cb_size + x + j] = 1;

							for (int i = 0; i < 16; i++)
							{
								*(uint64_t *)(Y + (y * 8 + i) * 64 + x * 8 + 0) |= *(uint64_t *)(FRAME_0 + i * 16 + 0);
								*(uint64_t *)(Y + (y * 8 + i) * 64 + x * 8 + 8) |= *(uint64_t *)(FRAME_0 + i * 16 + 8);
							}

							x += 2;
						}
						else
						if (tab_ct_depth[y*min_cb_size + x] == 3) //8x8
						{
							int pX = x*min_cb_size;
							int pY = y*min_cb_size;

							if (tab_skip[y*min_cb_size + x] == 1)
							{
								FILL_COLOR(Y, U, V, (pX + 2), (pY + 2), (pX / 2 + 1), (pY / 2 + 1), ovGreenY, ovGreenCr, ovGreenCb);
							}

							if (!m_bDisplayQP && (tab_ipm[y * 2 * (perCtbHeight / min_pu_size) + x * 2] & 0x80) == 0)
							{
								FILL_COLOR(Y, U, V, (pX + 2), (pY + 2), (pX / 2 + 1), (pY / 2 + 1), ovRedY, ovRedCr, ovRedCb);

								uint8_t ipm0 = tab_ipm[y * 2 * (perCtbHeight / min_pu_size) + x * 2];
								uint8_t ipm1 = tab_ipm[(y * 2 + 1) * (perCtbHeight / min_pu_size) + x * 2];
								uint8_t ipm2 = tab_ipm[y * 2 * (perCtbHeight / min_pu_size) + x * 2 + 1];
								uint8_t ipm3 = tab_ipm[(y * 2 + 1) * (perCtbHeight / min_pu_size) + x * 2 + 1];

								if (ipm0 != ipm1 || ipm0 != ipm2 || ipm0 != ipm3)
								{
									for (int i = 0; i < 8; i++)
									{
										*(uint64_t *)(Y + (y * 8 + i) * 64 + x * 8 + 0) |= *(uint64_t *)(FRAME_4x4 + i * 8 + 0);
									}
								}
							}

							if (m_bDisplayQC)
							{
								OVERLAY_QC(Y, U, V, pX, pY, x, y, tab_ct_depth[y*min_cb_size + x]);
							}

							if (m_bDisplayQP)
							{
								OVERLAY_QP(Y, U, V, pX, pY, x, y);
							}

							status8x8[(y + 0)*min_cb_size + x + 0] = 1;

							if (!m_bDisplayQP && x%2 == 0 &&  y % 2 == 0)
							{
								for (int i = 0; i < 16; i++)
								{
									*(uint64_t *)(Y + (y * 8 + i) * 64 + x * 8 + 0) |= *(uint64_t *)(FRAME_3 + i * 16 + 0);
									*(uint64_t *)(Y + (y * 8 + i) * 64 + x * 8 + 8) |= *(uint64_t *)(FRAME_3 + i * 16 + 8);
								}
							}

							x += 1;
						}


					}
				}

				for (int y = 0; y < perCtbHeight; y++)
				{
					for (int x = 0; x < perCtbHeight/8; x++)
					{
						*(uint64_t *)(m_drawBuffer[0] + (iCtbH*perCtbHeight + y)*pict->linesize[0] + iCtbW*perCtbHeight + x * 8) =
							(*(uint64_t *)(pict->data[0] + (iCtbH*perCtbHeight + y)*pict->linesize[0] + iCtbW*perCtbHeight + x * 8) & 
							MAKE_OV_VAL(Y + y*perCtbHeight + x * 8)) |
							*(uint64_t *)(Y + y*perCtbHeight + x * 8);
					}

					if (iCtbH*perCtbHeight + y == pict->height - 1) break;
				}

				for (int y = 0; y < perCtbHeight/2; y++)
				{
					for (int x = 0; x < perCtbHeight / 16; x++)
					{
						*(uint64_t *)(m_drawBuffer[1] + (iCtbH*perCtbHeight / 2 + y)*pict->linesize[1] + iCtbW*perCtbHeight / 2 + x * 8) = //(m_bDisplayQP) ? 0x8080808080808080ULL :
							(*(uint64_t *)(pict->data[1] + (iCtbH*perCtbHeight / 2 + y)*pict->linesize[1] + iCtbW*perCtbHeight / 2 + x * 8) & 
							MAKE_OV_VAL(U + y*perCtbHeight / 2 + x * 8)) |
							*(uint64_t *)(U + y*perCtbHeight/2 + x * 8);

						*(uint64_t *)(m_drawBuffer[2] + (iCtbH*perCtbHeight / 2 + y)*pict->linesize[1] + iCtbW*perCtbHeight / 2 + x * 8) = //(m_bDisplayQP) ? 0x8080808080808080ULL :
							(*(uint64_t *)(pict->data[2] + (iCtbH*perCtbHeight / 2 + y)*pict->linesize[1] + iCtbW*perCtbHeight / 2 + x * 8) & 
							MAKE_OV_VAL(V + y*perCtbHeight / 2 + x * 8)) |
							*(uint64_t *)(V + y*perCtbHeight / 2 + x * 8);
					}

					if (iCtbH*perCtbHeight/2 + y == pict->height/2 - 1) break;
				}

			}

		}

	}

	if (tmpBufferY0)
	{
		free(tmpBufferY0);
		free(tmpBufferY1);
		free(tmpBufferY2);
		free(tmpBufferY3);
	}
}

void BsrVideoDecoder::PostProcPicture(unsigned char** rgbBuffer, int* size, int* width, int* height)
{
	if (rgbBuffer == NULL) return;

	m_picWidth = m_avctx->width;
	m_picHeight = m_avctx->height;

	if (m_bDisplayQP || m_bDisplayMB || m_bDisplayQC)
	{
		m_drawBuffer[0] = *rgbBuffer;
		m_drawBuffer[1] = *rgbBuffer + m_picture->linesize[0] * m_picHeight;
		m_drawBuffer[2] = *rgbBuffer + m_picture->linesize[0] * m_picHeight * 5 / 4;

		if (m_avctx->codec->id == AV_CODEC_ID_H265)
		{
			drawH265UI();
		}
		else
			if (m_avctx->codec->id == AV_CODEC_ID_H264) drawH264UI();
	}
	else
	{
		memcpy(*rgbBuffer, m_picture->data[0], m_picture->linesize[0] * m_picHeight);
		memcpy(*rgbBuffer + m_picture->linesize[0] * m_picHeight, m_picture->data[1], m_picture->linesize[0] * m_picHeight / 4);
		memcpy(*rgbBuffer + m_picture->linesize[0] * m_picHeight * 5 / 4, m_picture->data[2], m_picture->linesize[0] * m_picHeight / 4);
	}

	//for SDL
	if (width != NULL)
	{
		*width = m_picture->linesize[0];
	}
}

int BsrVideoDecoder::getFrame(unsigned char** rgbBuffer, int* size, int* width, int* height)
{
    int got_picture = 0;    // 找到帧标志
    int len = 0;
    AVPacket avpkt = {0};

    av_init_packet(&avpkt);
	av_frame_unref(m_picture);

	// av_read_fram返回下一帧，发生错误或文件结束返回<0
    while (av_read_frame(m_fmtctx, &avpkt) >= 0)
    {
        // 解码视频流
        if (avpkt.stream_index == m_videoStream)
        {
            len = avcodec_decode_video2(m_avctx, m_picture, &got_picture, &avpkt);

			if (m_avctx->codec->id == AV_CODEC_ID_H265)
			{
				HEVCContext *h = (HEVCContext *)m_avctx->priv_data;
				pic_arrays_copy(h, &hevcCTX[h->poc]);
			}

            if (len < 0)
            {
                debug("error while decoding frame.\n");
                av_packet_unref(&avpkt);
                return -1;
            }

            if (got_picture)
            {
				if (m_skipStep)
				{
					m_skipStep--;
					display_poc = (display_poc == -1) ? 0 : display_poc+1;
				}
				else
				{
					if (m_avctx->codec->id == AV_CODEC_ID_H265)
					{
						display_poc = (m_picture->pict_type == 1) ? 0 : display_poc + 1;
						Calc_Info_H265(&hevcCTX[display_poc]);
					}
					else
					{
						Calc_Info_H264((H264Context *)m_avctx->priv_data);
					}

					PostProcPicture(rgbBuffer, size, width, height);

					av_packet_unref(&avpkt);
					return 1;
				}
            } // end of got picture
            // 这里是否在上面的if中判断len的值？
            else
            {
				//m_errFrames++;
                //m_skippedFrame++;
                //debug("skipped count: %d\n", m_skippedFrame);
            }
        } // end of video stream

        av_packet_unref(&avpkt);
    } // end of read frame

	m_skippedFrame = 1;

    return 0;
}

int BsrVideoDecoder::getSkippedFrame(unsigned char** rgbBuffer, int* size, int* width, int* height)
{
    int got_picture = 0;    // 找到帧标志
    int len = 0;
    AVPacket avpkt = {0};;

    av_init_packet(&avpkt);
	av_frame_unref(m_picture);

    // 是否还有缓存的帧
    //while (m_skippedFrame-- > 0)
    //do
	{
        // 注：avpkt要清空data和size，否则无法解码
        avpkt.data = NULL;
        avpkt.size = 0;
        // 解码视频流 注：此时如解码成功，返回值为0
        len = avcodec_decode_video2(m_avctx, m_picture, &got_picture, &avpkt);
        if (len < 0)
        {
            debug("error while decoding frame.\n");
            av_packet_unref(&avpkt);
            return -1;
        }
        if (got_picture)
        {
			PostProcPicture(rgbBuffer, size, width, height);

			av_packet_unref(&avpkt);
            return 1;
        } // end of got picture

        av_packet_unref(&avpkt);
	} // end of read frame

	m_skippedFrame = 0;
	m_errFrames = 0;
	return 0;
}

int BsrVideoDecoder::writeYUVFile(const char* filename, const char* mode, int width, int height)
{
    int len = m_picture->linesize[0];
    FILE* fp = NULL;

	int ww = width;
	int hh = height;

	if (width == 0 || height == 0)
	{
		ww = m_avctx->width;
		hh = m_avctx->height;
	}

	//if the saving picture size has not different with the decoding size, the scaling will be ignored
	BOOL NeedSWS = (ww != m_avctx->width) | (hh != m_avctx->height);

	if (NeedSWS)
	{
		if (m_imgctxyuv)
		{
			av_free(m_frameYUV);
			av_free(m_bufferYUV);
			sws_freeContext(m_imgctxyuv);
		}

		m_frameYUV = av_frame_alloc();
		if (!m_frameYUV)
		{
			return -1;
		}
		int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, ww, hh, 1);// m_avctx->width);
		m_bufferYUV = (unsigned char *)av_malloc(size);
		if (!m_bufferYUV)
		{
			av_free(m_frameYUV);
			return -1;
		}
		av_image_fill_arrays(m_frameYUV->data, m_frameYUV->linesize, m_bufferYUV, AV_PIX_FMT_YUV420P, ww, hh, 1);// m_avctx->width);
		m_imgctxyuv = sws_getContext(
			m_avctx->width, m_avctx->height, m_avctx->pix_fmt,
			ww, hh, AV_PIX_FMT_YUV420P,
			SWS_BICUBIC, NULL, NULL, NULL);
		if (m_imgctxyuv == NULL)
		{
			av_free(m_frameYUV);
			av_free(m_bufferYUV);
			return -1;
		}

		sws_scale(m_imgctxyuv, m_picture->data, m_picture->linesize, 0, m_avctx->height,
			m_frameYUV->data, m_frameYUV->linesize);
	}

	if(mode == NULL)
		fp = fopen(filename, "wb");
	else
		fp = fopen(filename, mode);

	if (fp == NULL)
    {
        debug("open file %s failed.\n", filename);
        return -1;
    }

	//for NV12:data[0] data[2] data[1]
    fwrite(NeedSWS ? m_frameYUV->data[0] : m_picture->data[0], ww * hh, 1, fp);
    fwrite(NeedSWS ? m_frameYUV->data[1] : m_picture->data[1], ww * hh/4, 1, fp);
    fwrite(NeedSWS ? m_frameYUV->data[2] : m_picture->data[2], ww * hh/4, 1, fp);

	fclose(fp);
    return 0;
}

int BsrVideoDecoder::writeYUVFile(FILE *fp, int width, int height)
{
	if (fp == NULL)
	{
		debug("open file failed:writeYUVFile(fp).\n");
		return -1;
	}

	int len = m_picture->linesize[0];

	int ww = width;
	int hh = height;

	if (width == 0 || height == 0)
	{
		ww = m_avctx->width;
		hh = m_avctx->height;
	}

	//if the saving picture size has not different with the decoding size, the scaling will be ignored
	BOOL NeedSWS = (ww != m_avctx->width) | (hh != m_avctx->height);

	if (NeedSWS)
	{
		if (m_imgctxyuv)
		{
			av_free(m_frameYUV);
			av_free(m_bufferYUV);
			sws_freeContext(m_imgctxyuv);
		}

		m_frameYUV = av_frame_alloc();
		if (!m_frameYUV)
		{
			return -1;
		}
		int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, ww, hh, 1);// m_avctx->width);
		m_bufferYUV = (unsigned char *)av_malloc(size);
		if (!m_bufferYUV)
		{
			av_free(m_frameYUV);
			return -1;
		}
		av_image_fill_arrays(m_frameYUV->data, m_frameYUV->linesize, m_bufferYUV, AV_PIX_FMT_YUV420P, ww, hh, 1);// m_avctx->width);
		m_imgctxyuv = sws_getContext(
			m_avctx->width, m_avctx->height, m_avctx->pix_fmt,
			ww, hh, AV_PIX_FMT_YUV420P,
			SWS_BICUBIC, NULL, NULL, NULL);
		if (m_imgctxyuv == NULL)
		{
			av_free(m_frameYUV);
			av_free(m_bufferYUV);
			return -1;
		}

		sws_scale(m_imgctxyuv, m_picture->data, m_picture->linesize, 0, m_avctx->height,
			m_frameYUV->data, m_frameYUV->linesize);
	}


	//for NV12:data[0] data[2] data[1]
	fwrite(NeedSWS ? m_frameYUV->data[0] : m_picture->data[0], ww * hh, 1, fp);
	fwrite(NeedSWS ? m_frameYUV->data[1] : m_picture->data[1], ww * hh / 4, 1, fp);
	fwrite(NeedSWS ? m_frameYUV->data[2] : m_picture->data[2], ww * hh / 4, 1, fp);

	return 0;
}

unsigned char* BsrVideoDecoder::convertToRgb()
{
	sws_scale(m_imgctx, m_picture->data, m_picture->linesize, 0, m_avctx->height,
		m_frameRGB->data, m_frameRGB->linesize);

	return m_frameRGB->data[0];
}

int BsrVideoDecoder::writeBMPFile2(const char* filename)
{
    unsigned char* rgbBuffer = NULL;
    rgbBuffer = convertToRgb();
    return write_bmp_file(filename, rgbBuffer, m_picWidth, m_picHeight);
}

int BsrVideoDecoder::writeJPGFile(const char* filename)
{
    return 0;
}

