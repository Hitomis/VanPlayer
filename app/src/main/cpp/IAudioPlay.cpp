//
// Created by 赵帆 on 2020-02-17.
//

#include "IAudioPlay.h"
#include "XLog.h"

// 生产者
void IAudioPlay::update(XData &data) {
    if (data.size <= 0 || !data.data) return;
    // 压入缓冲队列
    while (!isExit) {
        framesMutex.lock();

        if (frames.size() < maxSize) {
            frames.push_back(data);
            framesMutex.unlock();
            break;
        }

        framesMutex.unlock();
        XSleep(1);
    }
}

// 消费者
XData IAudioPlay::getFrame() {
    XData data;
    isRunning = true;
    while (!isExit) {
        if (isPause()) {
            XSleep(2);
            continue;
        }

        framesMutex.lock();
        if (!frames.empty()) {
            // 取出队列中头部的数据
            data = frames.front();
            frames.pop_front();
            framesMutex.unlock();
            pts = data.pts;
            return data;
        }

        framesMutex.unlock();
        XSleep(1);
    }
    isRunning = false;
    return data;
}

void IAudioPlay::clear() {
    framesMutex.lock();
    while(!frames.empty())
    {
        frames.front().drop();
        frames.pop_front();
    }
    framesMutex.unlock();

}
