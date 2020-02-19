//
// Created by 赵帆 on 2020-02-17.
//

#ifndef VANPLAYER_IAUDIOPLAY_H
#define VANPLAYER_IAUDIOPLAY_H


#include <list>
#include "IObserver.h"
#include "XParameter.h"

class IAudioPlay : public IObserver {
public:
    // 缓冲满后阻塞
    virtual void update(XData &data);

    virtual XData getFrame();

    virtual bool startPlay(XParameter outPar) = 0;

    // 最大的队列数量
    int maxSize = 100;

protected:
    std::list<XData> frames;
    std::mutex framesMutex;
};


#endif //VANPLAYER_IAUDIOPLAY_H
