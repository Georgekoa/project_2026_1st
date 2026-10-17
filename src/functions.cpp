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

void Matrix_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_matrix ){  //this function will return a vector of vectors so we can combine evrry sift descriptor (up until 500th row) , master mateix is a refernce so it will get updated
    
    if(new_matrix.rows > 500){ 
        for (int i = 0; i<500 ;i++){  //outer for is for rows
            std::vector<float> temp;    //create an empoty vector which is needed for pushing it back on our master vector of vectors
            for(int j = 0 ; j<128 ; j++){   //for collumns (always 128)
               temp.push_back(new_matrix.at<float>(i,j));  //copying each row
            }
            master_matrix.push_back(temp);
        }
        
    }
    if (new_matrix.rows <= 500 ) //could do it without second if, but we prefer it for clarity
    {
        for (int i = 0; i<new_matrix.rows;i++){     //we do the exact same iterastions but we do it for only the existing rows
            std::vector<float> temp;    
            for(int j = 0 ; j<128 ; j++){   
               temp.push_back(new_matrix.at<float>(i,j));  
            }
            master_matrix.push_back(temp);
        }
        
    }
    
}

// cv::Mat collect_training_descriptors(std::string& sequence_name){  //we will create a cv::mat for the 6 images in the sequence and we will pass it in the main to the matrix combine func

// }