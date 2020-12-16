

#ifndef QPLAYER_WLFFMPEG_H
#define QPLAYER_WLFFMPEG_H

#include "AndroidLog.h"
#include "pthread.h"
#include "BasePlayer.h"
#include "JavaCall.h"
#include "AudioPlayer.h"
#include "VideoPlayer.h"
#include "PlayStatus.h"
#include "AudioChannel.h"

extern "C"
{
#include <libavformat/avformat.h>
}


class FFmpeg {

public:
    const char *urlpath = NULL;
    JavaCall *pJavaCall = NULL;
    pthread_t decodThread;
    AVFormatContext *pFormatCtx = NULL;//封装格式上下文
    int duration = 0;
    AudioPlayer *pAudioPlayer = NULL;
    VideoPlayer *pVideoPlayer = NULL;
    PlayStatus *pPlayStatus = NULL;
    bool exit = false;
    bool exitByUser = false;
    int mimeType = 1;
    bool isAvi = false;
    bool isOnlyMusic = false;

    std::deque<AudioChannel*> audiochannels;
    std::deque<AudioChannel*> videochannels;

    pthread_mutex_t init_mutex;
    pthread_mutex_t seek_mutex;

public:
    FFmpeg(JavaCall *javaCall, const char *urlpath, bool onlymusic);
    ~FFmpeg();
    int preparedFFmpeg();
    int decodeFFmpeg();
    int start();
    int seek(int64_t sec);
    int getDuration();
    int getAvCodecContext(AVCodecParameters * parameters, BasePlayer *basePlayer);
    void release();
    void pause();
    void resume();
    int getMimeType(const char* codecName);
    void setAudioChannel(int id);
    void setVideoChannel(int id);
    int getAudioChannels();
    int getVideoWidth();
    int getVideoHeight();
};


#endif //QPLAYER_WLFFMPEG_H
