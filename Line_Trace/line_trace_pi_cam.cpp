// Line trace with pi camera


// includes for opencv stuff


// includes for the serial communication and other stuff
/*
#include <iostream>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <chrono>
#include <thread>

*/

#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sstream> 

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>

//using namespace std::chrono_literals;
using namespace std;
using namespace cv;



// being lazy for now
int fd;
char buf[100];


// used to sort contours (might not need this during real run)
// contour compare function
// void contour_compare(contoura contourb)
	// i = get area of one
	// j = get area of the other
	// return (i<j)
bool contour_compare(const vector<Point> &a, const vector<Point> &b) {
	return contourArea(a) < contourArea(b);
}

void line_trace(){
	// create video capture object
	VideoCapture cap(CAP_V4L2);
	if (!cap.isOpened()) {
		printf("Failed to access camera!!\n\n");
		while (1);// dont continue
	}

	// setting new resolution (default 640x480)
	// img.cols = 640 and img.rows = 480 by default
	// NOTE
	// width = colum = x
	// height = row = y
	cap.set(CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CAP_PROP_FRAME_HEIGHT, 240);

	// create image matrix
	Mat img;//, slice1, slice2, blur, thresh, canny;
	//Rect roi(100, 100, 200, 200); //slicing method
	
	
	// PID stuff
	int target_speed = 20, motor_change = 0, error = 0;
	int targetX = 160;
	float kp = 0.25;
	
	
	
	
	 
	// loop
	char input = 0;
	int leftmotor, rightmotor;
	bool stop_motors = 1;
	do{
		cap >> img;
		
		Mat proc = img.clone();
		cvtColor(proc, proc, COLOR_BGR2GRAY);
		GaussianBlur(proc, proc, Size(7,7), 0, 0);
		
		threshold(proc, proc, 30, 255, THRESH_BINARY_INV | THRESH_OTSU);
		//threshold(blur, thresh, 127, 255, THRESH_BINARY_INV);
		
		// show binary processed image
		imshow("proc", proc);
		
		
		// find contours
		vector<vector<Point>> contours;
		findContours(proc, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		
		
		// ------------------------ DEBUG --------------------------
		// sort by contour area
		/*
		printf("\nSorted %zu contours\n", contours.size());
		sort(contours.begin(), contours.end(), contour_compare);
		for (auto& cnt : contours) {
			printf("Contour area: %f\n", contourArea(cnt));
		}
		printf("\n\nDONE\n\n");
		*/
		
		// max element returns pointer so dereference here
		vector<Point> line = *max_element(contours.begin(), contours.end(), contour_compare);
		
		// bounding rect gets geometric center
		// moments is the center of mass
		Moments m = moments(line);
		
		// get center x and center y
		int cx = m.m10 / m.m00; // col
		int cy = m.m01 / m.m00; // row
		
		printf("\nMoments: %d, %d\n\n", cx, cy);
		
		// draw the new center over img
		circle(img, Point(160, 120), 10, Scalar(0, 255, 0), FILLED);
		circle(img, Point(cx, cy), 5, Scalar(255,0,0), FILLED);
		imshow("Main Image", img);
		
		
		


		// calculate speed
		error = cx - targetX;
		
		motor_change = (int)(error*kp);
		
		
		leftmotor = target_speed - motor_change;
		rightmotor = target_speed + motor_change;
		
		
		
		// set the motor speed
		if(stop_motors == 1){
			leftmotor = 0;
			rightmotor = 0;
		}
		
		// send the motor speed
		sprintf(buf, "[%d, %d]", leftmotor, rightmotor);
		printf("buf.size() = %d\n", strlen(buf));
		write(fd, buf, strlen(buf));
		printf("Sent the following string: %s\n", buf);
		
		
		// get keyboard input
		input = waitKey(1);
		
		if(input == ' '){
			stop_motors = 1;
		}else if(input == 'g'){
			stop_motors = 0;
		}
		
		
		//delay(100);
		
	} while (input != 'q');
	
	sprintf(buf, "[%d, %d]", 0, 0);
	printf("buf.size() = %d\n", strlen(buf));
	write(fd, buf, strlen(buf));
	printf("Sent the following string: %s\n", buf); 
	
}




int main(){
	if ((fd = serialOpen ("/dev/ttyS0", 9600)) < 0)
	{
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}


	// wait until pico starts up and is ready
	std::cout << fd << std::endl;
	int numchars;
	//char buf[100]; // just make it more than big enough and it will be fine
	this_thread::sleep_for(1000ms);
	while( (numchars = serialDataAvail(fd)) == 0){
		printf("Nothing to be read so far\n");
		this_thread::sleep_for(100ms);
	}
	printf("Data recieved\n");
	read(fd, buf, numchars);
	buf[numchars+1] = '\0';
	printf("Data read: %s\n", buf);
	this_thread::sleep_for(1000ms);

	
	
	
	// pico is ready, start line trace
	line_trace();
	
	
	
	
	/*
	// testing 
	while (true){
		// start command has been recieved
		// start line tracing and sending motor commands to the cytron board
		
		
		leftmotor = 100;
		rightmotor = -100;
		sprintf(buf, "[%d, %d, %d, %d]", abs(leftmotor), leftmotor, abs(rightmotor), rightmotor);
		printf("buf.size() = %d\n", strlen(buf));
		write(fd, buf, strlen(buf));
		printf("Sent the following string: %s\n", buf);
		this_thread::sleep_for(1000ms);
	}

	cout << numchars<<endl;
	*/

	serialClose (fd) ;

	return 0;
}
