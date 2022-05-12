#include "videoinformation.h"
#include <QDebug>

VideoInformation::VideoInformation()
{

    //获取动态内存
    input_AVFormat_context_ = avformat_alloc_context();
    //input_AVFormat_context_ = NULL;
}

VideoInformation::~VideoInformation()
{
    avformat_close_input(&input_AVFormat_context_);
}

QString VideoInformation::getFormat()
{
    return this->format_;
}

QString VideoInformation::getDuration()
{
    return this->duration_ ;
}

int VideoInformation::getFrameRate()
{
    return this->frame_rate_ ;
}

int VideoInformation::getVideoAverageBitRate()
{
    return this->video_average_bit_rate_ ;
}

int VideoInformation::getWidth()
{
    return this->width_ ;
}

int VideoInformation::getHeight()
{
    return this->height_ ;
}

float VideoInformation::getVideoSize()
{
    return this->video_size_ ;
}

QString VideoInformation::getVideoFormat()
{
    return this->video_format_ ;
}

QString VideoInformation::getAudioFormat()
{
    return this->audio_format_ ;
}

int VideoInformation::getAudioAverageBitRate()
{
    return this->audio_average_bit_rate_ ;
}

int VideoInformation::getSampleRate()
{
    return this->sample_rate_ ;
}

float VideoInformation::getAudioSize()
{
    return this->audio_size_ ;
}

void VideoInformation::getVideoInfo(QString file_path){

    if(avformat_open_input(&input_AVFormat_context_, file_path.toLatin1().data(), 0, NULL) < 0){
        //std::cout<<"file open error!"<<std::endl;
        return;
    }

    if(avformat_find_stream_info(input_AVFormat_context_, NULL) < 0){
        //printf("error");
        return ;
    }

    //得到流数量
    this->stream_numbers_ = input_AVFormat_context_->nb_streams;
    //计算视频长度
    int hours, mins, secs;
    secs = input_AVFormat_context_->duration / 1000000;
    mins = secs / 60;
    secs %= 60;
    hours = mins / 60;
    mins %= 60;

    //格式化视频长度
    char duration_foramt_[128];
    sprintf(duration_foramt_, "%d:%d:%d", hours, mins, secs);
    this->duration_ = duration_foramt_;

    //av_dump_format(input_AVFormat_context_, 0, file_path.c_str(), 0);

    //得到输入视频的封装格式

    //std::cout<<"format: "<<input_AVFormat_context_->streams[0]->codecpar->format<<std::endl;
    //std::cout<<"bit rate: "<<input_AVFormat_context_->bit_rate/1000.0<<std::endl;

    AVInputFormat* infoFormat = input_AVFormat_context_->iformat;
    this->format_ = infoFormat->name;

    //分别遍历视频的流
    for(unsigned int i = 0; i < stream_numbers_; i++){
        //取出一路流,并生成AVStream对象
        AVStream* input_stream = input_AVFormat_context_->streams[i];
         //AVDictionaryEntry *lang = av_dict_get(input_stream->metadata, "language", NULL, 0);
         //std::cout<<"ddd: "<<lang->value<<std::endl;
        //判断是否为视频流
        if(input_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){

            //avg_frame_rate -> AVRational(有理数),
            //avg_frame_rate.num : 分子
            //avg_frame_rate.den : 母
            //得到视频帧率
            this->frame_rate_ = input_stream->avg_frame_rate.num / input_stream->avg_frame_rate.den;

            //取出视频流中的编码参数部分, 生成AVCodecParamters对象
            AVCodecParameters* codec_par = input_stream->codecpar;

            //利用编码参数对象AVCdecParamters得到视频宽度，高度，码率，视频大小
            this->width_ = codec_par->width;
            this->height_ = codec_par->height;
            this->video_average_bit_rate_ = codec_par->bit_rate/1000;
            this->video_size_ = this->video_average_bit_rate_ * secs / (8.0*1024);

            //利用avcodec_paramters_to_context()函数产生AVCodecContext对象
            //input_stream->codec已经被淘汰，不推荐使用这种方式生成AVCodecContext
            AVCodecContext* avctx_video;
            avctx_video = avcodec_alloc_context3(NULL);
            int ret = avcodec_parameters_to_context(avctx_video, codec_par);
            if (ret < 0) {
                avcodec_free_context(&avctx_video);
                return;
             }
            //使用AVCodecContext得到视频编码格式（不推荐）
            char buf[128];
            avcodec_string(buf, sizeof(buf), avctx_video, 0);
            //使用AVCodecParameters得到视频编码方式
            this->video_format_ = avcodec_get_name((codec_par->codec_id));

         //判断是否为音频流
        }else if(input_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){

            //生成AVcodecParamters对象
            AVCodecParameters* codec_par = input_stream->codecpar;
            AVCodecContext* avctx_audio;
            avctx_audio = avcodec_alloc_context3(NULL);
            int ret = avcodec_parameters_to_context(avctx_audio, codec_par);
            if(ret < 0){
                avcodec_free_context(&avctx_audio);
                return;
            }

            this->audio_format_ = avcodec_get_name(avctx_audio->codec_id);

            this->audio_average_bit_rate_ = codec_par->bit_rate / 1000;

            this->channel_nums = codec_par->channels;

            this->sample_rate_ = codec_par->sample_rate;

            this->audio_size_ = this->audio_average_bit_rate_ * secs / (8.0*1024);
        }
    }
}
