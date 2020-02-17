package com.vansz.vanplayer;

/**
 * Created by Vans Z on 2020-02-10.
 */
public class NativePlayer {
    static {
        System.loadLibrary("native-lib");
    }

    private NativePlayer() {
    }

    private static class SingletonHolder {
        private final static NativePlayer instance = new NativePlayer();
    }

    public static NativePlayer getInstance() {
        return SingletonHolder.instance;
    }

    public native void vanPlay(String url, Object surface);

    public native void play(String url, Object surface);

    public native void audio(String url);

    public native void video(String url, Object surface);
}
