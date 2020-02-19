//
// Created by 赵帆 on 2020-02-18.
//

#include "IPlayer.h"
#include "IDemux.h"
#include "IDecode.h"
#include "IAudioPlay.h"
#include "IVideoView.h"
#include "IResample.h"
#include "XLog.h"

IPlayer *IPlayer::getInstance(unsigned int index) {
    static IPlayer player[256];
    return &player[index];
}

bool IPlayer::open(const char *path) {
    mux.lock();
    if (!demux || !demux->open(path)) {
        mux.unlock();
        XLOGE("VanPlayer::Demux open failed");
        return false;
    }
    outPar = demux->getAudioPar();
    if (!audioDecode || !audioDecode->open(demux->getAudioPar())) {
        mux.unlock();
        XLOGE("VanPlayer::AudioDecode open failed");
    }
    if (!videoDecode || !videoDecode->open(demux->getVideoPar(), isHardDecode)) {
        mux.unlock();
        XLOGE("VanPlayer::videoDecode open failed");
    }

    // 重采样 有可能不需要，解码后或者解封后可能是直接能播放的数据
    if (!resample || !resample->open(demux->getAudioPar(), outPar)) {
        XLOGE("VanPlayer::videoDecode open failed");
    }
    mux.unlock();
    return true;
}

bool IPlayer::start() {
    mux.lock();
    if (!demux || !demux->start()) {
        mux.unlock();
        XLOGE("VanPlayer::Demux start failed");
        return false;
    }
    if (videoDecode) videoDecode->start(); // 解码并下发视频流数据
    if (audioDecode) audioDecode->start(); // 解码并下发音频流数据
    if (audioPlay) audioPlay->startPlay(outPar); // 开始播放音频
    XThread::start();
    mux.unlock();
    return true;
}


void IPlayer::run() {
    XThread::run();
}

void IPlayer::initWindow(void *win) {
    if (videoView) {
        videoView->setRender(win);
    }
}
