//
// Created by 赵帆 on 2020-02-18.
//

#include "VanPlayerBuilder.h"
#include "VanDemux.h"
#include "VanDecode.h"
#include "VanResample.h"
#include "VanVideoView.h"
#include "VanAudioPlay.h"

VanPlayerBuilder &VanPlayerBuilder::getInstance() {
    static VanPlayerBuilder vanPlayerBuilder;
    return vanPlayerBuilder;
}

void VanPlayerBuilder::initHardDecode(void *vm) {
    VanDecode::registerHard(vm);
}

IDemux *VanPlayerBuilder::createDemux() {
    auto *demux = new VanDemux();
    return demux;
}

IDecode *VanPlayerBuilder::createDecode() {
    auto *decode = new VanDecode();
    return decode;
}

IResample *VanPlayerBuilder::createResample() {
    auto *resample = new VanResample();
    return resample;
}

IVideoView *VanPlayerBuilder::createVideoView() {
    auto *videoView = new VanVideoView();
    return videoView;
}

IAudioPlay *VanPlayerBuilder::createAudioPlay() {
    auto *audioPlay = new VanAudioPlay();
    return audioPlay;
}

IPlayer *VanPlayerBuilder::createPlayer(unsigned int index) {
    return IPlayer::getInstance(index);
}

