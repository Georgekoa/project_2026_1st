#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

int main() {
    
    std::cout << "hello world" << std::endl;
    cv::Mat image = cv::imread("data/samples/test.jpeg", cv::IMREAD_GRAYSCALE);
    //SIFT doesn't care about color
    
    if (image.empty()) {       //normal error printing in case of image load failure
        std::cerr << "Error: Could not load image\n";
        return 1;
    }


    std::cout << "Image loaded successfully!\n";
    std::cout << "Width: " << image.cols << '\n';
    std::cout << "Height: " << image.rows << '\n';


/////SIFT algorithm is given in class PDFs. will arrange it as well.



    return 0;
}