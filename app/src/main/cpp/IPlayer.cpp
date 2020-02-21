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
    close();
    mux.lock();
    if (!demux || !demux->open(path)) {
        mux.unlock();
        XLOGE("VanPlayer::Demux open failed");
        return false;
    }
    outPar = demux->getAudioPar();
    if (!audioDecode || !audioDecode->open(demux->getAudioPar())) {
        XLOGE("VanPlayer::AudioDecode open failed");
    }
    if (!videoDecode || !videoDecode->open(demux->getVideoPar(), isHardDecode)) {
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
    if (videoDecode) videoDecode->start(); // 解码并下发视频流数据
    if (audioPlay) audioPlay->startPlay(outPar); // 开始播放音频
    if (audioDecode) audioDecode->start(); // 解码并下发音频流数据
    if (!demux || !demux->start()) {
        mux.unlock();
        XLOGE("VanPlayer::Demux start failed");
        return false;
    }
    XThread::start();
    mux.unlock();
    return true;
}

void IPlayer::run() {
    while (!isExit) {
        mux.lock();

        if (!audioPlay || !videoDecode) {
            mux.unlock();
            XSleep(2);
            continue;
        }

        //同步
        //获取音频的 pts 告诉视频
        int apts = audioPlay->pts;
        videoDecode->synPts = apts;

        mux.unlock();
        XSleep(2);
    }
    XThread::run();
}

void IPlayer::initWindow(void *win) {
    if (videoView) {
        videoView->close();
        videoView->setRender(win);
    }
}

void IPlayer::close() {
    mux.lock();

    // 1.先关闭线程执行
    XThread::stop();
    // 解封装 - 音视频解码器
    if (demux)
        demux->stop();
    if (videoDecode)
        videoDecode->stop();
    if (audioDecode)
        audioDecode->stop();
    if (audioPlay)
        audioPlay->stop();

    // 2.清理缓冲队列
    if (videoDecode)
        videoDecode->clear();
    if (audioDecode)
        audioDecode->clear();
    if (audioPlay)
        audioPlay->clear();

    // 3.清理资源
    if (audioPlay)
        audioPlay->close();
    if (videoView)
        videoView->close();
    if (videoDecode)
        videoDecode->close();
    if (audioDecode)
        audioDecode->close();
    if (demux)
        demux->close();

    mux.unlock();
}
