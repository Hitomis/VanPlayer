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
    if (!demux || !demux->start()) {
        mux.unlock();
        XLOGE("VanPlayer::Demux start failed");
        return false;
    }
    if (audioDecode) audioDecode->start(); // 解码并下发音频流数据
    if (audioPlay) audioPlay->startPlay(outPar); // 开始播放音频
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

double IPlayer::getPlayPos() {
    double pos = 0.0;
    mux.lock();
    if (demux && demux->totalMs > 0 && videoDecode) {
        pos = (double) videoDecode->pts / (double) demux->totalMs;
    }
    mux.unlock();
    return pos;
}

bool IPlayer::seek(double progress) {
    if (!demux) return false;
    // 暂停播放
    pause(true);

    mux.lock();

    // 清理缓冲队列
    if (videoDecode)
        videoDecode->clear(); // 清理了缓冲队列和 ffmpeg 的内部缓冲
    if (audioDecode)
        audioDecode->clear();
    if (audioPlay)
        audioPlay->clear();

    bool re = demux->seek(progress);

    if (!videoDecode) {
        mux.unlock();
        pause(false);
        return re;
    }

    // 解码到实际需要显示的帧
    int seekPts = progress * demux->totalMs;
    while (!isExit) {
        XData pkt = demux->read();
        if (pkt.size <= 0) break;
        if (pkt.isAudio) {
            if (pkt.pts < seekPts) {
                pkt.drop();
                continue;
            }
            // 写入缓冲队列
            demux->notify(pkt);
            continue;
        }

        // 解码需要显示的帧之前的数据
        videoDecode->sendPacket(pkt);
        pkt.drop();
        XData data = videoDecode->receiveFrame();
        if (data.size <= 0) continue;
        if (data.pts >= seekPts) { // 跳出开始显示
            break;
        }
    }

    mux.unlock();
    pause(false);
    return re;
}

void IPlayer::pause(bool pause) {
    mux.lock();
    XThread::pause(pause);
    if (demux) demux->pause(pause);
    if (videoDecode) videoDecode->pause(pause);
    if (audioDecode) audioDecode->pause(pause);
    if (audioPlay) audioPlay->pause(pause);
    mux.unlock();
}
