#ifndef CAMPLAYER
#define CAMPLAYER

#include "stdafx.h"
#include <QTFFmpegWrapper/ffmpeg.h>

class camPlayer : public QObject
{
	Q_OBJECT;

public:
	camPlayer(QString name);
	~camPlayer();
	void camStop();

private:
	bool mShouldRun;
	QString cam_name;
	ffmpeg::AVFormatContext *pFormatCtx = nullptr;
	ffmpeg::AVCodecContext  *pCodecCtx = nullptr;
	ffmpeg::AVFrame *pFrame = nullptr, *pFrameRGB = nullptr;
	ffmpeg::SwsContext *img_convert_ctx = nullptr;
	ffmpeg::AVCodec *pCodec = nullptr;

signals:
	void imgReady(QImage img);
	void camErrLog(QString err);
	void camFinished();

public slots:
	void camRun();
};
#endif

