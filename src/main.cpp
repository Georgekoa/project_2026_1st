#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "functions.h"
#include "kmeans.h"
#include "algorithms.h"

int main() {
    //SIFT TESTER + histogram tester
    std::string path = "data/samples/test.jpeg";
    cv::Mat descriptors = SIFT_extractor(path);
    std::cout << "sift rows: " << descriptors.rows << "\n";  // # of keypoints
    std::cout << "sift collumns : " << descriptors.cols << "\n";  // 128 (std number of sift vector)
    // std::vector<std::vector<float>> master_vector;
    // std::mt19937 gen(214);
    //  std::cout << master_vector.size() << "\n";
    // vector_combine(descriptors,master_vector,gen);
    // std::cout << master_vector.size() << "\n";
    



    // SPLIT.TXT tester

    std::string split_txt = "split.txt";
    std::vector<std::string>  training_sequences = txt_reader(split_txt);
    std::cout << "vector size: " << training_sequences.size() << "\n" ; //if the size is 70 then the test is completed. no point on iterating to see every result. (it is 70 after the test)
    //master vector tester
    // std::vector<std::vector<float>> master_vector;
    // collect_training_descriptors(training_sequences, master_vector);
    // for(int i = 0 ; i < master_vector.size(); i ++){   //run once to check that the data is getting collected
    //     for (int j = 0 ; j < 128 ; j++){
    //         std::cout << master_vector[i][j];
    //     }
    // }
    // std::cout << "Master vector rows: " << master_vector.size() << '\n'; 
    // std::cout << "Master vector columns: " << master_vector[0].size() << '\n';

    // //Init centers tester
    // std::vector<std::vector<float>> centers = initialize_centers(master_vector,256);

    // std::cout << "Center count: " << centers.size() << '\n';  //the size of the vector should be 256
    // std::cout << "Dimensions per center: " << centers[0].size() << '\n';   //the size of the first cell on the vector should be 128 


//     // distance tester
//     std::vector<float> point_a = {1, 2, 3};  //test vectors
//     std::vector<float> point_b = {4, 6, 3};

//     //std::cout << calculate_distance(point_a, point_b , 3) << '\n';

//     //best centers tester
//     std::vector<std::vector<float>> points {  //predictable decriptors for testing purposes
//         {1,1,0},
//         {2,1,0},
//         {8,8,0},
//         {9,8,0}
//     };
//     // std::vector<std::vector<float>> centers = initialize_centers(points,2);   //initialze just 2 centers
//     // std::vector<int> assigned_index ; //assume assignments for testing
//     // for (int i = 0 ; i < points.size(); i ++){    //in the outer for loop we find the nearest cenbter for each point (descriptor) then we print it
//     //     int current_center_index = find_nearest_center(points[i], centers); //it returns the index of the nearest center
//     //     std::cout << current_center_index << "\n";
//     //     assigned_index.push_back(current_center_index); 
//     // }
    
//     //update centers tester
//     // for(int i = 0 ; i < centers.size(); i++){        //CENTERS BEFORE
//     //     for(int j = 0; j < 3 ; j++){
//     //         std::cout << centers[i][j]<<" ";
//     //     }
//     //     std::cout << "\n";
//     // }
// KMEANS TESTER
//     //update_centers(assigned_index,points,centers,3);
//     std::vector<std::vector<float>> centers = kmeans(points, 3, 100, 2);
//     for(int i = 0 ; i < centers.size(); i++){        //CENTERS AFTER
//         for(int j = 0; j < 3 ; j++){
//             std::cout << centers[i][j]<<" ";
//         }
//         std::cout << "\n";
//     }

    
//     return 0;



//DATA SET training
// int dimensions = 128;
// std::vector<std::vector<float>> centers = kmeans(master_vector, dimensions, 30, 256);
// bool all_valid = true;
// std::cout << centers.size() << "\n";
// std::cout <<centers[0].size() << "\n";

// for(int i = 0 ; i < centers.size(); i++){
//     for(int j = 0 ; j < dimensions; j++){
//         if(std::isfinite(centers[i][j])!=true){
//             all_valid = false;
//             break;
//         }
//     }
// }
//         std::cout<< all_valid;

std::filesystem::path vocabulary_30_txt = "./vocabulary_30.txt";
//std::filesystem::path test_txt = "./test.txt" ;   //testing purposes
// save_centers(centers, vocabulary_30_txt);  //file already exists, uncomment to create it
std::vector<std::vector<float>> centers_no_kmeans = load_centers(vocabulary_30_txt);
// if (centers_no_kmeans == centers){
//     std::cout << "loading works \n";
// }
// else{
//     std::cout << "loading doesnt work \n" ;
// }
//save_centers(centers_no_kmeans,test_txt);   //only for testing to see that the two files are identical
// std::vector<float> histogram = create_histogram(centers_no_kmeans, descriptors);    //test images descriptors 
// std::cout << "histogram size: " << histogram.size() << "\n";
// float sum=0;
// for(int i = 0 ; i < histogram.size(); i++){
//     if(histogram[i]>1 || histogram[i] < 0){
//         std::cerr << "found out-of-bounds value\n";
//     }
//     //std::cout << histogram[i] << " ";
//     sum+= histogram[i];
//     }
//     std::cout << "total sum is: " << sum << "\n" ; 

std::vector<ImageData> database;   //it stores histograms and id for all images
std::string rootpath = "data/hpatches-sequences-release/hpatches-sequences-release/" ;
for(int i = 0 ; i < training_sequences.size(); i++){
    for(int j = 1 ; j <=6 ;j++){
        std::string relative_path =  rootpath + training_sequences[i] + "/" +  std::to_string(j) + ".ppm" ;
        cv::Mat descriptors = SIFT_extractor(relative_path); //training sequences stores the sequences path
        if (descriptors.empty()) {
            std::cout << "skipping image: " << relative_path << "\n";
            continue;
        }
        ImageData image_data;
        image_data.image_id = relative_path;  //store it in the struct object
        image_data.histogram = create_histogram(centers_no_kmeans, descriptors);
        database.push_back(image_data);
    }
}
    // std::cout <<"total number of proccesses images "<<database.size()<<"\n";
    // //std::cout << database[10].image_id;
    // for(int i = 0 ; i < database.size(); i++){
    //     std::cout << database[i].image_id << " \n";
    // }

   std::vector<Searchresult> leaderboard = exact_search(database[10].histogram, database, 420);
    
    for(int i = 0 ; i < leaderboard.size(); i++){
        //std::cout << i << " " <<leaderboard[i].id << " " << leaderboard[i].distance << "\n" ;
        //std::string current_path = rootpath + training_sequences[i] + "/" + std::to_string(i) + ".ppm";
        if( leaderboard[i].image_id.find("/i_bridger/") != std::string::npos ){   //it means if this result is not NOT FOUND (we then find it)
            std::cout << i << " " <<leaderboard[i].image_id << " " << leaderboard[i].distance << "\n" ;
        }
    }

}