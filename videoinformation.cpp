#include "videoinformation.h"
#include "globaldata.h"
#include <QDebug>
#include <QImage>
VideoInformation::VideoInformation()
{

    //获取动态内存
    input_AVFormat_context_ = avformat_alloc_context();
    this->flag = false ;
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

QString VideoInformation::getname()
{
    return this->name ;
}

int VideoInformation::getChannelNumbers()
{
    return this->channel_nums ;
}
void VideoInformation::getVideoInfo(QString file_path){
    //qDebug()<<file_path<<endl ;
    avformat_close_input(&input_AVFormat_context_) ;
    input_AVFormat_context_ = avformat_alloc_context();
    if(avformat_open_input(&input_AVFormat_context_, file_path.toStdString().std::string::c_str(), 0, NULL) < 0){
        //std::cout<<"file open error!"<<std::endl;
        return;
    }
    qDebug()<<file_path.toLatin1().data()<<endl ;
    QFileInfo fileInfo(file_path);
    this->name = fileInfo.fileName() ;

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

            this->flag = true ;

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
            //qDebug()<<codec_par->bit_rate<<endl ;
            this->video_size_ = this->video_average_bit_rate_ * secs / (8.0*1024);

            //利用avcodec_paramters_to_context()函数产生AVCodecContext对象
            //input_stream->codec已经被淘汰，不推荐使用这种方式生成AVCodecContext
            AVCodecContext* avctx_video;
            avctx_video = avcodec_alloc_context3(NULL);
            int ret = avcodec_parameters_to_context(avctx_video, codec_par);
            if (ret < 0) {
                return;
             }
            //使用AVCodecContext得到视频编码格式（不推荐）
            char buf[128];
            avcodec_string(buf, sizeof(buf), avctx_video, 0);
            //使用AVCodecParameters得到视频编码方式
            this->video_format_ = avcodec_get_name((codec_par->codec_id));
            avcodec_free_context(&avctx_video);

         //判断是否为音频流
        }else if(input_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){

            //生成AVcodecParamters对象
            AVCodecParameters* codec_par = input_stream->codecpar;
            AVCodecContext* avctx_audio;
            avctx_audio = avcodec_alloc_context3(NULL);
            int ret = avcodec_parameters_to_context(avctx_audio, codec_par);
            if(ret < 0){
                return;
            }

            this->audio_format_ = avcodec_get_name(avctx_audio->codec_id);

            this->audio_average_bit_rate_ = codec_par->bit_rate / 1000;

            this->channel_nums = codec_par->channels;

            this->sample_rate_ = codec_par->sample_rate;

            this->audio_size_ = this->audio_average_bit_rate_ * secs / (8.0*1024);
            avcodec_free_context(&avctx_audio);
        }
    }
}

void VideoInformation::test1()
{

        AVFormatContext* pFormatCtx = input_AVFormat_context_ ;
        int res;
        //res = avformat_open_input(&pFormatCtx, "D:/1.mkv", nullptr, nullptr);
        //if (res) {
        //    return;
        //}
        //avformat_find_stream_info(pFormatCtx, nullptr);
        int videoStream = -1;
        for (int i = 0; i < pFormatCtx->nb_streams; i++) {
            if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStream = i;
                break;
            }
        }
        if (videoStream == -1) {
            return;
        }
        //AVCodecParameters
        AVCodecParameters* pCodecCtxOrig = nullptr;
        // Get a pointer to the codec context for the video stream
        pCodecCtxOrig = pFormatCtx->streams[videoStream]->codecpar;
        AVCodec* pCodec = nullptr;
        // Find the decoder for the video stream
        pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
        if (pCodec == nullptr) {
            fprintf(stderr, "Unsupported codec!\n");
            return; // Codec not found
        }
        AVCodecContext* pCodecCtx = nullptr;
        // Copy context
        pCodecCtx = avcodec_alloc_context3(pCodec);
        //if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
        //    fprintf(stderr, "Couldn't copy codec context");
        //    return 0; // Error copying codec context
        //}

        int ret = avcodec_parameters_to_context(pCodecCtx, pCodecCtxOrig);

        // Open codec
        if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
            return;// Could not open codec
        }
        AVFrame* pFrameRGB = nullptr;
        pFrameRGB = av_frame_alloc();
        res = av_seek_frame(pFormatCtx, -1, 15 * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);//10(second)
        if (res < 0) {
            return;
        }
        AVPacket packet;
        while (1) {
            av_read_frame(pFormatCtx, &packet);
            if (packet.stream_index == videoStream) {
                res = avcodec_send_packet(pCodecCtx, &packet);
                int gotPicture = avcodec_receive_frame(pCodecCtx, pFrameRGB); //gotPicture = 0 success, a frame was returned
                if (gotPicture == 0) {
                    SwsContext* swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
                        SWS_BICUBIC, nullptr, nullptr, nullptr);
                    //AVFrame* frameRGB = av_frame_alloc();
                    //avpicture_alloc((AVPicture*)frameRGB, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
                    //int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
                    //uint8_t* buffer = (uint8_t*)av_malloc(num_bytes * sizeof(uint8_t));
                    //av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
                    //sws_scale(swsContext, pFrameRGB->data, pFrameRGB->linesize, 0, pCodecCtx->height, frameRGB->data, frameRGB->linesize);
                    //saveAsJPEG(pFrameRGB, pCodecCtx->width, pCodecCtx->height, 10);
                    // 创建
                    QImage img (pFrameRGB->width, pFrameRGB->height, QImage::Format_RGB888);
                    uint8_t* dst[] = { img.bits() };
                    int dstStride[4];
                    // AV_PIX_FMT_RGB24 对应于 QImage::Format_RGB888
                    av_image_fill_linesizes(dstStride, AV_PIX_FMT_RGB24, pFrameRGB->width);
                    // 转换
                    sws_scale(swsContext, pFrameRGB->data, (const int*)pFrameRGB->linesize,
                        0, pCodecCtx->height, dst, dstStride);
                    //qDebug()<<img<<endl ;
                    img.save("1.jpg") ;
                    GlobalData::qi = img ;
                    //avformat_close_input(&pFormatCtx);
                    return;
                }
            }
        }
}

bool VideoInformation::r_flag()
{
    return this->flag ;
}
