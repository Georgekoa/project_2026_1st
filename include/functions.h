#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <opencv2/core.hpp>
#include <random>
#include <filesystem>
#include <fstream>  
#include <sstream>
#include <string>

struct ImageData {
    std::string image_id;   //its path
    std::vector<float> histogram;   //its relative histogram 
};

struct DatasetSplit {    //for using all Hpatches images and not only training
    std::vector<std::string> training;   //create vectors of image paths
    std::vector<std::string> validation;
    std::vector<std::string> test;
};
cv::Mat SIFT_extractor(const std::string& path);
std::vector<std::string> txt_reader(std::string& file_path);  
void vector_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_vector, std::mt19937& gen, int S);   //S = our selection of at most descs used for training. while developing we used 500
void collect_training_descriptors(std::vector<std::string>  training_sequences,std::vector<std::vector<float>>& master_vector, int S);
std::vector<std::vector<float >> convert_mat_to_vector(cv::Mat& descriptors);
std::vector<float> create_histogram(std::vector<std::vector<float>>& centers, cv::Mat& descriptors_mat);
DatasetSplit read_dataset_split(std::string& file_path);


#endif