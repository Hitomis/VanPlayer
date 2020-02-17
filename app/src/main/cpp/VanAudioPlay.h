//
// Created by 赵帆 on 2020-02-17.
//

#ifndef VANPLAYER_VANAUDIOPLAY_H
#define VANPLAYER_VANAUDIOPLAY_H


#include "IAudioPlay.h"

class VanAudioPlay : public IAudioPlay{
public:
    virtual bool startPlay(XParameter outPar);
    void playCall(void *bufQueue);

    VanAudioPlay();
    virtual ~VanAudioPlay();

protected:
    unsigned char *buffer = 0;
    std::mutex mux;
};


#endif //VANPLAYER_VANAUDIOPLAY_H
