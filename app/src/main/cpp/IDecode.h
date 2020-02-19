//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_IDECODE_H
#define VANPLAYER_IDECODE_H

#include "XParameter.h"
#include "IObserver.h"
#include <list>

struct AVCodecContext;
struct AVFrame;

class IDecode : public IObserver {
public:
    // 打开解码器
    virtual bool open(XParameter param, bool isHard = false) = 0;

    virtual void close() = 0;

    virtual void clear();

    // 发送数据到线程解码
    virtual bool sendPacket(XData &packet) = 0;

    // 从线程中获取解码结果，再次调用会复用上次空间，线程不安全
    virtual XData receiveFrame() = 0;

    // 缓冲满后阻塞
    void update(XData &data) override;

    // 是否是音频解码器
    bool isAudio = false;

    // 最大的队列数量
    int maxSize = 100;

    // 同步时间（其实是音频的pts），再次打开文件要清理
    int synPts = 0;

    int pts = 0;

protected:
    void run() override;

    std::list<XData> packs; // 存放所有的缓冲帧
    std::mutex packsMutex;

};


#endif //VANPLAYER_IDECODE_H
