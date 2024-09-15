// Line trace with pi camera


// includes for opencv stuff
// includes for the serial communication and other stuff
#include <lccv.hpp>
#include <iostream>
#include <string.h>	// c
#include <string>	// c++
#include <errno.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <termios.h>                                                         
#include <stdio.h>
#include <stdlib.h>	                                                     
#include <fcntl.h>                                                                                                               
#include <sys/types.h> 
#include <stdint.h>
#include <sys/signal.h>
#include <time.h>
#include <stdbool.h>	

#include <opencv2/opencv.hpp>
#include <sstream> 

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>



#define BAUDRATE B115200                                                      
#define MODEMDEVICE "/dev/ttyAMA0"



using namespace std::chrono_literals;
using namespace std;
using namespace cv;



// buffer for sprintf
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


int uart0_filestream = -1;
void init(){
	// connect to the arduino device
	uart0_filestream = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(-1);
	}
	
	// config stuff
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}	




std::string camTitle(int num){
    std::string title("Video");
    title += std::to_string(num);
    return title;
}

void line_trace(){
	// create video capture object
	/*
	VideoCapture cap(CAP_V4L2);
	if (!cap.isOpened()) {
		printf("Failed to access camera!!\n\n");
		while (1);// dont continue
	}
	*/

	// setting new resolution (default 640x480)
	// img.cols = 640 and img.rows = 480 by default
	// NOTE
	// width = colum = x
	// height = row = y
	//cap.set(CAP_PROP_FRAME_WIDTH, 320);
	//cap.set(CAP_PROP_FRAME_HEIGHT, 240);
	//cv::Mat cam_im;

    lccv::PiCamera cameras[1];
    int index = 0;
    for (auto& cam : cameras){
        cam.options->camera = index++;
        cam.options->video_width=320;//640;
        cam.options->video_height=240;//480;
        cam.options->framerate=30;
        cam.options->verbose=true;
        cam.startVideo();
        auto title = camTitle(cam.options->camera);
        std::cout << title << " Started:"<< std::endl;
        cv::namedWindow(title,cv::WINDOW_NORMAL);
    }

	// create image matrix
	Mat img;//, slice1, slice2, blur, thresh, canny;
	//Rect roi(100, 100, 200, 200); //slicing method
	
	
	// PID stuff
	int target_speed = 20, motor_change = 0, error = 0, total_error = 0, previous_error = 0;
	int targetX = 160;
	// kp of .30 and .35 works well
	float kp = 0.30, ki = 0, kd = 0;
	
	
	
	
	 
	// loop
	char input = 0;
	int leftmotor, rightmotor;
	bool stop_motors = 1;
	int counter = 0;
	
	
	// main loop
	do{
		
		// old version
		//cap >> img;
		
		// new ppi 5 version
		for (auto& cam : cameras){
            auto title = camTitle(cam.options->camera);
            if(!cam.getVideoFrame(img,99999999)){
                std::cout<<"Timeout error " << title << "\t" <<counter <<std::endl;
                counter++;            
            }
            else{
                //cv::imshow(title.data(),img);
            }
		}
		
		// flip the image? depends on the camera mount
		//flip(img, img, -1);
		
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
		circle(img, Point(160, 120), 10, Scalar(0, 255, 0), FILLED); 	// center of the image itself
		circle(img, Point(cx, cy), 5, Scalar(255,0,0), FILLED);			// center of the contour
		imshow("Main Image", img);
		
		
		
		// checking the cy value of the moment
		//printf("cy: %4d\n", cy);
		
		// when cy gets above 200, it means the line is about to go out of sight of the camera view
		


		// calculate speed
		error = cx - targetX;
		
		total_error += error;
		if(abs(error) < 40){
			total_error = 0;
		}
		
		motor_change = (int)(error*kp);// + (int)(total_error*ki) + (int)((error - previous_error) * kd);
		
		
		leftmotor = target_speed - motor_change;
		rightmotor = target_speed + motor_change;
		
		previous_error = error;
		
		
		
		// set the motor speed
		if(stop_motors == 1){
			leftmotor = 0;
			rightmotor = 0;
		}
		
		// send the motor speed
		char tx_buffer[40];
		sprintf(tx_buffer, "[%d, %d]", leftmotor, rightmotor);
		int count = write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
		{
			printf("UART TX error\n");
		}else{
			//printf("UART TX ok\n");
			//printf("count: %d\n", count);
		}
		printf("Sent the following string: %s\n", tx_buffer);
		
		
		// get keyboard input
		input = waitKey(1);
		
		if(input == ' '){
			stop_motors = 1;
		}else if(input == 'g'){
			stop_motors = 0;
		}
		
		
		//delay(100);
		
	} while (input != 'q');
	
	char tx_buffer[20];
	sprintf(tx_buffer, "[%d, %d]", 0, 0);
	int count = write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));		//Filestream, bytes to write, number of bytes to write
	if (count < 0)
	{
		printf("UART TX error\n");
	}else{
		//printf("UART TX ok\n");
		//printf("count: %d\n", count);
	}
	printf("Sent the following string: %s\n", tx_buffer);
	
	// close cameras and stuff
	for (auto& cam : cameras){
        auto title = camTitle(cam.options->camera);
        
        std::cout << title << std::endl;
        sleep(2);
        cv::destroyWindow(title);
        sleep(2);
        cam.stopVideo();
        sleep(2);
    }
}




int main(){
	init();
	
	// should be all set by here
	printf("Serial is up and running\n");

	// wait until pico starts up and is ready
	this_thread::sleep_for(1000ms);
	// Read up to 255 characters from the port if they are there
	unsigned char rx_buffer[256];
	int rx_length = 0;
		
	printf("Waiting for pico to start...\n");
		
	while (rx_length <=0)  								//remove the while to make this non-blocking
		rx_length = read(uart0_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		
	// data recieved, meaning the pico is ready to go
	rx_buffer[rx_length] = '\0';
	printf("Start data recieved: %s\n", rx_buffer);
		
	if (rx_length < 0)
	{
		//An error occured (will occur if there are no bytes)
	}
	else if (rx_length == 0)
	{
		//No data waiting - if we are non-blocking
	}
	else
	{
		//Bytes received
		rx_buffer[rx_length] = '\0';
		printf("%i bytes read : %s", rx_length, rx_buffer);
	}

	printf("Starting line trace\n");
	
	
	
	// pico is ready, start line trace
	line_trace();
	
	
	
	
	


	return 0;
}
