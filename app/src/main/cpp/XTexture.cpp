//
// Created by 赵帆 on 2020-02-15.
//

#include "XTexture.h"
#import "XLog.h"
#include "XEGL.h"
#include "XShader.h"

class CXTexture : public XTexture {
public:
    XShader shader;

    bool init(void *win) override {
        if (!win) {
            XLOGE("XTexture init failed win is null");
            return false;
        }
        if (!XEGL::getInstance()->init(win)) return false;
        shader.init();
        return true;
    }

    void draw(unsigned char *data[], int width, int height) override {
        shader.getTexture(0, width, height, data[0]); // Y
        shader.getTexture(1, width / 2, height / 2, data[1]); // U
        shader.getTexture(2, width / 2, height / 2, data[2]); // V
        shader.draw();
        XEGL::getInstance()->draw();
    }
};


XTexture *XTexture::create() {
    return new CXTexture();
}