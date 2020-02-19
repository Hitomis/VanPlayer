//
// Created by 赵帆 on 2020-02-14.
//

#include <thread>
#include "XLog.h"
#include "XThread.h"

using namespace std;

void XSleep(int timeMillis) {
    chrono::microseconds du(timeMillis);
    this_thread::sleep_for(du);
}

bool XThread::start() {
    isExit = false;
    thread th(&XThread::threadMain, this);
    th.detach();
    return true;
}

void XThread::stop() {
    isExit = true;
    for (int i = 0; i < 200; i++) {
        if (!isRunning) {
            XLOGE("线程停止成功");
            return;
        }
        XSleep(1);
    }
    XLOGE("线程停止超时");
}

void XThread::threadMain() {
    isRunning = true;
    XLOGE("开始执行线程");
    this->run();
    XLOGE("线程执行完毕");
    isRunning = false;
}

bool XThread::isPause() {
    isPausing = isPaused;
    return isPausing;
}

void XThread::pause(bool flag) {
    isPaused = flag;
    //等待100毫秒
    for (int i = 0; i < 10; i++) {
        if (isPausing == flag) {
            break;
        }
        XSleep(10);
    }
}
