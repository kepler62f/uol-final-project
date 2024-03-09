#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
using namespace std;

std::string get_current_timestamp_iso() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d_%H-%M-%S") ;
    return ss.str();
}

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
              + system_clock::now());
    return system_clock::to_time_t(sctp);
}

// void clearStorage() {
void clearStorage(std::string path) {
    std::time_t min_t = std::numeric_limits<time_t>::max();
    std::string min_t_filepath;
    auto dir_iter = std::filesystem::directory_iterator(path);
    int file_count = std::count_if(
        begin(dir_iter),
        end(dir_iter),
        [](auto& entry) { return entry.is_regular_file(); }
    );

    // Keep a buffer of 20 frames on file storage.
    // Beyond that, remove the oldest frame
    if (file_count > 20) {
        for (auto& file : fs::directory_iterator(path)) {
            fs::file_time_type ftime = fs::last_write_time(file.path());
            std::time_t tt = to_time_t(ftime);
            if (tt < min_t) {
                min_t = tt;
                min_t_filepath = file.path();
            }
        }
        fs::remove(min_t_filepath);
    }
}

struct Detection
{
    int class_id{0};
    std::string className{};
    float confidence{0.0};
    cv::Scalar color{};
    cv::Rect box{};
};

int main(int argc, char **argv) {

    std::string defaultUrl = "rtsp://192.168.1.0:554/mjpeg/1";
    std::string url = argc > 1 ? argv[1] : defaultUrl;

    std::cout << get_current_timestamp_iso() << " Starting rtsp stream capture..." << url << std::endl;
    cv::VideoCapture cap;
    bool isConnected = false;
    bool debug = true;

    while (!isConnected) {
        isConnected = cap.open(url);
        if (!isConnected) {
            std::cerr << get_current_timestamp_iso() << " Failed to open RTSP stream. Retrying in 5 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    if (debug) {
        cv::namedWindow("RTSP Stream", cv::WINDOW_NORMAL);
    }

    int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double input_fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << get_current_timestamp_iso() << " Frame width is: " << frame_width << std::endl;
    std::cout << get_current_timestamp_iso() << " Frame height is: " << frame_height << std::endl;
    std::cout << get_current_timestamp_iso() << " Input FPS is: " << input_fps << std::endl;

    int i = 0;
    cv::Mat prev_frame;
    cv::Mat prev_frame_processed;

    while (true) {
        cv::Mat frame;

        if (!cap.read(frame)) {
            std::cerr << get_current_timestamp_iso() << " Failed to read frame from RTSP stream. Retrying in 5 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            isConnected = false;
            while (!isConnected) {
                isConnected = cap.open(url);
                if (!isConnected) {
                    std::cerr << get_current_timestamp_iso() << " Failed to open RTSP stream. Retrying in 5 seconds..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
            continue;
        }

        if (debug) {
            cv::imshow("RTSP Stream", frame);
        }

        // Save this frame to storage
        cv::imwrite("data/stream/stream-" + get_current_timestamp_iso() + ".jpg", frame);

        /////////////////////////////////////////
        // Below is first iteration of prototype 
        // drone detection proxy 
        // method using image differencing
        // to measure motion in the scene. White
        // pixels in differenced image represents
        // motion, proxying arrival of drone object.
        // Deprecated in favor of using neural network
        // object detection model (Yolov8)
        //////////////////////////////////////////

        // Variables for intermediate processing for drone detection
        // cv::Mat gray_frame;
        // cv::Mat blur_frame;
        // cv::Mat delta_frame;
        // cv::Mat threshold_frame;
        // cv::Mat contours;
        // cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
        // cv::GaussianBlur(gray_frame, blur_frame, cv::Size(25,25), 0);


        // The first captured frame is the baseline image
        // if (i == 0) {
        //     prev_frame_processed = blur_frame;
        //     i = 1;
        // }

        // cv::absdiff(prev_frame_processed, blur_frame, delta_frame);
        // cv::threshold(delta_frame, threshold_frame, 50, 255, cv::THRESH_BINARY);

        // int number_of_pixels = threshold_frame.rows * threshold_frame.cols;

        // // white pixels (representing movement)
        // int non_zero_pixels = cv::countNonZero(threshold_frame);

        // // percentage area in motion
        // float motion = float(non_zero_pixels) / float(number_of_pixels);

        // if (motion > 0.2) {
        //     // save in drone motion detected folder
        //     cv::imwrite("data/detection/detected-" + get_current_timestamp_iso() + ".jpg", frame);
        // }

        // prev_frame_processed = blur_frame;


        // Remove oldest frames to prevent overloading file storage
        clearStorage("data/stream/");
        // clearStorage("data/detection/");

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') {
            break;
        }
    }

    // Release the resources
    std::cout << get_current_timestamp_iso() << " Releasing rtsp stream..." << std::endl;
    cap.release();
    cv::destroyAllWindows();
    std::cout << get_current_timestamp_iso() << " Destroyed Windows. Bye!" << std::endl;
    return 0;
}
