#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sstream> 

// g++ -O3 -Wall -std=c++17 -o "pi_opencv_redball" "pi_opencv_redball.cpp" -I/usr/local/include/opencv4  -lwiringPi -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core

using namespace cv;
using namespace std;

bool compareContourAreas (const  std::vector<cv::Point>& contour1, const std::vector<cv::Point>& contour2 ) {
    double i = fabs( contourArea(contour1) );
    double j = fabs( contourArea(contour2) );
    return ( i < j );
}

int OpenSerial(int baud){
	int fd;
	if ((fd = serialOpen ("/dev/ttyS0", baud)) < 0)
	{
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		exit( 1) ;
	}

	if (wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
		exit(1) ;
	}
	return fd;
}

int main() {	
	int fd = OpenSerial(9600);
	this_thread::sleep_for(1000ms);
	
	Mat image;
	namedWindow("Display window");
	VideoCapture cap(CAP_V4L2);

	if (!cap.isOpened()) {
		cout << "cannot open camera";
		exit(-100);
	}

	while (true) {
		cap >> image;

		flip(image,image,-1);
		Mat blur = image.clone();
		Mat inrangeMask;
		Mat hsv;
		GaussianBlur( image, blur, Size( 3, 3 ), 0, 0 );
		
		auto low_hsv = Scalar( 96, 157,  71);
		auto high_hsv = Scalar(179, 220, 255);
		cvtColor(blur,hsv,COLOR_BGR2HSV);
		inRange(hsv, low_hsv, high_hsv, inrangeMask);

		vector<vector<Point> > contours;
		findContours( inrangeMask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
		if (contours.size() > 0){
			//std::sort(contours.begin(), contours.end(), compareContourAreas);
			//std::vector<cv::Point>&  biggestContour = contours.back();  // largest contour

			const auto  biggestContour = std::max_element(contours.begin(), contours.end(), compareContourAreas);
			
			//RotatedRect box = minAreaRect(*biggestContour);
			//Point2f rect_points[4];
			//box.points(rect_points);
			//for ( int j = 0; j < 4; j++ ){
				//line( image, rect_points[j], rect_points[(j+1)%4], Scalar(0,255,0),2 );
			//}
			Point2f center;
			float radius = 0.0;
			minEnclosingCircle(*biggestContour, center, radius);
			circle(image, center, cvRound(radius), Scalar(0, 255, 0), 2, LINE_AA);

			auto x = int(center.x - 320);

			string s = to_string(x) + '\n';
			cout << s;
			write(fd,s.c_str(),s.size());
		}
		else{

			string s = "-400\n";
			cout << s;
			write(fd,s.c_str(),s.size());	
			
		}
		imshow("Display window", image);

		auto key = waitKey(1);
		if (key == 'q') break;

	}
	cap.release();
	destroyAllWindows();
	return 0;

}
