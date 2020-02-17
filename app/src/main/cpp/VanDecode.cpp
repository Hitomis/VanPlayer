//
// Created by 赵帆 on 2020-02-14.
//

#include "VanDecode.h"
#include "XLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

bool VanDecode::open(XParameter param) {
    if (!param.codecParams) return false;
    AVCodecParameters *codecParams = param.codecParams;
    // 1. 查找解码器
    AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        XLOGE("avcodec_find_decoder %d failed!", codecParams->codec_id);
        return false;
    }
    XLOGE("avcodec_find_decoder success");
    // 2 创建上下文，复制参数
    this->codecCxt = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(this->codecCxt, codecParams);
    this->codecCxt->thread_count = 2;
    // 3 打开解码器
    int re = avcodec_open2(this->codecCxt, codec, nullptr);
    if (re != 0) {
        XLOGE("avcodec_open2 failed");
        return false;
    }
    this->isAudio = codecCxt->codec_type == AVMEDIA_TYPE_AUDIO;
    return true;
}

bool VanDecode::sendPacket(XData &packet) {
    if (packet.size <= 0 || !packet.data)return false;
    if (!codecCxt) {
        return false;
    }
    int re = avcodec_send_packet(codecCxt, (AVPacket *) packet.data);
    if (re != 0) {
        return false;
    }

    return true;
}

XData &VanDecode::receiveFrame() {
    XData data;
    if (!codecCxt) return data;
    if (!frame) {
        frame = av_frame_alloc();
    }
    int re = avcodec_receive_frame(codecCxt, frame);
    if (re != 0) {
        return data;
    }

    data.data = reinterpret_cast<unsigned char *>(frame);
    if (codecCxt->codec_type == AVMEDIA_TYPE_VIDEO) {
        data.size = (frame->linesize[0] + frame->linesize[1] + frame->linesize[2]) * frame->height;
        data.width = frame->width;
        data.height = frame->height;
    } else {
        //样本字节数 * 单通道样本数 * 通道数
        data.size = av_get_bytes_per_sample((AVSampleFormat) frame->format) * frame->nb_samples * 2;
    }
    memcpy(data.datas, frame->data, sizeof(data.datas));
    return data;
}
