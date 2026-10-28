#include <iostream>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 
#include <random>
#include <vector>
#include <algorithm>
#include <limits>
#include <algorithms.h>
#include <functions.h>
#include <kmeans.h>
#include <evaluations.h>


float calculate_recall(std::vector<Searchresult>& result, std::vector<ImageData>& relevant_imgs, int mode){   //mode refers to recall@5 or recall@10
   int count = 0 ;
   std::cout << "Results size: " << result.size() << '\n';
    for(int i = 0 ; i < mode; i++){
        std::cout << "Checking result: " << result[i].image_id << '\n';
        for(int j = 0 ; j < 5 ; j++){
            if(result[i].image_id == relevant_imgs[j].image_id){     //so for every image in results check if relevant images are there. (j to iterate through every relevant image to find matches)
                count++;
                std::cout << "MATCH: " << relevant_imgs[j].image_id << '\n';
            }
        }
    }
    return static_cast<float>(count) / 5 ;  //casting so we wont make an integer dividing
}


float calculate_ap10(std::vector<Searchresult>& result,std::vector<ImageData>& relevant_imgs){
    int count = 0 ;
    float sum = 0.0f;
    //int index[5]={0,0,0,0,0};  //init with zero to accomodate for images not appearing in the top 10
    std::vector<int> top10images;
    for(int i = 0 ; i < 10; i++){
        for(int j = 0 ; j < 5 ; j++){
            if(result[i].image_id == relevant_imgs[j].image_id){    
                //index[j]=i;  //find its ranking in the top10
                top10images.push_back(i+1); //so a vector with size as count, and contents the position of the "count" first images
                count++;
            }
        }
    }   //find how many relevant images are in the top 10
    
    for (int j  = 1 ; j <= count ; j ++){
        sum+= static_cast<float>(j)/top10images[j-1];  //1 to =count is needed because the relevant rank cannot be zero, so we find the j-1 element of top10images
    }
     
    return 0.2*sum;
}

std::vector<ImageData> relevant_imgs(std::vector<ImageData>& database, ImageData& image){
    
    std::vector<ImageData> relevant_images;
    std::filesystem::path image_path(image.image_id);
    
    std::string sequence_name = image_path.parent_path().filename().string(); //parent path strips the file name, file name gets only the last portion of the path, and string turn it back into a string
    
    
    for(int i = 0 ; i < database.size(); i ++){
        
       if (database[i].image_id.find(sequence_name) != std::string::npos && database[i].image_id != image.image_id) {  //if the stripped sequence name is is inside a database entry and exluding the same query image
            relevant_images.push_back(database[i]);
        }   
    }
    return relevant_images;
}

//leaderboard[i].image_id.find("/i_bridger/") != std::string::npos