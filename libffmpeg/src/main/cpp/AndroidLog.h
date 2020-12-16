
#pragma once
#ifndef QPLAYER_ANDROIDLOG_H
#define QPLAYER_ANDROIDLOG_H

#include <android/log.h>

#define LOG_SHOW true

#define LOGD(FORMAT,...){ if (LOG_SHOW){__android_log_print(ANDROID_LOG_DEBUG,"ffmpeg",FORMAT,##__VA_ARGS__);}}
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);

#endif //QPLAYER_ANDROIDLOG_H
