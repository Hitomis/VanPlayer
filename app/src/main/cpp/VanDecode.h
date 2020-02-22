//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_VANDECODE_H
#define VANPLAYER_VANDECODE_H

#include "XParameter.h"
#include "IDecode.h"
#include <mutex>

struct AVCodecContext;

class VanDecode : public IDecode {
public:
    static void registerHard(void *vm);

    bool open(XParameter param, bool isHard = false) override;

    void close() override;

    bool sendPacket(XData &packet) override;

    // 从线程中获取解码结果，再次调用会复用上次空间，线程不安全
    XData receiveFrame() override;

    void clear() override;

protected:
    AVCodecContext *codecCxt = 0;
    AVFrame *frame = 0;
    std::mutex mux;
};


#endif //VANPLAYER_VANDECODE_H
