//
// Created by 赵帆 on 2020-02-14.
//

#include "IDecode.h"
#include "XLog.h"

// 生产者
void IDecode::update(XData &data) {
    // 如果解码器类型和数据类型不一致，则不处理[音频解码器只处理音频数据，视频解码器只处理视频数据]
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

        //判断音视频同步, 让视频往音频的节奏靠
        if (!isAudio && synPts > 0) {
            if (synPts < pts) { // 音频的时间小于视频的时间，就让视频停下来等
                packsMutex.unlock();
                XSleep(1);
                continue;
            }
        }

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
                pts = frame.pts;
                // 接受的帧数据继续下发
                this->notify(frame);
            }
        }
        // 清理 packet 内存空间
        packet.drop();
        packsMutex.unlock();
    }
}
