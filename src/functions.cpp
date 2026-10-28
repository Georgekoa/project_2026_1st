#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 
#include <random>
#include <kmeans.h>
#include <functions.h>
#include <chrono>  //for query time

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

void vector_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_vector, std::mt19937& gen){  //this function will return a vector of vectors so we can combine evrry sift descriptor (up until 500th row) , master mateix is a refernce so it will get updated
    

    std::vector<int> indexes(new_matrix.rows);  //init a vector with size as the # of descr
    for(int i = 0 ; i < indexes.size(); i++){
        indexes[i] = i;  
    }
 
   
    std::shuffle(indexes.begin(), indexes.end(), gen);

    
    if(new_matrix.rows > 500){ 
        for(int i = 0 ; i < 500 ; i++){
            std::vector<float> temp;
            for(int j = 0 ; j < new_matrix.cols ;j++){
                temp.push_back(new_matrix.at<float>(indexes[i],j)); // pick the rows of the suffled order
            }
            master_vector.push_back(temp);   //push back the current row, which can be any rows at al.
    }
        return;
    }
    if (new_matrix.rows <= 500 ) //could do it without second if, but we prefer it for clarity
    {
        for (int i = 0; i<new_matrix.rows;i++){     //we do the exact same iterastions but we do it for only the existing rows
            std::vector<float> temp;    
            for(int j = 0 ; j<128 ; j++){   
               temp.push_back(new_matrix.at<float>(i,j));  
            }
            master_vector.push_back(temp);
        }
        
    }
    
}

void collect_training_descriptors(std::vector<std::string>  training_sequences,std::vector<std::vector<float>>& master_vector ){  
    std::filesystem::path rootpath = "data/hpatches-sequences-release/hpatches-sequences-release";
    std::mt19937 gen(214);   //initializing a generator whihc we are going to use over and over
    for (int i = 0 ;  i < training_sequences.size() ; i++ ){     //should be 70 no matter what
        std::filesystem::path current_sequence = training_sequences[i];  //even though training_sequences[i] is a string, c++ know how to convert because there is a constructor for it.
        for(int j = 1 ; j <= 6 ; j++){
           cv::Mat temp_matrix;
           std::filesystem::path cur_image_path = rootpath / current_sequence / (std::to_string(j) + ".ppm");
           std::string full_pathname = cur_image_path; //need the path as a string for sift_extrator()
           temp_matrix  = SIFT_extractor(full_pathname);
           vector_combine(temp_matrix, master_vector, gen);
        }
    }
}


std::vector<float> create_histogram(std::vector<std::vector<float>>& centers, cv::Mat& descriptors_mat){
    std::vector<float> histogram = std::vector<float>(centers.size(),0.0f);
    std::vector<std::vector<float>> descriptors_vector = convert_mat_to_vector(descriptors_mat);
    //int test_sum = 0 ;
    if(descriptors_vector.empty()){
        return histogram;
    }
    for(int i = 0 ; i < descriptors_vector.size() ; i++){
        int index = find_nearest_center(descriptors_vector[i], centers);   //we find that eg on descriptor 1 the nearest centers is 5 . so we go to cell 5 of the histogram and increment it.
        histogram[index]+=1;
    }
    for(int j = 0 ; j < centers.size(); j ++){
        //test_sum += histogram[j]; //before normalazation
        histogram[j] = histogram[j]/descriptors_vector.size();  // we divide by the # of descs so all the cells sum to 1.
    }
    //std::cout << "total unormalized sum is: " << test_sum << "\n";  //test sum, should be descriptors count
    return histogram;
}

std::vector<std::vector<float >> convert_mat_to_vector(cv::Mat& descriptors){
    std::vector<std::vector<float>> converted_vector;
    if(descriptors.empty()){  //cv::mat function 
        std::cerr<<"No descriptors \n" ;
        return converted_vector; //empty at this point
    }
    
    for(int i = 0 ; i < descriptors.rows ; i ++){       // this loop is for all the matrix
        std::vector<float> temp;
        for(int j = 0 ; j < descriptors.cols ; j ++){  //this loop is for individual descriptors
            temp.push_back(descriptors.at<float>(i,j));  //cv::mat doesnt support dimensions[i][j]
        }
        converted_vector.push_back(temp);
    }
    return converted_vector;
}