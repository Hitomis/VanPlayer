//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_XDATA_H
#define VANPLAYER_XDATA_H

enum XDataType {
    AV_PACKET_TYPE = 0,
    UN_CHAR_TYPE = 1
};

struct XData {
    unsigned char *data = nullptr; // 音视频帧数据
    unsigned char *datas[8] = {0}; // 存放解码后的数据， 8 == AV_NUM_DATA_POINTERS
    int type = 0;
    int size = 0; // 音视频的一帧数据大小
    int width = 0; // 视频帧宽度
    int height = 0; // 视频帧高度
    bool isAudio = false; // 是否是音频流
    
    bool alloc(int size, const char *data = 0);

    void drop();
};


#endif //VANPLAYER_XDATA_H
