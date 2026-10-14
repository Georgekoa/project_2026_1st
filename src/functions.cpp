#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>

    cv::Mat SIFT_extractor(const std::string& path){  //const & as to not create another copy
    cv::Mat image = cv::imread(path, cv::IMREAD_GRAYSCALE);  //we dont care about color

    if(image.empty()){  //error handling
        std::cerr << "Error: Could not load image\n";
        return cv::Mat(); //empty matrix
    }

    auto sift = cv::SIFT::create();    //create a sift object          
    std::vector<cv::KeyPoint> keypoints; 
    cv::Mat descriptors;
    sift->detectAndCompute(image,cv::noArray(),keypoints,descriptors);  //DAC fills te keypoints vector and the descriptor matrix
    return descriptors;

}


//   wget  http:  //press.liacs.nl/mirflickr/mirflickr25k.v3b/mirflickr25k.zip

std::vector<std::string> txt_reader(std::string& file_path){
    std::ifstream file(file_path);  //open the file
    std::vector<std::string> training_sequences;
    std::string sequence_name;
    std::string split;

    while (file >> sequence_name >> split) { //only whitespace is the actual space in between the line. , so ultimately as long as it can read the line it enters the while loop, when eof it will skip.
        if (split == "train"){
            training_sequences.push_back(sequence_name);
        }
    }

    return training_sequences;

}

