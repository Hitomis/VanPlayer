//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_IDEMUX_H
#define VANPLAYER_IDEMUX_H


#include "XData.h"
#include "XParameter.h"
#include "XThread.h"
#include "IObserver.h"

class IDemux : public IObserver {
public:
    // 打开文件，或者流媒体 rmtp，http, rtsp
    virtual bool open(const char *url) = 0;

    // 获取视频参数
    virtual XParameter getVideoPar() = 0;

    // 获取音频参数
    virtual XParameter getAudioPar() = 0;

    //读取一帧数据，数据由调用者清理
    virtual XData read() = 0;

    //总时长（毫秒）
    int totalMs = 0;

protected:
public:
    void run() override;
};


#endif //VANPLAYER_IDEMUX_H
