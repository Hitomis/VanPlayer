//
// Created by 赵帆 on 2020-02-18.
//

#ifndef VANPLAYER_IPLAYER_H
#define VANPLAYER_IPLAYER_H

#include <mutex>
#include "XThread.h"
#include "XParameter.h"

class IDemux;

class IDecode;

class IResample;

class IVideoView;

class IAudioPlay;

class IPlayer : public XThread {
public:
    static IPlayer *getInstance(unsigned int index = 0);
    virtual bool open(const char *path);
    virtual void initWindow(void *win);

    bool start() override;

    //是否视频硬解码
    bool isHardDecode = false;

    // 音频输出参数
    XParameter outPar;

    IDemux *demux = 0;
    IDecode *audioDecode = 0;
    IDecode *videoDecode = 0;
    IResample *resample = 0;
    IVideoView *videoView = 0;
    IAudioPlay *audioPlay = 0;

protected:
    IPlayer() {};

    void run() override;

    std::mutex mux;


};


#endif //VANPLAYER_IPLAYER_H
