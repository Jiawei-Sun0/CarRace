#include <iostream>
#include <string>
#include <thread>
#include <Windows.h>
#include <windows.h>
#include <conio.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include "nkcOpenCV.h"
#include "SerialPort.h"
using namespace std;
//#pragma comment( linker,"/subsystem:windows /entry:mainCRTStartup")
const cv::String WINDOW_NAME = "M5Camera Streaming";
cv::VideoCapture cap;
//CSerialPort port;
HANDLE com;
OVERLAPPED sendOverlapped, recieveOverlapped;
bool if_Lmousedown = false;
bool if_Rmousedown = false;
char mark = 'p';
int maxSpeed = 80;
int minSpeed = 20;
int speedL = 0;
int speedR = 0;
int bound = 720 / 2;
#pragma warning(disable : 4996)

void sendData() {
	// dont know why,but when pass the positive value to the port, it update rate will be weird.
	while (1) {
		string str = to_string(speedR) + "," + to_string(speedL) + "\n";
		DWORD length;
		WriteFile(com, str.c_str(), str.length(), &length, &sendOverlapped);
		cout << "send:" << str << endl;
		Sleep(100);
	}

	/*unsigned char* senddata = (unsigned char*)str.data();
	if (port.WriteData(senddata, sizeof(senddata))) {
		cout << "send:" << senddata << endl;
	}
	else {
		cout << "send failed." << endl;
	}*/
}


void speedControl(int event, int x, int y, int flags, void* param) {
	
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE: // CV_EVENT_MOUSEMOVE
	{
		//cout << x << ":" << y << endl;
		if (if_Lmousedown) {
			if (x > bound) {
				speedR = maxSpeed - (maxSpeed - minSpeed) * (x - bound) / bound;
				if (speedR < minSpeed)
					speedR = minSpeed;
				//cout << speedR << endl;
			}
			else if (x < bound)
			{
				speedL = maxSpeed - (maxSpeed - minSpeed) * (bound - x) / bound;
				if (speedL < minSpeed)
					speedL = minSpeed;
				//cout << speedL << endl;
			}
			//sendData();
		}
		if (if_Rmousedown) {
			if (x > bound) {
				speedR = -maxSpeed + (maxSpeed - minSpeed) * (x - bound) / bound;
				if (speedR > -minSpeed)
					speedR = -minSpeed;
				//cout << speedR << endl;
			}
			else if (x < bound)
			{
				speedL = -maxSpeed + (maxSpeed - minSpeed) * (bound - x) / bound;
				if (speedL > -minSpeed)
					speedL = -minSpeed;
				//cout << speedL << endl;
			}
			//sendData();
		}
		break;
	}
	case CV_EVENT_LBUTTONDOWN: // CV_EVENT_LBUTTONDOWN
		if_Lmousedown = true;
		speedR = maxSpeed;
		speedL = maxSpeed;
		//sendData();
		break;
	case CV_EVENT_LBUTTONUP:
		if_Lmousedown = false;
		speedR = 0;
		speedL = 0;
		//sendData();
		break;
	case CV_EVENT_RBUTTONDOWN:
		if_Rmousedown = true;
		speedR = -maxSpeed;
		speedL = -maxSpeed;
		//sendData();
		break;
	case CV_EVENT_RBUTTONUP:
		if_Rmousedown = false;
		speedR = 0;
		speedL = 0;
		//sendData();
		break;
	default:
		break;
	}
}
int main() {
	
	//cap.open("http://unitv2.py/video_feed");
	cap.open(0);
	if (!cap.isOpened()) {
		std::cout << "Can't open camera." << std::endl;
		std::cin.get();
		return 1;
	}
	cv::namedWindow(WINDOW_NAME, cv::WINDOW_NORMAL);
	cv::resizeWindow(WINDOW_NAME, cv::Size(720, 480));
	cv::setMouseCallback(WINDOW_NAME, speedControl);
	
	com = CreateFile(_T("COM5"), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (com == INVALID_HANDLE_VALUE) {
		cout << "open failed." << endl;
	}
	
	DCB dcb; // シリアルポ�`トの��撹秤�鵑�秘る��夛悶
	GetCommState(com, &dcb); // �F壓の�O協�､鰌iみ�zみ // DCB ��夛悶のメンバの匯何だけを�O協する��栽でも、麿のメンバも�m俳な�､砲垢襪燭瓠�GetCommState �v方を聞っていったん DCB ��夛悶を�O協してから、��輝するメンバの�､鯰渾�するようにします。(msdn) // つまり畠て�O協する��栽は音勣ではあるが、芦畠のために匯業�iみ�zんでおく��(書指は匯何しか�O協しないので駅勣)

	dcb.BaudRate = 9600; // 堀業
	dcb.ByteSize = 8; // デ�`タ�L
	dcb.Parity = NOPARITY; // パリティ
	dcb.StopBits = ONESTOPBIT; // ストップビット�L
	dcb.fOutxCtsFlow = FALSE; // 僕佚�rCTSフロ�`
	dcb.fRtsControl = RTS_CONTROL_DISABLE;//RTS_CONTROL_ENABLE; // RTSフロ�` // なし
	dcb.EvtChar = NULL;//つまり仝\0々に揖じ//ここで峺協した猟忖双を鞭佚した�r仝EV_RXFLAG々イベントが�k伏する。

	SetCommState(com, &dcb); // �筝�した�O協�､���き�zみ
	
	
	/*if (!port.InitPort(5,CBR_115200,'N',8,1,EV_RXCHAR)) {
		std::cout << "Init serial port succeeded." << std::endl;
	}
	else {
		std::cout << "Init serial port failed." << std::endl;
	}*/

	thread sending(sendData);
	while (true) {
		cv::resizeWindow(WINDOW_NAME, cv::Size(720, 480));
		cv::Mat img;
		if (cap.read(img)) {
			cv::imshow(WINDOW_NAME, img);
			if (cv::waitKey(1) == 27) break;
		}
	}

	cap.release();
	cv::destroyAllWindows();
	return 0;
}

// KEY AND NUM MODE CODE
//
//int ch;
//int mode = 1;
//int count = 0;
//unsigned char* str = new unsigned char[5];// 喇噐方象俊鞭寂侯��5厚嬬涙血藁俊��旺繍writedata議隠擦産喝曝個葎2
//unsigned char* strNUM = new unsigned char[20];
// 
//if (_kbhit()) {
//
//	if (!mode) {
//		ch = _getch();
//
//		str[4] = '\0';
//		str[count] = ch;
//		std::cout << str[count];
//		count++;
//
//		if (str[4] != '\0') {
//			if (port.WriteData(str, count)) {
//				std::cout << " send succeeded. mode:" << mode << std::endl;
//			}
//			else {
//				std::cout << " send failed. mode:" << mode << std::endl;
//			}
//			count = 0;
//			memset(str, '\0', sizeof(str));
//		}
//
//
//		if (ch == 110) {
//			str[0] = ch;
//			port.WriteData(str, 1);
//			count = 0;
//			memset(str, '\0', sizeof(str));
//			mode = 1;
//			std::cout << "mode change to 1(NUM)." << std::endl;
//		}
//		if (ch == 27) {
//			break;
//		}
//	}
//	else {
//		ch = _getch();
//		strNUM[count] = ch;
//		std::cout << strNUM[count];
//		count++;
//		if (ch == 13) {
//			if (port.WriteData(strNUM, count)) {
//				std::cout << "send succeeded. mode:" << mode << std::endl;
//			}
//			else {
//				std::cout << "send failed. mode:" << mode << std::endl;
//			}
//			if (strNUM[0] == 107 && strNUM[1] == 13) {
//				mode = 0;
//				std::cout << "mode change to 0(KEY)." << std::endl;
//			}
//			std::cout << std::endl;
//			count = 0;
//			memset(strNUM, '\0', sizeof(strNUM));
//		}
//		if (ch == 27) {
//			break;
//		}
//	}
//}