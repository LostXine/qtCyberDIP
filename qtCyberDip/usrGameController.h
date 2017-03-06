#ifdef VIA_OPENCV
#ifndef USRGAMECONTROLLER_H
#define USRGAMECONTROLLER_H

#include "qtcyberdip.h"

#define WIN_NAME "Frame"

//鼠标回调结构体
struct MouseArgs{
	cv::Rect box;
	bool Drawing, Hit;
	// init
	MouseArgs() :Drawing(false), Hit(false)
	{
		box = cv::Rect(0, 0, -1, -1);
	}
};
//鼠标回调函数
void  mouseCallback(int event, int x, int y, int flags, void*param);

class usrGameController
{
private:
	qtCyberDip* cyberDip;
	unsigned int imgCount;

	MouseArgs argM;
public:
	//构造函数，所有变量的初始化都应在此完成
	usrGameController(qtCyberDip* qtCD);
	//析构函数，回收本类所有资源
	~usrGameController();
	//处理图像函数，每次收到图像时都会调用
	int usrProcessImage(cv::Mat& img);
};

#endif
#endif