#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "XLog.h"
#include "IDemux.h"
#include "VanDemux.h"
#include "IDecode.h"
#include "VanDecode.h"
#include "XEGL.h"
#include "XShader.h"
#include "VanVideoView.h"
#include "VanResample.h"
#include "VanAudioPlay.h"
#include "IPlayerProxy.h"

extern "C" {
#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

using namespace std;

// 顶点着色器glsl
#define GET_STR(x) #x
static const char *vertexShader = GET_STR(
        attribute
        vec4 aPosition; // 顶点坐标
        attribute
        vec2 aTexCoord; // 材质顶点坐标
        varying
        vec2 vTexCoord; // 输出的材质坐标
        void main() {
            vTexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
            gl_Position = aPosition;
        }
);

// 片元着色器，软解码和部分x86硬解码
static const char *fragYUV420p = GET_STR(
        precision
        mediump float; // 进度
        varying
        vec2 vTexCoord; // 顶点着色器传递过来的坐标
        uniform
        sampler2D yTexture; // 输入的材质，灰度图
        uniform
        sampler2D uTexture; // 输入的材质
        uniform
        sampler2D vTexture; // 输入的材质
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;
            rgb = mat3(
                    1.0, 1.0, 1.0,
                    0.0, -0.39465, 2.03211,
                    1.13983, -0.5806, 0.0) * yuv;
            // 输出像素颜色
            gl_FragColor = vec4(rgb, 1.0);
        }
);

static double r2d(AVRational r) {
    return r.den == 0 || r.num == 0 ? 0 : (double) r.num / r.den;
}

static long long getCurrentTimeMillis() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    int sec = tv.tv_sec % 360000; // 取100个小时内的时间戳
    long long t = sec * 1000 + tv.tv_usec / 1000;
    return t;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_NativePlayer_play(JNIEnv *env, jobject thiz, jstring url,
                                           jobject surface) {
    // 初始化
    av_register_all();
    avformat_network_init();
    avcodec_register_all();
    // 打开文件
    AVFormatContext *fmt_context = nullptr;
    const char *urlStr = env->GetStringUTFChars(url, 0);
    int rect = avformat_open_input(&fmt_context, urlStr, nullptr, nullptr);
    if (rect != 0) {
        XLOGE(nullptr, AV_LOG_ERROR, "open failure %s", av_err2str(rect));
        return;
    }

    rect = avformat_find_stream_info(fmt_context, nullptr);
    if (rect != 0) {
        XLOGE(nullptr, AV_LOG_ERROR, "open failure %s", av_err2str(rect));
        return;
    }

    XLOGE("open success \n\n");
    XLOGE("duration = %lld \nnb_streams = %d\n\n", fmt_context->duration, fmt_context->nb_streams);

    int videoStreamIndex = 0, audioStreamIndex = 0;
    int fps = 0;
    for (int i = 0; i < fmt_context->nb_streams; i++) {
        AVStream *as = fmt_context->streams[i];
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) { // 视频流
            videoStreamIndex = i;
            fps = r2d(as->avg_frame_rate);
            XLOGE("视频数据 videoStreamIndex = %d\n", videoStreamIndex);
            XLOGE("fps = %d \nwidth = %d \nheight = %d \ncodec_id = %d \npixel_format=%d\n\n",
                  fps,
                  as->codecpar->width,
                  as->codecpar->height,
                  as->codecpar->codec_id,
                  as->codecpar->format);

        } else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {// 音频流
            audioStreamIndex = i;
            XLOGE("音频数据 audioStreamIndex = %d\n", audioStreamIndex);
            XLOGE("sample_rate = %d \nchannels = %d \nsample_format = %d \n\n",
                  as->codecpar->sample_rate,
                  as->codecpar->channels,
                  as->codecpar->format);
        }
    }

    videoStreamIndex = av_find_best_stream(fmt_context, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStreamIndex = av_find_best_stream(fmt_context, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    XLOGE("通过 av_find_best_stream 函数查找到的视频流和音频流索引：\n");
    XLOGE("videoStreamIndex = %d\n", videoStreamIndex);
    XLOGE("audioStreamIndex = %d\n", audioStreamIndex);

    // 视频软解码器初始化
    AVCodec *vc = avcodec_find_decoder(fmt_context->streams[videoStreamIndex]->codecpar->codec_id);
    // avcodec_find_decoder_by_name("h264_mediacodec"); // 找硬解码器
    if (!vc) {
        XLOGE("没有找到对应的视频解码器 \n");
        return;
    }
    AVCodecContext *video_codec = avcodec_alloc_context3(vc);
    avcodec_parameters_to_context(video_codec, fmt_context->streams[videoStreamIndex]->codecpar);
    video_codec->thread_count = 1;

    // 打开视频解码器
    rect = avcodec_open2(video_codec, nullptr, nullptr);
    if (rect != 0) {
        XLOGE("打开视频解码器失败 \n");
        return;
    }

    // 音频软解码器初始化
    AVCodec *ac = avcodec_find_decoder(fmt_context->streams[audioStreamIndex]->codecpar->codec_id);
    if (!ac) {
        XLOGE("没有找到对应的音频解码器 \n");
        return;
    }
    AVCodecContext *audio_codec = avcodec_alloc_context3(ac);
    avcodec_parameters_to_context(audio_codec, fmt_context->streams[audioStreamIndex]->codecpar);
    audio_codec->thread_count = 1;

    // 打开音频解码器
    rect = avcodec_open2(audio_codec, nullptr, nullptr);
    if (rect != 0) {
        XLOGE("打开音频解码器失败 \n");
        return;
    }

    // 读取帧数据
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    // 初始化像素格式转换的上下文
    SwsContext *vctx = nullptr;
    int outWidth = 1280;
    int outHeight = 720;
    char *rgba = new char[1920 * 1080 * 4];

    // 初始化音频重采样上下文
    SwrContext *actx = swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              audio_codec->channel_layout, AV_SAMPLE_FMT_S16,
                              audio_codec->sample_rate,
                              audio_codec->channel_layout, audio_codec->sample_fmt,
                              audio_codec->sample_rate,
                              0, nullptr);
    char *pcm = new char[48000 * 4 * 2];
    rect = swr_init(actx);
    if (rect != 0) {
        XLOGE("初始化音频重采样上下文失败 \n");
        return;
    }

    // 显示窗口初始化
    ANativeWindow *nativeWin = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(nativeWin, outWidth, outHeight, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer winBuffer;

    bool is_end_file = false;
    int frameCount = 0;
    long long start = getCurrentTimeMillis();
    for (;;) {
        if (getCurrentTimeMillis() - start >= 3000) { // 每 3 秒统计一次
            // 打印没秒解了多少帧
            XLOGE("Now decode fps is %d \n", frameCount / 3);
            start = getCurrentTimeMillis();
            frameCount = 0;
        }
        rect = av_read_frame(fmt_context, pkt);

        if (rect != 0) {
            is_end_file = true;
            XLOGE("读取到结尾处 \n");
        }

        // 解码音视频
        AVCodecContext *codec = video_codec;
        if (pkt->stream_index == audioStreamIndex) {
            codec = audio_codec;
        }

        // 内部会复制一份 pkt
        if (is_end_file) {
            rect = avcodec_send_packet(codec, nullptr);
        } else {
            rect = avcodec_send_packet(codec, pkt);
        }
        // 可以直接清理掉创建的pkt
        av_packet_unref(pkt);
        if (rect != 0) {
            XLOGE("send packet fail \n");
            continue;
        }
        // send 一次，需要 receive 多次
        for (;;) {
            rect = avcodec_receive_frame(codec, frame);
            if (rect != 0) {
                break;
            }
            if (codec == video_codec) {// 只统计视频，和视频像素格式转换
                frameCount++;
                // 视频帧像素格式转换
                vctx = sws_getCachedContext(vctx,
                                            frame->width,
                                            frame->height,
                                            (AVPixelFormat) frame->format,
                                            outWidth,
                                            outHeight,
                                            AV_PIX_FMT_RGBA,
                                            SWS_FAST_BILINEAR,
                                            nullptr, nullptr, nullptr);
                if (!vctx) {
                    XLOGE("sws_getCachedContext init failed! \n");
                } else { // 初始化像素格式转换上下文成功
                    uint8_t *dst_data[AV_NUM_DATA_POINTERS] = {nullptr};
                    dst_data[0] = reinterpret_cast<uint8_t *>(rgba);
                    int lines[AV_NUM_DATA_POINTERS] = {0};
                    lines[0] = outWidth * 4; // 因为输出像素格式是 AV_PIX_FMT_RGBA 表示每个像素点占用4个字节
                    int h = sws_scale(vctx, frame->data, frame->linesize, 0, frame->height,
                                      dst_data, lines);
                    if (h > 0) {
                        ANativeWindow_lock(nativeWin, &winBuffer, 0);
                        uint8_t *dst = static_cast<uint8_t *>(winBuffer.bits);
                        memcpy(dst, rgba, outWidth * outHeight * 4);
                        ANativeWindow_unlockAndPost(nativeWin);
                    }

                    XLOGE("sws_scale = %d \n", h);
                }
            } else { // 音频重采样
                uint8_t *out[2] = {nullptr};
                out[0] = reinterpret_cast<uint8_t *>(pcm);
                int len = swr_convert(actx, out, frame->nb_samples, (const uint8_t **) frame->data,
                                      frame->nb_samples);
                XLOGE("swr_convert= %d \n", len);
            }

        }
        if (is_end_file) break;
    }
    delete[] rgba;
    delete[] pcm;
    avformat_close_input(&fmt_context);
    env->ReleaseStringUTFChars(url, 0);
}

/**
 * 初始化引擎
 * @param engine_engine
 */
void createSL(SLObjectItf &engineObj, SLEngineItf &engineEngine) {
    SLresult re;

    re = slCreateEngine(&engineObj, 0, nullptr, 0, nullptr, nullptr);
    if (re != SL_RESULT_SUCCESS) return;
    re = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return;
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineEngine);
}

void pcmCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context) {
    XLOGE("pcmCallback");
    static FILE *fp;
    static char *buffer;
    if (!buffer) {
        buffer = new char[1024 * 1024];
    }
    if (!fp) {
        fp = fopen("/sdcard/jj.pcm", "rb");
    }
    if (!fp) return;
    if (feof(fp) == 0) { // 文件没有读到结尾
        int len = fread(buffer, 1, 1024, fp);
        if (len > 0) {
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, len);
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_NativePlayer_audio(JNIEnv *env, jobject thiz, jstring url) {
    SLresult re;
    SLObjectItf engineObj;
    SLEngineItf engineEngine;

    // 1.创建引擎
    createSL(engineObj, engineEngine);
    if (engineEngine) {
        XLOGE("Create engine success");
    } else {
        XLOGE("Create engine failed");
        return;
    }

    // 2.配置混音器
    SLObjectItf mix;
    re = (*engineEngine)->CreateOutputMix(engineEngine, &mix, 0, nullptr, nullptr);
    if (re == SL_RESULT_SUCCESS) {
        XLOGE("Create output mix success");
    } else {
        XLOGE("Create output mix failed");
        return;
    }
    (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSnk = {&outputMix, nullptr};

    // 3.配置 PCM 格式信息
    SLDataLocator_AndroidSimpleBufferQueue queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    // 音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2, // 声道数
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN,
    };
    // 创建 PCM 格式信息
    SLDataSource slDataSource = {
            &queue,//SLDataFormat_PCM配置输出
            &pcm   //输出数据格式
    };

    // 4.初始化播放器
    SLObjectItf playerObject; // 播放器对象
    SLPlayItf playerInterface; // 播放器接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    // 创建播放器对象
    re = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &slDataSource, &audioSnk,
                                            sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (re == SL_RESULT_SUCCESS) {
        XLOGE("Create player success");
    } else {
        XLOGE("Create player failed");
        return;
    }
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    // 获取 player 的接口
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerInterface);


    // 5.开始播放
    //注册回调缓冲区，获取缓冲队列接口
    SLAndroidSimpleBufferQueueItf pcmQueue;
    re = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &pcmQueue);
    if (re == SL_RESULT_SUCCESS) {
        XLOGE("Get queue interface success");
    } else {
        XLOGE("Get queue interface failed");
        return;
    }
    // 设置回调函数，回调函数会在每一次缓冲队列读完之后调用
    (*pcmQueue)->RegisterCallback(pcmQueue, pcmCallback, nullptr);
    (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_PLAYING);
    (*pcmQueue)->Enqueue(pcmQueue, "", 1);
}

GLuint initShader(const char *code, GLint type) {
    GLuint sh = glCreateShader(type);
    if (sh == 0) {
        XLOGE("glCreateShader %d failed!", type);
        return 0;
    }
    // 加载 shader
    glShaderSource(sh,
                   1,      // shader 数量
                   &code,  // shader 代码
                   0);     // 代码长度
    // 编译 shader
    glCompileShader(sh);
    // 获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        XLOGE("glCompileShader failed!");
        return 0;
    } else {
        XLOGE("glCompileShader %d success!", type);
    }
    return sh;
}

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_NativePlayer_video(JNIEnv *env, jobject thiz, jstring url,
                                            jobject surface) {
    const char *urlStr = env->GetStringUTFChars(url, 0);
    FILE *fp = fopen(urlStr, "rb");
    if (!fp) {
        XLOGE("Open file %s failed!", urlStr);
        return;
    }

    // 获取原始窗口
    ANativeWindow *nativeWin = ANativeWindow_fromSurface(env, surface);
    // 1. display 创建和初始化
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        XLOGE("eglGetDisplay failed");
        return;
    }
    if (EGL_TRUE != eglInitialize(eglDisplay, nullptr, nullptr)) {
        XLOGE("eglInitialize failed");
        return;
    }
    // 2. surface
    EGLConfig config; // 输出的配置项
    EGLint configNum; // 输出的配置项数量
    EGLint configSpec[] = { // 输入的配置项
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };
    if (EGL_TRUE != eglChooseConfig(eglDisplay, configSpec, &config, 1, &configNum)) {
        XLOGE("eglChooseConfig failed");
        return;
    }
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config, nativeWin, nullptr);
    if (EGL_NO_SURFACE == eglSurface) {
        XLOGE("eglCreateWindowSurface failed");
        return;
    }
    // 3.context
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    EGLContext eglContext = eglCreateContext(eglDisplay, config, EGL_NO_SURFACE, ctxAttr);
    if (EGL_NO_CONTEXT == eglContext) {
        XLOGE("eglCreateContext failed");
        return;
    }
    if (EGL_TRUE == eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        XLOGE("eglMakeCurrent success");
    }

    // 顶点和片元 shader 初始化
    GLuint vsh = initShader(vertexShader, GL_VERTEX_SHADER);
    GLuint fsh = initShader(fragYUV420p, GL_FRAGMENT_SHADER);

    // 创建渲染程序
    GLuint program = glCreateProgram();
    // 向渲染程序中加入着色器代码
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);
    // 链接程序
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        XLOGE("glLinkProgram failed");
        return;
    }
    // 着色器激活
    glUseProgram(program);
    XLOGE("glUseProgram success");

    // 加入三维顶点数据,两个三角形组成正方形
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
    };
    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    // 传递数据
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, ver);

    // 加入材质坐标数据
    static float txt[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint atex = static_cast<GLuint>(glGetAttribLocation(program, "aTexCoord"));
    glEnableVertexAttribArray(atex);
    // 传递数据
    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txt);

    int width = 424;
    int height = 240;

    // 材质纹理初始化
    // 配置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);

    // 创建 opengl 3个纹理
    GLuint textures[3] = {0};
    glGenTextures(3, textures);
    // 设置纹理属性
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理格式和大小
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_LUMINANCE, // gpu 内部格式，灰度图
                 width, // 尺寸必须是2的次方
                 height,
                 0,// 边框
                 GL_LUMINANCE, // 与上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr // 纹理的数据
    );

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理格式和大小
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_LUMINANCE, // gpu 内部格式，灰度图
                 width / 2, // 尺寸必须是2的次方
                 height / 2,
                 0,// 边框
                 GL_LUMINANCE, // 与上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr // 纹理的数据
    );

    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理格式和大小
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_LUMINANCE, // gpu 内部格式，灰度图
                 width / 2, // 尺寸必须是2的次方
                 height / 2,
                 0,// 边框
                 GL_LUMINANCE, // 与上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr // 纹理的数据
    );

    // 纹理的修改和显示
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];
    buf[1] = new unsigned char[width * height / 4];
    buf[2] = new unsigned char[width * height / 4];

    for (int i = 0; i < 10000; i++) {
        // 420p yyyyyyyy uu vv
        if (feof(fp) == 0) {// 文件没有读到结尾
            fread(buf[0], 1, width * height, fp);
            fread(buf[0], 1, width * height / 4, fp);
            fread(buf[0], 1, width * height / 4, fp);
        }

        glActiveTexture(GL_TEXTURE0); // 激活第一层纹理
        glBindTexture(GL_TEXTURE_2D, textures[0]); // 绑定到创建的纹理中
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        glActiveTexture(GL_TEXTURE1); // 激活第二层纹理
        glBindTexture(GL_TEXTURE_2D, textures[1]); // 绑定到创建的纹理中
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[1]);

        glActiveTexture(GL_TEXTURE2); // 激活第三层纹理
        glBindTexture(GL_TEXTURE_2D, textures[2]); // 绑定到创建的纹理中
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[2]);

        // 绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // 窗口显示
        eglSwapBuffers(eglDisplay, eglSurface);

    }
    env->ReleaseStringUTFChars(url, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_NativePlayer_vanPlay(JNIEnv *env, jobject thiz, jstring url,
                                              jobject surface) {
    const char *urlStr = env->GetStringUTFChars(url, 0);
    auto *nativeWin = ANativeWindow_fromSurface(env, surface);

    IPlayerProxy::getInstance().initWindow(nativeWin);
    IPlayerProxy::getInstance().open(urlStr, false);
    IPlayerProxy::getInstance().start();
}

extern "C" JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    IPlayerProxy::getInstance().init(vm);
    return JNI_VERSION_1_4;
}
