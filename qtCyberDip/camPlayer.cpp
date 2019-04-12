#include "camPlayer.h"

using namespace ffmpeg;

camPlayer::camPlayer(QString name){
	cam_name = "video=" + name;
	av_register_all();
	avdevice_register_all();
}

camPlayer::~camPlayer(){
	if (img_convert_ctx != nullptr){ sws_freeContext(img_convert_ctx); }
	if (pFrameRGB != nullptr){ av_frame_free(&pFrameRGB); }
	if (pFrame != nullptr){ av_frame_free(&pFrame); }
	if (pCodecCtx != nullptr){ avcodec_close(pCodecCtx); }
	if (pFormatCtx != nullptr){ avformat_close_input(&pFormatCtx); }
}

void camPlayer::camRun(){
	mShouldRun = true;
	int	i, videoindex, y_size;
	pFormatCtx = avformat_alloc_context();
	//在windows平台下最好使用dshow方式
	AVInputFormat *ifmt = av_find_input_format("dshow");
	//Set own video device's name
	//摄像头名称
	if (avformat_open_input(&pFormatCtx, cam_name.toLocal8Bit().constData(), ifmt, NULL) != 0)
	{
		emit camErrLog("Couldn't open input stream.");
		emit camFinished();
		return;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
	{
		emit camErrLog("Couldn't find stream information.");
		emit camFinished();
		return;
	}

	videoindex = -1;

	for (i = 0; i<pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
		}
	}

	if (videoindex == -1)
	{
		emit camErrLog("Couldn't find a video stream.");
		emit camFinished();
		return;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if (pCodec == NULL)
	{
		emit camErrLog("Codec not found.");
		emit camFinished();
		return;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
	{
		emit camErrLog("Could not open codec.");
		emit camFinished();
		return;
	}

	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameRGB, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	int ret, got_picture;
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	while (mShouldRun)
	{
		while (1)
		{
			if (av_read_frame(pFormatCtx, packet) < 0){
				mShouldRun = false;
				emit camErrLog("Could not read frame.");
				emit camFinished();
				return;
			}
			
			if (packet->stream_index == videoindex){
				break;
			}
		}

		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (ret < 0){
			emit camErrLog("Decode Error.");
			emit camFinished();
			return;
		}
		if (got_picture){
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameRGB->data, pFrameRGB->linesize);
			y_size = pCodecCtx->width * pCodecCtx->height;

			// Convert the frame to QImage
			QImage mLastFrame(pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888);
			for (int y = 0; y < pCodecCtx->height; y++)
			{
				memcpy(mLastFrame.scanLine(y), pFrameRGB->data[0] + y*pFrameRGB->linesize[0], pCodecCtx->width * 3);
			}
			emit imgReady(mLastFrame);
		}
	}
	emit camFinished();
}

void camPlayer::camStop(){
	mShouldRun = false;
}