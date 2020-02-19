//
// Created by 赵帆 on 2020-02-16.
//

#include "VanResample.h"
#include "XLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

bool VanResample::open(XParameter inPar, XParameter outPar) {
    // 初始化音频重采样上下文
    actx = swr_alloc();
    //  重采样上下文的参数设置
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(outPar.channels),
                              AV_SAMPLE_FMT_S16,
                              outPar.sampleRate,
                              inPar.codecParams->channel_layout,
                              (AVSampleFormat) inPar.codecParams->format,
                              inPar.codecParams->sample_rate,
                              0, nullptr);
    int rect = swr_init(actx);
    if (rect != 0) {
        XLOGE("swr_init failed");
        return false;
    }
    outChannels = inPar.codecParams->channels;
    outFormat = AV_SAMPLE_FMT_S16;
    return true;
}

XData VanResample::resample(XData &data) {
//    XLOGE("resample data size is %d", data.size);
    XData outData;
    if (!actx || data.size <= 0 || !data.data) return outData;
    // 输出空间的分配
    auto *frame = reinterpret_cast<AVFrame *>(data.data);
    // 通道数 * 单通道样本数 * 样本字节大小
    int size = outChannels * frame->nb_samples * av_get_bytes_per_sample(
            static_cast<AVSampleFormat>(outFormat));
    if (size <= 0) return outData;
    outData.alloc(size);

    uint8_t *outArray[2] = {nullptr};
    outArray[0] = outData.data;
    // 音频重采样
    int len = swr_convert(actx, outArray, frame->nb_samples, (const uint8_t **) frame->data,
                          frame->nb_samples);
    if (len <= 0) {
        outData.drop();
        return outData;
    }
//    XLOGE("swr_convert success = %d", len);
    return outData;
}
