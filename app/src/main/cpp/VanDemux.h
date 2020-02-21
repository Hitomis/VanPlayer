//
// Created by 赵帆 on 2020-02-14.
//

#import "IDemux.h"
#include <mutex>

#ifndef VANPLAYER_VANDEMUX_H
#define VANPLAYER_VANDEMUX_H

struct AVFormatContext;

class VanDemux : public IDemux {
public:
    bool open(const char *url) override;

    void close() override;

    XData read() override;

    XParameter getVideoPar() override;

    XParameter getAudioPar() override;

    bool seek(double progress) override;

    VanDemux();

private:
    AVFormatContext *fmtCtx = 0;
    std::mutex mux;
    int videoStreamIndex = 0;
    int audioStreamIndex = 0;
};


#endif //VANPLAYER_VANDEMUX_H
