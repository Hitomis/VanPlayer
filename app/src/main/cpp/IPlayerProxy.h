//
// Created by 赵帆 on 2020-02-19.
//

#ifndef VANPLAYER_IPLAYERPROXY_H
#define VANPLAYER_IPLAYERPROXY_H


#include <mutex>
#include "IPlayer.h"

class IPlayerProxy {
public:
    static IPlayerProxy &getInstance();

    void init(void *vm = 0);

    virtual bool open(const char *path, bool isHard = false);

    virtual void close();

    virtual bool start();

    virtual void initWindow(void *win);

protected:
    IPlayerProxy() {};
    IPlayer *player = 0;
    std::mutex mux;
};


#endif //VANPLAYER_IPLAYERPROXY_H
