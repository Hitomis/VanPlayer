#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "IPlayerProxy.h"

using namespace std;

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_player_NativePlayer_initPlayer(JNIEnv *env, jobject thiz, jobject surface) {
    auto *nativeWin = ANativeWindow_fromSurface(env, surface);
    IPlayerProxy::getInstance().initWindow(nativeWin);
}

extern "C" JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    IPlayerProxy::getInstance().init(vm);
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_player_NativePlayer_play(JNIEnv *env, jobject thiz, jstring url) {
    const char *urlStr = env->GetStringUTFChars(url, 0);
    IPlayerProxy::getInstance().open(urlStr, false);
    IPlayerProxy::getInstance().start();
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_vansz_vanplayer_player_NativePlayer_getPlayPosition(JNIEnv *env, jobject thiz) {
    return IPlayerProxy::getInstance().getPlayPos();
}

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_player_NativePlayer_seek(JNIEnv *env, jobject thiz, jdouble progress) {
    IPlayerProxy::getInstance().seek(progress);
}

extern "C" JNIEXPORT void JNICALL
Java_com_vansz_vanplayer_player_NativePlayer_pause(JNIEnv *env, jobject thiz) {
    IPlayerProxy::getInstance().pause(!IPlayerProxy::getInstance().isPause());
}