//
// Created by 赵帆 on 2020-02-14.
//

#include "VanDemux.h"
#include "XLog.h"

extern "C" {
#include "include/libavformat/avformat.h"
#include "include/libavcodec/avcodec.h"
}

static double r2d(AVRational r) {
    return r.den == 0 || r.num == 0 ? 0 : (double) r.num / r.den;
}

bool VanDemux::open(const char *url) {
    close();
    mux.lock();
    int re = avformat_open_input(&fmtCtx, url, nullptr, nullptr);
    if (re != 0) {
        XLOGE("avformat_open_input %s failed", url);
        mux.unlock();
        return false;
    }
    XLOGE("VanDemux open %s success", url);
    re = avformat_find_stream_info(fmtCtx, nullptr);
    if (re != 0) {
        XLOGE("avformat_find_stream_info failed");
        mux.unlock();
        return false;
    }
    // duration 单位是微秒, 媒体文件总时长， 可能没有，如果没有就去
    this->totalMs = fmtCtx->duration / (AV_TIME_BASE / 1000);
    mux.unlock();
    getVideoPar();
    getAudioPar();
    return true;
}

XParameter VanDemux::getVideoPar() {
    mux.lock();
    XParameter parameter;
    if (!this->fmtCtx) {
        mux.unlock();
        return parameter;
    }
    // 获取视频流索引
    int re = av_find_best_stream(this->fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (re < 0) {
        XLOGE("av_find_best_stream video stream index failed");
        mux.unlock();
        return parameter;
    }
    videoStreamIndex = re;
    parameter.codecParams = fmtCtx->streams[re]->codecpar;
    mux.unlock();
    return parameter;
}

XParameter VanDemux::getAudioPar() {
    mux.lock();
    XParameter parameter;
    if (!this->fmtCtx) {
        mux.unlock();
        return parameter;
    }
    // 获取音频流索引
    int re = av_find_best_stream(this->fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (re < 0) {
        XLOGE("av_find_best_stream audio stream index failed");
        mux.unlock();
        return parameter;
    }
    audioStreamIndex = re;
    parameter.codecParams = fmtCtx->streams[re]->codecpar;
    parameter.channels = parameter.codecParams->channels;
    parameter.sampleRate = parameter.codecParams->sample_rate;
    mux.unlock();
    return parameter;
}


XData VanDemux::read() {
    mux.lock();
    XData data;
    if (!fmtCtx) {
        mux.unlock();
        return data;
    }
    AVPacket *pkt = av_packet_alloc();
    int re = av_read_frame(fmtCtx, pkt);
    if (re != 0) {
        av_packet_free(&pkt);
        mux.unlock();
        return data;
    }
//    XLOGE("pack size is %d, pts %lld", pkt->size, pkt->pts);
    data.data = reinterpret_cast<unsigned char *>(pkt);
    data.size = pkt->size;
    if (pkt->stream_index == audioStreamIndex) {
        data.isAudio = true;
    } else if (pkt->stream_index == videoStreamIndex) {
        data.isAudio = false;
    } else {
        av_packet_free(&pkt);
        mux.unlock();
        return data;
    }

    // 转换 pts
    pkt->pts = pkt->pts * 1000 * r2d(fmtCtx->streams[pkt->stream_index]->time_base);
    pkt->dts = pkt->dts * 1000 * r2d(fmtCtx->streams[pkt->stream_index]->time_base);
    data.pts = pkt->pts;
    mux.unlock();
    return data;
}

VanDemux::VanDemux() {
    static bool isFirst = true;
    if (isFirst) {
        isFirst = false;
        av_register_all();
        avformat_network_init();
        avcodec_register_all();
        XLOGE("register ffmpeg");
    }
}

void VanDemux::close() {
    mux.lock();
    if (this->fmtCtx) {
        avformat_close_input(&fmtCtx);
    }
    mux.unlock();
}

bool VanDemux::seek(double progress) {
    if (progress < 0 || progress > 1) {
        return false;
    }
    mux.lock();
    if (!fmtCtx) {
        mux.unlock();
        return false;
    }
    // 清理读取的缓冲
    avformat_flush(fmtCtx);
    long long seekPts = fmtCtx->streams[videoStreamIndex]->duration * progress;
    // 以向后寻找关键帧的方式跳转到 progress 位置
    av_seek_frame(fmtCtx, videoStreamIndex, seekPts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    mux.unlock();
    return true;
}


