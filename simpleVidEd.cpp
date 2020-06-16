#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

//globals
VideoCapture cap = 0;   //class object to read video
Mat frame, ROI;         //frame - dynamic obj for showing frames
Rect myROI;             //ROI(region of interest) - to select rectange for cropping video
int rec_start_frame, rec_stop_frame;    //vars to set start/stop recording frame number 


void myTrackbarCback(int pos, void*)        //opencv func of trackbar (on change)
{
    cap.set(CAP_PROP_POS_FRAMES, pos);
}

int main(int argc, char* argv[])
{
    //descriptin of program
    cout << "\nVIDed\n"
            "You can slide through the video by trackbar(it shows number of frames passed)\n"
            "To exit press ESC\n"
            "You can choose ROI(region of interest) by pressing backspace and use LMB\n"
            "To record video(or part of it) you need to press 1 to select record start time "
            "and press 2 to select record stop time(you can slide trackbar to choose time)" << endl << endl;
    

    cout << "Video file path(main func argument): ";

    const char* filepath = argc == 2 ? argv[1] : "D:\\video2.mp4";      //sets user's path to file or use default value (to set value, add an argument when starting a program)
    printf("file: %s\n", filepath);

    cap.open(filepath, CAP_ANY);    //open file with path "filepath"
    

    const double FPS = cap.get(CAP_PROP_FPS);                   //fps in opened video
    const float WAIT_SEC = 1000/FPS;
    const long FRAME_COUNT = cap.get(CAP_PROP_FRAME_COUNT);     //number of frames in opened video
    
    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);            //width of opened video
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);          //height of opened video


	if (!cap.isOpened()) {          //if video hasnt opened - quit
			cout << "Error opening video stream or file" << endl;
			return -1;
	}
    
    Mat cropped_frame;      //obj to save cropped frame by ROI
    string std_window = "std";
    while (1) {
            
           
            cap >> frame;           // Capture frame-by-frame
            
            if (frame.empty())      // If the frame is empty, break
                break;
            
            int currpos = cap.get(CAP_PROP_POS_FRAMES);     //number of frames passed 
            if (FRAME_COUNT != 0)                           //creating trackbar, which shows currpos
            {
                createTrackbar("Pos", std_window, &currpos, FRAME_COUNT, myTrackbarCback);
            }
            
            // Display the resulting frame
            if (!myROI.empty() ) {                  //if ROI selected, showing cropped frame
                cropped_frame = frame(myROI);
                imshow("ROI vid", cropped_frame);   //showing frame
                
                char c = (char)cv::waitKey(WAIT_SEC);     // Press ESC to exit, backspace to select ROI, numbers 1,2 to select rec_start_frame, rec_stop_frame 
                if (c == 27) {      //ESC
                    break;
                }
                else if (c == 8) {  //backspace
                    myROI = selectROI(std_window, frame, false, false);
                }
                else if (c == 49) { //1
                    rec_start_frame = cap.get(CAP_PROP_POS_FRAMES);
                    cout << "rec_start_frame " << rec_start_frame << endl;
                }
                else if (c == 50) { //2
                    rec_stop_frame = cap.get(CAP_PROP_POS_FRAMES);
                    cout << "rec_stop_frame " << rec_stop_frame << endl;
                }
            }
            else {
                imshow(std_window, frame);      //if ROI isnt selected, showing default frame

                char c = (char)cv::waitKey(WAIT_SEC);     // Press ESC to exit, backspace to select ROI, numbers 1,2 to select rec_start_frame, rec_stop_frame
                if (c == 27)        //ESC
                    break;
                else if (c == 8) {  //backspace
                    myROI = selectROI(std_window, frame, false, false);
                }
                else if (c == 49) { //1
                    rec_start_frame = cap.get(CAP_PROP_POS_FRAMES);
                    cout <<"rec_start_frame " << rec_start_frame << endl;
                }
                else if (c == 50) { //2
                    rec_stop_frame = cap.get(CAP_PROP_POS_FRAMES);
                    cout<<"rec_stop_frame " << rec_stop_frame << endl;
                }
            }
            
    }

    if (rec_start_frame and rec_stop_frame) {           //if time range is set
        unsigned int var;       //variable to select - record cropped with ROI, or record in default size
        cout << "Record with ROI? 1-yes, 0-no(if you haven't selected ROI choose 0)" << endl;
        cin >> var;
        cap.set(CAP_PROP_POS_FRAMES, rec_start_frame);  //going to start pos
        
        int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
        
        if (var == 0) {
            VideoWriter video("outcpp.avi", codec, FPS, Size(frame_width, frame_height));       //obj to write wideo: VideoWriter(output filename,codec,fps,size(witdh, height))
            for (int frameInd = rec_start_frame; frameInd < rec_stop_frame; frameInd++) {       //loop to go through frames and record selected part of the video
                cap >> frame;
                if (frame.empty()) break;

                video.write(frame);
            }
            video.release();    //releasing VideoWriter obj
        }
        else if (var == 1) {
            VideoWriter video("outcpp.avi", codec, FPS, Size(myROI.width, myROI.height));       //obj to write wideo: VideoWriter(output filename,codec,fps,size(witdh, height))
            Mat vidROIframe;        //contains cropped frame
            for (int frameInd = rec_start_frame; frameInd < rec_stop_frame; frameInd++) {       //loop to go through frames and record selected part of the video
                cap >> frame;
                if (frame.empty()) break;
                vidROIframe = frame(myROI);     //cropping frame
                video.write(vidROIframe);       //writing cropped frame
            }
            video.release();    //releasing VideoWriter obj
        }
    }
    
    cap.release();  //release VideoCapture object

    // Closes all frames
    cv::destroyAllWindows();

	return 0;
}