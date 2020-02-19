//
// Created by 赵帆 on 2020-02-14.
//

#include "VanDemux.h"
#include "XLog.h"

extern "C" {
#include "include/libavformat/avformat.h"
#include "include/libavcodec/avcodec.h"
}

bool VanDemux::open(const char *url) {
    int re = avformat_open_input(&fmtCtx, url, nullptr, nullptr);
    if (re != 0) {
        XLOGE("avformat_open_input %s failed", url);
        return false;
    }
    XLOGE("VanDemux open %s success", url);
    re = avformat_find_stream_info(fmtCtx, nullptr);
    if (re != 0) {
        XLOGE("avformat_find_stream_info failed");
        return false;
    }
    // duration 单位是微秒
    this->totalMs = fmtCtx->duration / (AV_TIME_BASE / 1000);
    getVideoPar();
    getAudioPar();
    return true;
}

XParameter VanDemux::getVideoPar() {
    XParameter parameter;
    if (!this->fmtCtx) return parameter;
    // 获取视频流索引
    int re = av_find_best_stream(this->fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (re < 0) {
        XLOGE("av_find_best_stream video stream index failed");
        return parameter;
    }
    videoStreamIndex = re;

    parameter.codecParams = fmtCtx->streams[re]->codecpar;
    return parameter;
}

XParameter VanDemux::getAudioPar() {
    XParameter parameter;
    if (!this->fmtCtx) return parameter;
    // 获取音频流索引
    int re = av_find_best_stream(this->fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (re < 0) {
        XLOGE("av_find_best_stream audio stream index failed");
        return parameter;
    }
    audioStreamIndex = re;
    parameter.codecParams = fmtCtx->streams[re]->codecpar;
    parameter.channels = parameter.codecParams->channels;
    parameter.sampleRate = parameter.codecParams->sample_rate;
    return parameter;
}


XData VanDemux::read() {
    XData data;
    if (!fmtCtx) return data;
    AVPacket *pkt = av_packet_alloc();
    int re = av_read_frame(fmtCtx, pkt);
    if (re != 0) {
        av_packet_free(&pkt);
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
        return data;
    }
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


