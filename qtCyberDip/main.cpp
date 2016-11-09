#include "qtcyberdip.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qtCyberDip w;
	w.show();
	return a.exec();
}
