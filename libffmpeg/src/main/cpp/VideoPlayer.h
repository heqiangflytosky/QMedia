

#ifndef WLPLAYER_VIDEO_H
#define WLPLAYER_VIDEO_H


#include "BasePlayer.h"
#include "QQueue.h"
#include "JavaCall.h"
#include "AndroidLog.h"
#include "AudioPlayer.h"

extern "C"
{
    #include <libavutil/time.h>
};

class VideoPlayer : public BasePlayer{

public:
    QQueue *queue = NULL;
    AudioPlayer *pAudioPlayer = NULL;
    PlayStatus *pPlayStatus = NULL;
    pthread_t mVideoThread;
    pthread_t mDecFrame;
    JavaCall *pJavaCall = NULL;

    double delayTime = 0;
    int rate = 0;
    bool isExit = true;
    bool isExit2 = true;
    int codecType = -1;
    double video_clock = 0;
    double framePts = 0;
    bool frameRateBig = false;
    int playCount = -1;

public:
    VideoPlayer(JavaCall *javaCall, AudioPlayer *audio, PlayStatus *playStatus);
    ~VideoPlayer();

    void playVideo(int codecType);
    void decodVideo();
    void release();
    double synchronize(AVFrame *srcFrame, double pts);

    double getDelayTime(double diff);

    void setClock(int secds);

};


#endif //WLPLAYER_VIDEO_H
