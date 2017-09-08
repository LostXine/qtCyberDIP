#ifndef VODPLAYER
#define VODPLAYER

#include "stdafx.h"
#include <QTFFmpegWrapper/ffmpeg.h>

class vodPlayer : public QObject
{
	Q_OBJECT;

public:
	vodPlayer();
	~vodPlayer();
	int setPath(QString& path);
	void vodStop();
	void vodSwitchPause(){ mPause = !mPause; }
	bool vodGetPause(){ return mPause; }

private:
	QString mPath;
	bool mShouldRun = false;
	bool mPause = false;
	ffmpeg::AVFormatContext *pFormatCtx = NULL;
	ffmpeg::AVCodecContext  *pCodecCtx = NULL;
	ffmpeg::AVFrame *pFrame = NULL, *pFrameRGB = NULL;
	ffmpeg::SwsContext *img_convert_ctx = NULL;


signals:
	void imgReady(QImage img);
	void vodFinished();

public slots:
	void vodRun();
};
#endif