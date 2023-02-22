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
	cap.open("http://unitv2.py/video_feed");
	//cap.open(0);
	if (!cap.isOpened()) {
		std::cout << "Can't open camera." << std::endl;
		std::cin.get();
		return 1;
	}
	//cv::namedWindow(WINDOW_NAME, cv::WINDOW_AUTOSIZE);
	int ch;
	CSerialPort port;
	if (!port.InitPort(5,CBR_9600,'N',8,1,EV_RXCHAR)) {
		std::cout << "Init serial port succeeded." << std::endl;
	}
	else {
		std::cout << "Init serial port failed." << std::endl;
	}

	int mode = 1;
	int count = 0;
	unsigned char* strNUM = new unsigned char[20];
	while (true) {
		cv::Mat img;
		if (cap.read(img)) {
			cv::imshow(WINDOW_NAME, img);
			if (cv::waitKey(1) == 27) break;
		}
	
		if (_kbhit()) {//����а������£���_kbhit()����������

			if (!mode) {
				ch = _getch();//ʹ��_getch()������ȡ���µļ�ֵ

				unsigned char* str = new unsigned char[1];
				str[0] = ch;

				if (port.WriteData(str, 1)) {
					std::cout << "send succeeded. mode:" << mode << std::endl;
				}
				else {
					std::cout << "send failed. mode:" << mode << std::endl;
				}

				std::cout << str[0]<< std::endl;
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
			}
		}
	}
	cap.release();
	cv::destroyAllWindows();
	return 0;
}