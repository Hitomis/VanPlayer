//
// Created by 赵帆 on 2020-02-17.
//

#include "VanAudioPlay.h"
#include "XLog.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

static SLObjectItf engineObj = nullptr; // 引擎对象
static SLEngineItf engInterface = nullptr; // 引擎接口
static SLObjectItf mix = nullptr; // 混音器
static SLObjectItf playerObject = nullptr; // 播放器对象
static SLPlayItf playerInterface = nullptr; // 播放器接口
static SLAndroidSimpleBufferQueueItf pcmQueue = nullptr; // 播放队列

/**
 * 初始化引擎,并获取接口
 */
static void createSL() {
    SLresult re;

    re = slCreateEngine(&engineObj, 0, nullptr, 0, nullptr, nullptr);
    if (re != SL_RESULT_SUCCESS) return;
    re = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return;
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engInterface);
}


static void pcmCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context) {
    auto *vanAudioPlay = static_cast<VanAudioPlay *>(context);
    vanAudioPlay->playCall((void *) bufferQueueItf);
}

bool VanAudioPlay::startPlay(XParameter outPar) {
    close();
    mux.lock();
    SLresult re;
    // 1.创建引擎
    createSL();
    if (engInterface) {
        XLOGE("Create engine success");
    } else {
        XLOGE("Create engine failed");
        mux.unlock();
        return false;
    }

    // 2.配置混音器
    re = (*engInterface)->CreateOutputMix(engInterface, &mix, 0, nullptr, nullptr);
    if (re == SL_RESULT_SUCCESS) {
        XLOGE("Create output mix success");
    } else {
        XLOGE("Create output mix failed");
        mux.unlock();
        return false;
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        mux.unlock();
        XLOGE("(*mix)->Realize failed!");
        return false;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSnk = {&outputMix, nullptr};

    // 3.配置 PCM 格式信息
    SLDataLocator_AndroidSimpleBufferQueue queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    // 音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            static_cast<SLuint32>(outPar.channels), // 声道数
            static_cast<SLuint32>(outPar.sampleRate * 1000), // 采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN,
    };
    // 创建 PCM 格式信息
    SLDataSource slDataSource = {
            &queue,// SLDataFormat_PCM 配置输出
            &pcm   // 输出数据格式
    };

    // 4.初始化播放器
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    // 创建播放器对象
    re = (*engInterface)->CreateAudioPlayer(engInterface, &playerObject, &slDataSource, &audioSnk,
                                            sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (re == SL_RESULT_SUCCESS) {
        XLOGE("Create player success");
    } else {
        XLOGE("Create player failed");
        mux.unlock();
        return false;
    }
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    // 获取 player 的接口
    re = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerInterface);
    if (re != SL_RESULT_SUCCESS) {
        mux.unlock();
        XLOGE("GetInterface SL_IID_PLAY failed!");
        return false;
    }

    // 5.开始播放
    //注册回调缓冲区，获取缓冲队列接口
    re = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &pcmQueue);
    if (re == SL_RESULT_SUCCESS) {
        XLOGE("Get queue interface success");
    } else {
        XLOGE("Get queue interface failed");
        mux.unlock();
        return false;
    }
    // 设置回调函数，回调函数会在每一次缓冲队列读完之后调用
    (*pcmQueue)->RegisterCallback(pcmQueue, pcmCallback, this);
    (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_PLAYING);
    (*pcmQueue)->Enqueue(pcmQueue, "", 1);
    isExit = false;
    mux.unlock();
    XLOGE("VanAudioPlay start success");
    return true;
}

void VanAudioPlay::playCall(void *bufQueue) {
    if (!bufQueue) return;
    auto bufferQueueItf = static_cast<SLAndroidSimpleBufferQueueItf>(bufQueue);
    XData data = getFrame();
    if (data.size <= 0) {
        XLOGE("Audio frame data size is 0");
        return;
    }

    // 构造方法中对 buffer 进行了初始化
    if (!buffer) return;
    memcpy(buffer, data.data, data.size);
    mux.lock();
    if (pcmQueue && (*pcmQueue)) {
        (*pcmQueue)->Enqueue(pcmQueue, buffer, data.size);
    }
    mux.unlock();
    data.drop();
}

VanAudioPlay::VanAudioPlay() {
    buffer = new unsigned char[1024 * 1024];
}

VanAudioPlay::~VanAudioPlay() {
    delete buffer;
    buffer = nullptr;
}

void VanAudioPlay::close() {
    IAudioPlay::clear();
    mux.lock();
    // 停止播放
    if (playerInterface && (*playerInterface)) {
        (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_STOPPED);
    }
    // 清理播放队列
    if (pcmQueue && (*pcmQueue)) {
        (*pcmQueue)->Clear(pcmQueue);
    }
    // 销毁 player 对象
    if (playerObject && (*playerObject)) {
        (*playerObject)->Destroy(playerObject);
    }
    // 销毁混音器
    if (mix && (*mix)) {
        (*mix)->Destroy(mix);
    }
    // 销毁播放引擎
    if (engineObj && (*engineObj)) {
        (*engineObj)->Destroy(engineObj);
    }
    engineObj = nullptr;
    engInterface = nullptr;
    mix = nullptr;
    playerObject = nullptr;
    playerInterface = nullptr;
    pcmQueue = nullptr;
    mux.unlock();
}
