

#include "AudioChannel.h"

AudioChannel::AudioChannel(int id, AVRational base) {
    channelId = id;
    time_base = base;
}

AudioChannel::AudioChannel(int id, AVRational base, int f) {
    channelId = id;
    time_base = base;
    fps = f;
}
