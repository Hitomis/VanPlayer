//
// Created by 赵帆 on 2020-02-19.
//

#include "IPlayerProxy.h"
#include "VanPlayerBuilder.h"

IPlayerProxy &IPlayerProxy::getInstance() {
    static IPlayerProxy playerProxy;
    return playerProxy;
}

void IPlayerProxy::init(void *vm) {
    mux.lock();
    if (vm) {
        VanPlayerBuilder::initHardDecode(vm);
    }
    if (!player) {
        player = VanPlayerBuilder::getInstance().builderPlayer();
    }
    mux.unlock();
}

bool IPlayerProxy::open(const char *path, bool isHard) {
    bool re = false;
    mux.lock();
    if (player) {
        player->isHardDecode = isHard;
        re = player->open(path);
    }
    mux.unlock();
    return re;
}

bool IPlayerProxy::start() {
    bool re = false;
    mux.lock();
    if (player) re = player->start();
    mux.unlock();
    return re;
}

void IPlayerProxy::initWindow(void *win) {
    mux.lock();
    if (player) player->initWindow(win);
    mux.unlock();

}
