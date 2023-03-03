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
bool autocontrol = false;
int direction = 0; // 0:left 1:right 2:middel
int markx = 0;
int marky = 0;
int ox;
int oy;
float markf = 0;
int maxSpeed = 65;
int minSpeed = 21;
int turningAngle = 5;
int BLcount = 0;
int BRcount = 0;
int speedL = 0;
int speedR = 0;
int bound = 720 / 2;
#pragma warning(disable : 4996)

void sendData() {

	while (1) {
		string str = to_string(speedR) + "," + to_string(speedL) + "\n";
		DWORD length;
		WriteFile(com, str.c_str(), str.length(), &length, &sendOverlapped);
		//cout << "send:" << str << endl;
		Sleep(200);
	}

	// dont know why,but when pass the positive value to the port, it update rate will be weird.
	/*unsigned char* senddata = (unsigned char*)str.data();
	if (port.WriteData(senddata, sizeof(senddata))) {
		cout << "send:" << senddata << endl;
	}
	else {
		cout << "send failed." << endl;
	}*/
}

void calculateSpeed(cv::Mat img, int x, int y) {
	ox = img.cols / 2;
	oy = img.rows;
	for (int i = img.rows * 0.85; i < img.rows * 0.88; i++) { //y
		cv::Vec3b* p = img.ptr<cv::Vec3b>(i);
		for (int j = 0; j < img.cols; j++) { // x
			if (p[j][0] == 255) {
				ox = j;
				oy = i;
				break;
			}
		}
	}
	cv::Point origin = cv::Point(ox, oy);
	markx = x;
	marky = y;
	float end1 = 0.1;
	if (x > ox+end1*img.cols) {
		direction = 1;
	}
	else if (x > ox - img.cols * end1 && x < ox + img.cols * end1) {
		direction = 2;
	}
	else {
		direction = 0;
	}
	//if (y > img.rows * 0.5) {
	//	if (BLcount > BRcount) {
	//		speedL = 20;
	//		speedR = 60;
	//	}
	//	else if (BLcount < BRcount) {
	//		speedL = 60;
	//		speedR = 20;		
	//	}
	//	return;
	//}
	int edge0 = abs(origin.x - x);
	//int edge1 = abs(origin.y - y);
	//int edge2 = sqrt(pow(edge0, 2) + pow(edge1, 2));
	float percent = (origin.x-img.cols/2)/(img.cols/2.0);
	markf = percent;
	if (direction == 0) {
		int s = turningAngle * percent;
		speedL = minSpeed + s;
		speedR = maxSpeed;
		if (speedL < 12)
			speedL = 12;
	}
	else if (direction == 1) {
		int s = turningAngle * percent;
		speedL = maxSpeed;
		speedR = minSpeed + s;
		if (speedR < 12)
			speedR = 12;
	}
	else if (direction == 2) {
		speedL = 30;
		speedR = 30;
	}
	return;
}

void setSpeedByRed(cv::Mat img) {
	bool find = false;
	int minx = img.cols;
	int miny = img.rows;
	int maxx = 0;
	int maxy = 0;
	for (int i = img.rows*0.45; i < img.rows*0.75; i++) { //y
		cv::Vec3b* p = img.ptr<cv::Vec3b>(i);
		for (int j = 0; j < img.cols; j++) { // x
			if (p[j][0] == 255) {
				find = true;
				calculateSpeed(img, j, i);
				break;
			}
		}
		if (find)
			break;
	}
	if (!find) {
		speedL = 0;
		speedR = 0;
	}
}


void speedControl(int event, int x, int y, int flags, void* param) {
	
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE: // CV_EVENT_MOUSEMOVE
	{
		//cout << x << ":" << y << endl;
		if (if_Lmousedown && !autocontrol) {
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
		if (if_Rmousedown && !autocontrol) {
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
		if (!autocontrol) {
			speedR = maxSpeed;
			speedL = maxSpeed;
		}
		//sendData();
		break;
	case CV_EVENT_LBUTTONUP:
		if_Lmousedown = false;
		if (!autocontrol) {
			speedR = 0;
			speedL = 0;
		}
		//sendData();
		break;
	case CV_EVENT_RBUTTONDOWN:
		if_Rmousedown = true;
		if (!autocontrol) {
			speedR = -maxSpeed;
			speedL = -maxSpeed;
		}
		//sendData();
		break;
	case CV_EVENT_RBUTTONUP:
		if_Rmousedown = false;
		if (!autocontrol) {
			speedR = 0;
			speedL = 0;
		}
		//sendData();
		break;
	case CV_EVENT_MBUTTONDOWN:
		if (autocontrol) {
			autocontrol = false;
			speedR = 0;
			speedL = 0;
		}
		else
			autocontrol = true;
		cout << autocontrol << endl;
	default:
		break;
	}
}

void setColor(cv::Vec3b* img,int index, int b, int g, int r) {
	img[index][0] = b; //blue
	img[index][1] = g; //green
	img[index][2] = r; //red
}

void setColor(uchar* img, int index, int v) {
	img[index] = v;
}

int main() {
	
	cap.open("http://unitv2.py/video_feed");
	//cap.open(0);
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
	
	//cv::Mat test = cv::imread("1.png");
	

	thread sending(sendData);
	while (true) {
		cv::resizeWindow(WINDOW_NAME, cv::Size(720, 480));
		cv::Mat img;
		if (cap.read(img)) {

			if (autocontrol) {
				BLcount = 0;
				BRcount = 0;
				cv::Mat redline = img.clone();
				cv::Mat blackline = img.clone();
				//cv::cvtColor(redline, redline, cv::COLOR_BGR2GRAY);
				for (int i = 0; i < img.rows; i++) { //y
					cv::Vec3b* pOrigin = redline.ptr<cv::Vec3b>(i);
					//BGR TYPE
					cv::Vec3b* p3 = redline.ptr<cv::Vec3b>(i);
					//cv::Vec3b* p3b = blackline.ptr<cv::Vec3b>(i);

					for (int j = 0; j < img.cols; j++) { // x
						if (pOrigin[j][0] < 120 && pOrigin[j][1] < 120 && pOrigin[j][2] > 150) {
							//redline.at<uchar>(i, j) = 255;
							setColor(p3, j, 255, 255, 255);
							//setColor(p3b, j, 0, 0, 0);
						}
						//else if (pOrigin[j][0] < 120 && pOrigin[j][1] < 120 && pOrigin[j][2] < 120) {
						//	//redline.at<uchar>(i, j) = 0;
						//	setColor(p3, j, 0, 0, 0);
						//	//setColor(p3b, j, 255, 255, 255);
						//	//if (j > img.cols / 2) {
						//	//	BRcount++;
						//	//}
						//	//else {
						//	//	BLcount++;
						//	//}
						//}
						else {
							//redline.at<uchar>(i, j) = 0;
							setColor(p3, j, 0, 0, 0);
							//setColor(p3b, j, 0, 0, 0);
						}
					}
				}
				setSpeedByRed(redline);
			}

			// putText into picture
			cv::String str = "L:" + to_string(speedL) + "  R:" + to_string(speedR) + " Dir:" + to_string(direction)
				+ " x:"+to_string(markx)+",y:"+to_string(marky) + " factor:"+to_string(markf);
			cv::Point origin;
			int baseline;
			cv::Size size = cv::getTextSize(str, cv::FONT_HERSHEY_SIMPLEX, 1, 2,&baseline);
			origin.x = 0;
			origin.y = size.height;
			cv::putText(img, str, origin, cv::FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2, 8, 0);
			 
			cv::rectangle(img, cv::Point(markx - 10, marky - 10), cv::Point(markx + 10, marky + 10), (255, 0, 0), 6);
			cv::rectangle(img, cv::Point(ox - 10, oy - 10), cv::Point(ox + 10, oy + 10), (0, 125, 0),2);
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
// 	/*if (!port.InitPort(5,CBR_115200,'N',8,1,EV_RXCHAR)) {
//std::cout << "Init serial port succeeded." << std::endl;
//	}
//	else {
//	std::cout << "Init serial port failed." << std::endl;
//	}*/
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