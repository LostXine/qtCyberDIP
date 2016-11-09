#ifndef QTCYBERDIP_H
#define QTCYBERDIP_H

#include <QtWidgets/QMainWindow>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QTime>
#include <QListWidgetItem>
#include <QProcess>
#include <QtSerialPort/QSerialPort>  
#include <QtSerialPort/QSerialPortInfo>  
#include "bbqScreenForm.h"
#include "comSPHandler.h"
#include "capScreenForm.h"

namespace Ui {
	class qtCyberDip;
}

class bbqScreenForm;
class capScreenForm;

struct Device
{
	QString name;
	QString address;
	QTime lastPing;
};
#ifdef VIA_OPENCV
struct MouseArgs{
	CvRect box;
	bool Drawing, Hit;
	// init
	MouseArgs() :Drawing(false), Hit(false)
	{
		box = cvRect(0,0, -1, -1);
	}
};

#endif

class qtCyberDip : public QMainWindow
{
	Q_OBJECT

public:
	explicit qtCyberDip(QWidget *parent = 0);
	~qtCyberDip();
	void capAddhWnd(HWND hWnd, QString nameToShow);

protected:
	//Reimplemented functions;
	void closeEvent(QCloseEvent* evt);
	void timerEvent(QTimerEvent* evt);
	bool eventFilter(QObject* watched, QEvent* event);

private slots:
	void bbqDiscoveryReadyRead();
	void bbqClickConnect();
	void bbqSelectDevice(QListWidgetItem* item);
	void bbqDoubleClickDevice(QListWidgetItem* item);
	void bbqClickBootstrapUSB();
	void bbqClickConnectUSB();
	void bbqADBProcessFinishes();
	void bbqADBProcessReadyRead();
	void bbqADBErrorReadyRead();
	void bbqQualityChanged(int index);
	void bbqBitrateChanged(int value);
	void bbqClickShowDebugLog(); 
	void comClickConnectButton();
	void comClickSendButton();
	void comClickClearButton();
	void comClickHitButton();
	void comClickRetButton();
	void comMoveStepUp();
	void comMoveStepDown();
	void comMoveStepLeft();
	void comMoveStepRight();
	void comInitPara();
	void capClickClearButton();
	void capClickScanButton();
	void capClickConnect();
	void capDoubleClickWin(QListWidgetItem* item);

	void processImg(QImage img);

public slots:
	void  comLogAdd(QString txt, int type);
	
	
private:
	Ui::qtCyberDip *ui;
	QUdpSocket* bbqAnnouncer;
	// Pair device name, device ip / List order is listWidget of devices
	QList<Device*> bbqDevices;
	QProcess* bbqADBProcess;
	QStringList bbqADBLog;
	QStringList bbqADBErrorLog;
	QListWidget* bbqDebugWidget;
	QProcess* bbqRunAdb(const QStringList& params);
	int bbqCrashCount;
	bool bbqServiceShouldRun;
	bool bbqServiceStartError;
	void bbqStartUsbService();

	comSPHandler* comSPH;
	QList<QSerialPortInfo> comPorts;
	void comUpdateUI();
	void comUpdatePos();
	void comSendBytes();
	void comScanPorts();
	//运动控制所用的操作
	void comRequestToSend(QString txt);
	void comMoveTo(double x, double y);
	void comMoveToScale(double ratioX, double ratioY);
	void comHitDown();
	void comHitUp();

	QList<HWND> capWins;
	double comPosX, comPosY;
	bool initImg;
	bool hitDown;//电磁铁状态
	bool fetch;//防止Z轴越界

#ifdef VIA_OPENCV
	MouseArgs argM;
	cv::Mat QImage2cvMat(QImage image);

public:
	void closeCV();
	
#endif
	

};

Q_GUI_EXPORT QImage qt_imageFromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h);
BOOL CALLBACK capEveryWindowProc(HWND hWnd, LPARAM parameter);

#ifdef VIA_OPENCV
void  mouseCallback(int event, int x, int y, int flags, void*param);
#endif


#endif // QTCYBERDIP_H
