

#ifndef QPLAYER_BASEPLAYER_H
#define QPLAYER_BASEPLAYER_H

extern "C"
{
#include <libavcodec/avcodec.h>
};

class BasePlayer {

public:
    int streamIndex;
    int duration;
    double clock = 0;
    double now_time = 0;
    AVCodecContext *pAVCodecContext = NULL;
    AVRational time_base;

public:
    BasePlayer();
    ~BasePlayer();
};


#endif //QPLAYER_BASEPLAYER_H
