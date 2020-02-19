//
// Created by 赵帆 on 2020-02-18.
//

#ifndef VANPLAYER_IPLAYERBUILDER_H
#define VANPLAYER_IPLAYERBUILDER_H


#include "IPlayer.h"

class IPlayerBuilder {
public:
    virtual IPlayer *builderPlayer(unsigned int index = 0);

protected:
    virtual IDemux *createDemux() = 0;

    virtual IDecode *createDecode() = 0;

    virtual IResample *createResample() = 0;

    virtual IVideoView *createVideoView() = 0;

    virtual IAudioPlay *createAudioPlay() = 0;

    virtual IPlayer *createPlayer(unsigned int index = 0) = 0;
};


#endif //VANPLAYER_IPLAYERBUILDER_H
