package com.vansz.vanplayer.player;

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

    public native void initPlayer(Object surface);

    public native void play(String url);

    public native double getPlayPosition();

    public native void seek(double progress);

    public native void pause();
}
