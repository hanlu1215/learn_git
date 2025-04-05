#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <commdlg.h>

namespace fs = std::filesystem;

// Function to open a file dialog and get the selected file path
std::string openFileDialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Video Files\0*.mp4;*.avi;*.mkv;*.mov\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return std::string(filename);
    } else {
        return "";
    }
}

// Global variable to store the current frame
cv::Mat currentFrame;

// Mouse callback function to save the current frame on left button click
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) { // Left mouse button click
        std::string outputDir = *(std::string*)userdata;
        std::string outputFileName = outputDir + "/saved_frame_" + std::to_string(cv::getTickCount()) + ".png";
        if (!currentFrame.empty()) {
            if (cv::imwrite(outputFileName, currentFrame)) {
                std::cout << "Saved frame to: " << outputFileName << std::endl;
            } else {
                std::cerr << "Error: Could not save frame to " << outputFileName << std::endl;
            }
        }
    }
}

int main() {
    // 获取当前路径并打印
    std::string currentPath = fs::current_path().string();
    std::cout << "Current working directory: " << currentPath << std::endl;

    // Open a file dialog to select the video file
    std::string videoFile = openFileDialog();
    if (videoFile.empty()) {
        std::cerr << "Error: No video file selected." << std::endl;
        return -1;
    }
    std::cout << "Selected video file: " << videoFile << std::endl;

    cv::VideoCapture cap(videoFile);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return -1;
    }

    // 创建窗口并设置为可调整大小
    cv::namedWindow("HLL VideoPlayer", cv::WINDOW_NORMAL);

    // 设置窗口大小为 720p (1280x720)
    int windowWidth = 1280;
    int windowHeight = 720;
    cv::resizeWindow("HLL VideoPlayer", windowWidth, windowHeight);

    // 获取屏幕分辨率并将窗口移动到屏幕中央
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;
    cv::moveWindow("HLL VideoPlayer", windowX, windowY);

    // 设置鼠标回调函数
    std::string outputDir = currentPath + "\\img";
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }
    cv::setMouseCallback("HLL VideoPlayer", onMouse, &outputDir);

    cv::Mat frame;
    while (true) {
        cap >> frame; // Read the next frame
        if (frame.empty()) {
            break; // Exit the loop if no more frames
        }

        // 更新当前帧
        currentFrame = frame.clone();

        // 播放视频
        cv::imshow("HLL VideoPlayer", frame);

        // 等待 30ms，按下任意键退出
        if (cv::waitKey(30) >= 0) {
            break;
        }
    }

    return 0;
}