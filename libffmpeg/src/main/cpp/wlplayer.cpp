#include <jni.h>
#include <stddef.h>
#include "AndroidLog.h"
#include "WlJavaCall.h"
#include "WlFFmpeg.h"

_JavaVM *javaVM = NULL;
WlJavaCall *wlJavaCall = NULL;
WlFFmpeg *wlFFmpeg = NULL;

extern "C"
JNIEXPORT jstring JNICALL Java_com_android_hq_qmedia_ffmpeglib_QPlayer_getConfigure(JNIEnv *env, jobject obj) {

    return env->NewStringUTF(avcodec_configuration());
}
extern "C"
JNIEXPORT void JNICALL Java_com_android_hq_qmedia_ffmpeglib_QPlayer_getPlayerInfo(JNIEnv *env, jclass type) {
    av_register_all();
    // 遍历所支持的解码器
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL)
    {
        switch (c_temp->type)
        {
            case AVMEDIA_TYPE_VIDEO:
                LOGD("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGD("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGD("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlPrepared(JNIEnv *env, jobject instance, jstring url_, jboolean isOnlyMusic) {
    const char *url = env->GetStringUTFChars(url_, 0);
    // TODO
    if(wlJavaCall == NULL)
    {
        wlJavaCall = new WlJavaCall(javaVM, env, &instance);
    }
    if(wlFFmpeg == NULL)
    {
        wlFFmpeg = new WlFFmpeg(wlJavaCall, url, isOnlyMusic);
        wlJavaCall->onLoad(WL_THREAD_MAIN, true);
        wlFFmpeg->preparedFFmpeg();
    }
}


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint result = -1;
    javaVM = vm;
    JNIEnv* env;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        if(LOG_SHOW)
        {
            LOGE("GetEnv failed!");
        }
        return result;
    }
    return JNI_VERSION_1_4;
}extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlStart(JNIEnv *env, jobject instance) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        wlFFmpeg->start();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlStop(JNIEnv *env, jobject instance, bool exit) {
    // TODO
    if(wlFFmpeg != NULL)
    {
        wlFFmpeg->exitByUser = true;
        wlFFmpeg->release();
        delete(wlFFmpeg);
        wlFFmpeg = NULL;
        if(wlJavaCall != NULL)
        {
            wlJavaCall->release();
            wlJavaCall = NULL;
        }
        if(!exit)
        {
            jclass jlz = env->GetObjectClass(instance);
            jmethodID jmid_stop = env->GetMethodID(jlz, "onStopComplete", "()V");
            env->CallVoidMethod(instance, jmid_stop);
        }
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlPause(JNIEnv *env, jobject instance) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        wlFFmpeg->pause();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlResume(JNIEnv *env, jobject instance) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        wlFFmpeg->resume();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlSeek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        wlFFmpeg->seek(secds);
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlGetDuration(JNIEnv *env, jobject instance) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        return wlFFmpeg->getDuration();
    }
    return 0;

}extern "C"
JNIEXPORT jint JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlGetAudioChannels(JNIEnv *env, jobject instance) {

    if(wlFFmpeg != NULL)
    {
        return wlFFmpeg->getAudioChannels();
    }
    return 0;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlGetVideoHeidht(JNIEnv *env, jobject instance) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        return wlFFmpeg->getVideoHeight();
    }
    return 0;

}extern "C"
JNIEXPORT jint JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlGetVideoWidth(JNIEnv *env, jobject instance) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        return wlFFmpeg->getVideoWidth();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_hq_qmedia_ffmpeglib_QPlayer_wlSetAudioChannels(JNIEnv *env, jobject instance, jint index) {

    // TODO
    if(wlFFmpeg != NULL)
    {
        wlFFmpeg->setAudioChannel(index);
    }

}