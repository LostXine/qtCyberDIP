#include "stdafx.h"
#include "qtcyberdip.h"

#define ADB_PATH "prebuilts/adb.exe"

#ifdef VIA_OPENCV
#include "usrGameController.h"
#endif

qtCyberDip::qtCyberDip(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::qtCyberDip), bbqADBProcess(nullptr), bbqDebugWidget(nullptr), bbqServiceShouldRun(false), bbqCrashCount(0),
comSPH(nullptr), comPosX(0), comPosY(0), comIsDown(false), comFetch(false)
{
	ui->setupUi(this);

	// Setup UDP discovery socket
	bbqAnnouncer = new QUdpSocket(this);
	bbqAnnouncer->bind(QHostAddress::Any, 9876);
	connect(bbqAnnouncer, SIGNAL(readyRead()), this, SLOT(bbqDiscoveryReadyRead()));

	// Connect UI slots
	connect(ui->bbqListDevices, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(bbqSelectDevice(QListWidgetItem*)));
	connect(ui->bbqListDevices, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(bbqDoubleClickDevice(QListWidgetItem*)));
	connect(ui->bbqConnect, SIGNAL(clicked()), this, SLOT(bbqClickConnect()));
	connect(ui->bbqBootstrapUSB, SIGNAL(clicked()), this, SLOT(bbqClickBootstrapUSB()));
	connect(ui->bbqConnectUSB, SIGNAL(clicked()), this, SLOT(bbqClickConnectUSB()));
	connect(ui->bbqCbQuality, SIGNAL(currentIndexChanged(int)), this, SLOT(bbqQualityChanged(int)));
	connect(ui->bbqSpinBitrate, SIGNAL(valueChanged(int)), this, SLOT(bbqBitrateChanged(int)));
	connect(ui->bbqDebugLog, SIGNAL(clicked()), this, SLOT(bbqClickShowDebugLog()));
	connect(ui->comInitButton, SIGNAL(clicked()), this, SLOT(comInitPara()));
	connect(ui->comConnectButton, SIGNAL(clicked()), this, SLOT(comClickConnectButton()));
	connect(ui->comSendButton, SIGNAL(clicked()), this, SLOT(comClickSendButton()));
	connect(ui->comSendEdit, SIGNAL(returnPressed()), this, SLOT(comClickSendButton()));
	connect(ui->comClcButton, SIGNAL(clicked()), this, SLOT(comClickClearButton()));
	connect(ui->comHitButton, SIGNAL(clicked()), this, SLOT(comClickHitButton()));
	connect(ui->comReturnButton, SIGNAL(clicked()), this, SLOT(comClickRetButton()));
	connect(ui->comUpButton, SIGNAL(clicked()), this, SLOT(comMoveStepUp()));
	connect(ui->comDownButton, SIGNAL(clicked()), this, SLOT(comMoveStepDown()));
	connect(ui->comLeftButton, SIGNAL(clicked()), this, SLOT(comMoveStepLeft()));
	connect(ui->comRightButton, SIGNAL(clicked()), this, SLOT(comMoveStepRight()));
	connect(ui->capClcButton, SIGNAL(clicked()), this, SLOT(capClickClearButton()));
	connect(ui->capScanButton, SIGNAL(clicked()), this, SLOT(capClickScanButton()));
	connect(ui->capStartButton, SIGNAL(clicked()), this, SLOT(capClickConnect()));
	connect(ui->capList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(capDoubleClickWin(QListWidgetItem*)));

	comUpdatePos();

	//监听子控件事件
	ui->comSelList->installEventFilter(this);
	//     | Who sends event &&         | Who will watch event


	startTimer(500);
}

qtCyberDip::~qtCyberDip()
{
	delete ui;
}

void qtCyberDip::closeEvent(QCloseEvent* evt)
{
	Q_UNUSED(evt);
	if (bbqADBProcess != nullptr)
	{
		disconnect(bbqADBProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(bbqADBProcessFinishes()));
		bbqCleanADBProcess();
	}
	if (bbqDebugWidget != nullptr)
	{
		bbqDebugWidget->close();
		delete bbqDebugWidget;
		bbqDebugWidget = nullptr;
	}
	if (comSPH != nullptr)
	{
		comSPH->disConnect();
		delete comSPH;
		comSPH = nullptr;
	}
	formClosed();
}

void qtCyberDip::timerEvent(QTimerEvent* evt)
{
	Q_UNUSED(evt);

	// See if we have devices that disappeared. We make them timeout after 3 seconds.
	for (auto it = bbqDevices.begin(); it != bbqDevices.end(); ++it)
	{
		if ((*it)->lastPing.elapsed() > 3000)
		{
			ui->bbqListDevices->takeItem(bbqDevices.indexOf(*it));
			delete (*it);
			it = bbqDevices.erase(it);
			break;
		}
	}
}

bool qtCyberDip::eventFilter(QObject* watched, QEvent* event)
{
	//定义点击combobox之后刷新可用COM口
	if (watched == ui->comSelList && event->type() == QEvent::MouseButtonPress)
	{
		comScanPorts();
	}
	return QObject::eventFilter(watched, event);
}

void qtCyberDip::bbqDiscoveryReadyRead()
{
	QByteArray datagram;
	QHostAddress sender;
	quint16 senderPort;

	while (bbqAnnouncer->hasPendingDatagrams())
	{
		if (datagram.size() != bbqAnnouncer->pendingDatagramSize())
			datagram.resize(bbqAnnouncer->pendingDatagramSize());

		// Read pending UDP datagram
		bbqAnnouncer->readDatagram(datagram.data(), datagram.size(),
			&sender, &senderPort);

		// Format of announcer packet:
		// 0 : Protocol version
		// 1 : Device name size
		// 2+: Device name

		unsigned char protocolVersion = datagram.at(0),
			deviceNameSize = datagram.at(1);

		QString deviceName = QByteArray(datagram.data() + 2, deviceNameSize);
		QString remoteIp = sender.toString();

		// Make sure we don't already know this device
		bool exists = false;
		for (auto it = bbqDevices.begin(); it != bbqDevices.end(); ++it)
		{
			if ((*it)->name == deviceName && (*it)->address == remoteIp)
			{
				(*it)->lastPing.restart();
				exists = true;
				break;
			}
		}

		if (!exists)
		{
			// XXX: Protocol v3 indicates that audio can't be streamed, and v4
			// indicates that we can stream audio. However, the user can choose
			// to turn off audio even on v4. Maybe in the future we could indicate
			// that.
			Device* device = new Device;
			device->name = deviceName;
			device->address = remoteIp;
			device->lastPing.start();

			ui->bbqListDevices->addItem(QString("%1 - (%2)").arg(deviceName, remoteIp));
			bbqDevices.push_back(device);
		}
	}
}
void qtCyberDip::bbqClickConnect()
{
	setCursor(Qt::WaitCursor);
	// Check that the IP entered is valid
	QString ip = ui->bbqIP->text();
	QHostAddress address(ip);
	if (QAbstractSocket::IPv4Protocol != address.protocol())
	{
		QMessageBox::critical(this, "Invalid IP", "The IP address you entered is invalid");
		setCursor(Qt::ArrowCursor);
		return;
	}
	if (bbqSF != nullptr)
	{
		delete bbqSF;
		bbqSF = nullptr;
	}
	// The IP is valid, connect to there
	bbqSF = new bbqScreenForm(this);
#ifdef VIA_OPENCV
	usrGC = new usrGameController(this);
	connect(bbqSF, SIGNAL(imgReady(QImage)), this, SLOT(processImg(QImage)));
#endif
	connect(bbqSF, SIGNAL(bbqFinished()), this, SLOT(formClosed()), Qt::QueuedConnection);
	bbqSF->setAttribute(Qt::WA_DeleteOnClose);
	bbqSF->setShowFps(ui->bbqShowFps->isChecked());
	bbqSF->show();
	bbqSF->connectTo(ui->bbqIP->text());

	// Hide this dialog
	hide();
	setCursor(Qt::ArrowCursor);
}
void qtCyberDip::bbqSelectDevice(QListWidgetItem* item)
{
	Q_UNUSED(item);

	int index = ui->bbqListDevices->currentRow();
	if (index >= 0)
	{
		ui->bbqIP->setText(bbqDevices.at(index)->address);
	}
}

void qtCyberDip::bbqDoubleClickDevice(QListWidgetItem* item)
{
	bbqSelectDevice(item);
	bbqClickConnect();
}

QProcess* qtCyberDip::bbqRunAdb(const QStringList& params)
{
	QProcess* process = new QProcess(this);

	connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(bbqADBProcessReadyRead()));
	connect(process, SIGNAL(readyReadStandardError()), this, SLOT(bbqADBErrorReadyRead()));

#ifndef PLAT_APPLE
	process->start(ADB_PATH, params);
#else
	process->start(QDir(QCoreApplication::applicationDirPath()).absolutePath() + "/" + ADB_PATH, params);
#endif	

	return process;
}

void qtCyberDip::bbqClickBootstrapUSB()
{
	//qDebug()<< bbqADBProcess;
	if (!bbqServiceShouldRun)
	{
		bbqCrashCount = 0;
		bbqServiceShouldRun = true;
		bbqStartUsbService();
	}
	else
	{
		bbqServiceShouldRun = false;
		bbqCleanADBProcess();
	}
}

void qtCyberDip::bbqClickConnectUSB()
{
	// Forward TCP port to localhost and connect to it
	QStringList args;
	args << "forward";
	args << "tcp:9876";
	args << "tcp:9876";

	bbqRunAdb(args);

	ui->bbqIP->setText("127.0.0.1");
	bbqClickConnect();
}

void qtCyberDip::bbqADBProcessFinishes()
{
	//qDebug() << "finish";
	if (bbqServiceShouldRun)
	{
		bbqCrashCount++;

		if (bbqCrashCount > 20)
		{
			QMessageBox::critical(this, "Crash!", "It appears that the streaming process has crashed over 20 times. Please check the debug log window and send a screenshot to the support.");
			bbqServiceShouldRun = false;
		}
		// If the process crashed, reboot it
		//bbqStartUsbService();
	}
	else
	{
		//qDebug() << "Normal end";
		// Normal stop
		ui->bbqBootstrapUSB->setText("Start USB service");
		ui->bbqConnectUSB->setEnabled(false);
	}
}

void qtCyberDip::bbqADBProcessReadyRead()
{
	//qDebug() << "ReadyRead";
	QProcess* process = (QProcess*)QObject::sender();

	QByteArray stdOut = process->readAllStandardOutput();
	QString stdOutLine = QString(stdOut).trimmed();

	if (stdOutLine.contains("/data/data") && stdOutLine.contains("No such file or directory"))
	{
		bbqServiceShouldRun = false;
		bbqServiceStartError = true;
	}
	else if (stdOutLine.contains("Unable to chmod"))
	{
		bbqServiceShouldRun = false;
		bbqServiceStartError = true;
	}

	if (!stdOutLine.isEmpty())
	{
		bbqADBLog.push_back(stdOutLine);

		if (bbqDebugWidget != nullptr)
		{
			bbqDebugWidget->addItem(stdOutLine);
		}
	}
}

void qtCyberDip::bbqADBErrorReadyRead()
{
	QProcess* process = (QProcess*)QObject::sender();

	QByteArray stdErr = process->readAllStandardError();
	QString stdErrLine = QString(stdErr).trimmed();

	if (stdErrLine.contains("device not found"))
	{
		bbqServiceShouldRun = false;
		QMessageBox::critical(this, "Device not found or unplugged", "Cannot find an Android device connected via ADB. Make sure USB Debugging is enabled on your device, and that the ADB drivers are installed. Follow the guide on our website for more information.");
	}
	else if (stdErrLine.contains("device offline"))
	{
		bbqServiceShouldRun = false;
		QMessageBox::critical(this, "Device offline", "An Android device is connected but reports as offline. Check your device for any additional information, or try to unplug and replug your device");
	}
	else if (stdErrLine.contains("unauthorized"))
	{
		bbqServiceShouldRun = false;
		QMessageBox::critical(this, "Device unauthorized", "An Android device is connected but reports as unauthorized. Please check the confirmation dialog on your device.");
	}

	if (!stdErrLine.isEmpty())
	{
		bbqADBErrorLog.push_back(stdErrLine);

		if (bbqDebugWidget != nullptr)
		{
			QListWidgetItem* item = new QListWidgetItem(stdErrLine);
			item->setTextColor(QColor(255, 0, 0));
			bbqDebugWidget->addItem(item);
		}
	}
	bbqCleanADBProcess();
}

void qtCyberDip::bbqStartUsbService()
{
	bbqServiceStartError = false;

	ui->bbqBootstrapUSB->setEnabled(false);
	ui->bbqBootstrapUSB->setText("Starting...");
	setCursor(Qt::WaitCursor);
	qApp->processEvents();

	if (bbqADBProcess == nullptr)
	{
		bbqADBProcess = new QProcess(this);
		connect(bbqADBProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(bbqADBProcessFinishes()));
		connect(bbqADBProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(bbqADBProcessReadyRead()));
		connect(bbqADBProcess, SIGNAL(readyReadStandardError()), this, SLOT(bbqADBErrorReadyRead()));
	}

	// Copy binary to workaround some security restrictions on Lollipop and Knox
	QStringList args;
	args << "shell";
	args << "cp";
	args << "/data/data/org.bbqdroid.bbqscreen/files/bbqscreen";
	args << "/data/local/tmp/bbqScreen";
	QProcess* copyProc = bbqRunAdb(args);
	copyProc->waitForFinished();
	if (bbqServiceStartError)
	{
		bbqResetUSBAdbUI();
		QMessageBox::critical(this, "Unable to prepare the USB service", "Unable to copy the bbqscreen service to an executable zone on your device, as it hasn't been found. Please make sure the bbqscreen app is installed, and that you opened it once, and pressed 'USB' if prompted or turned it on once.");
		delete copyProc;
		return;
	}

	args.clear();
	args << "shell";
	args << "chmod";
	args << "755";
	args << "/data/local/tmp/bbqscreen";
	QProcess* chmodProc = bbqRunAdb(args);
	chmodProc->waitForFinished();
	if (bbqServiceStartError)
	{
		bbqResetUSBAdbUI();
		QMessageBox::critical(this, "Unable to prepare the USB service", "Unable to set the permissions of the bbqscreen service to executable. Please contact support.");
		delete chmodProc;
		return;
	}

	args.clear();
	args << "shell";
	args << "/data/local/tmp/bbqscreen";
	args << "-s";
	args << "50";
	switch (ui->bbqCbQuality->currentIndex())
	{
	case 0:
		args << "-1080";
		break;
	case 1:
		args << "-720";
		break;
	case 2:
		args << "-540";
		break;
	case 3:
		args << "-360";
		break;
	}
	args << "-q";
	args << QString::number(ui->bbqSpinBitrate->value());
	args << "-i";

	if (bbqADBProcess != nullptr)
	{
		bbqADBProcess->start(ADB_PATH, args);
	}
	else
	{
		bbqResetUSBAdbUI();
		delete chmodProc;
		delete copyProc;
		return;
	}
	ui->bbqConnectUSB->setEnabled(true);
	ui->bbqBootstrapUSB->setEnabled(true);
	ui->bbqBootstrapUSB->setText("Stop USB service");
	setCursor(Qt::ArrowCursor);
	delete chmodProc;
	delete copyProc;
}

void qtCyberDip::bbqQualityChanged(int index)
{
	bbqCrashCount = 0;
	bbqCleanADBProcess();
}

void qtCyberDip::bbqBitrateChanged(int value)
{
	bbqCrashCount = 0;
	bbqCleanADBProcess();
}

void qtCyberDip::bbqClickShowDebugLog()
{
	if (bbqDebugWidget != nullptr)
	{
		delete bbqDebugWidget;
	}
	bbqDebugWidget = new QListWidget();
	bbqDebugWidget->addItems(bbqADBLog);

	for (auto it = bbqADBErrorLog.begin(); it != bbqADBErrorLog.end(); ++it) {
		QListWidgetItem* item = new QListWidgetItem(*it);
		item->setTextColor(QColor(255, 0, 0));
		bbqDebugWidget->addItem(item);
	}

	bbqDebugWidget->show();

}

void qtCyberDip::bbqCleanADBProcess()
{
	if (bbqADBProcess != nullptr)
	{
		// Restart the app
		bbqADBProcess->terminate();
		bbqADBProcess->kill();
		delete bbqADBProcess;
		bbqADBProcess = nullptr;
	}
}

void qtCyberDip::bbqResetUSBAdbUI()
{
	setCursor(Qt::ArrowCursor);
	ui->bbqBootstrapUSB->setEnabled(true);
	ui->bbqBootstrapUSB->setText("Start USB service");
}

void qtCyberDip::comInitPara()
{
	/* 0.8c*/
	/*
	$0=250.000 (x, step/mm)
	$1=250.000 (y, step/mm)
	$2=250.000 (z, step/mm)
	$3=10 (step pulse, usec)
	$4=50.000 (default feed, mm/min)
	$5=2000.000 (default seek, mm/min)
	$6=192 (step port invert mask, int:11000000)
	$7=25 (step idle delay, msec)
	$8=120.000 (acceleration, mm/sec^2)
	$9=0.050 (junction deviation, mm)
	$10=0.100 (arc, mm/segment)
	$11=25 (n-arc correction, int)
	$12=3 (n-decimals, int)
	$13=0 (report inches, bool)
	$14=1 (auto start, bool)
	$15=0 (invert step enable, bool)
	$16=0 (hard limits, bool)
	$17=0 (homing cycle, bool)
	$18=0 (homing dir invert mask, int:00000000)
	$19=25.000 (homing feed, mm/min)
	$20=250.000 (homing seek, mm/min)
	$21=100 (homing debounce, msec)
	$22=1.000 (homing pull-off, mm)

	*/

	/*	0.9j */
	/*$0=10 (step pulse, usec)
$1=25 (step idle delay, msec)
$2=0 (step port invert mask:00000000)
$3=0 (dir port invert mask:00000000)
$4=0 (step enable invert, bool)
$5=0 (limit pins invert, bool)
$6=0 (probe pin invert, bool)
$10=3 (status report mask:00000011)
$11=0.010 (junction deviation, mm)
$12=0.002 (arc tolerance, mm)
$13=0 (report inches, bool)
$20=0 (soft limits, bool)
$21=0 (hard limits, bool)
$22=0 (homing cycle, bool)
$23=0 (homing dir invert mask:00000000)
$24=25.000 (homing feed, mm/min)
$25=500.000 (homing seek, mm/min)
$26=250 (homing debounce, msec)
$27=1.000 (homing pull-off, mm)
$100=250.000 (x, step/mm)
$101=250.000 (y, step/mm)
$102=250.000 (z, step/mm)
$110=2500.000 (x max rate, mm/min)
$111=2500.000 (y max rate, mm/min)
$112=500.000 (z max rate, mm/min)
$120=120.000 (x accel, mm/sec^2)
$121=120.000 (y accel, mm/sec^2)
$122=10.000 (z accel, mm/sec^2)
$130=200.000 (x max travel, mm)
$131=200.000 (y max travel, mm)
$132=200.000 (z max travel, mm)*/
	QList<QPair<int, float>> para;
	if (ui->comCheckS->isChecked())
	{

		para.push_back(qMakePair(0, 10));
		para.push_back(qMakePair(1, 25));
		para.push_back(qMakePair(2, 0));
		para.push_back(qMakePair(3, 0));
		para.push_back(qMakePair(4, 0));
		para.push_back(qMakePair(5, 0));
		para.push_back(qMakePair(6, 0));

		para.push_back(qMakePair(10, 3));
		para.push_back(qMakePair(11, 0.010));
		para.push_back(qMakePair(12, 0.002));
		para.push_back(qMakePair(13, 0));

		para.push_back(qMakePair(20, 0));
		para.push_back(qMakePair(21, 0));
		para.push_back(qMakePair(22, 0));
		para.push_back(qMakePair(23, 0));
		para.push_back(qMakePair(24, 25));
		para.push_back(qMakePair(25, 500));
		para.push_back(qMakePair(26, 250));
		para.push_back(qMakePair(27, 1));

		para.push_back(qMakePair(100, 250));
		para.push_back(qMakePair(101, 250));
		para.push_back(qMakePair(102, 250));

		para.push_back(qMakePair(110, 2500));
		para.push_back(qMakePair(111, 2500));
		para.push_back(qMakePair(112, 500));

		para.push_back(qMakePair(120, 120));
		para.push_back(qMakePair(121, 120));
		para.push_back(qMakePair(122, 10));

		para.push_back(qMakePair(130, 200));
		para.push_back(qMakePair(131, 200));
		para.push_back(qMakePair(132, 200));
	}
	else
	{
		para.push_back(qMakePair(0, 250));
		para.push_back(qMakePair(1, 250));
		para.push_back(qMakePair(2, 200));
		para.push_back(qMakePair(3, 10));
		para.push_back(qMakePair(4, 50));
		para.push_back(qMakePair(5, 2000));
		para.push_back(qMakePair(6, 192));
		para.push_back(qMakePair(7, 25));
		para.push_back(qMakePair(8, 120));
		para.push_back(qMakePair(9, 0.05));
		para.push_back(qMakePair(10, 0.1));
		para.push_back(qMakePair(11, 25));
		para.push_back(qMakePair(12, 3));
		para.push_back(qMakePair(13, 0));
		para.push_back(qMakePair(14, 1));
		para.push_back(qMakePair(15, 0));
		para.push_back(qMakePair(16, 0));
		para.push_back(qMakePair(17, 0));
		para.push_back(qMakePair(18, 0));
		para.push_back(qMakePair(19, 25));
		para.push_back(qMakePair(20, 250));
		para.push_back(qMakePair(21, 100));
		para.push_back(qMakePair(22, 1.0));

	}

	QList<QPair<int, float>>::iterator tp = para.begin();
	while (tp != para.end())
	{
		char cache[64];
		sprintf(cache, "$%d=%0.3f", tp->first, tp->second);
		comRequestToSend(QString(cache));
		tp++;
		Sleep(500);
	}

}

void qtCyberDip::comClickConnectButton()
{
	setCursor(Qt::WaitCursor);
	bool built = comSPH;
	bool online = false;
	if (built){ online = comSPH->isOpen(); }
	if (online)
	{
		comLogAdd("Disconnecting..", 2);
		comSPH->disConnect();
		if (!(comSPH->isOpen())){ comLogAdd("Done.", 2); }
		ui->comSelList->setCurrentIndex(-1);
	}
	else
	{
		int index = ui->comSelList->currentIndex();
		if (index >= 0 && index < comPorts.length())
		{
			comLogAdd("Connecting..", 2);
			if (!built)
			{
				comSPH = new comSPHandler(this);
				connect(comSPH, SIGNAL(serialPortSignals(QString, int)), this, SLOT(comLogAdd(QString, int)));
			}
			comSPH->setPort(comPorts[index]);

			if (comSPH->connectTo((ui->comCheckS->isChecked()) ? QSerialPort::Baud115200 : QSerialPort::Baud9600))
			{
				comLogAdd("Done.", 2);
			}
			else
			{
				comLogAdd("Failed", 2);
			}
		}
	}
	comUpdateUI();
	setCursor(Qt::ArrowCursor);
}

void qtCyberDip::comUpdateUI()
{
	bool online = comSPH;
	if (online){ online = comSPH->isOpen(); }
	ui->comConnectButton->setText((online) ? "Disconnect" : "Connect");
	ui->comSendButton->setEnabled(online);
	ui->comHitButton->setEnabled(online);
	ui->comHitButton->setEnabled(online);
	ui->comReturnButton->setEnabled(online);
	ui->comUpButton->setEnabled(online);
	ui->comDownButton->setEnabled(online);
	ui->comLeftButton->setEnabled(online);
	ui->comRightButton->setEnabled(online);
	ui->comSelList->setEnabled(!online);
}

void qtCyberDip::comUpdatePos()
{
	ui->comPosLabel->setText("X: " + QString::number(comPosX) + "\nY: " + QString::number(comPosY));
}


void  qtCyberDip::comLogAdd(QString txt, int type = 0)
{
	//0 -normal&receive
	//1 -send
	//2 -system
	if (txt.length() < 1){ return; }
	QDateTime time = QDateTime::currentDateTime();
	QString tmp = time.toString("[hh:mm:ss.zzz] ").append(txt);
	switch (type)
	{
	case 1:
		{
			ui->comMainLog->append(">>" + txt + "\n");
			break;
		}
	case 2:
		{
			ui->comMainLog->append("/***   " + txt + "   ***/\n");
			break;
		}
	default:
		{
			ui->comMainLog->insertPlainText(txt);
			break;
		}
	}
	ui->comMainLog->moveCursor(QTextCursor::End);
	qDebug() << tmp;
}

void qtCyberDip::comScanPorts()
{
	ui->comSelList->clear();
	comPorts.clear();
	foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		//qDebug() << "Name        : " << info.portName();
		//qDebug() << "Description : " << info.description();
		//qDebug() << "Manufacturer: " << info.manufacturer();
		ui->comSelList->addItem(info.portName() + " " + info.description());
		comPorts.push_back(info);
	}
}

void qtCyberDip::comMoveTo(double x, double y)
{
	if (x == comPosX && y == comPosY){ return; }
	comPosX = x;
	comPosY = y;
	comRequestToSend("G90");//绝对坐标
	comRequestToSend("G0 X" + QString::number(-comPosX) + " Y" + QString::number(-comPosY));
	comUpdatePos();
}

void qtCyberDip::comMoveToScale(double ratioX, double ratioY)
{
	double px = std::max(std::min(ratioX, 1.0), 0.0);
	double py = std::max(std::min(ratioY, 1.0), 0.0);
	comMoveTo(px*RANGE_X, py*RANGE_Y);
}

void  qtCyberDip::comClickSendButton()
{
	if (ui->comSendEdit->text().length() > 0)
	{
		comRequestToSend(ui->comSendEdit->text());
	}
}

void qtCyberDip::comClickClearButton()
{
	ui->comMainLog->clear();
	ui->comSendEdit->clear();
}

void qtCyberDip::comClickHitButton()
{
	comHitOnce();
}

void qtCyberDip::comDeviceDelay(float delay=0.01)
{
	char cmd[32];
	sprintf_s(cmd, "G1 Z%0.3f F5.", (comFetch) ? delay : -delay);
	comRequestToSend(cmd);
	comFetch = !comFetch;
}

void qtCyberDip::comHitOnce()
{
	comHitDown();
	comRequestToSend("G91");//相对坐标
	//用不存在的Z轴实现延时功能
	comDeviceDelay(0.01);
	comHitUp();
	comDeviceDelay(0.01);
}

void qtCyberDip::comClickRetButton()
{
	comHitUp();
	comMoveTo(0, 0);
}

void qtCyberDip::comRequestToSend(QString txt)
{
	if (!comSPH){ return; }
	comSPH->requestToSend(txt);
}

void qtCyberDip::comMoveStepUp()
{
	comMoveTo(comPosX, comPosY - ui->comSpinBox->value());

}

void qtCyberDip::comMoveStepDown()
{
	comMoveTo(comPosX, comPosY + ui->comSpinBox->value());
}
void qtCyberDip::comMoveStepLeft()
{
	comMoveTo(comPosX - ui->comSpinBox->value(), comPosY);
}
void qtCyberDip::comMoveStepRight()
{

	comMoveTo(comPosX + ui->comSpinBox->value(), comPosY);
}

void qtCyberDip::comHitDown()
{
	comIsDown = true;
	comRequestToSend("M3 S1000");
}
void qtCyberDip::comHitUp()
{
	comIsDown = false;
	comRequestToSend("M5");
}


void qtCyberDip::capClickClearButton()
{
	ui->capList->clear();
	capWins.clear();
}

void qtCyberDip::capClickScanButton()
{

	capClickClearButton();
	EnumWindows(capEveryWindowProc, (LPARAM) this);
}

void qtCyberDip::capAddhWnd(HWND hWnd, QString nameToShow)
{
	capWins.push_back(hWnd);
	ui->capList->addItem(nameToShow);
}

BOOL CALLBACK capEveryWindowProc(HWND hWnd, LPARAM parameter)
{
	// 不可见、不可激活的窗口不作考虑。
	if (!IsWindowVisible(hWnd)){ return true; }
	if (!IsWindowEnabled(hWnd)){ return true; }
	// 弹出式窗口不作考虑。
	LONG gwl_style = GetWindowLong(hWnd, GWL_STYLE);
	if ((gwl_style & WS_POPUP) && !(gwl_style & WS_CAPTION)){ return true; }

	// 父窗口是可见或可激活的窗口不作考虑。
	HWND hParent = (HWND)GetWindowLong(hWnd, GW_OWNER);
	if (IsWindowEnabled(hParent)){ return true; }
	if (IsWindowVisible(hParent)){ return true; }

	wchar_t szCaption[500];
	::GetWindowText(hWnd, szCaption, sizeof(szCaption));
	//if (wcslen(szCaption) <= 0){ return true; }
	((qtCyberDip*)parameter)->capAddhWnd(hWnd, "0x" + QString::number((uint)hWnd, 16) + "  " + QString::fromWCharArray(szCaption));
	return true;
}

void qtCyberDip::capClickConnect()
{

	int index = ui->capList->currentRow();
	if (index > capWins.size() - 1 || index < 0){ return; }
	setCursor(Qt::WaitCursor);
	qDebug() << "Windows Handle: " << capWins[index];
	if (capSF != nullptr)
	{
		delete capSF;
		capSF = nullptr;
	}
	capSF = new capScreenForm(this);
#ifdef VIA_OPENCV
	usrGC = new usrGameController(this);
	connect(capSF, SIGNAL(imgReady(QImage)), this, SLOT(processImg(QImage)));
#endif
	connect(capSF, SIGNAL(capFinished()), this, SLOT(formClosed()),Qt::QueuedConnection);
	capSF->capSetHWND(capWins[index]);
	hide();
	capClickClearButton();
	setCursor(Qt::ArrowCursor);
	capSF->show();
	capSF->capRun();	
}

void qtCyberDip::capDoubleClickWin(QListWidgetItem* item)
{
	capClickConnect();
}

void qtCyberDip::formClosed()
{
	comClickRetButton();
	if (capSF != nullptr)
	{
		this->disconnect(capSF, SIGNAL(imgReady(QImage)));
		delete capSF;
		capSF = nullptr;
	}
	if (bbqSF != nullptr)
	{
		this->disconnect(bbqSF, SIGNAL(imgReady(QImage)));
		delete bbqSF;
		bbqSF = nullptr;
	}
#ifdef VIA_OPENCV
	if (usrGC != nullptr)
	{
		delete (usrGameController*)usrGC; //delete并不会清空指针,需要为指针指定类型才可以正确释放该实例
		usrGC = nullptr;
	}
#endif
	show();
}

void qtCyberDip::processImg(QImage img)
{
#ifdef VIA_OPENCV
	if (usrGC != nullptr)
	{
		((usrGameController*)usrGC)->usrProcessImage(QImage2cvMat(img));
	}
#endif
}

#ifdef VIA_OPENCV
cv::Mat qtCyberDip::QImage2cvMat(QImage image)
{
	cv::Mat mat;
	//qDebug() << image.format();
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		cv::cvtColor(mat, mat, CV_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		break;
	}
	return mat;
}

void deviceCyberDip::comRequestToSend(QString txt)
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comRequestToSend(txt);
}
void deviceCyberDip::comMoveTo(double x, double y)
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comMoveTo(x, y);
}
void deviceCyberDip::comMoveToScale(double ratioX, double ratioY)
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comMoveToScale(ratioX, ratioY);
}
void deviceCyberDip::comHitDown()
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comHitDown();
}
void deviceCyberDip::comHitUp()
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comHitUp();
}
void deviceCyberDip::comHitOnce()
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comHitOnce();
}
void deviceCyberDip::comDeviceDelay(float delay = 0.01)
{
	if (qt_ptr == nullptr) { return; }
	qt_ptr->comDeviceDelay(delay);
}
#endif

