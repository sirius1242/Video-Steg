#include<iostream>
#include<fstream>

extern "C"{
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libswscale/swscale.h>
#include <libavutil/timestamp.h>
#include<libavutil/opt.h>
#include<libavutil/imgutils.h>
}

#include "steg.hpp"

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
	AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
	// std::string spts, tspts, sdts, tsdts, sdur, tsdur;
	char spts[AV_TS_MAX_STRING_SIZE], sdts[AV_TS_MAX_STRING_SIZE], sdur[AV_TS_MAX_STRING_SIZE];
	char tspts[AV_TS_MAX_STRING_SIZE], tsdts[AV_TS_MAX_STRING_SIZE], tsdur[AV_TS_MAX_STRING_SIZE];
	av_ts_make_string(spts, pkt->pts);
	av_ts_make_time_string(tspts, pkt->pts, time_base);
	av_ts_make_string(sdts, pkt->dts);
	av_ts_make_time_string(tsdts, pkt->dts, time_base);
	av_ts_make_string(sdur, pkt->duration);
	av_ts_make_time_string(tsdur, pkt->duration, time_base);

	printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		   tag, spts, tspts, sdts, tsdts, sdur, tsdur, pkt->stream_index);
}

int encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, AVStream *st, AVFormatContext *ofmt_ctx)
{
	int ret;

	av_packet_unref(pkt);
	if (frame)
	{
		printf("Send frame %3" PRId64 "\n", frame->pts);
	}

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0)
	{
		std::cerr << "Error sending a frame for encoding: " << ret << std::endl;
		exit(1);
	}

	while (ret >= 0)
	{
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0)
		{
			std::cerr << "Error during encoding" << std::endl;
			exit(1);
		}

		printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		pkt->stream_index = st->index;
		av_packet_rescale_ts(pkt, enc_ctx->time_base, st->time_base);
		log_packet(ofmt_ctx, pkt, "out");
		if (ret = av_interleaved_write_frame(ofmt_ctx, pkt) < 0)
		{
			std::cerr << "Error muxing packet" << std::endl;
			exit(1);
		}
	}
	return ret == AVERROR_EOF ? 1 : 0;
}

int main(int argc, char *argv[])
{
	AVFormatContext *pFormatCtx = NULL;
	int i, videostream;
	AVCodecParameters *pCodecPara = NULL;
	AVCodecContext *pCodeCtx = NULL;
	const AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVPacket packet;

	// output part
	uint8_t endcode[] = {0, 0, 1, 0xb7};
	AVCodecContext *pOutCodeCtx = NULL;
	AVFormatContext *pOutFmtCtx = NULL;
	const AVCodec *pOutCodec = NULL;
	AVFrame *pOutFrame = NULL;
	AVPacket *pOutPacket = NULL;
	std::ofstream destFile;
	std::ifstream fkey;
	std::string key;
	AVDictionary *opt = NULL;

	int write = 0;
	if (argc >= 3)
		write = 1;

	if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
	{
		std::cerr << "Could not open video file!" << std::endl;
		return -1;
	}

	if (avformat_find_stream_info(pFormatCtx, 0) < 0)
	{
		std::cerr << "Failed to retrieve input stream information" << std::endl;
		return -1;
	}
	av_dump_format(pFormatCtx, 0, argv[1], 0);

	videostream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (videostream == -1)
	{
		std::cerr << "Didn't find a video stream in file!" << std::endl;
		return -1;
	}
	pCodecPara = pFormatCtx->streams[videostream]->codecpar;
	pCodec = avcodec_find_decoder(pCodecPara->codec_id);
	if (pCodec == NULL)
	{
		std::cerr << "unsupport codec!" << std::endl;
		return -1;
	}
	pCodeCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_parameters_to_context(pCodeCtx, pCodecPara) != 0)
	{
		std::cerr << "Couldn't copy codec context!" << std::endl;
		return -1;
	}

	pOutPacket = av_packet_alloc();
	if (!pOutPacket)
	{
		std::cerr << "failed to alloc packet!" << std::endl;
		return -1;
	}

	if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0)
	{
		std::cerr << "failed to open decoder!" << std::endl;
		return -1;
	}

	pFrame = av_frame_alloc();

	int width = pCodeCtx->width;
	int height = pCodeCtx->height;
	int size = width / 8 * height / 8 / 8;
	if (write)
	{
		avformat_alloc_output_context2(&pOutFmtCtx, NULL, NULL, argv[3]);
		if (!pOutFmtCtx)
		{
			std::cerr << "Could not create output context" << std::endl;
			return AVERROR_UNKNOWN;
		}

		for (int i = 0; i < pFormatCtx->nb_streams; i++)
		{
			AVStream *out_stream;
			AVStream *in_stream = pFormatCtx->streams[i];
			AVCodecParameters *in_codecpar = in_stream->codecpar;
			if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
				in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
				in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
			{
				continue;
			}

			out_stream = avformat_new_stream(pOutFmtCtx, NULL);
			if (!out_stream)
			{
				std::cerr << "Failed allocating output stream" << std::endl;
				return AVERROR(AVERROR_UNKNOWN);
			}

			if (avcodec_parameters_copy(out_stream->codecpar, in_codecpar))
			{
				std::cerr << "Failed to copy codec parameters" << std::endl;
				return -1;
			}
			out_stream->codecpar->codec_tag = 0;

			pOutCodec = avcodec_find_encoder(pCodec->id);
			if (!pOutCodec)
			{
				std::cerr << "can't create codec!" << std::endl;
				return -1;
			}
			pOutCodeCtx = avcodec_alloc_context3(pOutCodec);
			if (!pOutCodeCtx)
			{
				std::cerr << "can't create codec context!" << std::endl;
				return -1;
			}
			pOutCodeCtx->width = pCodeCtx->width;
			pOutCodeCtx->height = pCodeCtx->height;
			pOutCodeCtx->sample_aspect_ratio = pCodeCtx->sample_aspect_ratio;
			pOutCodeCtx->time_base = in_stream->time_base;
			pOutCodeCtx->gop_size = pCodeCtx->gop_size;
			pOutCodeCtx->max_b_frames = pCodeCtx->max_b_frames;
			if (pOutCodec->pix_fmts)
				pOutCodeCtx->pix_fmt = pOutCodec->pix_fmts[0];
			else
				pOutCodeCtx->pix_fmt = pCodeCtx->pix_fmt;
			if (pOutCodec->id == AV_CODEC_ID_H264 || pOutCodec->id == AV_CODEC_ID_H265)
			{
				av_dict_set(&opt, "preset", "slow", 0);
				av_dict_set(&opt, "crf", "1.0", 0); // crf 0 will cause pixfmt became 4:4:4, so use 1 to ensure yuv420p and get least compress
			}
			if (avcodec_open2(pOutCodeCtx, pOutCodec, &opt) < 0)
			{
				std::cerr << "Failed to open codec!" << std::endl;
				return -1;
			}
			if (avcodec_parameters_from_context(out_stream->codecpar, pOutCodeCtx) < 0)
			{
				std::cerr << "Failed to copy encoder parameters to output stream" << std::endl;
				return -1;
			}

			out_stream->time_base = pOutCodeCtx->time_base;
		}
		av_dump_format(pOutFmtCtx, 0, argv[3], 1);
		if (!(pOutFmtCtx->oformat->flags & AVFMT_NOFILE))
		{
			if (avio_open(&pOutFmtCtx->pb, argv[3], AVIO_FLAG_WRITE))
			{
				std::cerr << "Could not open output file" << argv[3] << std::endl;
			}
		}
		if (avformat_write_header(pOutFmtCtx, NULL) < 0)
		{
			std::cerr << "Error occurred when opening output file" << std::endl;
			return -1;
		}
		fkey.open(argv[2]);
		fkey >> key;
		key = hamming_encode8(key);
		pOutFrame = av_frame_alloc();
		if (!pOutFrame)
		{
			std::cerr << "can't allocate video frame!" << std::endl;
			return -1;
		}
		pOutFrame->format = pOutCodeCtx->pix_fmt;
		pOutFrame->width = width;
		pOutFrame->height = height;
		if (av_frame_get_buffer(pOutFrame, 0) < 0)
		{
			std::cerr << "can't get frame buffer!" << std::endl;
			return -1;
		}
	}
	int cnt = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if (packet.stream_index == videostream)
		{
			// av_packet_rescale_ts(&packet, pFormatCtx->streams[packet.stream_index]->time_base, in_stream->time_base);
			avcodec_send_packet(pCodeCtx, &packet);
			while (avcodec_receive_frame(pCodeCtx, pFrame) >= 0)
			{
				if (write)
				{
					cv::Mat tmp;
					int key_end = 0;
					if ((cnt + 1) * size <= key.size())
						tmp = steg(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pFrame->data[0]), pFrame->width, key.substr(cnt * size, (cnt + 1) * size), size);
					else if (cnt * size <= key.size())
						tmp = steg(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pFrame->data[0]), pFrame->width, key.substr(cnt * size, key.size()), key.size() - cnt * size + 1);
					else
						key_end = 1;
					fflush(stdout);
					if (av_frame_make_writable(pOutFrame) < 0)
					{
						std::cerr << "can't make frame writable!" << std::endl;
						return -1;
					}
					av_frame_copy(pOutFrame, pFrame);
					if (!key_end)
						av_image_copy_plane(pOutFrame->data[0], pOutFrame->linesize[0], tmp.data, pFrame->linesize[0], pFrame->width, pFrame->height);
					pOutFrame->pts = pFrame->best_effort_timestamp;
					encode(pOutCodeCtx, pOutFrame, pOutPacket, pOutFmtCtx->streams[packet.stream_index], pOutFmtCtx);
					// av_packet_unref(pOutPacket);
				}
				else
					key += solve(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pFrame->data[0]), pFrame->width);
				cnt++;
			}
		}
		/*
		else
		{
			av_packet_rescale_ts(&packet, pFormatCtx->streams[packet.stream_index]->time_base, pOutFmtCtx->streams[packet.stream_index]->time_base);
			if(av_interleaved_write_frame(pOutFmtCtx, &packet) < 0)
			{
				std::cerr << "Error muxing packet" << std::endl;
				return -1;
			}
		}
		*/
	}
	if (write)
	{
		encode(pOutCodeCtx, NULL, pOutPacket, pOutFmtCtx->streams[packet.stream_index], pOutFmtCtx); // flush encoder
		av_write_trailer(pOutFmtCtx);
		av_packet_free(&pOutPacket);
		if (pOutFmtCtx && !(pOutFmtCtx->flags & AVFMT_NOFILE))
			avio_closep(&pOutFmtCtx->pb);
		avcodec_free_context(&pOutCodeCtx);
		avformat_free_context(pOutFmtCtx);
		av_frame_free(&pOutFrame);
	}
	else
	{
		key = hamming_decode8(key);
		std::cout << key << std::endl;
	}
	/*
	if(write)
	{
		//encode(pOutCodeCtx, NULL, pOutPacket, pFormatCtx, pOutFmtCtx);
		if(pCodec->id == AV_CODEC_ID_MPEG1VIDEO||pCodec->id == AV_CODEC_ID_MPEG2VIDEO)
			destFile << endcode;
		destFile.close();
		avcodec_free_context(&pOutCodeCtx);
		//av_frame_free(&pOutFrame);
		av_packet_free(&pOutPacket);
	}
	*/

	av_packet_unref(&packet);
	avformat_close_input(&pFormatCtx);
	return 0;
}
