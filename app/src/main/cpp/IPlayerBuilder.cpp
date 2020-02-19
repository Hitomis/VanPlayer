//
// Created by 赵帆 on 2020-02-18.
//

#include "IPlayerBuilder.h"
#include "IDemux.h"
#include "IDecode.h"
#include "IResample.h"
#include "IVideoView.h"
#include "IAudioPlay.h"

IPlayer *IPlayerBuilder::builderPlayer(unsigned int index) {
    // 创建 player 接口
    IPlayer *player = createPlayer(index);

    // 解封装器
    auto *demux = createDemux();
    // 音频解码器
    auto *audioDecode = createDecode();
    // 视频解码器
    auto *videoDecode = createDecode();

    // 解封装后的数据需要传递给音频和视频解码器
    demux->addObserver(audioDecode);
    demux->addObserver(videoDecode);

    // 音频重采样器
    auto *resample = createResample();
    // 音频播放器
    auto *audioPlay = createAudioPlay();

    // 音频解码后的数据需要传递给音频重采样器
    audioDecode->addObserver(resample);
    // 音频流重采样后的数据传递给音频播放器播放
    resample->addObserver(audioPlay);

    // 视频播放器
    auto *videoView = createVideoView();
    // 视频解码后的数据传递给视频窗口显示
    videoDecode->addObserver(videoView);

    player->demux = demux;
    player->audioDecode = audioDecode;
    player->videoDecode = videoDecode;
    player->resample = resample;
    player->audioPlay = audioPlay;
    player->videoView = videoView;
    return player;
}
