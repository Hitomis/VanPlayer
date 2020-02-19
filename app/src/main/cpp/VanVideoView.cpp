//
// Created by 赵帆 on 2020-02-15.
//

#include "VanVideoView.h"

void VanVideoView::setRender(void *win) {
    this->view = win;
}

void VanVideoView::render(XData &data) {
    if (!view) return;
    if (!texture) {
        texture = XTexture::create();
        texture->init(view, static_cast<XTextureType>(data.format));
    }
    texture->draw(data.datas, data.width, data.height);
}

void VanVideoView::close() {
    mux.lock();
    if (texture) {

        texture->drop();
        texture = nullptr;
    }
    mux.unlock();

}
