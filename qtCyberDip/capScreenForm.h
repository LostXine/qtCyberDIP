#ifndef CAPSCREENFORM_H
#define CAPSCREENFORM_H

#include "stdafx.h"
#include "ui_capScreenForm.h"

namespace Ui {
	class capScreenForm;
}

class capScreenForm : public QWidget
{
	Q_OBJECT

public:
	explicit capScreenForm(void* win, QWidget *parent = 0);
	~capScreenForm();

	void capSetHWND(HWND wnd);
	void capSetTitle(QString title);
	void capSetScaleRatio(QString scale);
	void capCapturingThreading();
	void closeEvent(QCloseEvent *evt);
	void capRun();
signals:
	void imgReady(QImage img);
	void capFinished();

private:
	Ui::capScreenForm *ui;
	HWND hWnd;
	bool shouldRun;
	bool mShowFps;
	float ratio = 1.0;
	QTime capFrameTimer;
};

#endif // CAPSCREENFORM_H
