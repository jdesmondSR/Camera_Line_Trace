// opencv stuff
#include <unistd.h>	// serial comms
#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>

// file writing
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace cv;
using namespace std;






#define WRITENEW 1








// file handle for writing training data
FILE* data_file, *label_file;

// contour compare function
// void contour_compare(contoura contourb)
	// i = get area of one
	// j = get area of the other
	// return (i<j)
bool contour_compare(const vector<Point> &a, const vector<Point> &b) {
	return contourArea(a) < contourArea(b);
}

void save_letter_data(char label, Mat data){
	
	// resize the letter
	resize(data, data, Size(20,20));
	imshow(to_string(label), data);
	
	
	// flatten
	cout << "data size = " << data.size() << endl;
	Mat flat = data.reshape(1,data.total());
	cout << "data size = " << flat.size() << endl;
	
	// write it out to a file...
	if(WRITENEW){
		// write the label
		fprintf(label_file, "%f\n", (float)label);
		
		// write the data
		for(int ct = 0; ct < flat.cols; ct++){
			printf("%d: %f\n", ct, flat.at(0, ct));
		}
	}
	
	
}

void proc_letter(Mat og_img, Mat img){
	cvtColor(img, img, COLOR_BGR2GRAY);
		GaussianBlur(img, img, Size(9,9), 0, 0);
		threshold(img, img, 80, 255, THRESH_BINARY_INV+THRESH_OTSU);
		imshow("thresh", img);
		
		vector<vector<Point>> contours;
		findContours(img, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		sort(contours.begin(), contours.end(), contour_compare);
		
		//int i = 0;
		Mat letter;
		Rect br;
		Mat blank = Mat::zeros(img.size(), CV_8UC1);
		for(auto& cnt : contours){
			double a = contourArea(cnt);
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
			/*
			vector<vector<Point>> letter_contours;
			findContours(rotated_letter, letter_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			sort(letter_contours.begin(), letter_contours.end(), contour_compare);
			//vector<Point> final_letter = letter_contours[0];
			Rect final_letter = boundingRect(letter_contours[0]);
			Mat res = rotated_letter(final_letter);
			//imshow("res", res);
			*/
			
			
			printf("\nWhich letter is it???\n");
			char key = 255;
			Mat save_letter = rotated_letter.clone();
			while(key != 'h' && key != 's' && key != 'u' && key != 'q'){
			
				printf("save or rotate??\n");
				imshow("Save image?", save_letter);
				key = (char)cv::waitKey(0);
				/*
				if(key == 'h'){
					printf("saved\n");
					save_letter_data('h', save_letter);
				}
				else if(key == 's'){
					printf("s saved\n");
					save_letter_data('s', save_letter);
				}
				else if(key == 'u'){
					printf("u saved\n");
					save_letter_data('u', save_letter);
				}
				* */
				if(key == 'h' || key == 's' || key == 'u'){
					// get rid of border around letter
					vector<vector<Point>> letter_contours;
					findContours(save_letter, letter_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
					sort(letter_contours.begin(), letter_contours.end(), contour_compare);
					//vector<Point> final_letter = letter_contours[0];
					Rect final_letter = boundingRect(letter_contours[0]);
					Mat res = save_letter(final_letter);
					
					// save the final sliced and cropped letter data to file
					save_letter_data(key, res);
					
					
				}
				else if(key == 'q'){
					printf("EXIT\n");
				}
				else{
					printf("ROTATING\n");
				}
				
				// rotate the image
				rot_matrix = getRotationMatrix2D(Point2f(save_letter.cols/2.0F, save_letter.rows/2.0F), 90, 1.0);
				//Mat rotated_letter(Size(letter.size().height, letter.size().width), letter.type());
				warpAffine(save_letter, save_letter, rot_matrix, save_letter.size());
				//imshow("rotated letter", rotated_letter);
				
				
			}
			
			// draw single contour
			//drawContours(og_img, vector<vector<Point>>(1,cnt), -1, Scalar(255, 0, 0), 2);
			
		}
		
		
		
		cv::imshow("Basic image", og_img);
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
	
	
	// open the file for writing
	if(WRITENEW){
		data_file = fopen("letter_data.txt", "w");
		if(data_file == NULL){
			printf("ERROR opening data_file...\n");
			return 0;
		}
		
		
		label_file = fopen("letter_label.txt", "w");
		if(label_file == NULL){
			printf("ERROR opening label_file...\n");
			return 0;
		}
		
		//fprintf(data_file, "TESTING\n");
		//fprintf(label_file, "TESTING\n");
		
		//else{
		//	fprintf(data_file, "TESTING\n");
		//}	
	}
	
	
	while(true){
		if(!cam.getVideoFrame(img,99999999)){
			std::cout << "Camera error" << std::endl;
			break;
		}
		Mat og_img = img.clone();
		// test flip
		//cv::flip(img, img, -1);
		
		
		//cv::Mat new_img = img(cv::Range(0,100), cv::Range(0,100));
		//cv::imshow("slice", new_img);
		char key = (char)cv::waitKey(1);
		if(key == 'q'){
			std::cout << "BYE" << std::endl;
			break;
		}
		else if(key == ' '){
			proc_letter(og_img, img);
		}
	}
	
	if(WRITENEW){
		fclose(data_file);
		fclose(label_file);
	}
	cam.stopVideo();
	cv::destroyAllWindows();
	
	
	return 0;
	
}
