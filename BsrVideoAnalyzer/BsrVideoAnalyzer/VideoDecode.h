#ifndef _VIDEODECODE_H
#define _VIDEOECODE_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"
#include "libavutil/mem.h"
#include "libavutil/avutil.h"

//#include "forhevc/cabac.h"
//#include "forhevc/get_bits.h"
//#include "forhevc/hevcpred.h"
//#include "forhevc/hevcdsp.h"
//#include "internal.h"
//#include "forhevc/thread.h"
//#include "forhevc/videodsp.h"

}

#include "hevc_moreinfo.h"

#ifdef WIN32
// ��̬��
//#pragma comment(lib, "libgcc.a") // divdi3(), etc.
//#pragma comment(lib, "libmingwex.a") // snprintf()....
//#pragma comment(lib, "libiconv.a") // libiconv_open(), etc.

#pragma comment(lib, "libavcodec.a")
#pragma comment(lib, "libavformat.a")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libswscale.a")
#pragma comment(lib, "libswresample.a")

#endif



#define _DEBUG_

typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef long            LONG;

// 2�ֽڶ���
#pragma pack(2)
typedef struct tagMYBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} MYBITMAPFILEHEADER;

// 40
// �������Ϊ2�ֽڶ��룬����Ҫ�ָ�Ϊԭ��Ĭ��8�ֽڶ���
#pragma pack(8)
typedef struct tagMYBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} MYBITMAPINFOHEADER;

class BsrVideoDecoder {
public:
    BsrVideoDecoder();
    ~BsrVideoDecoder();

    /**
    * openVideoFile - ��H264��Ƶ
    * 
    * @param  avifile H264��ʽ��AVI��Ƶ�ļ�����
    *
    * @return 0 - �ɹ�   -1 - ʧ��(�����Ҳ����������������ڴ治�ɹ�)
    */  
    int openVideoFile(const char* avifile);

    /**
    * closeVideoFile - �ر���Ƶ���ͷ���Դ
    * 
    */
    void closeVideoFile(void);

    /**
    * jumpToTime - ����ָ��ʱ��
    *
    * @param     time��ʱ�䣬����Ϊ��λ
    *
    * @return    0 - �ɹ�   -1 ����ת���ɹ���ָ��ʱ�䳬����Ƶ��ʱ���С��0
    */
    int jumpToTime(int64_t time);
	int seek(int64_t pos);
	void SetSkipStep(int step) { m_skipStep = step; };
    /**
    * getFrame - ����Ƶ�л�ȡһ֡����
    * 
    * @param[OUT] rgbBuffer ����������RGB����ָ��
    * @param[OUT] size      ����������ԭʼ���ݴ�С
    * @param[OUT] width     ������������
    * @param[OUT] height    ������������
    *
    * @return  1 - �ɹ�   0 - û�ж���֡(��Ƶ������)  -1 - ����ʧ��
    *
    * @note  ���������еĿ�͸�ָ��Ƶ���ݵĿ�͸�
    *        ԭʼ����ΪYUV420P��ʽ
    *        ��Щ��Ƶ����I֡��ʼ������jumpToTime(0)��������ʼ��I֡
    *        �ú�������av_read_frame���᷵��һ��֡����>0������Ƶ�л������ݣ����Լ�����ȡ
    */
    int getFrame(unsigned char** rgbBuffer = NULL, int* size = NULL, int* width = NULL, int* height = NULL);

    /**
    * getSkippedFrame - ��ȡffmpeg�����֡
    * 
    * @param[OUT] rgbBuffer ����������RGB����ָ��
    * @param[OUT] size      ����������ԭʼ���ݴ�С
    * @param[OUT] width     ������������
    * @param[OUT] height    ������������
    *
    * @return  1 - �ɹ�   0 - û�л���֡(��Ƶ������)  -1 - ����ʧ��
    *
    * @note  �е�֡A��Ҫ�ο������֡B������˳��ffmpeg��û�н⵽B֡��û��������A���Ỻ��������
    *        ���������ȡffmpeg�������л��е�֡��
    *        ���ʣ�����ǿ�ͷ��֡�������⣬�����ŵ�������������õ���֡�ǿ�ʼ�Ļ������ģ��������Խ��������������
    */
    int getSkippedFrame(unsigned char** rgbBuffer = NULL, int* size = NULL, int* width = NULL, int* height = NULL);

    /**
    * writeYUVFile - дYUV�ļ���ָ���ļ�
    *
    * @param   filename  ͼƬ�ļ�����
    *
    * @return  0 - �ɹ�  -1 - д�ļ�ʧ��
    *
    */
    int writeYUVFile(const char* filename, const char* mode=NULL, int width=0, int height=0);
	int writeYUVFile(FILE *fp, int width, int height);
    /**
    * writeBmpFile - дBMPͼƬ��ָ���ļ�
    *
    * @param   filename  ͼƬ�ļ�����
    *
    * @return  0 - �ɹ�  -1 - д�ļ�ʧ��
    *
    */
    int writeBMPFile2(const char* filename);

    /**
    * writeBmpFile - дJPEGͼƬ��ָ���ļ�
    *
    * @param   filename  ͼƬ�ļ�����
    *
    * @return  0 - �ɹ�  -1 - д�ļ�ʧ��
    *
    */
    int writeJPGFile(const char* filename);

    int writeJPGFile2(const char* filename);

	void SetDisplayQC() {
		m_bDisplayQC = !m_bDisplayQC;
		m_bDisplayMB = FALSE;
		//m_bDisplayMB = (m_bDisplayMB & m_bDisplayQP) ? FALSE : m_bDisplayMB;
	}

	void SetDisplayQP() { 
		m_bDisplayQP = !m_bDisplayQP; 
		//m_bDisplayQC = FALSE;
		//m_bDisplayMB = (m_bDisplayMB & m_bDisplayQP) ? FALSE : m_bDisplayMB;
	}
	void SetDisplayMB() { 
		m_bDisplayMB = !m_bDisplayMB;
		m_bDisplayQC = FALSE;
		//m_bDisplayQP = (m_bDisplayMB & m_bDisplayQP) ? FALSE : m_bDisplayQP;
	}
	char *StrStatusQP() {
		return (m_bDisplayQP) ? "QP-ON" : "QP-OFF";
	}
	char *StrStatusQC() {
		return (m_bDisplayQC) ? "QP_C-ON" : "QP_C-OFF";
	}
	char *StrStatusMB() {
		return (m_bDisplayMB) ? "MB-ON" : "MB-OFF";
	}
	void PostProcPicture(unsigned char** rgbBuffer = NULL, int* size = NULL, int* width = NULL, int* height = NULL);
	void Calc_Info_H264(H264Context *h);
	void Calc_Info_H265(HEVCContext *h);
	char *getCodecInfo() { return m_cCodecInfo; };
	int getFrameType() {
		if (m_picture)
			return (int)m_picture->pict_type;
		return 0;
	};
	BOOL NeedFlushCodec() { return m_skippedFrame == 1; };
	int GetErrFrames() { return m_errFrames; };
	void SetErrFrames(int val = 0) { m_errFrames = val; };
private:
    /**
    *  convertToRgb - �������������ת��ΪRGB��ʽ(ʵ��ΪBGR24)
    *
    * @return ����RGB����ָ��
    */
    unsigned char* convertToRgb();
	void drawH264UI();
	void drawH265UI();

private:
	int m_errFrames;
    int m_skippedFrame;
    int m_picWidth;
    int m_picHeight;

	//hcg
	BOOL m_bDisplayQP, m_bDisplayMB, m_bDisplayQC;
	int m_skipStep;

    int m_videoStream;    // ��Ƶ������
    unsigned char* m_picBuffer;
	unsigned char* m_drawBuffer[3];

	HEVCContext *hevcCTX;
	int display_poc;

	char m_cCodecInfo[256];

    AVFormatContext* m_fmtctx;
    AVCodecContext* m_avctx;
    AVFrame* m_picture;
    AVFrame* m_frameRGB;
    unsigned char* m_bufferYUV;
    AVFrame* m_frameYUV;
    struct SwsContext* m_imgctx;
    struct SwsContext* m_imgctxyuv;
};
#endif