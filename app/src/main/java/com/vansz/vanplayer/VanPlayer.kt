package com.vansz.vanplayer

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.SurfaceHolder
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * Created by Vans Z on 2020-02-10.
 */
class VanPlayer : GLSurfaceView, SurfaceHolder.Callback, GLSurfaceView.Renderer {

    constructor(context: Context?) : this(context, null)
    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs)

    override fun surfaceCreated(holder: SurfaceHolder?) {
        GlobalScope.launch {
            holder?.let {
                NativePlayer.getInstance().vanPlay("/sdcard/jj.mp4", it.surface)
                // Android 8.0 需要设置
                setRenderer(this@VanPlayer)
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

}