//
// Created by 赵帆 on 2020-02-18.
//

#ifndef VANPLAYER_VANPLAYERBUILDER_H
#define VANPLAYER_VANPLAYERBUILDER_H


#include "IPlayerBuilder.h"

class VanPlayerBuilder : public IPlayerBuilder {
public:
    static VanPlayerBuilder &getInstance();
    static void initHardDecode(void *vm);

protected:
    VanPlayerBuilder() {}

    IDemux *createDemux() override;

    IDecode *createDecode() override;

    IResample *createResample() override;

    IVideoView *createVideoView() override;

    IAudioPlay *createAudioPlay() override;

    IPlayer *createPlayer(unsigned int index) override;
};


#endif //VANPLAYER_VANPLAYERBUILDER_H
