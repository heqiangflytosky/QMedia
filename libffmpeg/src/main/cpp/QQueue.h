

#ifndef QPLAYER_QUEUE_H
#define QPLAYER_QUEUE_H

#include "queue"
#include "PlayStatus.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class QQueue {

public:
    std::queue<AVPacket*> queuePacket;
    std::queue<AVFrame*> queueFrame;
    pthread_mutex_t mutexFrame;
    pthread_cond_t condFrame;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    PlayStatus *pPlayStatus = NULL;

public:
    QQueue(PlayStatus *playStatus);
    ~QQueue();
    int putAvpacket(AVPacket *avPacket);
    int getAvpacket(AVPacket *avPacket);
    int clearAvpacket();
    int clearToKeyFrame();

    int putAvframe(AVFrame *avFrame);
    int getAvframe(AVFrame *avFrame);
    int clearAvFrame();

    void release();
    int getAvPacketSize();
    int getAvFrameSize();

    int noticeThread();
};


#endif //QPLAYER_QUEUE_H
