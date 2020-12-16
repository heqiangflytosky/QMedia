package com.android.hq.qmedia.ffmpeglib;

import android.graphics.Bitmap;
import android.view.Surface;

public class QMediaPlayer {
    public interface OnPreparedListener {
        void onPrepared();
    }


    public interface OnCompleteListener {

        void onComplete();

    }

    public interface OnCutVideoImgListener {

        void onCutVideoImg(Bitmap bitmap);

    }

    public interface OnErrorListener {

        void onError(int code, String msg);

    }

    public interface OnGlSurfaceViewOnCreateListener {

        void onGlSurfaceViewOncreate(Surface surface);

        void onCutVideoImg(Bitmap bitmap);

    }

    public interface OnInfoListener {

        void onInfo(TimeBean timeBean);

    }


    public interface OnLoadListener {

        void onLoad(boolean load);

    }

    public interface OnRenderRefreshListener {

        void onRefresh();

    }


    public interface OnStopListener {

        void onStop();

    }
}
