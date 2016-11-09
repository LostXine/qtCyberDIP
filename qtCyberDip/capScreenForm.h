#ifndef CAPSCREENFORM_H
#define CAPSCREENFORM_H

#include <QWidget>
#include "ui_capScreenForm.h"
#include "qtcyberdip.h"
#include <iostream>
#include <Windows.h>
#include <QTime>
#include <QPainter>
#include <QBuffer>
#include <QLabel>
#include <QCloseEvent>
#include "stdafx.h"

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
	void capStart();

	void closeEvent(QCloseEvent *evt);
signals:
	void imgReady(QImage img);

private:
	Ui::capScreenForm *ui;
	qtCyberDip* mParentWindow;
	HWND hWnd;
	bool shouldRun;
	bool mShowFps;
	QTime capFrameTimer;
};

#endif // CAPSCREENFORM_H
