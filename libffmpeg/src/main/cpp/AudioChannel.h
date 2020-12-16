

#ifndef QPLAYER_AUDIOCHANNEL_H
#define QPLAYER_AUDIOCHANNEL_H


extern "C"
{
#include <libavutil/rational.h>
};

class AudioChannel {
public:
    int channelId = -1;
    AVRational time_base;
    int fps;

public:
    AudioChannel(int id, AVRational base);
    AudioChannel(int id, AVRational base, int fps);
};


#endif //QPLAYER_AUDIOCHANNEL_H
