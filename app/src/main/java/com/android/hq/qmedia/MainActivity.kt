package com.android.hq.qmedia

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import com.android.hq.qmedia.ffmpeglib.QPlayer
import com.android.hq.qmedia.ffmpeglib.QGLSurfaceView

class MainActivity : AppCompatActivity() {
    private lateinit var surfaceview: QGLSurfaceView
    private lateinit var player:QPlayer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        surfaceview = findViewById(R.id.surface)

        player = QPlayer()
        player.setQGLSurfaceView(surfaceview)
        player.setOnPreparedListener{
            surfaceview.post {
                Utils.updateVideoSize(this,surfaceview,player.videoWidth,player.videoHeight,surfaceview.width,surfaceview.height)
            }
            player.start()
        }
        //player.setOnlyMusic(true);
        player.isOnlySoft = true

    }

    fun onClick(v: View) {
        when (v.id) {
            R.id.test -> {
                Log.e("Test","configure =  "+QPlayer.getConfigure())
                Log.e("Test","playerInfo =  "+QPlayer.getPlayerInfo())
            }
            R.id.play -> {
                player.setDataSource(Environment.getExternalStorageDirectory().absolutePath+"/test.mp4")

                player.prepared()


            }
            R.id.pause -> {
                player.pause()
            }
            R.id.resume -> {
                player.resume()
            }
        }
    }
}
