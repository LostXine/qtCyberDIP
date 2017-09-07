#ifndef CAPSCREENFORM_H
#define CAPSCREENFORM_H

#include "stdafx.h"
#include "ui_capScreenForm.h"
#include "qtcyberdip.h"
#include <iostream>
#include <QTime>
#include <QPainter>
#include <QBuffer>
#include <QLabel>
#include <QCloseEvent>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#define PLAT_WINDOWS
#elif defined(__linux) || defined(__unix) || defined(__posix)
#define PLAT_LINUX
#elif defined(__APPLE__)
#define PLAT_APPLE
#else
#warning "Unsupported keyboard platform"
#endif

namespace Ui {
	class capScreenForm;
}

class qtCyberDip;

class capScreenForm : public QWidget
{
	Q_OBJECT

public:
	explicit capScreenForm(qtCyberDip* win, QWidget *parent = 0);
	~capScreenForm();

	void capSetHWND(HWND wnd);
	void capSetTitle(QString title);
	void capCapturingThreading();
	void closeEvent(QCloseEvent *evt);
signals:
	void imgReady(QImage img);
public slots:
	void capRun();

private:
	Ui::capScreenForm *ui;
	qtCyberDip* mParentWindow;
	HWND hWnd;
	bool shouldRun;
	bool mShowFps;
	QTime capFrameTimer;
	QThread capThread;
};

#endif // CAPSCREENFORM_H
