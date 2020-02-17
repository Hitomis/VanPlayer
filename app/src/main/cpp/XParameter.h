//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_XPARAMETER_H
#define VANPLAYER_XPARAMETER_H


struct AVCodecParameters;

class XParameter {
public:
    AVCodecParameters *codecParams = 0; //音视频参数
};


#endif //VANPLAYER_XPARAMETER_H
