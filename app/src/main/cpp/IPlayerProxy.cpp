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

void IPlayerProxy::close() {
    mux.lock();
    if (player) {
        player->close();
    }
    mux.unlock();
}

double IPlayerProxy::getPlayPos() {
    double pos = 0.0;
    mux.lock();
    if (player) pos = player->getPlayPos();
    mux.unlock();
    return pos;
}

bool IPlayerProxy::seek(double progress) {
    bool re = false;
    mux.lock();
    if (player) re = player->seek(progress);
    mux.unlock();
    return re;
}

void IPlayerProxy::pause(bool pause) {
    mux.lock();
    if (player) {
        player->pause(pause);
    }
    mux.unlock();
}

bool IPlayerProxy::isPause() {
    bool re = false;
    mux.lock();
    if (player) re = player->isPause();
    mux.unlock();
    return re;
}

