#ifndef COMSPHANDLER_H
#define COMSPHANDLER_H

#include <QObject>
#include <QtSerialPort\QtSerialPort>

class comSPHandler : public QObject
{
	Q_OBJECT

public:
	comSPHandler(QObject *parent = 0);
	~comSPHandler();

	void setPort(QSerialPortInfo sp);
	bool isOpen();
	bool connectTo(int baud);
	void disConnect();

signals:
	void serialPortSignals(QString txt, int type);

private slots:
	void processReceived();
public slots:
	void requestToSend(QString txt);

private:
	QSerialPort* mSP;
	QByteArray mGlobalBytesBuffer;

	bool shouldStop;
};

#endif // COMSPHANDLER_H
