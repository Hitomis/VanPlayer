//
// Created by 赵帆 on 2020-02-16.
//

#ifndef VANPLAYER_IRESAMPLE_H
#define VANPLAYER_IRESAMPLE_H


#include "IObserver.h"
#include "XParameter.h"

class IResample : public IObserver {
public:
    virtual bool open(XParameter inPar, XParameter outPar = XParameter()) = 0;

    virtual void close() = 0;

    virtual XData resample(XData &data) = 0;

    void update(XData &data) override;

    int outChannels = 2;
    int outFormat = 1;
};


#endif //VANPLAYER_IRESAMPLE_H
