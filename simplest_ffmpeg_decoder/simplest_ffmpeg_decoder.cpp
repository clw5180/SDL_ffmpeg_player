/**
 * 最简单的基于FFmpeg的视频解码器
 * Simplest FFmpeg Decoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序实现了视频文件解码为YUV数据。它使用了libavcodec和
 * libavformat。是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest decoder based on FFmpeg.
 * It decodes video to YUV pixel data.
 * It uses libavcodec and libavformat.
 * Suitable for beginner of FFmpeg.
 *
 */


#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
 //Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};
#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif
#endif

int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame, *pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;

	//char filepath[]="1.mp4";
	char filepath[] = "Titanic.mkv";

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}

	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			//clw note：判断到底是音频还是视频
		{
			videoindex = i; //获取视频流所在序号的值；一般取值是0
			break;
		}
	}

	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) 
	{
		printf("Could not open codec.\n");
		return -1;
	}

	/******************************************************/
	/* clw note：打印一些调试信息，查看结构体内部变量
	/*           主要用于输出视频信息
	/******************************************************/
	printf("clw：视频时长(us) = %d\n", pFormatCtx->duration);
	printf("clw：封装格式 = %s\n", pFormatCtx->iformat->name);
	printf("clw：视频宽高 = %d * %d\n", pFormatCtx->streams[videoindex]->codec->width, pFormatCtx->streams[videoindex]->codec->height);
	//可以选择打印到文件，如下
	//FILE *fp = fopen("info.txt", "wb+");
	//fprintf(fp, "clw：视频时长(us) = %d\n", pFormatCtx->duration);
	//fprintf(fp, "clw：封装格式 = %s\n", pFormatCtx->iformat->name);
	//fprintf(fp, "clw：视频宽高 = %d * %d\n", pFormatCtx->streams[videoindex]->codec->width, pFormatCtx->streams[videoindex]->codec->height);
	//fclose(fp);

	/******************************************************/

	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


	FILE *fp_264 = fopen("test264.h264", "wb+"); //clw note：输出H.264格式文件
	FILE *fp_yuv = fopen("output.yuv", "wb+");  //clw note：输出YUV格式文件

	//clw note：解码的循环从码流中读取一帧H.264的压缩数据，存入packet
	int frame_cnt = 0;
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoindex)  //clw note：判断是视频流
		{
			//clw note：fprintf是把带格式的输出写到文件，
			//如果是直接把内存中的数据写成文件，用fwrite；
			//此处输出H.264码流（即packet->data），写入到一个文件中。
			fwrite(packet->data, 1, packet->size, fp_264);

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //clw note：将AVPacket类型数据解码成AVFrame类型数据
			if (ret < 0)
			{
				printf("Decode Error.\n");
				return -1;
			}
			if (got_picture)
			{
				//clw note：比如解码出来的宽度可能比实际宽度要宽，就会存在一个黑边；所以用到下面的函数，把多余的黑边裁掉。
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);

				/***********************************************************/
				/* clw note:把下面改成传给SDL显示在屏幕，就实现了一个播放器 */
				/***********************************************************/
				y_size = pCodecCtx->width * pCodecCtx->height;
				fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
				fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
				fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
				//clw note:此处输出YUV类型数据到一个文件中；
				//U和V的宽和高都只有Y数据的1/2，因此size只有1/4
				/***********************************************************/

				//printf("Succeed to decode 1 frame!\n");
				printf("Decode frame index: %d\n", frame_cnt);
				frame_cnt++;
			}
		}
		av_free_packet(packet);
	}


	//flush decoder
	//FIX: Flush Frames remained in Codec
	//while (1) 
	//{
	//	ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
	//	if (ret < 0)
	//		break;
	//	if (!got_picture)
	//		break;
	//	sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
	//		pFrameYUV->data, pFrameYUV->linesize);

	//	int y_size=pCodecCtx->width*pCodecCtx->height;  
	//	fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
	//	fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
	//	fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V

	//	printf("Flush Decoder: Succeed to decode 1 frame!\n");
	//}

	fclose(fp_264);
	fclose(fp_yuv);

	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

