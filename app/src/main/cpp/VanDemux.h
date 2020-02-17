//
// Created by 赵帆 on 2020-02-14.
//

#import "IDemux.h"

#ifndef VANPLAYER_VANDEMUX_H
#define VANPLAYER_VANDEMUX_H

struct AVFormatContext;

class VanDemux : public IDemux {
public:
    bool open(const char *url) override;

    XData &read() override;

    XParameter &getVideoPar() override;

    XParameter &getAudioPar() override;

    VanDemux();

    // 多媒体文件总时长（毫秒）
    int totalMs = 0;

private:
    AVFormatContext *fmtCtx = 0;
    int videoStreamIndex = 0;
    int audioStreamIndex = 0;
};


#endif //VANPLAYER_VANDEMUX_H
