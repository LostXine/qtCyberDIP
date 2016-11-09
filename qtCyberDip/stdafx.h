
//*************************************************************//
//                                                             //
//                   CyberDip 驱动程序v0.98                     //
//                                                             //
//             基于 Qt5.7 OpenCV2.4.9  VS2013 等更高版本        //
//             基于 FFmpeg 2.2.2                               //
//             基于 bbqScreenClient 的源代码			     	   // 
//			   适配 grbl v0.8c (Baud 9600)			    	   //
//			   适配 grbl v0.9j (Baud 115200)			    	   //
//                                                             //
//                       CVPR实验室 出品                       //
//                   地点：电信群楼 2 - 302B                    //
//                                                             //
//*************************************************************//
//上面的边框受编译器、字体限制是对不齐的，强迫症们放弃吧..

//STEP0:在上方QT菜单中配置QT版本
//STEP0:Set QT version

//STEP1: 注销掉#define VIA_OPENCV以在配置OpenCV前尝试编译本程序 编译通过后再配置OpenCV内容
//STEP1: Comment #define VIA_OPENCV and build -> Test your QT and FFmpeg settings.
//#define VIA_OPENCV

//STEP2: 测量并定义好设备的长宽以及窗口的上边缘
//RANGE  表示设备屏幕在CyberDIP中的尺寸
//UP_CUT 表示Capture时减去窗口头顶的标题框的大小，仅在使用AirPlayer时使用，使用bbq的时候应设置为0.
//STEP2: Measure the range of your device in CyberDIP.
//RANGE  means the screen size of device in CyberDIP coordinates.
//UP_CUT is used to chop the title of video window.
#define RANGE_X 61
#define RANGE_Y 47
#define UP_CUT 35.0

//STEP3: 在qtCyberDIP.cpp中修改相应的图像处理代码
//STEP3: Modify codes in qtCyberDIP.cpp for image processing.

//以下不必更改
#include <QtWidgets/QtWidgets>

#ifdef VIA_OPENCV
#include <opencv2\opencv.hpp>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#endif
