/**

H.264码流转可播放视频

*/

#ifndef H264TOVIDEO_H
#define H264TOVIDEO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}


//#define _LL_DEBUG_

// low level debug
#ifdef _LL_DEBUG_
    #ifndef debug
    #define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #endif
    #define LL_DEBUG(fmt, ...) printf("[DEBUG %s().%d @ %s]: " fmt, __func__, __LINE__, P_SRC, ##__VA_ARGS__)
#else
     #define debug(fmt, ...)
    #define LL_DEBUG(fmt, ...)
#endif


class H264BS2Video
{
public:
    H264BS2Video();
    ~H264BS2Video();

public:
	int openVideoFile(const char* rawfile, const char* videofile, int width=1920, int height=1080, int fps = 25, int gop = 10, int bitrate = 2097152);
    int writeFrame(void);
    int close(void);

private:
    AVFormatContext *m_infctx;
    AVFormatContext *m_outfctx;
    AVStream *m_instream;
    AVStream *m_outstream;
    int m_videoidx;
    int m_isfile;
};

#endif // H264TOVIDEO_H
