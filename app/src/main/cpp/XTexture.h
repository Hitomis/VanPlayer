//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_XTEXTURE_H
#define VANPLAYER_XTEXTURE_H


class XTexture {
public:
    static XTexture *create();

    virtual bool init(void *win) = 0;

    virtual void draw(unsigned char *data[], int width, int height) = 0;

};

#endif //VANPLAYER_XTEXTURE_H
