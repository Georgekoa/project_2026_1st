#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "functions.h"

int main() {
    //SIFT TESTER
    std::string path = "data/samples/test.jpeg";
    cv::Mat descriptors = SIFT_extractor(path);
    std::cout << "sift rows: " << descriptors.rows << "\n";  // # of keypoints
    std::cout << "sift collumns : " << descriptors.cols << "\n";  // 128 (std number of sift vector)

    // SPLIT.TXT tester

    std::string split_txt = "split.txt";
    std::vector<std::string>  training_sequences = txt_reader(split_txt);
    std::cout << "vector size: " << training_sequences.size() << "\n" ; //if the size is 70 then the test is completed. no point on iterating to see every result. (it is 70 after the test)
    //master vector tester
    std::vector<std::vector<float>> master_vector;
    collect_training_descriptors(training_sequences, master_vector);
    // for(int i = 0 ; i < master_vector.size(); i ++){   //run once to check that the data is getting collected
    //     for (int j = 0 ; j < 128 ; j++){
    //         std::cout << master_vector[i][j];
    //     }
    // }
    std::cout << "Master vector rows: " << master_vector.size() << '\n'; 
    std::cout << "Master vector columns: " << master_vector[0].size() << '\n';

    return 0;
}