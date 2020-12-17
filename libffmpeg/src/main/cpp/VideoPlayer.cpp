

#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(JavaCall *javaCall, AudioPlayer *audio, PlayStatus *playStatus) {
    streamIndex = -1;
    clock = 0;
    pJavaCall = javaCall;
    pAudioPlayer = audio;
    queue = new QQueue(playStatus);
    pPlayStatus = playStatus;
}

void VideoPlayer::release() {
    LOGE("开始释放audio ...");

    if (pPlayStatus != NULL) {
        pPlayStatus->exit = true;
    }
    if (queue != NULL) {
        queue->noticeThread();
    }
    int count = 0;
    while (!isExit || !isExit2) {
        LOGE("等待渲染线程结束...%d", count);

        if (count > 1000) {
            isExit = true;
            isExit2 = true;
        }
        count++;
        av_usleep(1000 * 10);
    }
    if (queue != NULL) {
        queue->release();
        delete (queue);
        queue = NULL;
    }
    if (pJavaCall != NULL) {
        pJavaCall = NULL;
    }
    if (pAudioPlayer != NULL) {
        pAudioPlayer = NULL;
    }
    if (pAVCodecContext != NULL) {
        avcodec_close(pAVCodecContext);
        avcodec_free_context(&pAVCodecContext);
        pAVCodecContext = NULL;
    }
    if (pPlayStatus != NULL) {
        pPlayStatus = NULL;
    }
}

void *decodVideoT(void *data) {
    VideoPlayer *videoPlayer = (VideoPlayer *) data;
    videoPlayer->decodVideo();
    pthread_exit(&videoPlayer->mVideoThread);

}

void *codecFrame(void *data) {
    VideoPlayer *videoPlayer = (VideoPlayer *) data;

    while (!videoPlayer->pPlayStatus->exit) {
        if (videoPlayer->pPlayStatus->seek) {
            continue;
        }
        videoPlayer->isExit2 = false;
        if (videoPlayer->queue->getAvFrameSize() > 20) {
            continue;
        }
        if (videoPlayer->codecType == DECODE_HARDWARE) {
            if (videoPlayer->queue->getAvPacketSize() == 0)//加载
            {
                if (!videoPlayer->pPlayStatus->load) {
                    videoPlayer->pJavaCall->onLoad(THREAD_CHILD, true);
                    videoPlayer->pPlayStatus->load = true;
                }
                continue;
            } else {
                if (videoPlayer->pPlayStatus->load) {
                    videoPlayer->pJavaCall->onLoad(THREAD_CHILD, false);
                    videoPlayer->pPlayStatus->load = false;
                }
            }
        }
        AVPacket *packet = av_packet_alloc();
        if (videoPlayer->queue->getAvpacket(packet) != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }

        int ret = avcodec_send_packet(videoPlayer->pAVCodecContext, packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(videoPlayer->pAVCodecContext, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            av_frame_free(&frame);
            av_free(frame);
            frame = NULL;
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }
        videoPlayer->queue->putAvframe(frame);
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    videoPlayer->isExit2 = true;
    pthread_exit(&videoPlayer->mDecFrame);
}


void VideoPlayer::playVideo(int type) {
    codecType = type;
    if (codecType == DECODE_SOFTWARE) {
        pthread_create(&mDecFrame, NULL, codecFrame, this);
    }
    pthread_create(&mVideoThread, NULL, decodVideoT, this);

}

void VideoPlayer::decodVideo() {
    while (!pPlayStatus->exit) {
        isExit = false;
        //暂停
        if (pPlayStatus->pause) {
            continue;
        }
        if (pPlayStatus->seek) {
            pJavaCall->onLoad(THREAD_CHILD, true);
            pPlayStatus->load = true;
            continue;
        }
        //加载
        if (queue->getAvPacketSize() == 0) {
            if (!pPlayStatus->load) {
                pJavaCall->onLoad(THREAD_CHILD, true);
                pPlayStatus->load = true;
            }
            continue;
        } else {
            if (pPlayStatus->load) {
                pJavaCall->onLoad(THREAD_CHILD, false);
                pPlayStatus->load = false;
            }
        }
        if (codecType == DECODE_HARDWARE) {
            AVPacket *packet = av_packet_alloc();
            if (queue->getAvpacket(packet) != 0) {
                av_free(packet->data);
                av_free(packet->buf);
                av_free(packet->side_data);
                packet = NULL;
                continue;
            }
            double time = packet->pts * av_q2d(time_base);

            LOGE("video clock is %f", time);
            LOGE("audio clock is %f", pAudioPlayer->clock);
            if (time < 0) {
                time = packet->dts * av_q2d(time_base);
            }

            if (time < clock) {
                time = clock;
            }
            clock = time;
            double diff = 0;
            if (pAudioPlayer != NULL) {
                diff = pAudioPlayer->clock - clock;
            }
            playCount++;
            if (playCount > 500) {
                playCount = 0;
            }
            if (diff >= 0.5) {
                if (frameRateBig) {
                    if (playCount % 3 == 0 && packet->flags != AV_PKT_FLAG_KEY) {
                        av_free(packet->data);
                        av_free(packet->buf);
                        av_free(packet->side_data);
                        packet = NULL;
                        continue;
                    }
                } else {
                    av_free(packet->data);
                    av_free(packet->buf);
                    av_free(packet->side_data);
                    packet = NULL;
                    continue;
                }
            }

            delayTime = getDelayTime(diff);
            LOGE("delay time %f diff is %f", delayTime, diff);

            av_usleep(delayTime * 1000);
            pJavaCall->onVideoInfo(THREAD_CHILD, clock, duration);
            pJavaCall->onDecMediacodec(THREAD_CHILD, packet->size, packet->data, 0);
            av_free(packet->data);
            av_free(packet->buf);
            av_free(packet->side_data);
            packet = NULL;
        } else if (codecType == DECODE_SOFTWARE) {
            AVFrame *frame = av_frame_alloc();
            if (queue->getAvframe(frame) != 0) {
                av_frame_free(&frame);
                av_free(frame);
                frame = NULL;
                continue;
            }
            if ((framePts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE) {
                framePts = 0;
            }
            framePts *= av_q2d(time_base);
            clock = synchronize(frame, framePts);
            double diff = 0;
            if (pAudioPlayer != NULL) {
                diff = pAudioPlayer->clock - clock;
            }
            delayTime = getDelayTime(diff);
            LOGE("delay time %f diff is %f", delayTime, diff);
//            if(diff >= 0.8)
//            {
//                av_frame_free(&frame);
//                av_free(frame);
//                frame = NULL;
//                continue;
//            }

            playCount++;
            if (playCount > 500) {
                playCount = 0;
            }
            if (diff >= 0.5) {
                if (frameRateBig) {
                    if (playCount % 3 == 0) {
                        av_frame_free(&frame);
                        av_free(frame);
                        frame = NULL;
                        queue->clearToKeyFrame();
                        continue;
                    }
                } else {
                    av_frame_free(&frame);
                    av_free(frame);
                    frame = NULL;
                    queue->clearToKeyFrame();
                    continue;
                }
            }

            av_usleep(delayTime * 1000);
            pJavaCall->onVideoInfo(THREAD_CHILD, clock, duration);
            pJavaCall->onGlRenderYuv(THREAD_CHILD, frame->linesize[0], frame->height, frame->data[0], frame->data[1],
                                     frame->data[2]);
            av_frame_free(&frame);
            av_free(frame);
            frame = NULL;
        }
    }
    isExit = true;

}

VideoPlayer::~VideoPlayer() {
    LOGE("video 释放完");
}

double VideoPlayer::synchronize(AVFrame *srcFrame, double pts) {
    double frame_delay;

    if (pts != 0)
        video_clock = pts; // Get pts,then set video clock to it
    else
        pts = video_clock; // Don't get pts,set it to video clock

    frame_delay = av_q2d(time_base);
    frame_delay += srcFrame->repeat_pict * (frame_delay * 0.5);

    video_clock += frame_delay;

    return pts;
}

double VideoPlayer::getDelayTime(double diff) {

    LOGD("audio video diff is %f", diff);

    if (diff > 0.003) {
        delayTime = delayTime / 3 * 2;
        if (delayTime < rate / 2) {
            delayTime = rate / 3 * 2;
        } else if (delayTime > rate * 2) {
            delayTime = rate * 2;
        }

    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < rate / 2) {
            delayTime = rate / 3 * 2;
        } else if (delayTime > rate * 2) {
            delayTime = rate * 2;
        }
    } else if (diff == 0) {
        delayTime = rate;
    }
    if (diff > 1.0) {
        delayTime = 0;
    }
    if (diff < -1.0) {
        delayTime = rate * 2;
    }
    if (fabs(diff) > 10) {
        delayTime = rate;
    }
    return delayTime;
}

void VideoPlayer::setClock(int secds) {
    clock = secds;
}



