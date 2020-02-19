//
// Created by 赵帆 on 2020-02-16.
//

#ifndef VANPLAYER_VANRESAMPLE_H
#define VANPLAYER_VANRESAMPLE_H

#include "IResample.h"
#include <mutex>

struct SwrContext;

class VanResample : public IResample {
public:
    bool open(XParameter inPar, XParameter outPar = XParameter()) override;

    void close() override;

    XData resample(XData &data) override;

protected:
    SwrContext *actx;

    std::mutex mux;
};


#endif //VANPLAYER_VANRESAMPLE_H
