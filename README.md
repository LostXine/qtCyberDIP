# qtCyberDIP
![CyberDIP](/pic/CyberDIP.JPG)

CyberDIP driver for windows in C++ 11.

CyberDIP is a USB device clicking touchscreens of smartphones or pads under PC's control. 
With the help of [grbl](https://github.com/grbl/grbl) running on an Arduino Nano, CyberDIP will translate serial port signals from your PC into 2D-motion and clicks.

本工程是依赖C++ 11标准编写的CyberDIP在Windows环境下的配套软件。

CyberDIP是通过计算机USB控制的触摸屏点击设备，通过搭载[grbl 0.8c/0.9j](https://github.com/grbl/grbl)驱动的Arduino Nano(ATmega328)控制器，CyberDIP可以将USB串口发来的指令翻译成相应的二维运动与点击操作，模拟单指对屏幕的操作。

***
###TODO
* Find solution to replace Airplayer.exe+PrintScreen

###Dependent
* [Qt](https://www.qt.io/) 5.7+
* [FFmpeg](https://ffmpeg.org/) 2.2.2 (included in [/3rdparty](/3rdparty))
* [QTFFmpegWrapper](https://inqlude.org/libraries/qtffmpegwrapper.html) (included in [/qtCyberDip/QTFFmpegWrapper](/qtCyberDip/QTFFmpegWrapper))
* [Android Debug Bridge](http://developer.android.com/tools/help/adb.html) (included in [/qtCyberDip/prebuilts](/qtCyberDip/prebuilts))
* (Optional)[OpenCV](http://www.opencv.org/) 2.4.9+

###Framework
qtCyberDIP contains 3 + 1 parts:
* __Serial Port(COM)__: Serial port and g-code module to work with Arduino.
* __[BBQScreen](https://github.com/xplodwild/bbqscreen_client)__: BBQScreen client module to decode frames from Android devices.
* __Capture__: Win32 API module to grab screen or windows, working with [AirPlayer](http://pro.itools.cn/airplayer) to read frames from iOS devices.
* __GameController__: User defined game controller, change this to play different games.

###Usage
* Install Qt 5.7+ and the Qt VS Add-In
* Install Driver for CH340 Chipset after connected to CyberDIP. ([Driver](/CH340) included)
* Install Git for windows
* Git Bash: 
```shell
git clone https://github.com/LostXine/qtCyberDIP
```
* Add '3rdparty/x64' (Default, if you are using 32-bit OS, add '3rdparty/x86') into Environment Variable 'Path'
* Open __qtCyberDIP.sln__ in Visual Studio 2013+ and follow the instructions in __stdafx.h__

###使用方法
* 安装Qt 5.7+与相应版本Qt的VS插件
* 连接CyberDIP后，安装CH340 芯片组驱动，驱动包含在[CH340](/CH340)文件夹中。
* 安装git for windows
* 在目标文件夹中右键选择Git Bash Here,在打开的命令行中输入：
```shell
git clone https://github.com/LostXine/qtCyberDIP
```
* 将3rdparty/x64(默认x64，32位操作系统添加3rdprty/x86)加入系统环境变量Path中
* 使用VS2013及以上版本打开工程文件__qtCyberDIP.sln__, 并按照__stdafx.h__中的说明操作

***
__请参加“数字图像处理基础”的同学注意:__
为了方便代码评阅，建议只修改__usrGameController.h__和__usrGameController.cpp__两个文件。

***
###Developer
* [Yue ZHOU](http://cvpr.sjtu.edu.cn/aboutme.aspx), A.P. Department of Automation, Shanghai Jiao Tong University.
* Xiang LI, MEng Department of Automation, Shanghai Jiao Tong University. 

###Contact me
* Email: lostxine@gmail.com
* Address: Room 2#302B, SEIEE Building, 800 DongchuanRd., Shanghai, PR China (200240) 
