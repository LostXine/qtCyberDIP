#include "vodPlayer.h"
#include <time.h> 

using namespace ffmpeg;

vodPlayer::vodPlayer(QString& path){
	av_register_all();//ע��FFMPEG���
	mPath = path;
}

vodPlayer::~vodPlayer()
{
	if (img_convert_ctx != nullptr){ sws_freeContext(img_convert_ctx); }
	if (pFrameRGB != nullptr){ av_frame_free(&pFrameRGB); }
	if (pFrame != nullptr){ av_frame_free(&pFrame); }
	if (pCodecCtx != nullptr){ avcodec_close(pCodecCtx); }
	if (pFormatCtx != nullptr){ avformat_close_input(&pFormatCtx); }
}

int vodPlayer::setPath(QString& path)
{
	mPath = path;
	return 0;
}

void vodPlayer::vodStop()
{
	mShouldRun = false;
}

void vodPlayer::vodRun()
{
	mShouldRun = true;
	
	uint8_t *out_buffer;
	AVPacket *packet;
	int             i, videoindex;
	int y_size;
	int ret, got_picture;
	int64_t start_time, pause_duration;

	QByteArray ba = mPath.toUtf8();
	pFormatCtx = avformat_alloc_context();//��ʼ��һ��AVFormatContext
	if (avformat_open_input(&pFormatCtx, ba.data(), NULL, NULL) != 0){//���������Ƶ�ļ�
		emit vodErrLog("Couldn't open input file:" + mPath);
		emit vodFinished();
		return;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0){//��ȡ��Ƶ�ļ���Ϣ
		emit vodErrLog("Couldn't find stream information.");
		emit vodFinished();
		return;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex = i;
			break;
		}

	if (videoindex == -1){
		emit vodErrLog("Didn't find a video stream.");
		emit vodFinished();
		return;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);//���ҽ�����
	if (pCodec == NULL){
		QString err("Codec not found:");
		err.append(pCodecCtx->codec_name);
		emit vodErrLog(err);
		emit vodFinished();
		return;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){//�򿪽�����
		QString err("Could not open codec.");
		err.append(pCodecCtx->codec_name);
		emit vodErrLog(err);
		emit vodFinished();
		return;
	}

	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameRGB, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	qDebug("--------------- File Information ----------------");
	av_dump_format(pFormatCtx, 0, ba.data(), 0);
	qDebug("-------------------------------------------------");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

	start_time = GetTickCount();
	pause_duration = 0;
	frame_num = 0;
	while (mShouldRun){//��ȡһ֡ѹ������
		if (mPause)
		{
			int64_t t1 = GetTickCount();
			QThread::msleep(10);
			pause_duration += (GetTickCount() - t1);
			continue;
		}
		if (av_read_frame(pFormatCtx, packet) >= 0)
		{
			if (packet->stream_index == videoindex){
				ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//����һ֡ѹ������
				if (ret < 0){
					emit vodErrLog("Decode Error.");
					emit vodFinished();
					return;
				}
				if (got_picture){
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
						pFrameRGB->data, pFrameRGB->linesize);
					y_size = pCodecCtx->width*pCodecCtx->height;

					// Convert the frame to QImage
					QImage mLastFrame(pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888);
					for (int y = 0; y < pCodecCtx->height; y++)
					{
						memcpy(mLastFrame.scanLine(y), pFrameRGB->data[0] + y*pFrameRGB->linesize[0], pCodecCtx->width * 3);
					}
					frame_num++;
					qDebug("Succeed to decode No.%05d frame!",frame_num);
					AVRational time_base = pFormatCtx->streams[videoindex]->time_base;
					AVRational time_base_q = { 1, AV_TIME_BASE };
					int64_t pts_time = av_rescale_q(packet->dts, time_base, time_base_q);
					int64_t now_time = (GetTickCount() - start_time - pause_duration) * 1000;
					if (pts_time > now_time){ QThread::usleep(pts_time - now_time); }

					emit imgReady(mLastFrame);
					
				}
			}
			ffmpeg::av_packet_unref(packet);
		}
		else
		{
			break;
		}
	}
	//flush decoder
	/*��av_read_frame()ѭ���˳���ʱ��ʵ���Ͻ������п��ܻ�����ʣ��ļ�֡���ݡ�
	�����Ҫͨ����flush_decoder�����⼸֡���������
	��flush_decoder�����ܼ����֮��ֱ�ӵ���avcodec_decode_video2()���AVFrame�������������������AVPacket��*/
	while (mShouldRun) {
		if (mPause)
		{
			int64_t t1 = GetTickCount();
			QThread::msleep(10);
			pause_duration += (GetTickCount() - t1);
			continue;
		}
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (ret < 0)
			break;
		if (!got_picture)
			break;
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
			pFrameRGB->data, pFrameRGB->linesize);
		QImage mLastFrame(pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888);
		for (int y = 0; y < pCodecCtx->height; y++)
		{
			memcpy(mLastFrame.scanLine(y), pFrameRGB->data[0] + y*pFrameRGB->linesize[0], pCodecCtx->width * 3);
		}
		frame_num++;
		qDebug("Flush Decoder: Succeed to decode No.%05d frame!", frame_num);
		AVRational time_base = pFormatCtx->streams[videoindex]->time_base;
		AVRational time_base_q = { 1, AV_TIME_BASE };
		int64_t pts_time = av_rescale_q(packet->dts, time_base, time_base_q);
		int64_t now_time = (GetTickCount() - start_time - pause_duration) * 1000;
		if (pts_time > now_time){ QThread::usleep(pts_time - now_time); }
		emit imgReady(mLastFrame);
	}
	emit vodFinished();
}
