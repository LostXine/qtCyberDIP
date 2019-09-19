#ifndef QTCYBERDIP_H
#define QTCYBERDIP_H

#include "stdafx.h"
#include "ui_qtcyberdip.h"
#include "comSPHandler.h"
#include "bbqScreenForm.h"
#include "capScreenForm.h"
#include "vodPlayer.h"
#include "camPlayer.h"
#include <QtNetwork/QUdpSocket>
#include <QCameraInfo>

#ifdef VIA_OPENCV
#include <opencv2\opencv.hpp>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#endif

namespace Ui {
	class qtCyberDip;
}

struct Device
{
	QString name;
	QString address;
	QTime lastPing;
};

class qtCyberDip : public QMainWindow
{
	Q_OBJECT

public:
	explicit qtCyberDip(QWidget *parent = 0);
	~qtCyberDip();

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
	void capHandleChanged(int p);
	void vodClickBrowseButton();
	void vodClickPlayButton();
	void vodClickPauseButton();
	void camClickOpenButton();
	void processImg(QImage img);
	void errLogWin(QString err);

public slots:
	void comLogAdd(QString txt, int type);
	void comDeviceDelay(float delay);
	void formClosed();//������ʾ������
	void formCleanning();//ֻ��������ʾ������

private:
	Ui::qtCyberDip *ui;
	/*******BBQ��ر����뷽��*******/
	QUdpSocket* bbqAnnouncer;
	// Pair device name, device ip / List order is listWidget of devices
	QList<Device*> bbqDevices;
	QProcess* bbqADBProcess;
	QStringList bbqADBLog;
	QStringList bbqADBErrorLog;
	QListWidget* bbqDebugWidget;
	QProcess* bbqRunAdb(const QStringList& params);
	bbqScreenForm* bbqSF = nullptr;
	int bbqCrashCount;
	bool bbqServiceShouldRun;
	bool bbqServiceStartError;
	void bbqStartUsbService();
	void bbqCleanADBProcess();
	void bbqResetUSBAdbUI();
	/*******���ڿ�����ر����뷽��*******/
	double comPosX, comPosY;
	bool comIsDown;//�����״̬
	bool comFetch;//��ֹZ��Խ��
	comSPHandler* comSPH = nullptr;
	QList<QSerialPortInfo> comPorts;
	void comUpdateUI();
	void comUpdatePos();
	void comSendBytes();
	void comScanPorts();
	//�˶��������õĲ���
	void comRequestToSend(QString txt);
	void comMoveTo(double x, double y);
	void comMoveToScale(double ratioX, double ratioY);
	void comHitDown();
	void comHitUp();
	void comHitOnce();
	friend class deviceCyberDip;	//������Ԫ
	/*******��Ļ��׽��ر����뷽��*******/
	QList<HWND> capWins;
	void capInitScale();
	void capHandleFilter(HWND hWnd);
	//ɨ����Ӵ���
	void capAddhWnd(HWND hWnd, QString nameToShow, bool isTarget);
	capScreenForm* capSF = nullptr;
	/*******¼���ȡ����ر����뷽��*******/
	int vodBrowsePath();
	void vodUpdateUI();
	vodPlayer* vodPF = nullptr;
	QThread vodThread;
	/*******�����ȡ����ر����뷽��*******/
	void camUpdateUI();
	void camScanCameras();
	QList<QCameraInfo> camDevices;
	camPlayer* camPF = nullptr;
	QThread camThread;
	/*******OPEN_CV����ر����뷽��*******/
#ifdef VIA_OPENCV
	cv::Mat QImage2cvMat(const QImage& image);
	//��Ϸ�߼���ͼ��ʶ����
	void* usrGC = nullptr;
#endif
};

#ifdef VIA_OPENCV
class deviceCyberDip
{
private:
	qtCyberDip* qt_ptr;
public:
	deviceCyberDip(void* qtCD) :qt_ptr((qtCyberDip*)qtCD){};
	//��¶����Ϸ���Ƶ��˶�����
	void comRequestToSend(QString txt);
	void comMoveTo(double x, double y);
	void comMoveToScale(double ratioX, double ratioY);
	void comHitDown();
	void comHitUp();
	void comDeviceDelay(float delay);
	void comHitOnce();
};
#endif
#endif // QTCYBERDIP_H

