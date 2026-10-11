#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

int main() {
    
    std::cout << "hello world" << std::endl;
    cv::Mat image = cv::imread("data/samples/test.jpeg", cv::IMREAD_GRAYSCALE);
    //SIFT doesn't care about color
    //in class's code we make the path a const char * for clarity but its not neccesary
   
   
    if (image.empty()) {       //normal error printing in case of image load failure
        std::cerr << "Error: Could not load image\n";
        return 1;
    }

    
    std::cout << "Image loaded successfully!\n";
    std::cout << "Width: " << image.cols << '\n';
    std::cout << "Height: " << image.rows << '\n';

    auto sift = cv::SIFT::create();
    std::vector<cv::KeyPoint> keypoints;  //vector of keypoints to store after the function call
    cv::Mat descriptors;  //object to store the descriptor matrix 
    //const char* image_path= "data/samples/test.jpeg";
    sift->detectAndCompute(image,cv::noArray(),keypoints,descriptors);

    std::cout << "keypoints size " << keypoints.size() << '\n' ;
    std::cout << "descripotrs size " << descriptors.rows << '\n' ; //total number of keypoints 
    std::cout << "descripotrs size " << descriptors.cols << '\n' ; //must always be 128 



    return 0;
}