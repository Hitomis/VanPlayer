//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_IVIDEOVIEW_H
#define VANPLAYER_IVIDEOVIEW_H


#include "XData.h"
#include "IObserver.h"

class IVideoView : public IObserver {
public:
    virtual void setRender(void *win) = 0;

    virtual void render(XData &data) = 0;

    void update(XData &data) override;
};


#endif //VANPLAYER_IVIDEOVIEW_H
