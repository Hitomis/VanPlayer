package com.vansz.vanplayer.player

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.SurfaceHolder
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * Created by Vans Z on 2020-02-10.
 */
class VanPlayer : GLSurfaceView, SurfaceHolder.Callback, GLSurfaceView.Renderer {

    constructor(context: Context?) : this(context, null)
    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        // Android 8.0 以下 需要设置
        setRenderer(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder?) {
        GlobalScope.launch {
            holder?.let {
                NativePlayer.getInstance().initPlayer(it.surface)
            }
        }
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, w: Int, h: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }

    override fun onDrawFrame(gl: GL10?) {
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
    }

    fun play(url: String) {
        NativePlayer.getInstance().play(url)
    }

    fun getCurrentProgress(): Double = NativePlayer.getInstance().playPosition

    fun seek(progress: Double) = NativePlayer.getInstance().seek(progress)

}