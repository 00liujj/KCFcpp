#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"
#include <opencv2/opencv.hpp>
#include "roiSelector.hpp"

#include <dirent.h>

using namespace std;
using namespace cv;


void output_box(int count, cv::Rect2f box, cv::Size2f size, string fn) {

    static float old_w = 0;
    float new_w = box.width / size.width;

    float rate = (new_w - old_w) / new_w;
    old_w = new_w;
    printf("The width change rate is %f %f\n", box.width, rate);
    printf("TRACKING_RESULTS: %05d %f %f %f %f\n", count, box.x/size.width, box.y/size.height,
           (box.x+box.width)/size.width, (box.y+box.height)/size.height);

    char* outbox = getenv("OUTPUT_BOX");
    if (outbox && strcmp(outbox, "1") == 0) {
        string::size_type pos = fn.find_last_of(".");
        string txtfn;
        if (pos != string::npos) {
            txtfn = string(fn, 0, pos) + ".txt";
        } else {
            txtfn = fn+".txt";
        }
        ofstream ofs(txtfn);
        cv::Rect box2 = box;
        ofs << cv::format("%d %d %d %d %d\n", 0, box2.x, box2.y, box2.width, box2.height);
    }
}


int main(int argc, char *argv[])
{
    bool HOG = true;
    bool FIXEDWINDOW = false;
    bool MULTISCALE = true;
    bool SILENT = true;
    bool LAB = false;
    bool HELP = false;
    string start_roi;

    vector<string> fns;

    for(int i = 1; i < argc; i++){
        if ( strcmp (argv[i], "--hog") == 0 )
            HOG = true;
        else if ( strcmp (argv[i], "--fixed_window") == 0 )
            FIXEDWINDOW = true;
        else if ( strcmp (argv[i], "--singlescale") == 0 )
            MULTISCALE = false;
        else if ( strcmp (argv[i], "--show") == 0 )
            SILENT = false;
        else if ( strcmp (argv[i], "--lab") == 0 ){
            LAB = true;
            HOG = true;
        }
        else if ( strcmp (argv[i], "--start_roi") == 0 ) {
            i = i+1;
            start_roi = argv[i];
        }
        else if ( strcmp (argv[i], "--gray") == 0 )
            HOG = false;
        else if ( strcmp (argv[i], "--help") == 0 )
            HELP = true;
        else
            fns.push_back(argv[i]);
    }

    if (HELP || fns.size() == 0) {
        printf("usage:\n"
               "    %s [--hog] [--fixed_window] [--show] [--lab] [--gray] [--help] video_filename\n"
               "    press 's' to select a ROI\n"
               "    press 'q' to quit\n"
               , argv[0]);
        return 0;
    }




    // Create KCFTracker object
    Ptr<KCFTracker> tracker = new KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);




    bool has_init = false;


    cv::Size2f size(640, 480);

    int count = 0;

    for (int i=0; i<fns.size(); i++) {
      string fn = fns[i];
      cout << "read " << fn << endl;
      cv::VideoCapture vc(fn);
      while (1) {
        cv::Mat mat, frame;
        vc >> mat;
        if (mat.empty()) break;

        cv::resize(mat, frame, size);

        if (start_roi.size() > 0) {
            istringstream iss(start_roi);
            int start_idx;

            iss >> start_idx;

            if (start_idx == count) {
                float x1, y1, x2, y2;
                iss >> x1 >> y1 >> x2 >> y2;

                cv::Rect box;

                box.x = x1 * size.width+0.5f;
                box.y = y1 * size.height+0.5f;
                box.width = (x2-x1) * size.width+0.5f;
                box.height = (y2-y1) * size.height+0.5f;

                if (box.width ==0 || box.height == 0) {
                    box = cv::selectROI("Frame", frame, true, false);
                }

                tracker->init(box, frame);
                has_init = true;
                output_box(count, box, size, fn);
            }
        }

        if (has_init) {
            cv::Rect2f box = tracker->update(frame);
            output_box(count, box, size, fn);
            cv::rectangle(frame, box, CV_RGB(0, 255, 0), 1);
        }

        cv::imshow("Frame", frame);
        char key = cv::waitKey(1);

        if ('s' == key) {
            cv::Rect box = cv::selectROI("Frame", frame, true, false);
            std::cout << "select roi " << box << std::endl;
            if (box.width > 0  && box.height > 0) {
                tracker->init(box, frame);
                has_init = true;
                output_box(count, box, size, fn);
            }
        } else if ('q' == key) {
            break;
        }



        //if (count > 1000)
        //cv::imwrite(cv::format("NCAP/%05d.jpg", count), mat);

        count++;

      }
    }
    return 0;
}



int main_old(int argc, char* argv[]){

	if (argc > 5) return -1;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;
	bool LAB = false;

	for(int i = 0; i < argc; i++){
		if ( strcmp (argv[i], "hog") == 0 )
			HOG = true;
		if ( strcmp (argv[i], "fixed_window") == 0 )
			FIXEDWINDOW = true;
		if ( strcmp (argv[i], "singlescale") == 0 )
			MULTISCALE = false;
		if ( strcmp (argv[i], "show") == 0 )
			SILENT = false;
		if ( strcmp (argv[i], "lab") == 0 ){
			LAB = true;
			HOG = true;
		}
		if ( strcmp (argv[i], "gray") == 0 )
			HOG = false;
	}
	
	// Create KCFTracker object
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);

	// Frame readed
	Mat frame;

	// Tracker results
	Rect result;

	// Path to list.txt
	ifstream listFile;
	string fileName = "images.txt";
  	listFile.open(fileName);

  	// Read groundtruth for the 1st frame
  	ifstream groundtruthFile;
	string groundtruth = "region.txt";
  	groundtruthFile.open(groundtruth);
  	string firstLine;
  	getline(groundtruthFile, firstLine);
	groundtruthFile.close();
  	
  	istringstream ss(firstLine);

  	// Read groundtruth like a dumb
  	float x1, y1, x2, y2, x3, y3, x4, y4;
  	char ch;
	ss >> x1;
	ss >> ch;
	ss >> y1;
	ss >> ch;
	ss >> x2;
	ss >> ch;
	ss >> y2;
	ss >> ch;
	ss >> x3;
	ss >> ch;
	ss >> y3;
	ss >> ch;
	ss >> x4;
	ss >> ch;
	ss >> y4; 

	// Using min and max of X and Y for groundtruth rectangle
	float xMin =  min(x1, min(x2, min(x3, x4)));
	float yMin =  min(y1, min(y2, min(y3, y4)));
	float width = max(x1, max(x2, max(x3, x4))) - xMin;
	float height = max(y1, max(y2, max(y3, y4))) - yMin;

	
	// Read Images
	ifstream listFramesFile;
	string listFrames = "images.txt";
	listFramesFile.open(listFrames);
	string frameName;


	// Write Results
	ofstream resultsFile;
	string resultsPath = "output.txt";
	resultsFile.open(resultsPath);

	// Frame counter
	int nFrames = 0;


	while ( getline(listFramesFile, frameName) ){
		frameName = frameName;

		// Read each frame from the list
		frame = imread(frameName, CV_LOAD_IMAGE_COLOR);

		// First frame, give the groundtruth to the tracker
		if (nFrames == 0) {
			tracker.init( Rect(xMin, yMin, width, height), frame );
			rectangle( frame, Point( xMin, yMin ), Point( xMin+width, yMin+height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << xMin << "," << yMin << "," << width << "," << height << endl;
		}
		// Update
		else{
			result = tracker.update(frame);
			rectangle( frame, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << result.x << "," << result.y << "," << result.width << "," << result.height << endl;
		}

		nFrames++;

		if (!SILENT){
			imshow("Image", frame);
			waitKey(1);
		}
	}
	resultsFile.close();

	listFile.close();

}
