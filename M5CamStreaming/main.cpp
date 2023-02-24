#include <iostream>
#include <thread>
#include <Windows.h>
#include <conio.h>
#include "nkcOpenCV.h"
#include "SerialPort.h"
using namespace std;
//#pragma comment( linker,"/subsystem:windows /entry:mainCRTStartup")
const cv::String WINDOW_NAME = "M5Camera Streaming";
#pragma warning(disable : 4996)
int main() {
	//FreeConsole();
	cv::VideoCapture cap;
	//cap.open("http://unitv2.py/video_feed");
	cap.open(0);
	if (!cap.isOpened()) {
		std::cout << "Can't open camera." << std::endl;
		std::cin.get();
		return 1;
	}
	//cv::namedWindow(WINDOW_NAME, cv::WINDOW_AUTOSIZE);
	int ch;
	CSerialPort port;
	if (!port.InitPort(5,CBR_115200,'N',8,1,EV_RXCHAR)) {
		std::cout << "Init serial port succeeded." << std::endl;
	}
	else {
		std::cout << "Init serial port failed." << std::endl;
	}

	int mode = 1;
	int count = 0;
	unsigned char* str = new unsigned char[5];// 由于数据接受间隔，5更能无缝衔接，并将writedata的保护缓冲区改为2
	unsigned char* strNUM = new unsigned char[20];
	while (true) {
		/*cv::Mat img;
		if (cap.read(img)) {
			cv::imshow(WINDOW_NAME, img);
			if (cv::waitKey(1) == 27) break;
		}*/
	
		if (_kbhit()) {//如果有按键按下，则_kbhit()函数返回真

			if (!mode) {
				ch = _getch();//使用_getch()函数获取按下的键值

				str[4] = '\0';
				str[count] = ch;
				count++;
				if (str[4] != '\0') {
					if (port.WriteData(str, count)) {
						std::cout << "send succeeded. mode:" << mode << std::endl;
					}
					else {
						std::cout << "send failed. mode:" << mode << std::endl;
					}
					std::cout << str[0] << " end: " << str[4] << std::endl;
					count = 0;
					memset(str, '\0', sizeof(str));
				}
					
				
				if (ch == 110) {
					mode = 1;
					std::cout << "mode change to 1(NUM)." << std::endl;
				}
				if (ch == 27) {
					break;
				}
			}
			else {
				ch = _getch();
				strNUM[count] = ch;
				std::cout << strNUM[count];
				count++;
				if (ch == 13) {
					if (port.WriteData(strNUM, count)) {
						std::cout << "send succeeded. mode:"<< mode << std::endl;
					}
					else {
						std::cout << "send failed. mode:" << mode << std::endl;
					}
					if ( strNUM[0] == 107 && strNUM[1] == 13) {
						mode = 0;
						std::cout << "mode change to 0(KEY)." << std::endl;
					}
					std::cout << std::endl;
					count = 0;
					memset(strNUM, '\0', sizeof(strNUM));
				}
				if (ch == 27) {
					break;
				}
			}
		}
	}
	cap.release();
	cv::destroyAllWindows();
	return 0;
}