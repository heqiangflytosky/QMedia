package com.android.hq.qmedia.ffmpeglib;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class QGLSurfaceView extends GLSurfaceView {

    private QGLRender QGLRender;
    private QMediaPlayer.OnGlSurfaceViewOnCreateListener onGlSurfaceViewOncreateListener;

    public QGLSurfaceView(Context context) {
        this(context, null);
    }

    public QGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        QGLRender = new QGLRender(context);
        //设置egl版本为2.0
        setEGLContextClientVersion(2);
        //设置render
        setRenderer(QGLRender);
        //设置为手动刷新模式
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        QGLRender.setOnRenderRefreshListener(new QMediaPlayer.OnRenderRefreshListener() {
            @Override
            public void onRefresh() {
                requestRender();
            }
        });
    }

    public void setOnGlSurfaceViewOncreateListener(QMediaPlayer.OnGlSurfaceViewOnCreateListener onGlSurfaceViewOncreateListener) {
        if(QGLRender != null)
        {
            QGLRender.setOnGlSurfaceViewOncreateListener(onGlSurfaceViewOncreateListener);
        }
    }

    public void setCodecType(int type)
    {
        if(QGLRender != null)
        {
            QGLRender.setCodecType(type);
        }
    }


    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v)
    {
        if(QGLRender != null)
        {
            QGLRender.setFrameData(w, h, y, u, v);
            requestRender();
        }
    }

    public void cutVideoImg()
    {
        if(QGLRender != null)
        {
            QGLRender.cutVideoImg();
            requestRender();
        }
    }
}
