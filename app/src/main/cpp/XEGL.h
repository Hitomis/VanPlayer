//
// Created by 赵帆 on 2020-02-15.
//

#ifndef VANPLAYER_XEGL_H
#define VANPLAYER_XEGL_H


class XEGL {
public:
    static XEGL *getInstance();
    virtual bool init(void *nativeWin) = 0;
    virtual void close() = 0;
    virtual void draw() = 0;

protected:
    XEGL(){}
};


#endif //VANPLAYER_XEGL_H
