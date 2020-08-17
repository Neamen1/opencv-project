#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

//globals
VideoCapture cap = 0;   //class object to read video
Mat frame, ROI;         //frame - dynamic obj for showing frames
Rect myROI;             //ROI(region of interest) - to select rectange for cropping video
int rec_start_frame = -1;    //vars to set start/stop recording frame number 
int rec_stop_frame = -1;

void myTrackbarCback(int pos, void*)        //opencv func of trackbar (callback)
{
    cap.set(CAP_PROP_POS_FRAMES, pos);
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() and std::isdigit(*it))++it;
    return !s.empty() and it == s.end();
}

string make_folder(string filepath)         //makes folder from filepath
{
    filepath = filepath.substr(0, filepath.find_last_of("\\/"));
    std::string folder_name;
    cout << "Enter folder name: ";
    cin >> folder_name;//enter name of folder

    //creating dir with cmd command
    std::string folder_path = filepath + "\\" + folder_name;
    folder_name = "mkdir " + filepath + "\\" + folder_name;

    const char* c = folder_name.c_str();

    system(c);
    return folder_path;
}

int main(int argc, char* argv[])
{
    //description of program
    std::cout << "\nVIDed\n"
        "You can slide through the video by trackbar(it shows number of frames passed)\n"
        "To exit press ESC (or if you want to close only cropped window press q)\n"
        "Space for pause\n"
        "Press 5 to save current frame\n"
        "Pressing 7 you decrease and pressing 8 you increase the playback speed \n"
        "You can choose ROI(region of interest) by pressing backspace and using LMB\n"
        "To record video(or part of it) you need to press 1 to select record start time "
        "and press 2 to select record stop time(you can slide trackbar to choose time)" << endl << endl;


    std::cout << "Enter video file path(or number for video stream(0 - any stream)): ";

    std::string filen;   //sets user's file path
    cin >> filen;
    const std::string filepath = filen;

    const string filedir = filepath.substr(0, filepath.find_last_of("\\/"));      //"find_last_of" find pos of argument; "substr" makes str in measures from ..to..;
    const string filename = filepath.substr(filepath.find_last_of("\\/") + 1, (filepath.length() - filepath.find_last_of("\\/") - 1) - (filepath.length() - filepath.find_last_of(".")));
    const string screendir = filedir + "\\" + filename + "_screenshots";


    bool stream = false;
    int count_video_saves = 1;     
    bool stream_rec = false;
    if (is_number(filepath))
    {
        stream = true;
        cap.open(stoi(filepath), CAP_ANY);    //open stream (filepath - number)
    }
    else
    {
        cap.open(filepath, CAP_ANY);    //open file with path "filepath"
    }

    if (!cap.isOpened()) {          //if video hasnt opened - quit
        std::cout << "\n!!Error opening video stream or file!!\n" << endl;
        return -1;
    }

    double FPS = cap.get(CAP_PROP_FPS);                   //fps in opened video
    if (stream) {//getting more accurate FPS value if stream 

        int num_frames = 120;//some frames must pass to get time
        time_t start, end;
        time(&start);
        // Grab a few frames
        for (int i = 0; i < num_frames; i++)
        {
            cap >> frame;
        }
        // End Time
        time(&end);
        double seconds = difftime(end, start);
        FPS = num_frames / seconds;     //
    }
    const float WAIT_SEC = 1000 / FPS;
    const long FRAME_COUNT = cap.get(CAP_PROP_FRAME_COUNT);     //number of frames in opened video

    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);            //width of opened video
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);          //height of opened video

    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');


    bool ispaused = false;
    Mat cropped_frame;      //obj to save cropped frame by ROI
    string std_window = "std";  //name of default window
    int screenshot_count = 1;
    float fps_reduse_increase = 1;  //koefficien of video speed


    const char cropped_window_name[8] = "ROI vid"; //name of cropped frame window

    std::string name = format("%d out.avi", count_video_saves); //name for stream_writer object, which will change further
    VideoWriter stream_writer;       //obj to write wideo: VideoWriter(output filename,codec,fps,size(witdh, height))
    string videorecord_folder_path;

    while (1) {

        if (!ispaused) {
            cap >> frame;           // Capture frame-by-frame
        }
        if (frame.empty()) {      // If the frame is empty, break
            break;
        }

       
        // Display the resulting frame
        if (!myROI.empty()) {                  //if ROI selected, showing cropped frame(+keys)
            cropped_frame = frame(myROI);

            imshow(cropped_window_name, cropped_frame);   //showing frame

            int currpos = cap.get(CAP_PROP_POS_FRAMES);     //number of frames passed 
            if (FRAME_COUNT != 0 and !is_number(filepath))                           //creating slider - trackbar, which shows currpos
            {
                createTrackbar("Pos", cropped_window_name, &currpos, FRAME_COUNT, myTrackbarCback);
            }

            if (stream_rec and stream) {        //recording stream frames if ...
                stream_writer.write(cropped_frame);
            }

            //key events
            char c = (char)cv::waitKey(WAIT_SEC / fps_reduse_increase);     // Press ESC to exit, backspace to select ROI, numbers 1,2 to select rec_start_frame, rec_stop_frame 
            if (c == 113) {      //q
                destroyWindow(cropped_window_name);
                myROI = Rect();
            }
            else if (c == 27)        //ESC
                break;
            else if (c == 53) {  //5    //screenshot
                if (screenshot_count == 1)
                {
                    system(("mkdir " + screendir).c_str());
                }
                string saveName = screendir + format("\\%dscreenshot.png", screenshot_count);
                screenshot_count++;
                bool check = cv::imwrite(saveName, cropped_frame);
                if (check)
                {
                    cout << "Screenshot saved" << endl;
                }
                else
                {
                    cout << "Screenshot not saved!" << endl;
                }
                waitKey(100);
            }
            else if (c == 32) {  //space    //pause
                ispaused = !ispaused;
                waitKey(300);
            }
            else if (c == 8) {  //backspace
                myROI = selectROI(std_window, frame, false, false);     //select ROI
            }
            else if (c == 49) { //1
                rec_start_frame = cap.get(CAP_PROP_POS_FRAMES);         //choosing record start time(saves current frame)
                std::cout << "rec_start_frame " << rec_start_frame << endl;
            }
            else if (c == 50) { //2
                rec_stop_frame = cap.get(CAP_PROP_POS_FRAMES);          //choosing record stop time(saves current frame)
                std::cout << "rec_stop_frame " << rec_stop_frame << endl;
            }
            else if (c == 55) { //7
                if (fps_reduse_increase == 0.25) { continue; } //if min value - skip
                else if (fps_reduse_increase > 1) {     //reduse var by 1 if var>1; by 0.25 if 0<var<=1
                    fps_reduse_increase -= 1;
                }
                else if (fps_reduse_increase > 0 and fps_reduse_increase <= 1) {
                    fps_reduse_increase -= 0.25;
                }
                cout << "Current speed variation is: " << fps_reduse_increase << endl;
            }
            else if (c == 56) { //8
                if (fps_reduse_increase == 4) { continue; } ////if max value - skip
                else if (fps_reduse_increase >= 1) {    //increase on 1 if var>=1; on 0.25 if 0<var<1
                    fps_reduse_increase += 1;
                }
                else if (fps_reduse_increase > 0 and fps_reduse_increase < 1) {
                    fps_reduse_increase += 0.25;
                }
                cout << "Current speed variation is: " << fps_reduse_increase << endl;
            }
            else if (c == 120 and stream) {        //x  //stop recording stream video 
                stream_writer.release();    //releasing VideoWriter obj

                count_video_saves++;
                stream_rec = false;
                cout << "saving video" << name << endl;
            }
            else if (c == 122 ) {        //z  //start recording stream video 
                if (count_video_saves == 1) {
                    videorecord_folder_path = make_folder(filepath);
                }
                if (rec_start_frame >= 0 and rec_stop_frame >= 0 and !stream) {           //if time range is set and not stream - record
                    int return_frame = cap.get(CAP_PROP_POS_FRAMES);

                    cap.set(CAP_PROP_POS_FRAMES, rec_start_frame);  //setting rec start frame pos

                    string outFileName = videorecord_folder_path + "\\cv_croppedout.avi";
                    VideoWriter video(outFileName, codec, FPS, Size(myROI.width, myROI.height));       //obj to write wideo: VideoWriter(output filename,codec,fps,size(witdh, height))
                    for (int frameInd = rec_start_frame; frameInd < rec_stop_frame; frameInd++) {       //loop to go through frames and record selected part of the video
                        cap >> frame;
                        if (frame.empty()) break;
                        video.write(frame(myROI));
                    }
                    video.release();    //releasing VideoWriter obj
                    count_video_saves += 1;
                    rec_start_frame = 0;
                    rec_stop_frame = 0;
                    cap.set(CAP_PROP_POS_FRAMES, return_frame);
                }
                else if (stream) {
                    if (count_video_saves == 1)
                    {
                        system("mkdir stream_records");
                    }
                    name = format("stream_records\\%d out.avi", count_video_saves);
                    stream_writer.open(name, codec, FPS, Size(myROI.width, myROI.height));
                    stream_rec = true;
                    cout << "Stream record started" << endl;
                }
            }

        }

        else {
            imshow(std_window, frame);      //if ROI isnt selected, showing default frame

            int currpos = cap.get(CAP_PROP_POS_FRAMES);     //number of frames passed 
            if (FRAME_COUNT != 0 and !is_number(filepath))                           //creating slider - trackbar, which shows currpos
            {
                createTrackbar("Pos", std_window, &currpos, FRAME_COUNT, myTrackbarCback);
            }

            if (stream_rec and stream)          //recording stream frames if ...
            {        
                stream_writer.write(frame);
            }
            char c = (char)cv::waitKey(WAIT_SEC / fps_reduse_increase);     // Press ESC to exit, backspace to select ROI, numbers 1,2 to select rec_start_frame, rec_stop_frame

            //key events
            if (c == 27) //ESC  //quit program
            {
                destroyWindow(std_window);
                break;
            }
            else if (c == 32) //space   //pause
            {
                waitKey(300);   
                ispaused = !ispaused;
            }
            else if (c == 8) //backspace        //select ROI
            {
                myROI = selectROI(std_window, frame, false, false);
            }
            else if (c == 53) {  //5        //screenshot
                if (screenshot_count == 1)
                {
                    system(("mkdir " + screendir).c_str());
                }
                string saveName = screendir + format("\\%dscreenshot.png", screenshot_count);
                screenshot_count++;
                bool check = cv::imwrite(saveName, frame);
                if (check)
                {
                    cout << "Screenshot saved" << endl;
                }
                else
                {
                    cout << "Screenshot not saved!" << endl;
                }
                waitKey(100);
            }
            else if (c == 49) { //1
                rec_start_frame = cap.get(CAP_PROP_POS_FRAMES);
                std::cout << "rec_start_frame " << rec_start_frame << endl;
            }
            else if (c == 50) { //2
                rec_stop_frame = cap.get(CAP_PROP_POS_FRAMES);
                std::cout << "rec_stop_frame " << rec_stop_frame << endl;
            }
            else if (c == 55) { //7
                if (fps_reduse_increase == 0.25) { continue; } //if min value - skip
                else if (fps_reduse_increase > 1) {     //reduse var by 1 if var>1; by 0.25 if 0<var<=1
                    fps_reduse_increase -= 1;
                }
                else if (fps_reduse_increase > 0 and fps_reduse_increase <= 1) {
                    fps_reduse_increase -= 0.25;
                }
                cout << "Current speed variation is: " << fps_reduse_increase << endl;
            }
            else if (c == 56) { //8
                if (fps_reduse_increase == 4) { continue; } ////if max value - skip
                else if (fps_reduse_increase >= 1) {    //increase on 1 if var>=1; on 0.25 if 0<var<1
                    fps_reduse_increase += 1;
                }
                else if (fps_reduse_increase > 0 and fps_reduse_increase < 1) {
                    fps_reduse_increase += 0.25;
                }
                cout << "Current speed variation is: " << fps_reduse_increase << endl;
            }
            else if (c == 120 and stream) {        //x  //stop recording stream video 
                stream_writer.release();        //releasing VideoWriter obj
                count_video_saves++;
                stream_rec = false;
                cout << "saving video" << name << endl;
            }
            else if (c == 122 ) {        //z  //start recording video 
                if (count_video_saves == 1) {
                    videorecord_folder_path =make_folder(filepath);
                }
                if (rec_start_frame >= 0 and rec_stop_frame >= 0 and !stream) {           //if time range is set and not stream - record
                    int return_frame = cap.get(CAP_PROP_POS_FRAMES);

                    cap.set(CAP_PROP_POS_FRAMES, rec_start_frame);  //setting rec start frame pos

                    string outFileName = videorecord_folder_path + "\\cv_out.avi";
                    VideoWriter video(outFileName, codec, FPS, Size(frame_width, frame_height));       //obj to write wideo: VideoWriter(output filename,codec,fps,size(witdh, height))
                    for (int frameInd = rec_start_frame; frameInd < rec_stop_frame; frameInd++) {       //loop to go through frames and record selected part of the video
                        cap >> frame;
                        if (frame.empty()) break;

                        video.write(frame);
                    }
                    video.release();    //releasing VideoWriter obj
                    count_video_saves += 1;
                    rec_start_frame = 0;
                    rec_stop_frame = 0;
                    cap.set(CAP_PROP_POS_FRAMES, return_frame);
                    }
                
                else if (stream){
                if (count_video_saves == 1)
                {
                    system("mkdir stream_records");
                }
                name = format("stream_records\\%d out.avi", count_video_saves);
                stream_writer.open(name, codec, FPS, Size(frame_width, frame_height));
                stream_rec = true;
                cout << "Stream record started" << endl;
                }
            }
        }
    }

    cap.release();  //release VideoCapture object

    // Closes all windows
    cv::destroyAllWindows();

    return 0;
}
