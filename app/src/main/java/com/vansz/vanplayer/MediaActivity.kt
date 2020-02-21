package com.vansz.vanplayer

import android.content.Intent
import android.view.View
import kotlinx.android.synthetic.main.activity_media.*

class MediaActivity : BaseActivity(), View.OnClickListener {

    override fun getContentView(): Int = R.layout.activity_media
    override fun init() {
        btn_play.setOnClickListener(this)
    }

    override fun onClick(v: View?) {
        when (v) {
            btn_play -> {
                with(Intent()) {
                    putExtra("url", et_storage_path.text.toString())
                    setResult(200, this)
                }
                finish()
            }
        }
    }


}
