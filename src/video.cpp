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

//AVFormatContext *pOutFmtCtx = NULL;
//const AVOutputFormat *pOutFmt = NULL;
//AVCodecParameters *pOutCodecPara = NULL;
//AVCodecParameters *pCodecPara = NULL;
//AVCodecContext *pOutCodeCtx = NULL;
//AVCodecContext *pCodeCtx = NULL;
//const AVCodec *pOutCodec = NULL;
//const AVCodec *pCodec = NULL;
//AVFrame *pOutFrame = NULL;
//AVStream *pOutStream = NULL;
int stream_index = 0;
int *stream_mapping = NULL;
int stream_mapping_size = 0;


static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
	//std::string spts, tspts, sdts, tsdts, sdur, tsdur;
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
	//AVStream *in_stream, *out_stream;
	int ret;
	//AVPacket pkt;

	/*
	in_stream = ifmt_ctx->streams[pkt->stream_index];
	if(pkt->stream_index >= stream_mapping_size ||
	   stream_mapping[pkt->stream_index] < 0) {
		   av_packet_unref(pkt);
		   return 0;
	   }
	   */

	av_packet_unref(pkt);
	if(frame)
	{
		//std::cout << "Send frame " << frame->pts << PRId64 << std::endl;
		printf("Send frame %3" PRId64 "\n", frame->pts);
	}

	//avcodec_flush_buffers(enc_ctx);
	ret = avcodec_send_frame(enc_ctx, frame);
	if(ret < 0)
	{
		std::cerr << "Error sending a frame for encoding: " << ret << std::endl;
		exit(1);
	}

	/*
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	*/
	while(ret >= 0)
	{
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if(ret < 0)
		{
			std::cerr << "Error during encoding" << std::endl;
			exit(1);
		}

		printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		//std::cout << "Writing packet " << pkt->pts << PRId64 << "(size=" << pkt->size << ")" << std::endl;
		//destFile << *pkt->data;
		//destFile.write(reinterpret_cast<char *>(pkt->data), pkt->size);
		//pkt->stream_index = stream_mapping[pkt->stream_index];
		pkt->stream_index = st->index;
		av_packet_rescale_ts(pkt, enc_ctx->time_base, st->time_base);
		//out_stream = ofmt_ctx->streams[pkt->stream_index];
		//log_packet(ifmt_ctx, pkt, "in");
		log_packet(ofmt_ctx, pkt, "out");
		if(ret = av_interleaved_write_frame(ofmt_ctx, pkt) < 0)
		{
			std::cerr << "Error muxing packet" << std::endl;
			exit(1);
		}
		//av_packet_unref(pkt);
	}
	return ret == AVERROR_EOF ? 1:0;
}
int wfile_init(char filename[], int height, int width, int bitrate, int fpsrate, const AVCodec *pCodec, AVCodecContext *pCodeCtx)
{
	/*
	pOutFmt = av_guess_format(nullptr, filename, nullptr);
	if(!pOutFmt)
	{
		std::cerr << "can't create output format!" << std::endl;
		return -1;
	}
	if(avformat_alloc_output_context2(&pOutFmtCtx, pOutFmt, nullptr, filename))
	{
		std::cerr << "can't create output context!" << std::endl;
		return -1;
	}
	*/

	//pOutCodec = avcodec_find_encoder(pOutFmt->video_codec);
	/*
	pOutStream = avformat_new_stream(pOutFmtCtx, pOutCodec);
	if(!pOutStream)
	{
		std::cerr << "can't find format!" << std::endl;
		return -1;
	}
	*/
	/*
	pOutStream->codecpar->codec_id = pOutFmt->video_codec;
	pOutStream->codecpar->codec_id = pOutCodeCtx->codec_id;
	pOutStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	pOutStream->codecpar->width = width;
	pOutStream->codecpar->height = height;
	pOutStream->codecpar->format = AV_PIX_FMT_YUV420P;
	pOutStream->codecpar->bit_rate = bitrate*1000;

	avcodec_parameters_to_context(pOutCodeCtx, pOutStream->codecpar);
	pOutCodeCtx->time_base = {1, fpsrate};
	pOutCodeCtx->max_b_frames = 2;
	pOutCodeCtx->gop_size = 12;
	*/
	//if(pOutStream->codecpar->codec_id == AV_CODEC_ID_H264)
	/*
	if(pOutFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
	{
		pOutCodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	avcodec_parameters_from_context(pOutStream->codecpar, pOutCodeCtx);
	*/
	/*
	if(!(pOutFmt->flags & AVFMT_NOFILE))
		if(avio_open(&pOutFmtCtx->pb, filename, AVIO_FLAG_WRITE) < 0)
		{
			std::cerr << "Failed to open file!" << std::endl;
			return -1;
		}
	if(avformat_write_header(pOutFmtCtx, NULL) < 0)
	{
		std::cerr << "Failed to write header!" << std::endl;
		return -1;
	}

	av_dump_format(pOutFmtCtx, 0, filename, 1);
	*/
	return 0;
	/*
	avformat_alloc_output_context2(&pOutFmtCtx, 0, "mp4", filename);
	if(!pOutFmtCtx)
	{
		std::cerr << "Could not open video file!" << std::endl;
		return -1;
	}
	pOutFmt = pOutFmtCtx->oformat;
	if(pOutFmt->video_codec != AV_CODEC_ID_NULL)
	{
		if(!)
	}
	*/
}

int main(int argc, char* argv[])
{
	const AVOutputFormat *pOutFmt = NULL;
	AVFormatContext *pFormatCtx = NULL;
	int i, videostream;
	AVCodecParameters *pCodecPara = NULL;
	AVCodecContext *pCodeCtx = NULL;
	const AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVPacket packet;

	//output part
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	AVCodecContext *pOutCodeCtx = NULL;
	AVFormatContext *pOutFmtCtx = NULL;
	const AVCodec *pOutCodec = NULL;
	AVFrame *pOutFrame = NULL;
	AVPacket *pOutPacket = NULL;
	std::ofstream destFile;
	std::ifstream fkey;
	std::string key;
	AVDictionary *opt = NULL;

	int write=0;
	if(argc >= 3)
		write = 1;

	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
	{
		std::cerr << "Could not open video file!" << std::endl;
		return -1;
	}

	if(avformat_find_stream_info(pFormatCtx, 0) < 0)
	{
		std::cerr << "Failed to retrieve input stream information" << std::endl;
		return -1;
    }
	av_dump_format(pFormatCtx, 0, argv[1], 0);

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

	pOutPacket = av_packet_alloc();
	if(!pOutPacket)
	{
		std::cerr << "failed to alloc packet!" << std::endl;
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
	int size = width/8*height/8/8;
	if(write)
	{
		//pOutCodec = avcodec_find_encoder_by_name(pCodec->name);
		avformat_alloc_output_context2(&pOutFmtCtx, NULL, NULL, argv[3]);
		if(!pOutFmtCtx)
		{
			std::cerr << "Could not create output context" << std::endl;
			return AVERROR_UNKNOWN;
		}

		stream_mapping_size = pFormatCtx->nb_streams;
		stream_mapping = (int*)av_calloc(stream_mapping_size, sizeof(*stream_mapping));
		if(!stream_mapping){
			return AVERROR(ENOMEM);
		}

		pOutFmt = pOutFmtCtx->oformat;

		for(int i=0;i < pFormatCtx->nb_streams; i++) {
			AVStream *out_stream;
			AVStream *in_stream = pFormatCtx->streams[i];
			AVCodecParameters *in_codecpar = in_stream->codecpar;
			if(in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
			   in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
			   in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
				   stream_mapping[i] = -1;
				   continue;
			   }

			stream_mapping[i] = stream_index++;

			out_stream = avformat_new_stream(pOutFmtCtx, NULL);
			if(!out_stream)
			{
				std::cerr << "Failed allocating output stream" << std::endl;
				return AVERROR(AVERROR_UNKNOWN);
			}

			if(avcodec_parameters_copy(out_stream->codecpar, in_codecpar))
			{
				std::cerr << "Failed to copy codec parameters" << std::endl;
				return -1;
			}
			out_stream->codecpar->codec_tag = 0;
			av_dump_format(pOutFmtCtx, 0, argv[3], 1);

			if(!(pOutFmtCtx->flags & AVFMT_NOFILE))
			{
				if(avio_open(&pOutFmtCtx->pb, argv[3], AVIO_FLAG_WRITE))
				{
					std::cerr << "Could not open output file: " << argv[3] << std::endl;
					return -1;
				}
			}

			if(avformat_write_header(pOutFmtCtx, NULL))
			{
				std::cerr << "Error occurred when opening output file" << std::endl;
				return -1;
			}

			pOutCodec = avcodec_find_encoder(pCodec->id);
			if(!pOutCodec)
			{
				std::cerr << "can't create codec!" << std::endl;
				return -1;
			}
			pOutCodeCtx = avcodec_alloc_context3(pOutCodec);
			if(!pOutCodeCtx)
			{
				std::cerr << "can't create codec context!" << std::endl;
				return -1;
			}
			/*
			if(avcodec_parameters_to_context(pOutCodeCtx, pOutFmtCtx->streams[i]->codecpar)<0)
			{
				std::cerr << "can't convert parameters to context!" << std::endl;
				return -1;
			}
			*/
			pOutCodeCtx->width = pCodeCtx->width;
			pOutCodeCtx->height = pCodeCtx->height;
			//pOutCodeCtx->bit_rate = pCodeCtx->bit_rate;
			pOutCodeCtx->sample_aspect_ratio = pCodeCtx->sample_aspect_ratio;
			//pOutCodeCtx->time_base = av_inv_q(pCodeCtx->framerate);
			pOutCodeCtx->time_base = (AVRational){1, pCodeCtx->time_base.den};
			//pOutCodeCtx->framerate = (AVRational){25, 1};
			pOutCodeCtx->gop_size = pCodeCtx->gop_size;
			pOutCodeCtx->max_b_frames = pCodeCtx->max_b_frames;
			//pOutCodeCtx->pix_fmt = pCodeCtx->pix_fmt;
			if(pOutCodec->pix_fmts)
				pOutCodeCtx->pix_fmt = pOutCodec->pix_fmts[0];
			else
				pOutCodeCtx->pix_fmt = pCodeCtx->pix_fmt;
			//pOutCodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
			if(pOutCodec->id == AV_CODEC_ID_H264||pOutCodec->id == AV_CODEC_ID_H265)
			{
				//av_opt_set(pOutCodeCtx, "preset", "veryslow", 0);
				//av_opt_set_int(pOutCodeCtx, "crf", 18, 0);
				av_dict_set(&opt, "preset", "slow", 0);
				av_dict_set(&opt, "crf", "1.0", 0); // crf 0 will cause pixfmt became 4:4:4, so use 1 to ensure yuv420p and get least compress
			}
			//if(avcodec_open2(pOutCodeCtx, pOutCodec, &opt) < 0)
			if(avcodec_open2(pOutCodeCtx, pOutCodec, &opt) < 0)
			{
				std::cerr << "Failed to open codec!" << std::endl;
				return -1;
			}
			if(avcodec_parameters_from_context(out_stream->codecpar, pOutCodeCtx) < 0)
			{
				std::cerr << "Failed to copy encoder parameters to output stream" << std::endl;
				return -1;
			}

			out_stream->time_base = pOutCodeCtx->time_base;
			//wfile_init(argv[2], height, width, pCodeCtx->bit_rate, pCodeCtx->time_base.den, pCodec, pCodeCtx);
			//destFile.open(argv[3], std::ios::binary);
			fkey.open(argv[2]);
			//key((std::istreambuf_iterator<char>(fkey)), std::istreambuf_iterator<char>());
			fkey >> key;
			key = hamming_encode8(key);
			pOutFrame = av_frame_alloc();
			if(!pOutFrame)
			{
				std::cerr << "can't allocate video frame!" << std::endl;
				return -1;
			}
			//pOutFrame->format = pCodeCtx->pix_fmt;
			pOutFrame->format = AV_PIX_FMT_YUV420P;
			pOutFrame->width = width;
			pOutFrame->height = height;
			if(av_frame_get_buffer(pOutFrame, 0) < 0)
			{
				std::cerr << "can't get frame buffer!" << std::endl;
				return -1;
			}
			int cnt = 0;
			while(av_read_frame(pFormatCtx, &packet) >= 0)
			{
				if(packet.stream_index == videostream)
				{
					avcodec_send_packet(pCodeCtx, &packet);
					while(avcodec_receive_frame(pCodeCtx, pFrame) == 0)
					{
						//steg(cv::Mat(pFrame->data[0]), key, keysize);
						//std::cout << width << "x" << height << std::endl;
						if(write)
						{
							cv::Mat tmp;
							int key_end = 0;
							if((cnt+1)*size<=key.size())
								tmp = steg(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pFrame->data[0]), pFrame->width, key.substr(cnt*size, (cnt+1)*size), size);
							else if(cnt*size<=key.size())
								tmp = steg(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pFrame->data[0]), pFrame->width, key.substr(cnt*size, key.size()), key.size()-cnt*size+1);
							else
								key_end = 1;
							fflush(stdout);
							//av_init_packet(&OutPacket);
							//OutPacket.data = NULL;
							//OutPacket.size = 0;
							if(av_frame_make_writable(pOutFrame) < 0)
							{
								std::cerr << "can't make frame writable!" << std::endl;
								return -1;
							}
							/*
							pOutFrame->data[0] = tmp.data;
							pOutFrame->data[1] = pFrame->data[1];
							pOutFrame->data[2] = pFrame->data[2];
							memcpy(pOutFrame->data[0], tmp.data, height*width*sizeof(uchar));
							memcpy(pOutFrame->data[1], pFrame->data[1], height*width*sizeof(uchar)/2);
							memcpy(pOutFrame->data[2], pFrame->data[2], height*width*sizeof(uchar)/2);
							*/
							av_frame_copy(pOutFrame, pFrame);
							if(!key_end)
							{
								//memcpy(pOutFrame->data[0], tmp.data, sizeof(pFrame->data[0]));
								//memcpy(pOutFrame->data[0], tmp.reshape(0,1).data, pFrame->height*pFrame->linesize[0]);
								//memcpy(pOutFrame->data[0], pFrame->data[0], pFrame->height*pFrame->linesize[0]);
								av_image_copy_plane(pOutFrame->data[0], pOutFrame->linesize[0], tmp.data, pFrame->linesize[0], pFrame->width, pFrame->height);
								/*
								for(int i=0;i<pOutFrame->height;i++)
									for(int j=0;j<pOutFrame->linesize[0];j++)
										pOutFrame->data[0][i*pOutFrame->linesize[0]+j] = tmp.reshape(0,1).data[i*pOutFrame->linesize[0]+j];
								*/
								//std::cout << solve(cv::Mat(pOutFrame->height, pOutFrame->width, CV_8U, pOutFrame->data[0])) << std::endl;
								//struct SwsContext *pSwsCtx = sws_getContext(pOutFrame->width, pOutFrame->height, AV_PIX_FMT_YUV420P, pCodeCtx->width, pCodeCtx->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
								//sws_scale(pSwsCtx, pOutFrame->data, pOutFrame->linesize, 0, pCodeCtx->height, pOutFrame->data, pOutFrame->linesize);
								//std::string dbg = solve(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pOutFrame->data[0]), pFrame->width);
								//std::cout << hamming_decode(dbg) << std::endl;
							}
							//memcpy(pOutFrame->data[2], pFrame->data[2], pFrame->height*pFrame->linesize[2]/2);
							/*
							pOutFrame->linesize[0] = pFrame->linesize[0];
							pOutFrame->linesize[1] = pFrame->linesize[1];
							pOutFrame->linesize[2] = pFrame->linesize[2];
							*/
							//pOutFrame->pts = cnt++;
							pOutFrame->pts = pFrame->best_effort_timestamp;
							//pOutFrame->pts = (int)((1/25.)*pCodeCtx->bit_rate*(++cnt));
							encode(pOutCodeCtx, pOutFrame, pOutPacket, out_stream, pOutFmtCtx);
							/*
							AVStream *in_stream, *out_stream;
							int ret;
							//AVPacket pkt;

							in_stream = pFormatCtx->streams[packet.stream_index];
							if (packet.stream_index >= stream_mapping_size ||
								stream_mapping[packet.stream_index] < 0) {
								av_packet_unref(&packet);
								continue;
							}

							if(pOutFrame)
								//std::cout << "Send frame " << frame->pts << PRId64 << std::endl;
								printf("Send frame %3" PRId64 "\n", pOutFrame->pts);

							//avcodec_flush_buffers(enc_ctx);
							ret = avcodec_send_frame(pOutCodeCtx, pOutFrame);
							if(ret < 0)
							{
								std::cerr << "Error sending a frame for encoding: " << ret << std::endl;
								exit(1);
							}

							while(ret >= 0)
							{
								ret = avcodec_receive_packet(pOutCodeCtx, pOutPacket);
								if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
									continue;
								else if(ret < 0)
								{
									std::cerr << "Error during encoding" << std::endl;
									exit(1);
								}

								printf("Write packet %3" PRId64 " (size=%5d)\n", pOutPacket->pts, pOutPacket->size);
								//std::cout << "Writing packet " << pkt->pts << PRId64 << "(size=" << pkt->size << ")" << std::endl;
								//destFile << *pkt->data;
								//destFile.write(reinterpret_cast<char *>(pkt->data), pkt->size);
								pOutPacket->stream_index = stream_mapping[packet.stream_index];
								out_stream = pOutFmtCtx->streams[packet.stream_index];

								log_packet(pFormatCtx, pOutPacket, "in");
								av_packet_rescale_ts(pOutPacket, in_stream->time_base, out_stream->time_base);
								log_packet(pOutFmtCtx, pOutPacket, "out");
								if(av_interleaved_write_frame(pOutFmtCtx, pOutPacket) < 0)
								{
									std::cerr << "Error muxing packet" << std::endl;
									exit(1);
								}
								av_packet_unref(pOutPacket);
							}
								*/
							av_packet_unref(pOutPacket);
						}
						else
						{
							key += solve(cv::Mat(pFrame->height, pFrame->linesize[0], CV_8UC1, pFrame->data[0]), pFrame->width);
						}
						//cnt++;
					}
				}
			}
			if(write)
			{
				encode(pOutCodeCtx, NULL, pOutPacket, out_stream, pOutFmtCtx);
				av_write_trailer(pOutFmtCtx);
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
		}
	}
	else
	{
		key = hamming_decode8(key);
		std::cout << key << std::endl;
	}

	av_packet_unref(&packet);
	return 0;
}
