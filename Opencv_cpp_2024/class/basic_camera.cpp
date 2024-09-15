#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <unistd.h>
//using namespace cv;
int main(int argc, char* argv[])
{
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
	
	
	cv::VideoWriter video("outcpp.avi", cv::VideoWriter::fourcc('M','J','P','G'),30, cv::Size(320,240));


	while(true){
		if(!cam.getVideoFrame(img,99999999)){
			std::cout << "Camera error" << std::endl;
			break;
		}
		// test flip
		cv::flip(img, img, -1);
		
		video.write(img);
		
		cv::imshow("Basic image", img);
		//cv::Mat new_img = img(cv::Range(0,100), cv::Range(0,100));
		//cv::imshow("slice", new_img);
		char key = (char)cv::waitKey(1);
		if(key == 'q'){
			std::cout << "BYE" << std::endl;
			break;
		}
	}
	cam.stopVideo();
	video.release();
	cv::destroyAllWindows();
}
