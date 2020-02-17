//
// Created by 赵帆 on 2020-02-16.
//

#ifndef VANPLAYER_VANRESAMPLE_H
#define VANPLAYER_VANRESAMPLE_H

#include "IResample.h"

struct SwrContext;

class VanResample : public IResample {
public:
    bool open(XParameter inPar, XParameter outPar = XParameter()) override;

    XData &resample(XData &data) override;

    SwrContext *actx;
};


#endif //VANPLAYER_VANRESAMPLE_H
