
#include "FFmpeg.h"

void *decodeThread(void *data)
{
    FFmpeg *ffmpeg = (FFmpeg *) data;
    ffmpeg->decodeFFmpeg();
    pthread_exit(&ffmpeg->decodThread);
}


int FFmpeg::preparedFFmpeg() {
    pthread_create(&decodThread, NULL, decodeThread, this);
    return 0;
}

FFmpeg::FFmpeg(JavaCall *javaCall, const char *url, bool onlymusic) {
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
    exitByUser = false;
    isOnlyMusic = onlymusic;
    pJavaCall = javaCall;
    urlpath = url;
    pPlayStatus = new PlayStatus();
}

int avformat_interrupt_cb(void *ctx)
{
    FFmpeg *ffmpeg = (FFmpeg *) ctx;
    if(ffmpeg->pPlayStatus->exit)
    {
        LOGE("avformat_interrupt_cb return 1")
        return AVERROR_EOF;
    }
    LOGE("avformat_interrupt_cb return 0")
    return 0;
}

int FFmpeg::decodeFFmpeg() {
    pthread_mutex_lock(&init_mutex);
    exit = false;
    isAvi = false;
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, urlpath, NULL, NULL) != 0)
    {
        LOGE("can not open url:%s", urlpath);
        if(pJavaCall != NULL)
        {
            pJavaCall->onError(THREAD_CHILD, ERROR_FFMPEG_CAN_NOT_OPEN_URL, "can not open url");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    pFormatCtx->interrupt_callback.callback = avformat_interrupt_cb;
    pFormatCtx->interrupt_callback.opaque = this;

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        LOGE("can not find streams from %s", urlpath);
        if(pJavaCall != NULL) {
            pJavaCall->onError(THREAD_CHILD, ERROR_FFMPEG_CAN_NOT_FIND_STREAMS,
                                "can not find streams from url");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if(pFormatCtx == NULL)
    {
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    duration = pFormatCtx->duration / 1000000;
    LOGD("channel numbers is %d", pFormatCtx->nb_streams);
    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO )//音频
        {
            LOGE("音频");
            AudioChannel *ac = new AudioChannel(i, pFormatCtx->streams[i]->time_base);
            audiochannels.push_front(ac);
        }
        else if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)//视频
        {
            if(!isOnlyMusic)
            {
                LOGE("视频");
                int num = pFormatCtx->streams[i]->avg_frame_rate.num;
                int den = pFormatCtx->streams[i]->avg_frame_rate.den;
                if(num != 0 && den != 0)
                {
                    int fps = pFormatCtx->streams[i]->avg_frame_rate.num / pFormatCtx->streams[i]->avg_frame_rate.den;
                    AudioChannel *ac = new AudioChannel(i, pFormatCtx->streams[i]->time_base, fps);
                    videochannels.push_front(ac);
                }
            }
        }
    }


    if(audiochannels.size() > 0)
    {
        pAudioPlayer = new AudioPlayer(pPlayStatus, pJavaCall);
        setAudioChannel(0);
        if(pAudioPlayer->streamIndex >= 0 && pAudioPlayer->streamIndex < pFormatCtx->nb_streams)
        {
            if(getAvCodecContext(pFormatCtx->streams[pAudioPlayer->streamIndex]->codecpar, pAudioPlayer) != 0)
            {
                exit = true;
                pthread_mutex_unlock(&init_mutex);
                return 1;
            }
        }


    }
    if(videochannels.size() > 0)
    {
        pVideoPlayer = new VideoPlayer(pJavaCall, pAudioPlayer, pPlayStatus);
        setVideoChannel(0);
        if(pVideoPlayer->streamIndex >= 0 && pVideoPlayer->streamIndex < pFormatCtx->nb_streams)
        {
            if(getAvCodecContext(pFormatCtx->streams[pVideoPlayer->streamIndex]->codecpar, pVideoPlayer) != 0)
            {
                exit = true;
                pthread_mutex_unlock(&init_mutex);
                return 1;
            }
        }
    }

    if(pAudioPlayer == NULL && pVideoPlayer == NULL)
    {
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return 1;
    }
    if(pAudioPlayer != NULL)
    {
        pAudioPlayer->duration = pFormatCtx->duration / 1000000;
        pAudioPlayer->sample_rate = pAudioPlayer->pAVCodecContext->sample_rate;
        if(pVideoPlayer != NULL)
        {
            pAudioPlayer->setVideo(true);
        }
    }
    if(pVideoPlayer != NULL)
    {
        LOGE("codec name is %s", pVideoPlayer->pAVCodecContext->codec->name);
        LOGE("codec long name is %s", pVideoPlayer->pAVCodecContext->codec->long_name);
        if(!pJavaCall->isOnlySoft(THREAD_CHILD))
        {
            mimeType = getMimeType(pVideoPlayer->pAVCodecContext->codec->name);
        } else{
            mimeType = -1;
        }

        if(mimeType != -1)
        {
            pJavaCall->onInitMediacodec(THREAD_CHILD, mimeType, pVideoPlayer->pAVCodecContext->width, pVideoPlayer->pAVCodecContext->height, pVideoPlayer->pAVCodecContext->extradata_size, pVideoPlayer->pAVCodecContext->extradata_size, pVideoPlayer->pAVCodecContext->extradata, pVideoPlayer->pAVCodecContext->extradata);
        }
        pVideoPlayer->duration = pFormatCtx->duration / 1000000;
    }
    LOGD("准备ing");
    LOGD("视频信息：------------------------------------------------------------->");
    LOGD("视频地址：%s",urlpath);
    LOGD("视频时长：%d",duration);
    LOGD("视频宽度：%d",getVideoWidth());
    LOGD("视频高度：%d",getVideoHeight());
    LOGD("视频信息：<-------------------------------------------------------------");
    pJavaCall->onParpared(THREAD_CHILD);
    LOGD("准备end");
    exit = true;
    pthread_mutex_unlock(&init_mutex);
    return 0;
}

int FFmpeg::getAvCodecContext(AVCodecParameters *parameters, BasePlayer *basePlayer) {

    AVCodec *dec = avcodec_find_decoder(parameters->codec_id);
    if(!dec)
    {
        pJavaCall->onError(THREAD_CHILD, 3, "get avcodec fail");
        exit = true;
        return 1;
    }
    basePlayer->pAVCodecContext = avcodec_alloc_context3(dec);
    if(!basePlayer->pAVCodecContext)
    {
        pJavaCall->onError(THREAD_CHILD, 4, "alloc avcodecctx fail");
        exit = true;
        return 1;
    }
    if(avcodec_parameters_to_context(basePlayer->pAVCodecContext, parameters) != 0)
    {
        pJavaCall->onError(THREAD_CHILD, 5, "copy avcodecctx fail");
        exit = true;
        return 1;
    }
    if(avcodec_open2(basePlayer->pAVCodecContext, dec, 0) != 0)
    {
        pJavaCall->onError(THREAD_CHILD, 6, "open avcodecctx fail");
        exit = true;
        return 1;
    }
    return 0;
}


FFmpeg::~FFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    LOGE("FFmpeg 释放了");
}


int FFmpeg::getDuration() {
    return duration;
}

int FFmpeg::start() {
    exit = false;
    int count = 0;
    int ret  = -1;
    if(pAudioPlayer != NULL)
    {
        pAudioPlayer->playAudio();
    }
    if(pVideoPlayer != NULL)
    {
        if(mimeType == -1)
        {
            pVideoPlayer->playVideo(DECODE_SOFTWARE);
        }
        else
        {
            pVideoPlayer->playVideo(DECODE_HARDWARE);
        }
    }

    AVBitStreamFilterContext* mimType = NULL;
    if(mimeType == CODEC_TYPE_H264){
        mimType =  av_bitstream_filter_init("h264_mp4toannexb");
    }
    else if(mimeType == CODEC_TYPE_HEVC){
        mimType =  av_bitstream_filter_init("hevc_mp4toannexb");
    }
    else if(mimeType == CODEC_TYPE_MPEG4){
        mimType =  av_bitstream_filter_init("h264_mp4toannexb");
    }
    else if(mimeType == CODEC_TYPE_WMV){
        mimType =  av_bitstream_filter_init("h264_mp4toannexb");
    }

    while(!pPlayStatus->exit){
        exit = false;
        //暂停
        if(pPlayStatus->pause){
            av_usleep(1000 * 100);
            continue;
        }
        if(pAudioPlayer != NULL && pAudioPlayer->queue->getAvPacketSize() > 100){
//            LOGE("pAudioPlayer 等待..........");
            av_usleep(1000 * 100);
            continue;
        }
        if(pVideoPlayer != NULL && pVideoPlayer->queue->getAvPacketSize() > 100){
//            LOGE("pVideoPlayer 等待..........");
            av_usleep(1000 * 100);
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        ret = av_read_frame(pFormatCtx, packet);
        pthread_mutex_unlock(&seek_mutex);
        if(pPlayStatus->seek){
            av_packet_free(&packet);
            av_free(packet);
            continue;
        }
        if(ret == 0){
            if(pAudioPlayer != NULL && packet->stream_index ==  pAudioPlayer->streamIndex){
                count++;
                LOGE("解码第 %d 帧", count);
                pAudioPlayer->queue->putAvpacket(packet);
            }else if(pVideoPlayer != NULL && packet->stream_index == pVideoPlayer->streamIndex){
                if(mimType != NULL && !isAvi){
                    uint8_t *data;
                    av_bitstream_filter_filter(mimType, pFormatCtx->streams[pVideoPlayer->streamIndex]->codec, NULL, &data, &packet->size, packet->data, packet->size, 0);
                    uint8_t *tdata = NULL;
                    tdata = packet->data;
                    packet->data = data;
                    if(tdata != NULL){
                        av_free(tdata);
                    }
                }
                pVideoPlayer->queue->putAvpacket(packet);
            }
            else{
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;
            }
        } else{
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            if((pVideoPlayer != NULL && pVideoPlayer->queue->getAvFrameSize() == 0)
            || (pAudioPlayer != NULL && pAudioPlayer->queue->getAvPacketSize() == 0)){
                pPlayStatus->exit = true;
                break;
            }
        }
    }
    if(mimType != NULL){
        av_bitstream_filter_close(mimType);
    }
    if(!exitByUser && pJavaCall != NULL){
        pJavaCall->onComplete(THREAD_CHILD);
    }
    exit = true;
    return 0;
}

void FFmpeg::release() {
    pPlayStatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    LOGE("开始释放 ffmpeg");
    int sleepCount = 0;
    while(!exit){
        if(sleepCount > 1000)//十秒钟还没有退出就自动强制退出
        {
            exit = true;
        }
        LOGE("wait ffmpeg  exit %d", sleepCount);

        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }
    LOGE("释放audio....................................");

    if(pAudioPlayer != NULL){
        LOGE("释放audio....................................2");

        pAudioPlayer->realease();
        delete(pAudioPlayer);
        pAudioPlayer = NULL;
    }
    LOGE("释放video....................................");

    if(pVideoPlayer != NULL){
        LOGE("释放video....................................2");

        pVideoPlayer->release();
        delete(pVideoPlayer);
        pVideoPlayer = NULL;
    }
    LOGE("释放format...................................");

    if(pFormatCtx != NULL){
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    LOGE("释放javacall.................................");

    if(pJavaCall != NULL){
        pJavaCall = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

void FFmpeg::pause() {
    if(pPlayStatus != NULL){
        pPlayStatus->pause = true;
        if(pAudioPlayer != NULL){
            pAudioPlayer->pause();
        }
    }
}

void FFmpeg::resume() {
    if(pPlayStatus != NULL){
        pPlayStatus->pause = false;
        if(pAudioPlayer != NULL){
            pAudioPlayer->resume();
        }
    }
}

int FFmpeg::getMimeType(const char *codecName) {

    if(strcmp(codecName, "h264") == 0){
        return CODEC_TYPE_H264;
    }
    if(strcmp(codecName, "hevc") == 0){
        return CODEC_TYPE_HEVC;
    }
    if(strcmp(codecName, "mpeg4") == 0){
        isAvi = true;
        return CODEC_TYPE_MPEG4;
    }
    if(strcmp(codecName, "wmv3") == 0){
        isAvi = true;
        return CODEC_TYPE_WMV;
    }

    return -1;
}

int FFmpeg::seek(int64_t sec) {
    if(sec >= duration){
        return -1;
    }
    if(pPlayStatus->load){
        return -1;
    }
    if(pFormatCtx != NULL){
        pPlayStatus->seek = true;
        pthread_mutex_lock(&seek_mutex);
        int64_t rel = sec * AV_TIME_BASE;
        int ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
        if(pAudioPlayer != NULL){
            pAudioPlayer->queue->clearAvpacket();
//            av_seek_frame(pFormatCtx, pAudioPlayer->streamIndex, sec * pAudioPlayer->time_base.den, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
            pAudioPlayer->setClock(0);
        }
        if(pVideoPlayer != NULL){
            pVideoPlayer->queue->clearAvFrame();
            pVideoPlayer->queue->clearAvpacket();
//            av_seek_frame(pFormatCtx, pVideoPlayer->streamIndex, sec * pVideoPlayer->time_base.den, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
            pVideoPlayer->setClock(0);
        }
        pAudioPlayer->clock = 0;
        pAudioPlayer->now_time = 0;
        pthread_mutex_unlock(&seek_mutex);
        pPlayStatus->seek = false;
    }
    return 0;
}

void FFmpeg::setAudioChannel(int index) {
    if(pAudioPlayer != NULL){
        int channelsize = audiochannels.size();
        if(index < channelsize){
            for(int i = 0; i < channelsize; i++){
                if(i == index){
                    pAudioPlayer->time_base = audiochannels.at(i)->time_base;
                    pAudioPlayer->streamIndex = audiochannels.at(i)->channelId;
                }
            }
        }
    }

}

void FFmpeg::setVideoChannel(int id) {
    if(pVideoPlayer != NULL){
        pVideoPlayer->streamIndex = videochannels.at(id)->channelId;
        pVideoPlayer->time_base = videochannels.at(id)->time_base;
        pVideoPlayer->rate = 1000 / videochannels.at(id)->fps;
        if(videochannels.at(id)->fps >= 60){
            pVideoPlayer->frameRateBig = true;
        } else{
            pVideoPlayer->frameRateBig = false;
        }

    }
}

int FFmpeg::getAudioChannels() {
    return audiochannels.size();
}

int FFmpeg::getVideoWidth() {
    if(pVideoPlayer != NULL && pVideoPlayer->pAVCodecContext != NULL){
        return pVideoPlayer->pAVCodecContext->width;
    }
    return 0;
}

int FFmpeg::getVideoHeight() {
    if(pVideoPlayer != NULL && pVideoPlayer->pAVCodecContext != NULL){
        return pVideoPlayer->pAVCodecContext->height;
    }
    return 0;
}
