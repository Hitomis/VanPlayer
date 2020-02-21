package com.vansz.vanplayer

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.gyf.immersionbar.ImmersionBar

/**
 * Created by Vans Z on 2020-02-21.
 */
abstract class BaseActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(getContentView())
        ImmersionBar.with(this)
            .transparentBar()
            .statusBarDarkFont(true, 0.2f)
            .init()
        init()
    }

    abstract fun getContentView(): Int

    abstract fun init()

}