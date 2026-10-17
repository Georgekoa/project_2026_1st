#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <opencv2/core.hpp>

cv::Mat SIFT_extractor(const std::string& path);
std::vector<std::string> txt_reader(std::string& file_path);  
void Matrix_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_matrix);
cv::Mat collect_training_descriptors(std::string& sequence_name);

#endif