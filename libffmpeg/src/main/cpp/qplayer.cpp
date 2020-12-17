#include <jni.h>
#include <stddef.h>
#include "AndroidLog.h"
#include "JavaCall.h"
#include "FFmpeg.h"

_JavaVM *javaVM = NULL;
JavaCall *wlJavaCall = NULL;
FFmpeg *wlFFmpeg = NULL;

extern "C"
JNIEXPORT jstring JNICALL getConfigure(JNIEnv *env, jobject obj) {

    return env->NewStringUTF(avcodec_configuration());
}
extern "C"
JNIEXPORT void JNICALL getPlayerInfo(JNIEnv *env, jclass type) {
    av_register_all();
    // 遍历所支持的解码器
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO: LOGD("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO: LOGD("[Audio]:%s", c_temp->name);
                break;
            default: LOGD("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}


extern "C"
JNIEXPORT void JNICALL prepare(JNIEnv *env, jobject instance, jstring url_, jboolean isOnlyMusic) {
    const char *url = env->GetStringUTFChars(url_, 0);
    // TODO
    if (wlJavaCall == NULL) {
        wlJavaCall = new JavaCall(javaVM, env, &instance);
    }
    if (wlFFmpeg == NULL) {
        wlFFmpeg = new FFmpeg(wlJavaCall, url, isOnlyMusic);
        wlJavaCall->onLoad(THREAD_MAIN, true);
        wlFFmpeg->preparedFFmpeg();
    }
}

extern "C"
JNIEXPORT void JNICALL start(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->start();
    }

}

extern "C"
JNIEXPORT void JNICALL stop(JNIEnv *env, jobject instance, bool exit) {
    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->exitByUser = true;
        wlFFmpeg->release();
        delete (wlFFmpeg);
        wlFFmpeg = NULL;
        if (wlJavaCall != NULL) {
            wlJavaCall->release();
            wlJavaCall = NULL;
        }
        if (!exit) {
            jclass jlz = env->GetObjectClass(instance);
            jmethodID jmid_stop = env->GetMethodID(jlz, "onStopComplete", "()V");
            env->CallVoidMethod(instance, jmid_stop);
        }
    }

}extern "C"
JNIEXPORT void JNICALL pause(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->pause();
    }

}extern "C"
JNIEXPORT void JNICALL resume(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->resume();
    }

}extern "C"
JNIEXPORT void JNICALL seek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->seek(secds);
    }

}extern "C"
JNIEXPORT jint JNICALL getDuration(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        return wlFFmpeg->getDuration();
    }
    return 0;

}extern "C"
JNIEXPORT jint JNICALL getAudioChannels(JNIEnv *env, jobject instance) {

    if (wlFFmpeg != NULL) {
        return wlFFmpeg->getAudioChannels();
    }
    return 0;
}extern "C"
JNIEXPORT jint JNICALL getVideoHeidht(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        return wlFFmpeg->getVideoHeight();
    }
    return 0;

}extern "C"
JNIEXPORT jint JNICALL getVideoWidth(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        return wlFFmpeg->getVideoWidth();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL setAudioChannels(JNIEnv *env, jobject instance, jint index) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->setAudioChannel(index);
    }

}

// 建立jni映射表，将c和java的函数关联起来
const JNINativeMethod methods[] = {
        {"getConfigure",       "()Ljava/lang/String;",   (jobject *) getConfigure},
        {"getPlayerInfo",      "()V",                    (void *) getPlayerInfo},
        {"wlPrepared",         "(Ljava/lang/String;Z)V", (void *) prepare},
        {"wlStart",            "()V",                    (void *) start},
        {"wlStop",             "(Z)V",                   (void *) stop},
        {"wlPause",            "()V",                    (void *) pause},
        {"wlResume",           "()V",                    (void *) resume},
        {"wlSeek",             "(I)V",                   (void *) seek},
        {"wlGetDuration",      "()I",                    (void *) getDuration},
        {"wlGetAudioChannels", "()I",                    (void *) getAudioChannels},
        {"wlGetVideoHeidht",   "()I",                    (void *) getVideoHeidht},
        {"wlGetVideoWidth",    "()I",                    (void *) getVideoWidth},
        {"wlSetAudioChannels", "(I)V",                   (void *) setAudioChannels}
};

//当动态库被加载时这个函数被系统调用
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("GetEnv failed!");
        return result;
    }
    // 获取对应的 Java class
    jclass cls = env->FindClass("com/android/hq/qmedia/ffmpeglib/QPlayer");
    if (cls == NULL) {
        return JNI_ERR;
    }

    //注册映射表
    if (env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(JNINativeMethod)) < 0) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}