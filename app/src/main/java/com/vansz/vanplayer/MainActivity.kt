package com.vansz.vanplayer

import android.content.Intent
import android.view.View
import android.widget.SeekBar
import com.blankj.utilcode.constant.PermissionConstants
import com.blankj.utilcode.util.PermissionUtils
import com.blankj.utilcode.util.ToastUtils
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class MainActivity : BaseActivity(), View.OnClickListener {
    private var isPause = false

    override fun getContentView() = R.layout.activity_main

    override fun init() {
        iv_open.setOnClickListener(this)
        iv_rew.setOnClickListener(this)
        ic_pause.setOnClickListener(this)
        ic_ff.setOnClickListener(this)
        seek_video.max = 1000
        seek_video.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                seekBar?.let {
                    van_player.seek(it.progress * 1.0 / it.max)
                }
            }

        })
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
            ic_pause -> {
                isPause = !isPause
                ic_pause.setImageResource(
                    if (isPause) android.R.drawable.ic_media_pause
                    else android.R.drawable.ic_media_play
                )
                van_player.pauseOrResume()
            }
            ic_ff -> null
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == 100 && resultCode == 200) {
            startPlay(data)
        }
    }

    private fun startPlay(data: Intent?) {
        val url = data?.getStringExtra("url") ?: ""
        van_player.play(url)
        seek_video.progress = 0

        GlobalScope.launch {
            while (true) {
                delay(50)
                seek_video.progress = (van_player.getCurrentProgress() * 1000).toInt()
            }
        }
    }
}
