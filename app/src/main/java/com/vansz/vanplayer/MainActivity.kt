package com.vansz.vanplayer

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.blankj.utilcode.constant.PermissionConstants
import com.blankj.utilcode.util.PermissionUtils
import com.gyf.immersionbar.ImmersionBar
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        ImmersionBar.with(this)
            .transparentBar()
            .statusBarDarkFont(true, 0.2f)
            .init()

        setContentView(R.layout.activity_main)
        PermissionUtils.permission(PermissionConstants.STORAGE)
            .callback(object : PermissionUtils.SimpleCallback {
                override fun onGranted() {
                }

                override fun onDenied() {}
            }).request()

//        btn_audio.setOnClickListener{
//            NativePlayer.getInstance().audio("")
//        }

    }
}
