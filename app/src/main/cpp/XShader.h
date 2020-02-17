//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_XSHADER_H
#define VANPLAYER_XSHADER_H

enum XShaderType {
    XSHADER_YUV420P = 0, // y4 u1 v1
    XSHADER_NV12 = 25, // y4 uv1
    XSHADER_NV21 = 26 // y4 vu1
};


class XShader {
public:
    virtual bool init(XShaderType type = XSHADER_YUV420P);

    // 获取材质并映射到内存
    virtual void getTexture(unsigned int index, int width, int height, unsigned char *buffer,
                            bool isAlpha = false);

    virtual void draw();

protected:
    unsigned int vsh = 0;
    unsigned int fsh = 0;
    unsigned int program = 0;
    unsigned int textures[3] = {0};
};


#endif //VANPLAYER_XSHADER_H
