#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <opencv2/core.hpp>

cv::Mat SIFT_extractor(const std::string& path);
std::vector<std::string> txt_reader(std::string& file_path);  

#endif