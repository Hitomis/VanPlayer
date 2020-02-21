package com.vansz.vanplayer

import android.content.Intent
import android.view.View
import com.blankj.utilcode.constant.PermissionConstants
import com.blankj.utilcode.util.PermissionUtils
import com.blankj.utilcode.util.ToastUtils
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : BaseActivity(), View.OnClickListener {

    override fun getContentView() = R.layout.activity_main

    override fun init() {
        iv_open.setOnClickListener(this)
        iv_rew.setOnClickListener(this)
        ic_play.setOnClickListener(this)
        ic_ff.setOnClickListener(this)
    }

    override fun onClick(v: View?) {
        when (v) {
            iv_open -> {
                PermissionUtils.permission(PermissionConstants.STORAGE)
                    .callback(object : PermissionUtils.SimpleCallback {
                        override fun onGranted() {
                            val intent = Intent(this@MainActivity, MediaActivity::class.java)
                            startActivityForResult(intent, 100)
                        }

                        override fun onDenied() {
                            ToastUtils.showShort("打开失败，请授予文件访问权限。")
                        }
                    }).request()
            }
            iv_rew -> null
            ic_play -> null
            ic_ff -> null
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == 100 && resultCode == 200) {
            val url = data?.getStringExtra("url") ?: ""
            van_player.play(url)
        }
    }
}
