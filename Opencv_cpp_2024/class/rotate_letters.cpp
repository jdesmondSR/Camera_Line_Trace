// opencv stuff
#include <unistd.h>	// serial comms
#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace cv;
using namespace std;


// contour compare function
// void contour_compare(contoura contourb)
	// i = get area of one
	// j = get area of the other
	// return (i<j)
bool contour_compare(const vector<Point> &a, const vector<Point> &b) {
	return contourArea(a) < contourArea(b);
}

int main(){
	// start the cam and stuff
	cv::Mat img;
	lccv::PiCamera cam;

	cam.options->camera = 0;
    cam.options->video_width=320;//640;
    cam.options->video_height=240;//480;
	cam.options->framerate=30;
	cam.options->verbose=true;
	cam.startVideo();
	//auto title = std::to_string(cam.options->camera);
	//std::cout << title << " Started:"<< std::endl;
	//cv::namedWindow(title,cv::WINDOW_NORMAL);
	cv::namedWindow("hello",cv::WINDOW_NORMAL);

	
	while(true){
		if(!cam.getVideoFrame(img,99999999)){
			std::cout << "Camera error" << std::endl;
			break;
		}
		Mat og_img = img.clone();
		// test flip
		//cv::flip(img, img, -1);
		cvtColor(img, img, COLOR_BGR2GRAY);
		GaussianBlur(img, img, Size(9,9), 0, 0);
		threshold(img, img, 80, 255, THRESH_BINARY_INV+THRESH_OTSU);
		imshow("thresh", img);
		
		vector<vector<Point>> contours;
		findContours(img, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		//sort(contours.begin(), contours.end(), contour_compare);
		
		int i = 0;
		Mat letter;
		Rect br;
		
		Mat blank = Mat::zeros(img.size(), CV_8UC1);
		for(auto& cnt : contours){
			
			double a = contourArea(cnt);
			printf("cnt %d:  area(%.2f)\n", i, a);
			i++;
			
			// filter with area
			if(!(a < 1100 && a > 300)){
				drawContours(og_img, vector<vector<Point>>(1,cnt), -1, Scalar(0, 0, 255), 1);
				continue;
			}
			
			
			// filter with bounding rect height and width
			br = boundingRect(cnt);
			float ar = (float)br.height/br.width;
			
			printf("ar: %.2f\n", ar);
			
			if(ar > 1.5 || ar < 0.5){
				drawContours(og_img, vector<vector<Point>>(1,cnt), -1, Scalar(0, 255, 255), 1);
				continue;
			}
			
			// should be a letter
			rectangle(og_img, br, Scalar(0, 255, 0), 1);
			
			// redraw contour on blank image to get rid of any extra crap
			drawContours(blank, vector<vector<Point>>(1, cnt), -1, 255, -1);
			imshow("blank", blank);
			
			// slice the letter
			letter = blank(Rect(br.x - 10, br.y-10, br.width+20, br.height+20));
			imshow("letter", letter);
			
			
			RotatedRect rr = minAreaRect(cnt);
			printf("ANGLE: %.2f\n", rr.angle);
			
			cout << "center: " << rr.center << endl;
			
			// warp affine stuff
			//Point2f center(letter.cols/2.0F, letter.rows/2.0F);
			//Mat rot_matrix = getRotationMatrix2D(center, rr.angle, 1.0);
			Mat rot_matrix = getRotationMatrix2D(Point2f(letter.cols/2.0F, letter.rows/2.0F), rr.angle, 1.0);
			Mat rotated_letter(Size(letter.size().height, letter.size().width), letter.type());
			warpAffine(letter, rotated_letter, rot_matrix, letter.size());
			imshow("rotated letter", rotated_letter);
			
			
			// reslice the rotated letter
			vector<vector<Point>> letter_contours;
			findContours(rotated_letter, letter_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			sort(letter_contours.begin(), letter_contours.end(), contour_compare);
			//vector<Point> final_letter = letter_contours[0];
			Rect final_letter = boundingRect(letter_contours[0]);
			Mat res = rotated_letter(final_letter);
			imshow("res", res);
			
			
			
			
			
			// draw single contour
			//drawContours(og_img, vector<vector<Point>>(1,cnt), -1, Scalar(255, 0, 0), 2);
			
		}
		
		
		
		cv::imshow("Basic image", og_img);
		
		//cv::Mat new_img = img(cv::Range(0,100), cv::Range(0,100));
		//cv::imshow("slice", new_img);
		char key = (char)cv::waitKey(1);
		if(key == 'q'){
			std::cout << "BYE" << std::endl;
			break;
		}
	}
	

	cam.stopVideo();
	cv::destroyAllWindows();
	
	
	return 0;
	
}
