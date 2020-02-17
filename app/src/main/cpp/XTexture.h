//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_XTEXTURE_H
#define VANPLAYER_XTEXTURE_H

enum XTextureType {
    XTEXTURE_YUV420P = 0, // y4 u1 v1
    XTEXTURE_NV12 = 25, // y4 uv1
    XTEXTURE_NV21 = 26 // y4 vu1
};

class XTexture {
public:
    static XTexture *create();

    virtual bool init(void *win, XTextureType type = XTEXTURE_YUV420P) = 0;

    virtual void draw(unsigned char *data[], int width, int height) = 0;

};

#endif //VANPLAYER_XTEXTURE_H
