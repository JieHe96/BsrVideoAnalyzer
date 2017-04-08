#include "stdafx.h" // for MFC

#include <stdio.h>

#include "H264ToVideo.h"

H264BS2Video::H264BS2Video()
    :m_infctx(NULL),
    m_outfctx(NULL),
    m_instream(NULL),
    m_outstream(NULL),
    m_videoidx(-1),
    m_isfile(0)
{
}

H264BS2Video::~H264BS2Video()
{    
    close();
}

int H264BS2Video::openVideoFile(const char* rawfile, const char* videofile, int width, int height, int fps, int gop, int bitrate)
{
    int ret = 0;

    av_register_all(); // 注册协议，等

	// 从文件判断视频格式
    ret = avformat_open_input(&m_infctx, rawfile, NULL, NULL);
    if (ret != 0)
    {
        debug("open input file failed. ret: %d\n", ret);
        return -1;
    }

    ret = avformat_find_stream_info(m_infctx, NULL);
    if (ret < 0)
    {
        debug("find stream info failed. ret: %d\n", ret);
        return -1;
    }

    // 找到视频流
    for (unsigned int i = 0; i < m_infctx->nb_streams; i++)
    {
        if (m_infctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoidx = i;
            m_instream = m_infctx->streams[i];
            break;
        }
    }
    if (m_videoidx == -1)
    {
        debug("no video stream.\n");
        return -1;
    }

    avformat_alloc_output_context2(&m_outfctx, NULL, NULL, videofile);

    m_outstream = avformat_new_stream(m_outfctx, NULL);
    if (!m_outstream)
    {
        debug("avformat_new_stream failed.\n");
        return -1;
    }

    // 复制
    avcodec_copy_context(m_outstream->codec, m_instream->codec);

    if (m_outfctx->oformat->flags & AVFMT_GLOBALHEADER)
        m_outstream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if (!(m_outfctx->flags & AVFMT_NOFILE))
    {
        if (avio_open(&m_outfctx->pb, videofile, AVIO_FLAG_WRITE)<0)
        {
            debug("avio_open failed.\n");
            return -1;
        }
    }
    if (!m_outfctx->nb_streams)
    {
        debug("nb_streams failed.\n");
        return -1;
    }

    ret = avformat_write_header(m_outfctx, NULL);
    if (ret < 0)
    {
        debug("avformat_write_header failed %d\n", ret);
        return -1;
    }

    m_isfile = 1;

    return 0;
}

int H264BS2Video::writeFrame(void)
{
    AVPacket avpkt = { 0 };
    int idx = 0;


    av_init_packet(&avpkt);

    // av_read_fram返回下一帧，发生错误或文件结束返回<0
    while (av_read_frame(m_infctx, &avpkt) >= 0)
    {
        // 解码视频流
        if (avpkt.stream_index == m_videoidx)
        {
            //debug("write %d, size: %d\n", idx++, avpkt.size);

            if (avpkt.pts == AV_NOPTS_VALUE)
            {
                // 计算PTS/DTS
                AVRational time_base = m_instream->time_base;
                int64_t duration=(int64_t)((double)AV_TIME_BASE/(double)av_q2d(m_instream->r_frame_rate));
                avpkt.pts=(int64_t)((double)(idx*duration)/(double)(av_q2d(time_base)*AV_TIME_BASE));
                avpkt.dts=avpkt.pts;
                avpkt.duration=(int)((double)duration/(double)(av_q2d(time_base)*AV_TIME_BASE));

                // 转换 PTS/DTS
                avpkt.pts = av_rescale_q_rnd(avpkt.pts, m_instream->time_base, m_outstream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                avpkt.dts = av_rescale_q_rnd(avpkt.dts, m_instream->time_base, m_outstream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                avpkt.duration = (int)av_rescale_q(avpkt.duration, m_instream->time_base, m_outstream->time_base);

                idx++;
#if 0
                sprintf(buffer, "111 %d pts: %d dts: %d duration: %d\n", idx, avpkt.pts, avpkt.dts, avpkt.duration);
                fwrite(buffer, 1, strlen(buffer), fp);
#endif
            }

            if (av_interleaved_write_frame(m_outfctx, &avpkt) < 0)
            {
                debug("write frame failed.\n");
                break;
            }
        }

        av_packet_unref(&avpkt);

        avpkt.data = NULL;
        avpkt.size = 0;
    }

    //fclose(fp);

    return 0;
}

int H264BS2Video::close()
{
    if ( m_outfctx != NULL )
    {
        //printf("write trailer....\n");
        av_write_trailer(m_outfctx);
        if (m_isfile)
        {
            //printf("close...\n");
            avio_close(m_outfctx->pb); // 关闭文件
        }
        avformat_free_context(m_outfctx);
        m_outfctx = NULL;
        avformat_close_input(&m_infctx);
        m_infctx = NULL;
    }

    return 0;
}
