#include "capScreenForm.h"
#include <Windows.h>

Q_GUI_EXPORT QImage qt_imageFromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h);

capScreenForm::capScreenForm(void* win, QWidget *parent)
	: QWidget(parent), shouldRun(false)
{
	ui = new Ui::capScreenForm;
	ui->setupUi(this);
#ifdef VIA_OPENCV
	resize(200, 95);
#endif
}

capScreenForm::~capScreenForm()
{
	if (ui)
		delete ui;

	ui = nullptr;
}


void capScreenForm::closeEvent(QCloseEvent *evt)
{
	shouldRun = false;
	QWidget::closeEvent(evt);
	emit capFinished();
}

void capScreenForm::capSetHWND(HWND wnd)
{
	hWnd = wnd;
	this->setWindowTitle("Capture 0x" + QString::number((uint)hWnd, 16));
}

void capScreenForm::capRun()
{
	shouldRun = true;
	while (true)
	{
		if (!ui || !shouldRun){ qDebug() << QString::number((uint)hWnd, 16) + " is not visible. "; break; }
		::HDC hdc = ::GetWindowDC(hWnd);
		::LPRECT wRect = new ::RECT();
		if (!::GetWindowRect(hWnd, wRect)){ break; }
		qDebug() << "Windows Info:\n Left:" << wRect->left << " Right:" << wRect->right << " Top:" << wRect->top << " Bottom" << wRect->bottom;
		int width = wRect->right - wRect->left;
		int height = wRect->bottom - wRect->top;
		::HDC hdcDst = ::CreateCompatibleDC(hdc);
		::HBITMAP bmpDst = ::CreateCompatibleBitmap(hdc, width, height);
		::HGDIOBJ bmpHDst = ::SelectObject(hdcDst, bmpDst);
		bool isSame = true, isAlive = true;
		while (!(!ui || !shouldRun) && isSame && isAlive)
		{
			isAlive = ::GetWindowRect(hWnd, wRect);
			int nW = wRect->right - wRect->left;
			int nH = wRect->bottom - wRect->top;
			isSame = (nW == width) && (nH == height);
			BitBlt(hdcDst, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
			QImage img = qt_imageFromWinHBITMAP(hdcDst, bmpDst, width, height);
#ifndef VIA_OPENCV
			ui->capDisplay->setImage(img);
#endif
			qApp->processEvents();
			emit imgReady(img);
		}
		::DeleteDC(hdcDst);
		::ReleaseDC(hWnd, hdc);
		::DeleteObject(bmpDst);
		//如果是窗口关闭/窗口折叠导致的记得删去那个窗口
		if (!isAlive || wRect->right < wRect->left || wRect->top > wRect->bottom)
		{
			qDebug() << QString::number((uint)hWnd, 16) + " Disappeared.";
			shouldRun = false;
		}
		//如果仅仅是尺寸不同并且符合规范则重启
		else if (!isSame&&isAlive&&shouldRun)
		{
			continue;
		}
		break;//结束循环
	}
	close();
}
