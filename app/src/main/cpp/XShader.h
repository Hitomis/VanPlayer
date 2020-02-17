//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_XSHADER_H
#define VANPLAYER_XSHADER_H


class XShader {
public:
    virtual bool init();

    // 获取材质并映射到内存
    virtual void getTexture(unsigned int index, int width, int height, unsigned char *buffer);

    virtual void draw();

protected:
    unsigned int vsh = 0;
    unsigned int fsh = 0;
    unsigned int program = 0;
    unsigned int textures[3] = {0};
};


#endif //VANPLAYER_XSHADER_H
