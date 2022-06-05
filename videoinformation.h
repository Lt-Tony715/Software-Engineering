#ifndef VIDEOINFORMATION_H
#define VIDEOINFORMATION_H

#include "qimage.h"
#include<QString>
#include<iostream>
#include<QFileInfo>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
}
class VideoInformation
{
public:
    VideoInformation();

    ~VideoInformation();

    //@brief: 得到视频信息
    //@param: file_path: 文件路径
    //@ret  : void
    //@birth: created by LucasNan on 20210218
    void getVideoInfo(QString file_path);

    //@brief: 得到格式
    //@param: void
    //@ret  : 视频格式
    //@birth: created by LucasNan on 20210219
    QString getFormat();

    //@brief: 得到视频长度
    //@param: void
    //@ret  : 视频长度
    //@birth: created by LucasNan on 20210219
    QString getDuration();

    //@brief: 得到视频帧率
    //@param: void
    //@ret  : 视频帧率
    //@birth: created by LucasNan on 20210219

    int getFrameRate();

    //@brief: 得到视频码率
    //@param: void
    //@ret  : 视频平均码率
    //@birth: created by LucasNan on 20210219
    int getVideoAverageBitRate();

    //@brief: 得到视频宽度
    //@param: void
    //@ret  : 视频宽度
    //@birth: created by LucasNan on 20210219
    int getWidth();

    //@brief: 得到视频高度
    //@param: void
    //@ret  : 视频高度
    //@birth: created by LucasNan on 20210219
    int getHeight();

    //@brief: 得到视频流大小
    //@param: void
    //@ret  : 视频流大小
    //@birth: created by LucasNan on 20210219
    float getVideoSize();

    //@brief: 得到视频编码格式
    //@param: void
    //@ret  : 视频编码格式
    //@birth: created by LucasNan on 20210219

    QString getVideoFormat();

    QString getAudioFormat();


    //@brief: 得到音频平均码率
    //@param: void
    //@ret  : 音频平均码率
    //@birth: created by LucasNan on 20210219
    int getAudioAverageBitRate();

    //@brief: 得到音频通道数
    //@param: void
    //@ret  : 音频通道数
    //@birth: created by LucasNan on 20210219
    int getChannelNumbers();


    //@brief: 得到音频采样率
    //@param: void
    //@ret  : 音频采样率
    //@birth: created by LucasNan on 20210219
    int getSampleRate();


    //@brief: 得到音频大小
    //@param: void
    //@ret  : 音频大小
    //@birth: created by LucasNan on 20210219
    float getAudioSize();

    void test1() ;

    bool r_flag() ;

    QString getname() ;

    bool pic_flag();

private:
    AVFormatContext* input_AVFormat_context_;

    //流数
    unsigned int stream_numbers_;

    //视频流索引号
    unsigned int video_stream_index_;
    //音频流索引号
    unsigned int audio_stream_index_;


    //(1) 总体
    QString format_;            //格式
    QString duration_;                //时长
    QString name ;


    //(2) 对于视频
    int frame_rate_;                //帧率

    int video_average_bit_rate_;    //平均码率

    int width_;                     //视频宽度
    int height_;                    //视频高度

    float video_size_;              //视频流大小

    QString video_format_;       //视频编码格式

    //(3) 对于音频
    QString audio_format_;      //音频编码格式

    int audio_average_bit_rate_;    //音频平均码率

    int channel_nums;               //声道数

    int sample_rate_;               //采样率

    float audio_size_;              //音频流大小

    bool flag ;

    bool flagpic ;                  //图片判断

};
#endif // VIDEOINFORMATION_H
