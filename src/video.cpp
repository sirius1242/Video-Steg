#include<iostream>

extern "C"{
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libswscale/swscale.h>
}

#include "steg.hpp"

int main(int argc, char* argv[])
{
	AVFormatContext *pFormatCtx = NULL;
	int i, videostream;
	AVCodecParameters *pCodecPara = NULL;
	AVCodecContext *pCodeCtx = NULL;
	const AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVPacket packet;

	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
	{
		std::cerr << "Could not open video file!" << std::endl;
		return -1;
	}

	videostream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if(videostream == -1)
	{
		std::cerr << "Didn't find a video stream in file!" << std::endl;
		return -1;
	}
	pCodecPara = pFormatCtx->streams[videostream]->codecpar;
	pCodec = avcodec_find_decoder(pCodecPara->codec_id);
	if(pCodec == NULL)
	{
		std::cerr << "unsupport codec!" << std::endl;
		return -1;
	}
	pCodeCtx = avcodec_alloc_context3(pCodec);
	if(avcodec_parameters_to_context(pCodeCtx, pCodecPara) != 0)
	{
		std::cerr << "Couldn't copy codec context!" << std::endl;
		return -1;
	}
	if(avcodec_open2(pCodeCtx, pCodec, NULL) < 0)
	{
		std::cerr << "failed to open decoder!" << std::endl;
		return -1;
	}

	pFrame = av_frame_alloc();

	int width = pCodeCtx->width;
	int height = pCodeCtx->height;
	while(av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if(packet.stream_index == videostream)
		{
			avcodec_send_packet(pCodeCtx, &packet);
			while(avcodec_receive_frame(pCodeCtx, pFrame) == 0)
			{
				//steg(cv::Mat(pFrame->data[0]), key, keysize);
				std::cout << width << "x" << height << std::endl;
				steg(cv::Mat(height, width, CV_8U, pFrame->data[0]), "test", 5);
			}
		}
	}

	av_packet_unref(&packet);
	return 0;
}
