#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
//#include <opencv2\highgui.h>
#include "opencv2/highgui/highgui.hpp"
//#include <opencv2\cv.h>
#include "opencv2/opencv.hpp"


using namespace std;
using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

int **values;
char **colorNames;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

const int TRACKBAR_WINDOW_INDEX = 3;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const std::string windowName = "Original Image";
const std::string windowName1 = "HSV Image";
const std::string windowName2 = "Thresholded Image";
const std::string windowName3 = "After Morphological Operations";

const std::string windowColor1 = "Blue color";
const std::string windowColor2 = "Red color";


const std::string trackbarWindowName = "Trackbars";
const std::string trackbarColorWindowName = "Trackbar color";


void loadColorValues();
void initWithColor(int index);
void parseCoords();

void on_mouse(int e, int x, int y, int d, void *ptr)
{
	if (e == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
}

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
}

string intToString(int number) {
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName,0);
	//create memory to store trackbar name on window
	// char TrackbarName[50];
	// sprintf(TrackbarName, "H_MIN", H_MIN);
	// sprintf(TrackbarName, "H_MAX", H_MAX);
	// sprintf(TrackbarName, "S_MIN", S_MIN);
	// sprintf(TrackbarName, "S_MAX", S_MAX);
	// sprintf(TrackbarName, "V_MIN", V_MIN);
	// sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//            

	for(int i = 0; i < 6; i ++) {
		// cout <<values[4][i]<<" ";

	}
	// cout<<endl;

	// createTrackbar("H_MIN", trackbarWindowName, &(values[4][0]), values[4][0], on_trackbar);
	// createTrackbar("H_MAX", trackbarWindowName, &(values[4][1]), values[4][1], on_trackbar);
	// createTrackbar("S_MIN", trackbarWindowName, &(values[4][2]), values[4][2], on_trackbar);
	// createTrackbar("S_MAX", trackbarWindowName, &(values[4][3]), values[4][3], on_trackbar);
	// createTrackbar("V_MIN", trackbarWindowName, &(values[4][4]), values[4][4], on_trackbar);
	// createTrackbar("V_MAX", trackbarWindowName, &(values[4][5]), values[4][5], on_trackbar);
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);

}
void drawObject(int x, int y, Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25 > 0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25 < FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25 > 0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25 < FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
	//cout << "x,y: " << x << ", " << y;

}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				//cout << x << "," << y;
				drawObject(x, y, cameraFeed);

			}


		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}


//x and y values for the location of the object
int x[5], y[5];
int limitInf, limitSup;
int main(int argc, char* argv[])
{
	loadColorValues();
	//initWithColor(3);
	limitInf = 0;
	limitSup = TRACKBAR_WINDOW_INDEX;
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;

	Point p;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold[5];
	//create slider bars for HSV filtering
	createTrackbars();

	
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	//capture.open("rtmp://172.16.254.63/live/live");
	capture.open(0); //default webcam
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	
	while (1) {


		//store image to matrix
		capture.read(cameraFeed);

		if(cameraFeed.empty()) {
			return 1;
		}

		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix

		for(int i=limitInf; i<=limitSup; i++) {


			if(i == TRACKBAR_WINDOW_INDEX) {
				inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold[i]);
			}
			else {
				inRange(HSV, Scalar(values[i][0], values[i][2], values[i][4]), 
					Scalar(values[i][1], values[i][3], values[i][5]), threshold[i]);
			}
		

			// inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
			//perform morphological operations on thresholded image to eliminate noise
			//and emphasize the filtered object(s)
			if (useMorphOps)
				morphOps(threshold[i]);
			//pass in thresholded frame to our object tracking function
			//this function will return the x and y coordinates of the
			//filtered object
			if (trackObjects)
				trackFilteredObject(x[i], y[i], threshold[i], cameraFeed);

			imshow(colorNames[i], threshold[i]);
			//show frames
		}


		imshow(windowName, cameraFeed);
		// imshow(windowName1, HSV);
		setMouseCallback("Original Image", on_mouse, &p);
		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);

		for(int i = limitInf; i <= limitSup; i++){
			cout<<colorNames[i]<<"\t\t"<<x[i]<<","<<y[i]<<"\n";
		}
		 printf("%d %d %d %d %d %d\n", H_MIN, S_MIN, V_MIN, H_MAX, S_MAX, V_MAX);
		//parseCoords();
	}

	return 0;
}

void initWithColor(int index) {
	H_MIN = values[index][0];
	H_MAX = values[index][1];
	S_MIN = values[index][2];
	S_MAX = values[index][3];
	V_MIN = values[index][4];
	V_MAX = values[index][5];
}

void loadColorValues() {

	values = (int**)malloc(5*sizeof(int*));
	colorNames = (char **)malloc(5*sizeof(char*));

	int x = -1;
	char str[100];

	ifstream fs;
  	fs.open ("../colorConfigs.in");

  	for(int i = 0; i < TRACKBAR_WINDOW_INDEX; i ++) {
  		fs >> str;
  		colorNames[i] = (char*)malloc(strlen(str) + 1);
  		strcpy(colorNames[i], str);
  		values[i] = (int*)malloc(7*sizeof(int));

  		for(int j = 0; j < 6; j ++){
  			fs >> values[i][j];
  		}
  	}

  	colorNames[TRACKBAR_WINDOW_INDEX] = (char*)malloc(trackbarColorWindowName.length());
	strcpy(colorNames[TRACKBAR_WINDOW_INDEX], trackbarColorWindowName.c_str());
	values[TRACKBAR_WINDOW_INDEX] = (int*)malloc(7*sizeof(int));

  		for(int j = 0; j < 6; j ++){
  			values[TRACKBAR_WINDOW_INDEX][j] = 0;
  		}

  	fs.close();

 //  	for(int i = 0; i < 4; i++) {
 //  		cout<<colorNames[i]<<" ";
 //  		for(int j = 0; j < 6; j++){
 //  			cout<<values[i][j]<<" ";
 //  		}
 //  		cout<<endl;
 //  	}

	// exit(0);
}



void parseCoords() {

	int xDif, yDif;
	int obj1=1, obj2=2;
	xDif = x[obj1] - x[obj2];
	yDif = y[obj1] - y[obj2];


	if(xDif < 1) {

	}

	cout<<xDif<<","<<yDif<<"\n";


}

