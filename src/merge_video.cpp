#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <codecvt>
#include <locale>

namespace fs = std::filesystem;

// 将 UTF-8 转换为 UTF-16 (Windows wchar_t)
std::wstring utf8ToUtf16(const std::string& utf8Str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstrTo[0], size_needed);
    return wstrTo;
}

// 将 UTF-16 转换为 UTF-8
std::string utf16ToUtf8(const std::wstring& utf16Str) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, utf16Str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16Str.c_str(), -1, &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

// 检查文本是否包含中文字符
bool containsChinese(const std::string& text) {
    // 将 UTF-8 转换为 UTF-16 (Windows wchar_t)
    std::wstring wstr = utf8ToUtf16(text);
    
    // 检查是否包含中文字符
    for (wchar_t wc : wstr) {
        if (wc >= 0x4e00 && wc <= 0x9fff) {
            return true;
        }
    }
    return false;
}

// 创建黑色背景带文字的图像
cv::Mat createBlackImageWithText(const std::string& text, int width = 1920, int height = 1080) {
    cv::Mat img = cv::Mat::zeros(height, width, CV_8UC3);
    cv::Point textOrg(width / 2, height / 2);
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 2.0;
    int thickness = 3;
    cv::Scalar textColor(255, 255, 255); // 白色文字
    
    // 获取文本大小来居中显示
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    textOrg.x = (width - textSize.width) / 2;
    textOrg.y = (height + textSize.height) / 2;
    
    cv::putText(img, text, textOrg, fontFace, fontScale, textColor, thickness);
    return img;
}

// 执行系统命令
int executeCommand(const std::string& command) {
    std::cout << "Executing command: " << command << std::endl;
    return system(command.c_str());
}

// 选择文件夹对话框
std::string selectFolder() {
    BROWSEINFOA bi = { 0 };
    bi.lpszTitle = "Select folder containing video files";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl == NULL) {
        return "";
    }
    
    char path[MAX_PATH];
    if (SHGetPathFromIDListA(pidl, path)) {
        CoTaskMemFree(pidl);
        return std::string(path);
    }
    
    CoTaskMemFree(pidl);
    return "";
}

int main() {
    // 初始化 COM 库 (用于 Windows 对话框)
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    
    // 选择文件夹
    std::string folderPath = selectFolder();
    if (folderPath.empty()) {
        std::cout << "No folder selected." << std::endl;
        CoUninitialize();
        return 1;
    }
    
    std::cout << "Selected folder: " << folderPath << std::endl;
    
    // 获取文件夹名称
    std::string folderName = fs::path(folderPath).filename().string();
    std::cout << "Folder name: " << folderName << std::endl;
    
    // 创建临时文件夹
    std::string tempFolder = folderPath + "\\temp";
    if (fs::exists(tempFolder)) {
        for (const auto& entry : fs::directory_iterator(tempFolder)) {
            fs::remove_all(entry.path());
        }
    } else {
        fs::create_directory(tempFolder);
    }
    
    // 获取所有视频文件
    std::vector<std::string> mainVideos;
    std::vector<std::string> otherVideos;
    
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            
            if (extension == ".mp4" || extension == ".avi" || extension == ".mov" || extension == ".mkv") {
                std::string filename = entry.path().filename().string();
                std::string lowerFilename = filename;
                std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);
                
                if (lowerFilename.find("main") != std::string::npos) {
                    mainVideos.push_back(entry.path().string());
                } else {
                    otherVideos.push_back(entry.path().string());
                }
            }
        }
    }
    
    // 按名称排序
    std::sort(mainVideos.begin(), mainVideos.end());
    std::sort(otherVideos.begin(), otherVideos.end());
    
    // 合并主视频和其他视频
    std::vector<std::string> allVideos;
    allVideos.insert(allVideos.end(), mainVideos.begin(), mainVideos.end());
    allVideos.insert(allVideos.end(), otherVideos.begin(), otherVideos.end());
    
    if (allVideos.empty()) {
        std::cout << "No video files found in the folder!" << std::endl;
        CoUninitialize();
        return 1;
    }
    
    // 收集所有需要合并的视频
    std::vector<std::string> allVideosToMerge;
    int videoIndex = 0;
    
    for (const auto& videoPath : allVideos) {
        fs::path path = fs::path(videoPath);
        std::string videoFileName = path.filename().string();
        std::cout << "Processing video: " << videoFileName << std::endl;
        
        // 创建黑色背景带文字的图像
        cv::Mat titleImage = createBlackImageWithText(videoFileName);
        std::string titleImagePath = tempFolder + "\\title_" + std::to_string(videoIndex) + ".jpg";
        cv::imwrite(titleImagePath, titleImage);
        
        // 创建1秒图片视频
        std::string titleVideoPath = tempFolder + "\\title_" + std::to_string(videoIndex) + ".mp4";
        std::string ffmpegCmd = "ffmpeg -y -loop 1 -i \"" + titleImagePath + 
                               "\" -c:v libx264 -t 1 -pix_fmt yuv420p -r 30 \"" + titleVideoPath + "\"";
        if (executeCommand(ffmpegCmd) != 0) {
            std::cout << "Failed to create title video!" << std::endl;
            continue;
        }
        
        // 处理原始视频 (缩放到1080p并保持宽高比)
        std::string scaledVideoPath = tempFolder + "\\scaled_" + std::to_string(videoIndex) + ".mp4";
        ffmpegCmd = "ffmpeg -y -i \"" + videoPath + 
                   "\" -vf \"scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2:black\" " +
                   "-c:v libx264 -c:a aac \"" + scaledVideoPath + "\"";
        if (executeCommand(ffmpegCmd) != 0) {
            std::cout << "Failed to scale video!" << std::endl;
            continue;
        }
        
        // 添加标题视频和缩放后的视频到合并列表
        allVideosToMerge.push_back(titleVideoPath);
        allVideosToMerge.push_back(scaledVideoPath);
        
        videoIndex++;
    }
    
    // 检查 disclaimer 视频是否存在
    std::string disclaimerPath = "F:\\V_of_paper\\disclaimer_video.mp4";
    if (!fs::exists(disclaimerPath)) {
        std::cout << "Disclaimer video does not exist: " << disclaimerPath << std::endl;
        
        // 创建临时免责声明视频
        cv::Mat disclaimerImage = createBlackImageWithText("DISCLAIMER\nFor educational purposes only");
        std::string disclaimerImagePath = tempFolder + "\\disclaimer.jpg";
        cv::imwrite(disclaimerImagePath, disclaimerImage);
        
        disclaimerPath = tempFolder + "\\disclaimer_video.mp4";
        std::string ffmpegCmd = "ffmpeg -y -loop 1 -i \"" + disclaimerImagePath + 
                              "\" -c:v libx264 -t 3 -pix_fmt yuv420p -r 30 \"" + disclaimerPath + "\"";
        executeCommand(ffmpegCmd);
    }
    
    // 将 disclaimer 视频添加到合并列表
    allVideosToMerge.push_back(disclaimerPath);
    
    // 创建最终合并文件列表
    std::string finalConcatListPath = tempFolder + "\\final_concat.txt";
    std::ofstream finalConcatList(finalConcatListPath);
    for (const auto& video : allVideosToMerge) {
        finalConcatList << "file '" << video << "'\n";
    }
    finalConcatList.close();
    
    // 最终一次性合并所有视频
    std::string outputPath = "F:\\V_of_paper\\" + folderName + "merge.mp4";
    std::string finalCommand = "ffmpeg -y -f concat -safe 0 -i \"" + finalConcatListPath + 
                              "\" -c:v libx264 -crf 23 -preset medium -c:a aac \"" + outputPath + "\"";
    
    if (executeCommand(finalCommand) == 0) {
        std::cout << "Video merged successfully! Output file: " << outputPath << std::endl;
    } else {
        std::cout << "Final video merge failed!" << std::endl;
    }
    
    CoUninitialize();
    
    std::cout << "Processing completed. Press any key to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}