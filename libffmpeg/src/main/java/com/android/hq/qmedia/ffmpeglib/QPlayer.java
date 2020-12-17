package com.android.hq.qmedia.ffmpeglib;

import android.graphics.Bitmap;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import java.nio.ByteBuffer;

public class QPlayer {
    static {
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");

        System.loadLibrary("native-lib");
    }

    /**
     * 播放文件路径
     */
    private String dataSource;
    /**
     * 硬解码mime
     */
    private MediaFormat mediaFormat;
    /**
     * 视频硬解码器
     */
    private MediaCodec mediaCodec;
    /**
     * 渲染surface
     */
    private Surface surface;
    /**
     * opengl surfaceview
     */
    private QGLSurfaceView QGLSurfaceView;
    /**
     * 视频解码器info
     */
    private MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

    /**
     * 准备好时的回调
     */
    private QMediaPlayer.OnPreparedListener onPreparedListener;
    /**
     * 错误时的回调
     */
    private QMediaPlayer.OnErrorListener onErrorListener;
    /**
     * 加载回调
     */
    private QMediaPlayer.OnLoadListener onLoadListener;
    /**
     * 更新时间回调
     */
    private QMediaPlayer.OnInfoListener onInfoListener;
    /**
     * 播放完成回调
     */
    private QMediaPlayer.OnCompleteListener onCompleteListener;
    /**
     * 视频截图回调
     */
    private QMediaPlayer.OnCutVideoImgListener onCutVideoImgListener;
    /**
     * 停止完成回调
     */
    private QMediaPlayer.OnStopListener onStopListener;
    /**
     * 是否已经准备好
     */
    private boolean parpared = false;
    /**
     * 时长实体类
     */
    private TimeBean timeBean;
    /**
     * 上一次播放时间
     */
    private int lastCurrTime = 0;

    /**
     * 是否只有音频（只播放音频流）
     */
    private boolean isOnlyMusic = false;

    private boolean isOnlySoft = false;

    public QPlayer() {
        timeBean = new TimeBean();
    }

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setOnlyMusic(boolean onlyMusic) {
        isOnlyMusic = onlyMusic;
    }

    private void setSurface(Surface surface) {
        this.surface = surface;
    }

    public void setQGLSurfaceView(QGLSurfaceView QGLSurfaceView) {
        this.QGLSurfaceView = QGLSurfaceView;
        QGLSurfaceView.setOnGlSurfaceViewOncreateListener(new QMediaPlayer.OnGlSurfaceViewOnCreateListener() {
            @Override
            public void onGlSurfaceViewOncreate(Surface s) {
                if (surface == null) {
                    setSurface(s);
                }
                if (parpared && !TextUtils.isDigitsOnly(dataSource)) {
                    wlPrepared(dataSource, isOnlyMusic);
                }
            }

            @Override
            public void onCutVideoImg(Bitmap bitmap) {
                if (onCutVideoImgListener != null) {
                    onCutVideoImgListener.onCutVideoImg(bitmap);
                }
            }
        });
    }


    /**
     * 准备
     *
     * @param url
     */
    private native void wlPrepared(String url, boolean isOnlyMusic);

    /**
     * 开始
     */
    private native void wlStart();

    /**
     * 停止并释放资源
     */
    private native void wlStop(boolean exit);

    /**
     * 暂停
     */
    private native void wlPause();

    /**
     * 播放 对应暂停
     */
    private native void wlResume();

    /**
     * seek
     *
     * @param secds
     */
    private native void wlSeek(int secds);

    /**
     * 设置音轨 根据获取的音轨数 排序
     *
     * @param index
     */
    private native void wlSetAudioChannels(int index);

    /**
     * 获取总时长
     *
     * @return
     */
    private native int wlGetDuration();

    /**
     * 获取音轨数
     *
     * @return
     */
    private native int wlGetAudioChannels();

    /**
     * 获取视频宽度
     *
     * @return
     */
    private native int wlGetVideoWidth();

    /**
     * 获取视频长度
     *
     * @return
     */
    private native int wlGetVideoHeidht();

    public static native String getConfigure();

    public static native void getPlayerInfo();

    public int getDuration() {
        return wlGetDuration();
    }

    public int getAudioChannels() {
        return wlGetAudioChannels();
    }

    public int getVideoWidth() {
        return wlGetVideoWidth();
    }

    public int getVideoHeight() {
        return wlGetVideoHeidht();
    }

    public void setAudioChannels(int index) {
        wlSetAudioChannels(index);
    }


    public void setOnPreparedListener(QMediaPlayer.OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }


    public void setOnErrorListener(QMediaPlayer.OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void prepared() {
        if (TextUtils.isEmpty(dataSource)) {
            onError(Status.WL_STATUS_DATASOURCE_NULL, "datasource is null");
            return;
        }
        parpared = true;
        if (isOnlyMusic) {
            wlPrepared(dataSource, isOnlyMusic);
        } else {
            if (surface != null) {
                wlPrepared(dataSource, isOnlyMusic);
            }
        }
    }

    public void start() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (TextUtils.isEmpty(dataSource)) {
                    onError(Status.WL_STATUS_DATASOURCE_NULL, "datasource is null");
                    return;
                }
                if (!isOnlyMusic) {
                    if (surface == null) {
                        onError(Status.WL_STATUS_SURFACE_NULL, "surface is null");
                        return;
                    }
                }

                if (timeBean == null) {
                    timeBean = new TimeBean();
                }
                wlStart();
            }
        }).start();
    }

    public void stop(final boolean exit) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                wlStop(exit);
                if (mediaCodec != null) {
                    try {
                        mediaCodec.flush();
                        mediaCodec.stop();
                        mediaCodec.release();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    mediaCodec = null;
                    mediaFormat = null;
                }
                if (QGLSurfaceView != null) {
                    QGLSurfaceView.setCodecType(-1);
                    QGLSurfaceView.requestRender();
                }

            }
        }).start();
    }

    public void pause() {
        wlPause();

    }

    public void resume() {
        wlResume();
    }

    public void seek(final int secds) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                wlSeek(secds);
                lastCurrTime = secds;
            }
        }).start();
    }

    public void setOnlySoft(boolean soft) {
        this.isOnlySoft = soft;
    }

    public boolean isOnlySoft() {
        return isOnlySoft;
    }


    private void onLoad(boolean load) {
        if (onLoadListener != null) {
            onLoadListener.onLoad(load);
        }
    }

    private void onError(int code, String msg) {
        if (onErrorListener != null) {
            onErrorListener.onError(code, msg);
        }
        stop(true);
    }

    private void onParpared() {
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }

    public void mediacodecInit(int mimetype, int width, int height, byte[] csd0, byte[] csd1) {
        if (surface != null) {
            try {
                QGLSurfaceView.setCodecType(1);
                String mtype = getMimeType(mimetype);
                mediaFormat = MediaFormat.createVideoFormat(mtype, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_WIDTH, width);
                mediaFormat.setInteger(MediaFormat.KEY_HEIGHT, height);
                mediaFormat.setLong(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd1));
                Log.d("ywl5320", mediaFormat.toString());
                mediaCodec = MediaCodec.createDecoderByType(mtype);
                if (surface != null) {
                    mediaCodec.configure(mediaFormat, surface, null, 0);
                    mediaCodec.start();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            if (onErrorListener != null) {
                onErrorListener.onError(Status.WL_STATUS_SURFACE_NULL, "surface is null");
            }
        }
    }

    public void mediacodecDecode(byte[] bytes, int size, int pts) {
        if (bytes != null && mediaCodec != null && info != null) {
            try {
                int inputBufferIndex = mediaCodec.dequeueInputBuffer(10);
                if (inputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(bytes);
                    mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, pts, 0);
                }
                int index = mediaCodec.dequeueOutputBuffer(info, 10);
                while (index >= 0) {
                    mediaCodec.releaseOutputBuffer(index, true);
                    index = mediaCodec.dequeueOutputBuffer(info, 10);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void setOnLoadListener(QMediaPlayer.OnLoadListener onLoadListener) {
        this.onLoadListener = onLoadListener;
    }

    private String getMimeType(int type) {
        if (type == 1) {
            return "video/avc";
        } else if (type == 2) {
            return "video/hevc";
        } else if (type == 3) {
            return "video/mp4v-es";
        } else if (type == 4) {
            return "video/x-ms-wmv";
        }
        return "";
    }

    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v) {
        if (QGLSurfaceView != null) {
            LogUtil.d("setFrameData");
            QGLSurfaceView.setCodecType(0);
            QGLSurfaceView.setFrameData(w, h, y, u, v);
        }
    }

    public void setOnInfoListener(QMediaPlayer.OnInfoListener onInfoListener) {
        this.onInfoListener = onInfoListener;
    }

    public void setVideoInfo(int currt_secd, int total_secd) {
        if (onInfoListener != null && timeBean != null) {
            if (currt_secd < lastCurrTime) {
                currt_secd = lastCurrTime;
            }
            timeBean.setCurrt_secds(currt_secd);
            timeBean.setTotal_secds(total_secd);
            onInfoListener.onInfo(timeBean);
            lastCurrTime = currt_secd;
        }
    }

    public void setOnCompleteListener(QMediaPlayer.OnCompleteListener onCompleteListener) {
        this.onCompleteListener = onCompleteListener;
    }

    public void videoComplete() {
        if (onCompleteListener != null) {
            setVideoInfo(wlGetDuration(), wlGetDuration());
            timeBean = null;
            onCompleteListener.onComplete();
        }
    }

    public void setOnCutVideoImgListener(QMediaPlayer.OnCutVideoImgListener onCutVideoImgListener) {
        this.onCutVideoImgListener = onCutVideoImgListener;
    }

    public void cutVideoImg() {
        if (QGLSurfaceView != null) {
            QGLSurfaceView.cutVideoImg();
        }
    }

    public void setOnStopListener(QMediaPlayer.OnStopListener onStopListener) {
        this.onStopListener = onStopListener;
    }

    public void onStopComplete() {
        if (onStopListener != null) {
            onStopListener.onStop();
        }
    }
}
