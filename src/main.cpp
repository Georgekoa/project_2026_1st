#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "functions.h"

int main() {
    //SIFT TESTEr
    std::string path = "data/samples/test.jpeg";
    cv::Mat descriptors = SIFT_extractor(path);
    std::cout << "sift rows: " << descriptors.rows << "\n";  // # of keypoints
    std::cout << "sift collumns : " << descriptors.cols << "\n";  // 128 (std number of sift vector)

    // SPLIT.TXT tester

    std::string split_txt = "split.txt";
    std::vector<std::string> tester = txt_reader(split_txt);
    std::cout << "vector size: " << tester.size() << "\n" ; //if the size is 70 then the test is completed. no point on iterating to see every result. (it is 70 after the test)
    //master matrix tester
    std::vector<std::vector<float>> master_matrix;
    



    return 0;
}