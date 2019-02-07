/**
 * ��򵥵Ļ���FFmpeg����Ƶ������
 * Simplest FFmpeg Decoder
 *
 * ������ Lei Xiaohua
 * leixiaohua1020@126.com
 * �й���ý��ѧ/���ֵ��Ӽ���
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * ������ʵ������Ƶ�ļ�����ΪYUV���ݡ���ʹ����libavcodec��
 * libavformat������򵥵�FFmpeg��Ƶ���뷽��Ľ̡̳�
 * ͨ��ѧϰ�����ӿ����˽�FFmpeg�Ľ������̡�
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
			//clw note���жϵ�������Ƶ������Ƶ
		{
			videoindex = i; //��ȡ��Ƶ��������ŵ�ֵ��һ��ȡֵ��0
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
	/* clw note����ӡһЩ������Ϣ���鿴�ṹ���ڲ�����
	/*           ��Ҫ���������Ƶ��Ϣ
	/******************************************************/
	printf("clw����Ƶʱ��(us) = %d\n", pFormatCtx->duration);
	printf("clw����װ��ʽ = %s\n", pFormatCtx->iformat->name);
	printf("clw����Ƶ��� = %d * %d\n", pFormatCtx->streams[videoindex]->codec->width, pFormatCtx->streams[videoindex]->codec->height);
	//����ѡ���ӡ���ļ�������
	//FILE *fp = fopen("info.txt", "wb+");
	//fprintf(fp, "clw����Ƶʱ��(us) = %d\n", pFormatCtx->duration);
	//fprintf(fp, "clw����װ��ʽ = %s\n", pFormatCtx->iformat->name);
	//fprintf(fp, "clw����Ƶ��� = %d * %d\n", pFormatCtx->streams[videoindex]->codec->width, pFormatCtx->streams[videoindex]->codec->height);
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


	FILE *fp_264 = fopen("test264.h264", "wb+"); //clw note�����H.264��ʽ�ļ�
	FILE *fp_yuv = fopen("output.yuv", "wb+");  //clw note�����YUV��ʽ�ļ�

	//clw note�������ѭ���������ж�ȡһ֡H.264��ѹ�����ݣ�����packet
	int frame_cnt = 0;
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoindex)  //clw note���ж�����Ƶ��
		{
			//clw note��fprintf�ǰѴ���ʽ�����д���ļ���
			//�����ֱ�Ӱ��ڴ��е�����д���ļ�����fwrite��
			//�˴����H.264��������packet->data����д�뵽һ���ļ��С�
			fwrite(packet->data, 1, packet->size, fp_264);

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); //clw note����AVPacket�������ݽ����AVFrame��������
			if (ret < 0)
			{
				printf("Decode Error.\n");
				return -1;
			}
			if (got_picture)
			{
				//clw note�������������Ŀ�ȿ��ܱ�ʵ�ʿ��Ҫ���ͻ����һ���ڱߣ������õ�����ĺ������Ѷ���ĺڱ߲õ���
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);

				/***********************************************************/
				/* clw note:������ĳɴ���SDL��ʾ����Ļ����ʵ����һ�������� */
				/***********************************************************/
				y_size = pCodecCtx->width * pCodecCtx->height;
				fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
				fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
				fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
				//clw note:�˴����YUV�������ݵ�һ���ļ��У�
				//U��V�Ŀ�͸߶�ֻ��Y���ݵ�1/2�����sizeֻ��1/4
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

