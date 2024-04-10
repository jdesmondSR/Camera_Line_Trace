#include <opencv2/opencv.hpp>

using namespace cv;

using namespace std;

bool compareContourAreas ( std::vector<cv::Point>& contour1, std::vector<cv::Point>& contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}




int main() {

Mat image;

namedWindow("Display window");

VideoCapture cap(CAP_V4L2);

if (!cap.isOpened()) {

cout << "cannot open camera";

}

while (true) {

cap >> image;

flip(image,image,0);
Mat dst = image.clone();
Mat inrangeMask;
GaussianBlur( image, dst, Size( 3, 3 ), 0, 0 );

inRange(dst, Scalar(0, 0, 120), Scalar(50, 50, 255), inrangeMask);

vector<vector<Point> > contours;
findContours( inrangeMask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
if (contours.size() > 0){
	std::sort(contours.begin(), contours.end(), compareContourAreas);

	// grab contours
	std::vector<cv::Point> biggestContour = contours[0];
	
	RotatedRect box = minAreaRect(biggestContour);
	Point2f rect_points[4];
	box.points(rect_points);
	for ( int j = 0; j < 4; j++ ){
		line( image, rect_points[j], rect_points[(j+1)%4], Scalar(0,255,0),2 );
		
	}
	
}
imshow("Display window", image);

auto key = waitKey(1);
if (key == 'q') break;

}
cap.release();
destroyAllWindows();
return 0;

}
