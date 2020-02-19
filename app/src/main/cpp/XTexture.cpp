//
// Created by 赵帆 on 2020-02-15.
//

#include "XTexture.h"
#import "XLog.h"
#include "XEGL.h"
#include "XShader.h"
#include <mutex>

class CXTexture : public XTexture {
public:
    XShader shader;
    XTextureType type;
    std::mutex mux;

    bool init(void *win, XTextureType type) override {
        mux.lock();
        XEGL::getInstance()->close();
        shader.close();
        this->type = type;
        if (!win) {
            XLOGE("XTexture init failed win is null");
            mux.unlock();
            return false;
        }
        if (!XEGL::getInstance()->init(win)) {
            mux.unlock();
            return false;
        }
        shader.init(static_cast<XShaderType>(type));
        mux.unlock();
        return true;
    }

    void draw(unsigned char *data[], int width, int height) override {
        mux.lock();
        shader.getTexture(0, width, height, data[0]); // Y
        if (type == XTEXTURE_YUV420P) {
            shader.getTexture(1, width / 2, height / 2, data[1]); // U
            shader.getTexture(2, width / 2, height / 2, data[2]); // V
        } else {
            shader.getTexture(1, width / 2, height / 2, data[1], true); // UV
        }
        shader.draw();
        XEGL::getInstance()->draw();
        mux.unlock();
    }

    void drop() override {
        mux.lock();
        XEGL::getInstance()->close();
        shader.close();
        mux.unlock();
        delete this;
    }
};


XTexture *XTexture::create() {
    return new CXTexture();
}
