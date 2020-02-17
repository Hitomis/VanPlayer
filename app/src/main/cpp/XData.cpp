//
// Created by 赵帆 on 2020-02-14.
//

#include "XData.h"

extern "C" {
#include <libavformat/avformat.h>
}

bool XData::alloc(int size, const char *data) {
    drop();
    type = UN_CHAR_TYPE;
    if (size <= 0) return false;
    this->data = new unsigned char[size];
    if (!this->data) return false;
    if (data)memcpy(this->data, data, size);
    return true;
}

void XData::drop() {
    if (!data) return;
    if (type == AV_PACKET_TYPE) {
        av_packet_free(reinterpret_cast<AVPacket **>(&data));
    } else {
        delete data;
    }
    data = nullptr;
    size = 0;
}
