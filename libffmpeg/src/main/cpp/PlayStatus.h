

#ifndef WLPLAYER_PLAYSTATUS_H
#define WLPLAYER_PLAYSTATUS_H


class PlayStatus {

public:
    bool exit;
    bool pause;
    bool load;
    bool seek;

public:
    PlayStatus();
    ~PlayStatus();

};


#endif //WLPLAYER_PLAYSTATUS_H
