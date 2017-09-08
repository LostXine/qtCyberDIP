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
	int64_t frame_num = 0;
	ffmpeg::AVFormatContext *pFormatCtx = nullptr;
	ffmpeg::AVCodecContext  *pCodecCtx = nullptr;
	ffmpeg::AVFrame *pFrame = nullptr, *pFrameRGB = nullptr;
	ffmpeg::SwsContext *img_convert_ctx = nullptr;


signals:
	void imgReady(QImage img);
	void vodErrLog(QString err);
	void vodFinished();

public slots:
	void vodRun();
};
#endif