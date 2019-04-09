
//*************************************************************//
//                                                             //
//                   CyberDip 驱动程序v1.4                     //
//                                                             //
//             基于 Qt5.7 OpenCV2.4.9  VS2013 等更高版本        //
//             基于 FFmpeg 2.2.2                               //
//             基于 bbqScreenClient 的源代码                    // 
//             适配 grbl v0.8c (Baud 9600)                     //
//             适配 grbl v0.9j (Baud 115200)                   //
//                                                             //
//                       CVPR实验室 出品                        //
//                   地点：电信群楼 2 - 302B                    //
//                                                             //
//*************************************************************//
//上面的边框受编译器、字体限制是对不齐的，强迫症们放弃吧..

//*************************相关定义*****************************//
//************************Definitions***************************//

//#define VIA_OPENCV
#define RANGE_X 1
#define RANGE_Y 1
#define UP_CUT 0.0

//*************************使用说明*****************************//
//************************Instruction***************************//

//STEP0:在上方QT菜单中配置QT版本(如果没有则说明QT的VS插件安装失败)
//STEP0:Configure QT version

//STEP1: 注释掉#define VIA_OPENCV以在配置OpenCV前尝试编译本程序 编译通过后再配置OpenCV内容
//STEP1: Comment #define VIA_OPENCV and build -> Test your QT and FFmpeg settings.

//STEP2: 测量并定义好设备的长宽以及窗口的上边缘
//RANGE  表示设备屏幕在CyberDIP中的尺寸
//UP_CUT 表示Capture时减去窗口头顶的标题框的大小，仅在使用screenCapture时使用，使用bbq的时候应设置为0.
//STEP2: Measure the range of your device in CyberDIP.
//RANGE  means the screen size of device in CyberDIP coordinates.
//UP_CUT is used to chop the title of video window.

//STEP3: 配置OpenCV并取消VIA_OPENCV的注释 尝试编译本程序
//STEP3: Config OpenCV and uncomment VIA_OPENCV

//STEP4: 在usrGameController.h和usrGameController.cpp中修改相应的图像处理代码
//STEP4: Modify codes in usrGameController.h and usrGameController.cpp for image processing.

//注意！！！
//为了方便评阅，建议只修改usrGameController.h和usrGameController.cpp两个文件

//以下不必更改
#include <QtWidgets/QtWidgets>
