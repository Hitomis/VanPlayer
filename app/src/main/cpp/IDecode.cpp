//
// Created by 赵帆 on 2020-02-14.
//

#include "IDecode.h"
#include "XLog.h"

// 生产者
void IDecode::update(XData &data) {
    // 只处理音频缓冲
    if (data.isAudio != isAudio) return;

    while (!isExit) {
        packsMutex.lock();

        if (packs.size() < maxSize) {
            packs.push_back(data);
            packsMutex.unlock();
            break;
        }

        packsMutex.unlock();
        XSleep(1);
    }
}

// 消费者
void IDecode::run() {
    while (!isExit) {
        packsMutex.lock();
        if (packs.empty()) {
            packsMutex.unlock();
            XSleep(1);
            continue;
        }
        // 取出 packet
        XData packet = packs.front();
        packs.pop_front();

        // 一次数据包的发送，需要多次接受结果
        if (this->sendPacket(packet)) {
            while (!isExit) {
                // frame 数据每次调用空间会被重用，所以不需要清理
                XData frame = this->receiveFrame();
                if (!frame.data) break;
//                XLOGE("receiveFrame %d", frame.size);
                // 接受的帧数据继续下发
                this->notify(frame);
            }
        }
        // 清理 packet 内存空间
        packet.drop();
        packsMutex.unlock();
    }
}