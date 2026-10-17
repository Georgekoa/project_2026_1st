#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 

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

void vector_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_vector){  //this function will return a vector of vectors so we can combine evrry sift descriptor (up until 500th row) , master mateix is a refernce so it will get updated
    
    if(new_matrix.rows > 500){ 
        for (int i = 0; i<500 ;i++){  //outer for is for rows
            std::vector<float> temp;    //create an empoty vector which is needed for pushing it back on our master vector of vectors
            for(int j = 0 ; j<128 ; j++){   //for collumns (always 128)
               temp.push_back(new_matrix.at<float>(i,j));  //copying each row
            }
            master_vector.push_back(temp);
            
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
    for (int i = 0 ;  i < training_sequences.size() ; i++ ){     //should be 70 no matter what
        std::filesystem::path current_sequence = training_sequences[i];  //even though training_sequences[i] is a string, c++ know how to convert because there is a constructor for it.
        for(int j = 1 ; j <= 6 ; j++){
           cv::Mat temp_matrix;
           std::filesystem::path cur_image_path = rootpath / current_sequence / (std::to_string(j) + ".ppm");
           std::string full_pathname = cur_image_path; //need the path as a string for sift_extrator()
           temp_matrix  = SIFT_extractor(full_pathname);
           vector_combine(temp_matrix, master_vector);
        }
    }
}