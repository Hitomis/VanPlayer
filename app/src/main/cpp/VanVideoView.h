//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_VANVIDEOVIEW_H
#define VANPLAYER_VANVIDEOVIEW_H


#include "IVideoView.h"
#include "XTexture.h"

class VanVideoView : public IVideoView {
public:
    void setRender(void *win) override;

    void render(XData &data) override;

protected:
    void *view = 0;
    XTexture *texture = 0;
};


#endif //VANPLAYER_VANVIDEOVIEW_H
