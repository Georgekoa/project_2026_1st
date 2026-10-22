#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <opencv2/core.hpp>
#include <random>

cv::Mat SIFT_extractor(const std::string& path);
std::vector<std::string> txt_reader(std::string& file_path);  
void vector_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_vector, std::mt19937& gen);
void collect_training_descriptors(std::vector<std::string>  training_sequences,std::vector<std::vector<float>>& master_vector);

#endif