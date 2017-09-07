/**
 * Copyright (C) 2013 Guillaume Lesniak
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef BBQSCREENFORM_H
#define BBQSCREENFORM_H

#include "ui_bbqScreenForm.h"
#include "QStreamDecoder.h"
#include <QtNetwork/QTcpSocket>

#define FPS_AVERAGE_SAMPLES 50


#if defined(_WIN32) || defined(_WIN64)
#define PLAT_WINDOWS
#elif defined(__linux) || defined(__unix) || defined(__posix)
#define PLAT_LINUX
#elif defined(__APPLE__)
#define PLAT_APPLE
#else
#warning "Unsupported keyboard platform"
#endif

enum TouchEventType {
	TET_UP,
	TET_DOWN,
	TET_MOVE
};

enum InputEventType {
	IET_KEYBOARD,
	IET_TOUCH
};

namespace Ui {
	class bbqScreenForm;
}


class bbqScreenForm : public QWidget
{
	Q_OBJECT

public:
	explicit bbqScreenForm(void* win, QWidget *parent = 0);
	~bbqScreenForm();

	void connectTo(const QString& host);

	void timerEvent(QTimerEvent *evt);
	void closeEvent(QCloseEvent *evt);
	/*
	void keyPressEvent(QKeyEvent *evt);
	void keyReleaseEvent(QKeyEvent *evt);
	void mousePressEvent(QMouseEvent *evt);
	void mouseReleaseEvent(QMouseEvent *evt);
	void mouseMoveEvent(QMouseEvent *evt);
	*/
	quint8 bytesToUInt8(const QByteArray& bytes, int offset);
	quint16 bytesToUInt16(const QByteArray& bytes, int offset);
	quint32 bytesToUInt32(const QByteArray& bytes, int offset);
	QByteArray numberToBytes(unsigned int value, int size);

	void setQuality(bool high);
	void setShowFps(bool show);
	/*
	void sendKeyboardInput(bool down, unsigned int keyCode);
	void sendTouchInput(TouchEventType type, unsigned char finger, unsigned short x, unsigned short y);
	*/
	QPoint getScreenSpacePoint(int x, int y);

#ifdef __APPLE__
	bool nativeEvent(const QByteArray& eventType, void* message, long* result);
#endif

signals:
	void imgReady(QImage img);
	void bbqFinished();

protected:
	void attemptConnection();

private slots:
	void processPendingDatagrams();
	void onSocketStateChanged();
	void onDecodeFinished(bool result, bool isAudio);

private:
	Ui::bbqScreenForm *ui;
	QTcpSocket mTcpSocket;

	// Decoders
	QStreamDecoder mDecoder;
	QStreamDecoder mAudioDecoder;
	QThread mAudioDecoderThread;
	QThread mVideoDecoderThread;

	// Session settings
	bool mHighQuality;
	bool mIsConnecting;
	bool mShowFps;
	bool mStopped;
	int mRotationAngle;
	QString mHost;

	// Session data
	QByteArray mGlobalBytesBuffer;
	int mVideoFrameSize;
	int mAudioFrameSize;
	int mConnectionAttempts;
	int mConnectionTimerId;

	// Remote frame info
	int mTotalFrameReceived;
	int mRemoteOrientation;
	int mOrientationOffset;
	QPoint mOriginalSize;
	QTime mFrameTimer;
	QImage mLastImage;
	bool mLastImageDisplayed;

	/*
	// Local input info
	bool mIsMouseDown;
	bool mCtrlDown;
	QTime mTimeSinceLastTouchEvent;
	QByteArray mTouchEventPacket;
	*/
};

#endif // SCREENFORM_H
