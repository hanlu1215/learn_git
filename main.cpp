#include <opencv2/opencv.hpp>
int main() {
    cv::Mat img(100, 100, CV_8UC3, cv::Scalar(255, 0, 0));
    cv::imwrite("test.jpg", img);
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}
