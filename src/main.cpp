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

    // Create the output directory if it doesn't exist
    std::string outputDir = currentPath+"\\img";
    
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    cv::VideoCapture cap(videoFile);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return -1;
    }

    int frameIndex = 0;
    int maxFrames = 10; // Maximum number of frames to save
    cv::Mat frame;

    while (frameIndex < maxFrames) {
        cap >> frame; // Read the next frame
        if (frame.empty()) {
            break; // Exit the loop if no more frames
        }

        // Construct the output file name
        std::string videoBaseName = fs::path(videoFile).stem().string();
        std::string outputFileName = outputDir + "/" + videoBaseName + "_frame_" + std::to_string(frameIndex) + ".png";
        // Save the frame as an image
        if (!cv::imwrite(outputFileName, frame)) {
            std::cerr << "Error: Could not save frame to " << outputFileName << std::endl;
            return -1;
        }

        std::cout << "Saved: " << outputFileName << std::endl;
        frameIndex++;
    }

    std::cout << "First " << maxFrames << " video frames have been saved to the '" << outputDir << "' directory." << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}