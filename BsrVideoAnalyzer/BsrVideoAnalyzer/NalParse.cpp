#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <memory.h>

#include "BsrVideoAnalyzerDlg.h"
#include "NaLParse.h"

//For CPP call C function
extern "C"
{
//#include "Audio_G726/bsrAudioCodec.h"
#include "libavformat/avformat.h"
}

#define SAVE_OTHER_FILE 1
#define MAX_BUFFER_SIZE (1024*1024*2)

#define	BIT_RATE	4	     //2, 3, 4, 5.
#define SAMPLES     16000	//8000
#define SAMPLE_BITS 16

#define SAVE_OTHER_FILE 1

typedef char      s8;    //有符号8位整数
typedef short     s16;   //有符号16位整数
typedef int       s32;   //有符号32位整数
typedef struct WAVE_TAG
{
	s8     riff[4];            //"RIFF",资源交换文件标志
	s32    fsize;              //文件大小(从下个地址开始到文件尾的总字节数)
	s8     wave[4];            //"WAVE",文件标志
}WAVE_TAG;

typedef struct WAVE_FMT
{
	s8     fmt[4];             //"fmt ",波形格式标志
	s32    chunsize;           //文件内部格式信息大小
	s16    wformattag;         //音频数据编码方式 1PCM
	s16    wchannels;          //声道数 1,2
	s32    dwsamplespersec;    //采样率 441000
	s32    dwavgbytespersec;   //波形数据传输速率（每秒平均字节数）
	s16    wblockalign;        //数据的调整数（按字节计算）
	s16    wbitspersample;     //样本数据位数
}WAVE_FMT;

typedef struct WAVE_DAT
{
	s8     data[4];            //"data",数据标志符
	s32    datasize;           //采样数据总长度
}WAVE_DAT;


inline int llfseek(FILE* fp, fpos_t Offset, int Origin)
{
	return _fseeki64(fp, Offset, Origin);
}


int saveToWav(FILE *pPcmFile, FILE *pWavFile, int writeBytes, int samples)
{
	if (NULL == pPcmFile || NULL == pWavFile)
		return -1;

	WAVE_TAG tag;
	WAVE_FMT fmt;
	WAVE_DAT dat;

	tag.riff[0] = 'R';	tag.riff[1] = 'I';	tag.riff[2] = 'F';	tag.riff[3] = 'F';
	tag.fsize = 44 - 8 + writeBytes;
	tag.wave[0] = 'W';	tag.wave[1] = 'A';	tag.wave[2] = 'V';	tag.wave[3] = 'E';

	fmt.fmt[0] = 'f';	fmt.fmt[1] = 'm';	fmt.fmt[2] = 't';	fmt.fmt[3] = ' ';
	fmt.chunsize = 0x10;
	fmt.wformattag = 0x01;
	fmt.wchannels = 0x01;
	fmt.dwsamplespersec = samples;
	fmt.dwavgbytespersec = samples * SAMPLE_BITS / 8;
	fmt.wblockalign = 2;
	fmt.wbitspersample = SAMPLE_BITS;

	dat.data[0] = 'd';	dat.data[1] = 'a';	dat.data[2] = 't';	dat.data[3] = 'a';
	dat.datasize = writeBytes;

	fwrite(&tag, sizeof(tag), 1, pWavFile);
	fwrite(&fmt, sizeof(fmt), 1, pWavFile);
	fwrite(&dat, sizeof(dat), 1, pWavFile);

	unsigned char buffer[1024] = { 0 };
	int bytesLeft = writeBytes;
	while (bytesLeft > 0)
	{
		if (bytesLeft < 1024)
		{
			fread(buffer, 1, bytesLeft, pPcmFile);
			fwrite(buffer, 1, bytesLeft, pWavFile);
			break;
		}

		fread(buffer, 1, 1024, pPcmFile);
		fwrite(buffer, 1, 1024, pWavFile);
		bytesLeft -= 1024;
		//printf("left: %d\n", bytesLeft);
	}
	return 0;
}


int GetCodecInfo(const char *in_filename, CodecInfo* m_pCodecInfo)
{
	AVFormatContext *ifmt_ctx = NULL;
	int ret = 0;
	int videoindex = -1, audioindex = -1;
	//m_pCodecInfo->type = FILE_H265;
	//return ret;
	av_register_all();
	//Input
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		printf("Could not open input file.");
		return -1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		return -1;
	}

	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
		}
		else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioindex = i;
		}
	}

	//		rc_max_rate	128000	__int64
	//		codec_id	AV_CODEC_ID_AAC (86018)	AVCodecID
	//		sample_rate	44100	int
	//		channels	2	int
	//		sample_fmt	AV_SAMPLE_FMT_FLTP (8)	AVSampleFormat
	//		const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt);
	//		const AVCodecDescriptor *av_codec_get_codec_descriptor(const AVCodecContext *avctx);


	if (ifmt_ctx->streams[videoindex]->codec->codec_id == AV_CODEC_ID_H265)
	{
		//ret = AV_CODEC_ID_H265;
		m_pCodecInfo->type = FILE_H265;
	}
	else
		if (ifmt_ctx->streams[videoindex]->codec->codec_id == AV_CODEC_ID_H264)
		{
			m_pCodecInfo->type = FILE_H264;
		}
		else
		{
			ret = -1;
		}

	m_pCodecInfo->framerate = (float)ifmt_ctx->streams[videoindex]->avg_frame_rate.num / ifmt_ctx->streams[videoindex]->avg_frame_rate.den;
	m_pCodecInfo->bitrate = ifmt_ctx->streams[videoindex]->codec->bit_rate;

	avformat_close_input(&ifmt_ctx);
	return ret;

}
/*
void ShowProgessTransFile(int size0, int size1)
{
	extern char g_mess[256];
#define SIZE10M (1024*1024*10)
	if (size0 / SIZE10M != size1 / SIZE10M)
	{
		sprintf(g_mess, "Saving the media file's raw video: %d", size1);
	}
}

void Clear_G_Mess()
{
	extern char g_mess[256];
	g_mess[0] = 0;
}
*/
#define USE_H264BSF 1
#define SAVE_AUDIO  0
int CNalParser::demuxFile(const char *in_filename, char *moreInfo, CodecInfo *m_pCodecInfo)
{
	AVFormatContext *ifmt_ctx = NULL;
	AVPacket pkt;
	int ret = 0;
	int videoindex = -1, audioindex = -1;
	//const char *in_filename = "cuc_ieschool.flv";//Input file URL
	char out_filename_v[256];//Output file URL
#if SAVE_AUDIO
	char out_filename_a[256];//Output file URL
	sprintf(out_filename_a, "%s.mp3", in_filename);
#endif
	av_register_all();
	//Input
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		printf("Could not open input file.");
		return -1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		return -1;
	}

	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
		}
		else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioindex = i;
		}
	}

	if (videoindex == -1)
	{
		ret = -2;
		goto error;
	}
	//		rc_max_rate	128000	__int64
	//		codec_id	AV_CODEC_ID_AAC (86018)	AVCodecID
	//		sample_rate	44100	int
	//		channels	2	int
	//		sample_fmt	AV_SAMPLE_FMT_FLTP (8)	AVSampleFormat
	//		const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt);
	//		const AVCodecDescriptor *av_codec_get_codec_descriptor(const AVCodecContext *avctx);

	/*
	FIX: H.264 in some container format (FLV, MP4, MKV etc.) need
	"h264_mp4toannexb" bitstream filter (BSF)
	*Add SPS,PPS in front of IDR frame
	*Add start code ("0,0,0,1") in front of NALU
	H.264 in some container (MPEG2TS) don't need this BSF.
	*/
#if USE_H264BSF

	char codecName[256];
	if (ifmt_ctx->streams[videoindex]->codec->codec_id == AV_CODEC_ID_H265)
	{
		sprintf(codecName, "hevc_mp4toannexb");
		m_pCodecInfo->type = FILE_H265;
		sprintf(out_filename_v, "%s.h265", in_filename);
	}
	else
		if (ifmt_ctx->streams[videoindex]->codec->codec_id == AV_CODEC_ID_H264)
		{
			sprintf(codecName, "h264_mp4toannexb");
			sprintf(out_filename_v, "%s.h264", in_filename);
			m_pCodecInfo->type = FILE_H264;
		}
		else
		{
			avformat_close_input(&ifmt_ctx);
			return -1;
		}
	m_pCodecInfo->framerate = (float)ifmt_ctx->streams[videoindex]->avg_frame_rate.num / ifmt_ctx->streams[videoindex]->avg_frame_rate.den;
	m_pCodecInfo->bitrate = ifmt_ctx->streams[videoindex]->codec->bit_rate;

	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init(codecName);// "h264_mp4toannexb");
	uint8_t *oBuffer = NULL;
	int oSize = 0;

	fpos_t wSize = 0;
#endif

	FILE *fp_video = fopen(out_filename_v, "wb+");
#if SAVE_AUDIO
	FILE *fp_audio = fopen(out_filename_a, "wb+");
#endif

	if (audioindex >= 0)
	{

		AVCodecDescriptor *avd_a = (AVCodecDescriptor *)av_codec_get_codec_descriptor(ifmt_ctx->streams[audioindex]->codec);
		if (avd_a)
		{
			sprintf(moreInfo, "\r\n===Audio Info===\r\n");
			sprintf(moreInfo, "%sAudio Codec Name: %s\r\n", moreInfo, avd_a->long_name);
			sprintf(moreInfo, "%sbitrate: %dbps\r\n", moreInfo, ifmt_ctx->streams[audioindex]->codec->rc_max_rate);
			sprintf(moreInfo, "%ssample rate: %dHz\r\n", moreInfo, ifmt_ctx->streams[audioindex]->codec->sample_rate);
			sprintf(moreInfo, "%schannels: %d\r\n", moreInfo, ifmt_ctx->streams[audioindex]->codec->channels);
			if (ifmt_ctx->streams[audioindex]->codec->sample_fmt == AV_SAMPLE_FMT_FLTP)
				sprintf(moreInfo, "%ssample bits: float(%dbit)\r\n", moreInfo, 8 * sizeof(float));
			else
				sprintf(moreInfo, "%ssample bits: %s\r\n", moreInfo, av_get_sample_fmt_name(ifmt_ctx->streams[audioindex]->codec->sample_fmt));
		}
	}

	av_init_packet(&pkt);

	while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
		if (pkt.stream_index == videoindex) {
#if USE_H264BSF
			int ret1 = av_bitstream_filter_filter(h264bsfc, ifmt_ctx->streams[videoindex]->codec, NULL, &oBuffer, &oSize, pkt.data, pkt.size, 0);
#endif
			if (ret1 == 0)
			{
				fwrite(pkt.data, 1, pkt.size, fp_video);
				((BsrVideoAnalyzerDlg *)m_pCaller)->ShowParsingProgess(wSize, wSize + pkt.size);
				wSize += pkt.size;
			}
			else
			{
				fwrite(oBuffer, 1, oSize, fp_video);
				((BsrVideoAnalyzerDlg *)m_pCaller)->ShowParsingProgess(wSize, wSize + oSize);
				wSize += oSize;
			}
			if (oBuffer && oBuffer != pkt.data)
				av_freep(&oBuffer);
		}
#if SAVE_AUDIO
		else if (pkt.stream_index == audioindex) {
			/*
			AAC in some container format (FLV, MP4, MKV etc.) need to add 7 Bytes
			ADTS Header in front of AVPacket data manually.
			Other Audio Codec (MP3...) works well.
			*/
			fwrite(pkt.data, 1, pkt.size, fp_audio);
		}
#endif
		av_packet_unref(&pkt);
	}

	m_pCodecInfo->filesize = wSize;

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif

	fclose(fp_video);
#if SAVE_AUDIO
	fclose(fp_audio);
#endif

error:
	avformat_close_input(&ifmt_ctx);

	//Clear_G_Mess();

	return ret;
}


int GetDetailNALU(NALU_t *nalu)
{
	if (!nalu) return -1;
	nalu->strType[0] = 0;
	nalu->strInfo[0] = 0;

	int ret = 0;

	if (nalu->type == 0)
	{
		// NAL单元类型
		switch (nalu->nalType)
		{
		case NAL_UNIT_TYPE_UNSPECIFIED:
			sprintf(nalu->strType, "Unspecified");
			break;
		case NAL_UNIT_TYPE_CODED_SLICE_NON_IDR:
			sprintf(nalu->strType, "Coded slice of a non-IDR picture");
			switch (nalu->sliceType)
			{
			case 0:
			case 5:
				setPFrame(ret);
				break;
			case 1:
			case 6:
				setBFrame(ret);
				break;
			case 2:
			case 7:
				setISlice(ret);
				break;
			}
			break;
		case NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A:
			sprintf(nalu->strType, "DPA");
			break;
		case NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B:
			sprintf(nalu->strType, "DPB");
			break;
		case NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C:
			sprintf(nalu->strType, "DPC");
			break;
		case NAL_UNIT_TYPE_CODED_SLICE_IDR:
			sprintf(nalu->strType, "Coded slice of an IDR picture");
			setIFrame(ret);
			break;
		case NAL_UNIT_TYPE_SEI:
			sprintf(nalu->strType, "Supplemental enhancement information");
			sprintf(nalu->strInfo, "SEI");
			setPPS(ret);
			break;
		case NAL_UNIT_TYPE_SPS:
			sprintf(nalu->strType, "Sequence parameter set");
			sprintf(nalu->strInfo, "SPS");
			setPPS(ret);
			break;
		case NAL_UNIT_TYPE_PPS:
			sprintf(nalu->strType, "Picture parameter set");
			sprintf(nalu->strInfo, "PPS");
			setPPS(ret);
			break;
		case NAL_UNIT_TYPE_AUD:
			sprintf(nalu->strType, "Access UD");
			sprintf(nalu->strInfo, "AUD");
			break;
		case NAL_UNIT_TYPE_END_OF_SEQUENCE:
			sprintf(nalu->strType, "END_SEQUENCE");
			break;
		case NAL_UNIT_TYPE_END_OF_STREAM:
			sprintf(nalu->strType, "END_STREAM");
			break;
		case NAL_UNIT_TYPE_FILLER:
			sprintf(nalu->strType, "FILLER_DATA");
			break;
		case NAL_UNIT_TYPE_SPS_EXT:
			sprintf(nalu->strType, "SPS_EXT");
			break;
		case NAL_UNIT_TYPE_CODED_SLICE_AUX:
			sprintf(nalu->strType, "AUXILIARY_SLICE");
			break;
		default:
			sprintf(nalu->strType, "Other");
			break;
		}
	}
	else
	{
		// NAL单元类型
		switch (nalu->nalType)
		{
			// to confirm type...
		case NAL_UNIT_CODED_SLICE_TRAIL_N:
		case NAL_UNIT_CODED_SLICE_TRAIL_R:
			sprintf(nalu->strType, "Coded ... non-TSA, non-STSA trailing picture");
			switch (nalu->sliceType)
			{
			case H265_SH_SLICE_TYPE_B:
				setBFrame(ret);
				break;
			case H265_SH_SLICE_TYPE_P:
				setPFrame(ret);
				break;
			case H265_SH_SLICE_TYPE_I:
				setIFrame(ret);
				break;
			}
			break;
		case NAL_UNIT_CODED_SLICE_TSA_N:
		case NAL_UNIT_CODED_SLICE_TSA_R:
			sprintf(nalu->strType, "Coded slice segment of a TSA picture");
			switch (nalu->sliceType)
			{
			case H265_SH_SLICE_TYPE_B:
				setBFrame(ret);
				break;
			case H265_SH_SLICE_TYPE_P:
				setPFrame(ret);
				break;
			case H265_SH_SLICE_TYPE_I:
				setIFrame(ret);
				break;
			}
			break;
		case NAL_UNIT_CODED_SLICE_RADL_N:
		case NAL_UNIT_CODED_SLICE_RADL_R:
			sprintf(nalu->strType, "Coded slice segment of a TSA picture");
			switch (nalu->sliceType)
			{
			case H265_SH_SLICE_TYPE_B:
				setBFrame(ret);
				break;
			case H265_SH_SLICE_TYPE_P:
				setPFrame(ret);
				break;
			case H265_SH_SLICE_TYPE_I:
				setIFrame(ret);
				break;
			}
			break;
		case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
		case NAL_UNIT_CODED_SLICE_IDR_N_LP:
			sprintf(nalu->strType, "Coded slice of an IDR picture");
			setIFrame(ret);
			break;
		case NAL_UNIT_PREFIX_SEI:
		case NAL_UNIT_SUFFIX_SEI:
			sprintf(nalu->strType, "Supplemental enhancement information");
			sprintf(nalu->strInfo, "SEI");
			setPPS(ret);
			break;
		case NAL_UNIT_VPS:
			sprintf(nalu->strType, "Video parameter set");
			sprintf(nalu->strInfo, "VPS");
			setPPS(ret);
			break;
		case NAL_UNIT_SPS:
			sprintf(nalu->strType, "Sequence parameter set");
			sprintf(nalu->strInfo, "SPS");
			setPPS(ret);
			break;
		case NAL_UNIT_PPS:
			sprintf(nalu->strType, "Picture parameter set");
			sprintf(nalu->strInfo, "PPS");
			setPPS(ret);
			break;
		case NAL_UNIT_AUD:
			sprintf(nalu->strType, "Access UD");
			sprintf(nalu->strInfo, "AUD");
			break;
		case NAL_UNIT_EOS:
			sprintf(nalu->strType, "END_SEQUENCE");
			break;
		case NAL_UNIT_EOB:
			sprintf(nalu->strType, "END_STREAM");
			break;
		case NAL_UNIT_FILLER_DATA:
			sprintf(nalu->strType, "FILLER_DATA");
			break;
		default:
			sprintf(nalu->strType, "Unknown");
			break;
		}
	}

	return ret;
}


CNalParser::CNalParser()
{
	m_nType = FILE_H264; // default
	m_hH264 = NULL;
	m_hH265 = NULL;
	m_naluData = NULL;
	m_pFile = NULL;
	m_outputInfo = NULL;
}

CNalParser::~CNalParser()
{
	release();
}

int CNalParser::init(const char* filename, CTreeCtrl* tree, char* moreInfo)
{
	int ret = 0;

	release();

	memset(m_tmpStore, '\0', 1024);

	if (!m_outputInfo)
		m_outputInfo = (char *)malloc(OUTPUT_SIZE);
	if (m_outputInfo)
		memset(m_outputInfo, '\0', OUTPUT_SIZE);


	sprintf(m_filename, "%s", filename);

	m_pMoreInfo = moreInfo;
	*m_pMoreInfo = 0;

	cInfo.type = FILE_UNK;
	cInfo.filesize = 0;
	cInfo.bitrate = 0;
	cInfo.framerate = 0.0;

	// judge file 
	m_nType = preProcVideoFile(m_filename);

	if (m_nType == FILE_UNK)
	{
		return -1;
	}
	// init handle
	else if (m_nType == FILE_H265)
	{
		m_hH265 = h265_new();
	}
	else
	{
		m_hH264 = h264_new();
	}

	if (tree != NULL)
	{
		m_pTree = tree;
		m_pTree->DeleteAllItems();
	}

	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	m_pFile = fopen(m_filename, "r+b");

	return ret;
}

int CNalParser::release(void)
{
	if (m_hH264 != NULL)
	{
		h264_free(m_hH264);
		m_hH264 = NULL;
	}
	if (m_hH265 != NULL)
	{
		h265_free(m_hH265);
		m_hH265 = NULL;
	}
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	if (m_naluData != NULL)
	{
		free(m_naluData);
		m_naluData = NULL;
	}
	if (m_outputInfo)
	{
		free(m_outputInfo);
		m_outputInfo = NULL;
	}

	m_willBreak = FALSE;
	return 0;
}

#define SET_NALU_INFO(ret, nalu) \
{\
	if (isIFrame(ret)) \
		sprintf(nalu->strInfo, "IDR #%d", nalu->sliceIndex); \
	if (isPFrame(ret)) \
		sprintf(nalu->strInfo, "P Slice #%d-%d", nalu->sliceIndex, nalu->poc); \
	if (isBFrame(ret)) \
		sprintf(nalu->strInfo, "B Slice #%d-%d", nalu->sliceIndex, nalu->poc); \
	if (isISlice(ret)) \
		sprintf(nalu->strInfo, "I Slice #%d-%d", nalu->sliceIndex, nalu->poc); \
}

//int CNalParser::parseBuffer(const char *inBuffer, vector<NALU_t>& vNal, ParsingNaluPara_t &inPara){}

int CNalParser::probeNALU(vector<NALU_t>& vNal, int num)
{
	NALU_t n;
	int nal_num = 0;

	memset(&n, '\0', sizeof(NALU_t));

	n.type = m_nType;

	llfseek(m_pFile, 0, SEEK_END);

	fpos_t fileLen = 0;
	fgetpos(m_pFile, &fileLen);
	llfseek(m_pFile, 0, SEEK_SET);

#define READ_BUFFER_SIZE (2*1024*1024)

	char *rBuffer = (char *)malloc(READ_BUFFER_SIZE + 48);

	fpos_t pos = 0;
	int iOff = 4, prev = 0;
	int findFirst = 0;

	int readBytes = fread(rBuffer, 1, READ_BUFFER_SIZE, m_pFile);
	int bufferLen = readBytes;

	int needMerge = 0;

	char *frameBuf0 = (char *)malloc(READ_BUFFER_SIZE*2);

	int lenFrameBuf0 = 0;
	int end = 0;

	int m_picGOP = 0;
	int m_nSliceIndex = 0;

	int has_I = -1;


	int prev_val = 0, curr_val = 0, prev_poc = 0, iFramePoc = -1;

	for (int i = 0; i < bufferLen - 4; )
	{
		unsigned int val4Bytes = *(unsigned int *)(rBuffer + i);
		unsigned int val3Bytes = (*(unsigned int *)(rBuffer + i)) & 0x00ffffff;
		int scLen = 0;

		if (val4Bytes == 0x01000000) scLen = 4;
		else
		{
			if (val3Bytes == 0x00010000) scLen = 3;
		}

		if (scLen > 0)
		{
			if (findFirst)
			{
				n.len = (UINT)(pos - n.offset);
				n.num = nal_num;
				nal_num++;
				//

				uint8_t *tempPoint = NULL;
				if (needMerge)
				{
					memcpy(frameBuf0 + lenFrameBuf0, rBuffer, i + 1);
					needMerge = 0; lenFrameBuf0 = 0;
					tempPoint = (uint8_t *)frameBuf0;
				}
				else
				{
					tempPoint = (uint8_t *)rBuffer + prev;
				}

				NALU_t *nalu = &n;
				if (nalu->type)
				{
					if (scLen == 3)
						sprintf(nalu->startcodeBuffer, "00 00 01 %02x %02x", *tempPoint, *(tempPoint+1));
					else
						sprintf(nalu->startcodeBuffer, "00 00 00 01 %02x %02x", *tempPoint, *(tempPoint + 1));

					m_hH265->sh->read_slice_type = 1;
					h265_read_nal_unit(m_hH265, tempPoint, nalu->len - nalu->startcodeLen);
					nalu->nalType = m_hH265->nal->nal_unit_type;
					nalu->sliceType = m_hH265->sh->slice_type;
					//hcg
					int ret = GetDetailNALU(nalu);
					nalu->isFrame = ret;

					if (has_I == -1 && notPPS(ret))
						has_I = 1;

					if (has_I == 1 && isPPS(ret))
					{
						m_picGOP++;
						has_I = 0;
					}

					if (isIFrame(ret))
					{
						iFramePoc = m_nSliceIndex;
						prev_val = 0;
						prev_poc = 0;
						has_I = 1;
					}

					int poc_lsb = 1 << (m_hH265->sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
					curr_val = m_hH265->sh->slice_pic_order_cnt_lsb;

					int factor = (prev_val - curr_val > 15) ? 1 :
						((curr_val - prev_val > 15) ? -1 : 0);

					nalu->poc = ((prev_poc / poc_lsb) + factor) * poc_lsb + curr_val;
					prev_poc = nalu->poc;
					prev_val = curr_val;
					nalu->poc += iFramePoc;

					//nalu->poc = m_hH265->sh->slice_pic_order_cnt_lsb;

					nalu->gop = m_picGOP;
					nalu->sliceIndex = m_nSliceIndex;

					if (notPPS(ret))
						m_nSliceIndex++;

					SET_NALU_INFO(ret, nalu);

					m_hH265->sh->read_slice_type = 0;
					nalu->firstMB = 0;
				}
				else
				{
					if (scLen == 3)
						sprintf(nalu->startcodeBuffer, "00 00 01 %02x", *tempPoint);
					else
						sprintf(nalu->startcodeBuffer, "00 00 00 01 %02x", *tempPoint);
					m_hH264->sh->read_slice_type = 1;
					int retErr = read_nal_unit(m_hH264, tempPoint, nalu->len - nalu->startcodeLen);
					nalu->nalType = m_hH264->nal->nal_unit_type;
					nalu->sliceType = m_hH264->sh->slice_type;

					//hcg
					int ret = GetDetailNALU(nalu);
					nalu->isFrame = ret;

					if (has_I == -1 && notPPS(ret))
						has_I = 1;

					if (has_I == -1 && isPPS(ret))
						has_I = 0;

					if (has_I == 1 && isPPS(ret))
					{
						m_picGOP++;
						has_I = 0;
					}

					if (isIFrame(ret))
					{
						iFramePoc = m_nSliceIndex;
						prev_val = 0;
						prev_poc = 0;
						has_I = 1;
					}

					if ((isBPFrame(ret) | isIFrame(ret)) && m_hH264->sh->first_mb_in_slice > 0 && m_nSliceIndex > 0)
						m_nSliceIndex--;

					if (notPPS(ret))
					{
						if (m_hH264->sps->pic_order_cnt_type == 0)
						{
							curr_val = m_hH264->sh->pic_order_cnt_lsb;
#if 0
							int poc_lsb = 1 << (m_hH264->sps->log2_max_frame_num_minus4 + 4);
							if (m_hH264->sps->log2_max_frame_num_minus4 == 0)
								poc_lsb = 1 << (m_hH264->sps->log2_max_pic_order_cnt_lsb_minus4 + 4 - 1);
#else
							int poc_lsb = 1 << (m_hH264->sps->log2_max_pic_order_cnt_lsb_minus4 + 4);

							while (curr_val >= poc_lsb) 
								poc_lsb *= 2;
							poc_lsb >>= 1;
#endif
							int factor = (prev_val - curr_val > 30) ? 1 :
								((curr_val - prev_val > 30) ? -1 : 0);

							nalu->poc = ((prev_poc / poc_lsb) + factor) * poc_lsb + curr_val / 2;

							prev_poc = nalu->poc;
							prev_val = curr_val;

							nalu->poc += iFramePoc;
						}
						else
							nalu->poc = m_nSliceIndex;
					}

					nalu->gop = m_picGOP;
					//nalu->poc = m_hH264->sh->pic_order_cnt_lsb;
					nalu->sliceIndex = m_nSliceIndex;

					if (notPPS(ret))
						m_nSliceIndex++;

					SET_NALU_INFO(ret, nalu);

					m_hH264->sh->read_slice_type = 0;
					if (retErr < 0 && retErr != -1)
					{
						n.errOffset = (((unsigned int)retErr) & 0x7fffffff) + 2;
					}
				}


				vNal.push_back(n);
				//memset(&n, '\0', sizeof(NALU_t));
				n.errOffset = 0;

				if (end) break;
			}
			else
				findFirst = 1;

			n.offset = pos;
			n.startcodeLen = scLen;

			pos += scLen;
			i += scLen;
			prev = i;

		}
		else
		{
			pos++;
			i++;
		}

		if (i >= bufferLen - 4)
		{
			iOff = bufferLen - i;

			//当startcode在最后几个字节时的处理
			for (int j = 0; j < iOff; j++)
				*(rBuffer + j) = *(rBuffer + bufferLen - iOff + j);

			//当一个NALU在两个buffer中时的处理
			if (findFirst)
			{
				needMerge = 1;
				memcpy(frameBuf0 + lenFrameBuf0, rBuffer + prev, bufferLen - prev - iOff);
				lenFrameBuf0 += bufferLen - prev - iOff;
				prev = 0;
			}
			fpos_t fileReadPos = 0;
			fgetpos(m_pFile, &fileReadPos);
			readBytes = fread(rBuffer + iOff, 1, READ_BUFFER_SIZE, m_pFile);
			fgetpos(m_pFile, &fileReadPos);

			//
			((BsrVideoAnalyzerDlg *)m_pCaller)->ShowParsingProgess(0, fileReadPos, fileLen);

			if (m_willBreak) break;

			//文件结束时的处理
			if (fileReadPos==fileLen)
			{
				end = 1;
				*(int*)(rBuffer + iOff + readBytes) = 0x01000000;
				*(int*)(rBuffer + iOff + readBytes + 4) = 0x0;
				readBytes += 8;
			}

			//初始化变量，再次开始循环
			bufferLen = iOff + readBytes;
			i = 0;
		}
	}

	cInfo.frames = m_nSliceIndex;

	free(frameBuf0);
	free(rBuffer);

	return 0;
}

int CNalParser::parseNALU(NALU_t& vNal, char** naluData, char** naluInfo)
{
	if (m_naluData != NULL)
	{
		free(m_naluData);
		m_naluData = NULL;
	}
	m_naluData = (uint8_t *)malloc(vNal.len);



	llfseek(m_pFile, vNal.offset, SEEK_SET);
	fread(m_naluData, vNal.len, 1, m_pFile);

	// 不需要再次查询nal
	//find_nal_unit(m_naluData, vNal.len, &nal_start, &nal_end);
	memset(m_outputInfo, '\0', OUTPUT_SIZE); // 输出内容先清空
	if (m_nType == 1)
	{
		// 此函数分析时，是不包含startcode的，所以要减去startcodeLen
		h265_read_nal_unit(m_hH265, &m_naluData[vNal.startcodeLen], vNal.len - vNal.startcodeLen);
		h265_debug_nal_t(m_hH265, m_hH265->nal);    // 打印到m_outputInfo中
	}
	else
	{
		read_nal_unit(m_hH264, &m_naluData[vNal.startcodeLen], vNal.len - vNal.startcodeLen);
		h264_debug_nal_t(m_hH264, m_hH264->nal);  // 打印到m_outputInfo中
	}

	*naluData = (char*)m_naluData;
	*naluInfo = m_outputInfo;

	return 0;
}

FileType CNalParser::checkVideoType(FILE* fp)
{
	FileType type = FILE_UNK; // default

	llfseek(fp, 0, SEEK_SET);

#undef READ_BUFFER_SIZE
#define READ_BUFFER_SIZE (4*1024*1024)

	char *rBuffer = (char *)malloc(READ_BUFFER_SIZE + 4);


	int readBytes = fread(rBuffer, 1, READ_BUFFER_SIZE, fp);

	int scLen = 0, i = 0;

	for (i = 0; i < readBytes - 8 && scLen == 0; i++)
	{
		unsigned int val4Bytes = *(unsigned int *)(rBuffer + i);
		unsigned int val3Bytes = (*(unsigned int *)(rBuffer + i)) & 0x00ffffff;

		if (val4Bytes == 0x01000000) scLen = 4;
		else
		{
			if (val3Bytes == 0x00010000) scLen = 3;
		}

	}

	if (scLen > 0)
	{
		// check h265 first...
		unsigned char nalHeader = *(rBuffer + scLen);
		unsigned char nalHeader1 = *(rBuffer + scLen + 1);
		unsigned char nalType = (nalHeader >> 1) & 0x3f; // 6 bit
		unsigned char nuh_layer_id = ((nalHeader & 0x1) << 5) | ((nalHeader1 >> 3));
		unsigned char nuh_layer_id_plus1 = nalHeader1 & 0x7;

		if (nalType >= 0 && nalType <= 47 && nuh_layer_id == 0 && nuh_layer_id_plus1 == 1) // ok
		{
			type = FILE_H265;
		}
		else
		{
			nalType = nalHeader & 0x1f; // 5 bit


			if (nalType > 0 && nalType < 22) // ok
			{
				type = FILE_H264;
			}
		}
	}

	free(rBuffer);
	return type;
}

FileType CNalParser::preProcVideoFile(char* filename)
{
	char szExt[16] = { 0 };
	FileType type = FILE_UNK; // default

	_splitpath(filename, NULL, NULL, NULL, szExt);
	if (!strcmp(&szExt[1], "h265") || !strcmp(&szExt[1], "265") ||
		!strcmp(&szExt[1], "hevc") ||
		!strcmp(&szExt[1], "h264") || !strcmp(&szExt[1], "264") ||
		!strcmp(&szExt[1], "avc"))
	{
		FILE* fp = NULL;
		fp = fopen(filename, "r+b");
		llfseek(fp, 0, SEEK_END);
		fgetpos(fp,	&cInfo.filesize);
		fclose(fp);

		GetCodecInfo(filename, &cInfo);
		return cInfo.type;
	}
	else if (!strcmp(&szExt[1], "mp4") || !strcmp(&szExt[1], "avi") || !strcmp(&szExt[1], "asf") ||
		     !strcmp(&szExt[1], "mkv") || !strcmp(&szExt[1], "mov") || !strcmp(&szExt[1], "flv"))
	{
		// read content 
		FILE* fp = NULL;

		char filename_bsr[MAX_PATH];
		sprintf(filename_bsr, "%s.bin", filename);

		//将媒体文件demux，取视频流
		int ret = demuxFile(filename, m_pMoreInfo, &cInfo);

		if (ret == -2) return FILE_UNK;

		if (ret == -1 || cInfo.type == FILE_UNK) return type;

		if (cInfo.type == FILE_H264)
			sprintf(filename, "%s.h264", filename);
		if (cInfo.type == FILE_H265)
			sprintf(filename, "%s.h265", filename);

		return cInfo.type;
	}

	return type;
}
