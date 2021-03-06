//
// Created by 赵帆 on 2020-02-15.
//

#include <android/native_window.h>
#include <EGL/egl.h>
#include <mutex>
#include "XEGL.h"
#include "XLog.h"

class CXEGL : public XEGL {
public:
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglContext = EGL_NO_CONTEXT;
    std::mutex mux;

    bool init(void *nativeWin) override {
        close();
        mux.lock();
        // 获取原始窗口
        auto *win = static_cast<ANativeWindow *>(nativeWin);
        // 1.获取EGLDisplay对象 显示设备
        eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (eglDisplay == EGL_NO_DISPLAY) {
            XLOGE("eglGetDisplay failed");
            mux.unlock();
            return false;
        }
        if (EGL_TRUE != eglInitialize(eglDisplay, nullptr, nullptr)) {
            XLOGE("eglInitialize failed");
            mux.unlock();
            return false;
        }
        // 2.获取配置并创建surface
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
            mux.unlock();
            return false;
        }
        eglSurface = eglCreateWindowSurface(eglDisplay, config, win, nullptr);
        if (EGL_NO_SURFACE == eglSurface) {
            XLOGE("eglCreateWindowSurface failed");
            mux.unlock();
            return false;
        }
        // 3.创建并打开EGL上下文
        const EGLint ctxAttr[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
        };
        eglContext = eglCreateContext(eglDisplay, config, EGL_NO_SURFACE, ctxAttr);
        if (EGL_NO_CONTEXT == eglContext) {
            XLOGE("eglCreateContext failed");
            mux.unlock();
            return false;
        }
        if (EGL_TRUE == eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
            XLOGE("eglMakeCurrent success");
        }
        mux.unlock();
        return true;
    }

    void draw() override {
        mux.lock();
        if (eglDisplay == EGL_NO_DISPLAY || eglSurface == EGL_NO_SURFACE) {
            mux.unlock();
            return;
        }
        eglSwapBuffers(eglDisplay, eglSurface);
        mux.unlock();
    }

    void close() override {
        mux.lock();
        if (eglDisplay != EGL_NO_DISPLAY) {
            mux.unlock();
            return;
        }
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(eglDisplay, eglSurface);
        }
        if (eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(eglDisplay, eglContext);
        }

        eglTerminate(eglDisplay);
        eglDisplay = EGL_NO_DISPLAY;
        eglSurface = EGL_NO_SURFACE;
        eglContext = EGL_NO_CONTEXT;
        mux.unlock();
    }
};

XEGL *XEGL::getInstance() {
    static CXEGL egl;
    return &egl;
}
