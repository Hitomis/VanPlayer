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
    XTextureType type;

    bool init(void *win, XTextureType type) override {
        this->type = type;
        if (!win) {
            XLOGE("XTexture init failed win is null");
            return false;
        }
        if (!XEGL::getInstance()->init(win)) return false;
        shader.init(static_cast<XShaderType>(type));
        return true;
    }

    void draw(unsigned char *data[], int width, int height) override {
        shader.getTexture(0, width, height, data[0]); // Y
        if (type == XTEXTURE_YUV420P) {
            shader.getTexture(1, width / 2, height / 2, data[1]); // U
            shader.getTexture(2, width / 2, height / 2, data[2]); // V
        } else {
            shader.getTexture(1, width / 2, height / 2, data[1], true); // UV
        }
        shader.draw();
        XEGL::getInstance()->draw();
    }
};


XTexture *XTexture::create() {
    return new CXTexture();
}