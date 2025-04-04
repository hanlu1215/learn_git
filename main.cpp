#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // Create the output directory if it doesn't exist
    std::string outputDir = "./img";
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    // Open the video file
    std::string videoFile = "./video.mp4"; // Replace with your video file name
    cv::VideoCapture cap(videoFile);

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return -1;
    }

    int frameIndex = 0;
    cv::Mat frame;

    while (true) {
        cap >> frame; // Read the next frame
        if (frame.empty()) {
            break; // Exit the loop if no more frames
        }

        // Construct the output file name
        std::string outputFileName = outputDir + "/frame_" + std::to_string(frameIndex) + ".png";

        // Save the frame as an image
        if (!cv::imwrite(outputFileName, frame)) {
            std::cerr << "Error: Could not save frame to " << outputFileName << std::endl;
            return -1;
        }

        std::cout << "Saved: " << outputFileName << std::endl;
        frameIndex++;
    }

    std::cout << "Video frames have been saved to the '" << outputDir << "' directory." << std::endl;
    return 0;
}