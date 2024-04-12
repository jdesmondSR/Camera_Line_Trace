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

//#define DEBUG_IMSHOW(name, frame) imshow(name, frame);

#define DEBUG_IMSHOW(name,frame)



using namespace std::chrono_literals;
using namespace std;
using namespace cv;



// buffer for sprintf
char buf[100];

// for double green bounds checking
int RECHECK = 0;


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

// return 1:left 2:right 3:uturn
int green_square(Mat &img){
	// img is just regular image, need to process for hsv and inrange
	
	// low and high range for inrange
	Scalar low_range(70, 107, 58), high_range(100, 255, 160);
	
	// process the image directly
	Mat debug = img.clone();
	Mat line = img.clone();
	cvtColor(img, img, COLOR_BGR2HSV);
	inRange(img, low_range, high_range, img);
	
	// find contours
	vector<vector<Point>> contours;
	findContours(img, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	// sort by contour area
	//printf("\nSorted %zu contours\n", contours.size());
	sort(contours.begin(), contours.end(), contour_compare);
	int left_green_count = 0, right_green_count = 0;
	
	// threshold the line to make it more polarized values
	cvtColor(line, line, COLOR_BGR2GRAY);
	GaussianBlur(line, line, Size(7,7), 0, 0);
	threshold(line, line, 30, 255, THRESH_BINARY_INV | THRESH_OTSU);
	
	for (auto& cnt : contours) {
		// area is typically around 2900 for full green square
		if(contourArea(cnt) > 1500){
			//printf("Checking green contour\n");
			// green square identified, now determine if it is left or right green square (or both i guess)
			Rect br = boundingRect(cnt);
			Point center(br.x + (br.width / 2), br.y + (br.height / 2));
			
			// ***** can use center y to ignore green square untill it gets closer... *****
			
			// debug drawing to see center point
			circle(debug, center, 4, Scalar(0, 0, 255), FILLED);
			
			// now need to use the center to determine if it is false, left, or right
			//printf("center: (%3d,%3d)\n", center.x, center.y);			
			// when the center.x < 160 its on left
			// when the center.x > 160 its on right
			// when center.y gets close to 240 (max height) it is closer to robot
			
			
			// before moving on check if the green square is ON THE EDGE OF THE IMAGE, ignore if it is too far out
			
			// ONLY CHECK THIS IS NO GREEN HAS BEEN DETECED SO FAR.... the second green is always out of bounds
			if(RECHECK == 0){
				if(center.x < 50 || center.x > 270) { //ARBITRARY BOUNDARIES
					printf("x out of bounds");
					continue;
				}
				if(center.y < 120 || center.y + (int)(br.height*(2.0/3)) >= 240) {
					continue;
				}
			}
			
			
			// checking individual points around the green square
			// CHECK LEFT OF GREEN
			Point check_left(center.x - (int)(br.width*(2.0/3)), center.y);
			circle(debug, check_left, 3, Scalar(255, 0, 0), FILLED);			// BLUE
			
			// CHECK RIGHT OF GREEN
			Point check_right(center.x + (int)(br.width*(2.0/3)), center.y);
			circle(debug, check_right, 3, Scalar(0, 255, 0), FILLED);			// GREEN
			
			// CHECK ABOVE GREEN
			Point check_above(center.x, center.y - (int)(br.height*(2.0/3)));
			circle(debug, check_above, 3, Scalar(255, 255, 0), FILLED);			// CYAN
			
			// CHECK BELOW OF GREEN
			Point check_below(center.x, center.y + (int)(br.height*(2.0/3)));
			circle(debug, check_below, 3, Scalar(255, 0, 255), FILLED);			// PURPLE
			
			
			// check each side for the line
			int left_check_color = line.at<unsigned char>(check_left);
			//printf("left check: %d\n", left_check_color);
			int right_check_color = line.at<unsigned char>(check_right);
			//printf("right check: %d\n", right_check_color);
			int above_check_color = line.at<unsigned char>(check_above);
			//printf("above check: %d\n", above_check_color);
			int below_check_color = line.at<unsigned char>(check_below);
			//printf("below check: %d\n", below_check_color);
			
			// basic if/else version
			if(above_check_color == 0){
				printf("FALSE FALSE FALSE FALSE\n");
			}
			else {
				//if(above_check_color == 255 && right_check_color == 255){
				if(below_check_color == 0 && right_check_color == 255){
					printf("LEFT GREEN DETECTED\n");
					left_green_count++;
				}
				if(below_check_color == 0 && left_check_color == 255){
					printf("RIGHT GREEN DETECTED\n");
					right_green_count++;
				}
			}
			
			
			//printf("GREEN SQUARE FOUND: area: %f\n", contourArea(cnt));
		}
	}
	
	// make final decision
	if(left_green_count && right_green_count){
		printf("double green\n");
		return 3;
	}else if(left_green_count){
		printf("left green only\n");
		return 1;
	}else if(right_green_count){
		printf("right green only\n");
		return 2;
	}
	
	
	DEBUG_IMSHOW("green center", debug);
	//printf("\n\nDONE\n\n");
	return 0;
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
	int target_speed = 20, motor_change = 0, error = 0;//, total_error = 0, previous_error = 0;
	int targetX = 160;
	// kp of .30 and .35 works well
	float kp = 0.35;//, ki = 0, kd = 0;
	const float base_kp = kp;
	
	
	
	
	 
	// loop
	char input = 0;
	int leftmotor, rightmotor;
	bool stop_motors = 0;
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
		
		// first check for green square
		Mat g_img = img.clone();
		Mat g_img_recheck = img.clone();
		int green_result = green_square(g_img);
		if(green_result){
			// green detected, do stuff...
			RECHECK = 1;
			
			// forward a little bit then stop
			char tx_buffer[40];
			sprintf(tx_buffer, "[%d, %d]", 20, 20);
			write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
			this_thread::sleep_for(600ms);									// arbitrary forward amount (might be too long...)
			/*
			sprintf(tx_buffer, "[%d, %d]", 0, 0);
			write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
			this_thread::sleep_for(1000ms);
			*/
			
			// recheck green for all possible values
			//printf("\n\nRECHECK THE GREEN.........\n");
			green_result = green_square(g_img_recheck); //1Left 2Right 3Uturn
			
			// done doing checks
			RECHECK = 0;
			
			if(green_result == 1){
				// left turn
				sprintf(tx_buffer, "[%d, %d]", 20, 20);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(600ms);	
				sprintf(tx_buffer, "[%d, %d]", -50, 50);								// ???????????????????????????
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(350ms);
				// stop
				/*
				sprintf(tx_buffer, "[%d, %d]", 0, 0);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(1000ms);
				*/
				
			}else if(green_result == 2){
				// right turn
				sprintf(tx_buffer, "[%d, %d]", 20, 20);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(600ms);
				sprintf(tx_buffer, "[%d, %d]", 50, -50);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(350ms);
				// stop
				/*
				sprintf(tx_buffer, "[%d, %d]", 0, 0);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(1000ms);
				*/
				
			}else if(green_result == 3){
				// turn around
				sprintf(tx_buffer, "[%d, %d]", 20, 20);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(600ms);
				sprintf(tx_buffer, "[%d, %d]", -50, 50);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(900ms);
				// stop
				/*
				sprintf(tx_buffer, "[%d, %d]", 0, 0);
				write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));
				this_thread::sleep_for(1000ms);
				*/		
			}
			
		}
		//imshow("g stuff", g_img);
		
		
		Mat proc = img.clone();
		cvtColor(proc, proc, COLOR_BGR2GRAY);
		GaussianBlur(proc, proc, Size(7,7), 0, 0);
		
		threshold(proc, proc, 30, 255, THRESH_BINARY_INV | THRESH_OTSU);
		//threshold(blur, thresh, 127, 255, THRESH_BINARY_INV);
		
		// show binary processed image
		//imshow("proc", proc);
		
		
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
		
		//printf("\nMoments: %d, %d\n\n", cx, cy);
		
		// draw the new center over img
		circle(img, Point(160, 120), 10, Scalar(0, 255, 0), FILLED); 	// center of the image itself
		circle(img, Point(cx, cy), 5, Scalar(255,0,0), FILLED);			// center of the contour
		DEBUG_IMSHOW("Main Image", img);
		
		
		
		// checking the cy value of the moment
		//printf("cy: %4d\n", cy);
		
		// when cy gets above 200, it means the line is about to go out of sight of the camera view
		if(cy > 210){
			kp = 0.7;
		}
		else{
			kp = base_kp;
		}


		// calculate speed
		error = cx - targetX;
		
		/*
		total_error += error;
		if(abs(error) < 40){
			total_error = 0;
		}
		* */
		
		motor_change = (int)(error*kp);// + (int)(total_error*ki) + (int)((error - previous_error) * kd);
		
		
		leftmotor = target_speed + motor_change;
		rightmotor = target_speed - motor_change;
		
		//previous_error = error;
		
		
		
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
		//printf("Sent the following string: %s\n", tx_buffer);
		
		
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
