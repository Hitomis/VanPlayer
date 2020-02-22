//
// Created by 赵帆 on 2020-02-14.
//

#include "VanDecode.h"
#include "XLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/jni.h>
}

void VanDecode::registerHard(void *vm) {
    av_jni_set_java_vm(vm, 0);
}

bool VanDecode::open(XParameter param, bool isHard) {
    close();
    if (!param.codecParams) return false;
    AVCodecParameters *codecParams = param.codecParams;
    // 1. 查找解码器
    AVCodec *codec;
    if (isHard) { // 硬解码器
        codec = avcodec_find_decoder_by_name("h264_mediacodec");
    } else { // 软解码器
        codec = avcodec_find_decoder(codecParams->codec_id);
    }
    if (!codec) {
        XLOGE("avcodec_find_decoder %d failed!", codecParams->codec_id);
        return false;
    }
    XLOGE("avcodec_find_decoder success %d", isHard);
    mux.lock();
    // 2 创建上下文，复制参数
    this->codecCxt = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(this->codecCxt, codecParams);
    this->codecCxt->thread_count = 2;
    // 3 打开解码器
    int re = avcodec_open2(this->codecCxt, codec, nullptr);
    if (re != 0) {
        XLOGE("avcodec_open2 failed");
        mux.unlock();
        return false;
    }
    this->isAudio = codecCxt->codec_type == AVMEDIA_TYPE_AUDIO;
    mux.unlock();
    return true;
}

bool VanDecode::sendPacket(XData &packet) {
    if (packet.size <= 0 || !packet.data)return false;
    mux.lock();
    if (!codecCxt) {
        mux.unlock();
        return false;
    }
    int re = avcodec_send_packet(codecCxt, (AVPacket *) packet.data);
    mux.unlock();
    return re == 0;

}

XData VanDecode::receiveFrame() {
    mux.lock();
    XData data;
    if (!codecCxt) {
        mux.unlock();
        return data;
    }
    if (!frame) {
        frame = av_frame_alloc();
    }
    int re = avcodec_receive_frame(codecCxt, frame);
    if (re != 0) {
        mux.unlock();
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
    data.format = frame->format;
    data.pts = frame->pts;
    pts = data.pts;
    memcpy(data.datas, frame->data, sizeof(data.datas));
    mux.unlock();
    return data;
}

void VanDecode::close() {
    IDecode::clear();
    mux.lock();
    pts = 0;
    if (frame) {
        av_frame_free(&frame);
    }
    if (codecCxt) {
        avcodec_close(codecCxt);
        avcodec_free_context(&codecCxt);
    }
    mux.unlock();
}

void VanDecode::clear() {
    IDecode::clear();
    mux.lock();
    if (codecCxt)
        avcodec_flush_buffers(codecCxt);
    mux.unlock();
}

