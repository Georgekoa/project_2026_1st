#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "functions.h"

int main() {
    
    std::cout << "hello world" << std::endl;
    std::string path = "data/samples/test.jpeg";
    cv::Mat descriptors = SIFT_extractor(path);
    std::cout << "sift rows: " << descriptors.rows << "\n";  // # of keypoints
    std::cout << "sift collumns : " << descriptors.cols << "\n";  // 128 (std number of sift vector)

    return 0;
}