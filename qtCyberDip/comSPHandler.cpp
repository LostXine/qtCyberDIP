#include "comSPHandler.h"

comSPHandler::comSPHandler(QObject *parent)
	: QObject(parent), mSP(nullptr), shouldStop(true)
{
}

comSPHandler::~comSPHandler()
{
	shouldStop = true;
	mSP->close();
	delete mSP;
}

void comSPHandler::setPort(QSerialPortInfo sp)
{
	if (!mSP){ mSP = new QSerialPort(sp); }
	else
	{
		mSP->setPort(sp);
	}
}
void comSPHandler::processReceived()
{
	if (!mSP || shouldStop){ return; }
	if (!(mSP->isOpen())){ return; }
	while (mSP->bytesAvailable() > 0)
	{
		if (!mSP || shouldStop){ return; }
		QByteArray tp = mSP->readAll();
		mGlobalBytesBuffer.append(tp);
		mSP->waitForReadyRead(50);
	}
	emit serialPortSignals(QString(mGlobalBytesBuffer), 0);
	mGlobalBytesBuffer.clear();
}
void comSPHandler::requestToSend(QString txt)
{
	if (!mSP || shouldStop){ return; }
	if (!(mSP->isOpen())){ return; }
	QByteArray tp = (txt+"\n").toLatin1();
	mSP->write(tp);
	mSP->waitForBytesWritten(50);
	emit serialPortSignals(txt, 1);
}

bool comSPHandler::isOpen()
{
	if (!mSP){ return false; }
	return mSP->isOpen();
	
}
bool comSPHandler::connectTo(int baud)
{
	if (!mSP){ return false; }
	connect(mSP, SIGNAL(readyRead()), this, SLOT(processReceived()));
	mSP->setBaudRate(baud);
	shouldStop = !(mSP->open(QIODevice::ReadWrite));
	return !shouldStop;
}
void comSPHandler::disConnect()
{
	shouldStop = true;
	if (!mSP){ return; }
	mSP->waitForBytesWritten(500);
	mSP->close();
}