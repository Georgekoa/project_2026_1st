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
    for(int i = 0 ; i < mode; i++){
        for(int j = 0 ; j < 5 ; j++){
            if(result[i].image_id == relevant_imgs[j].image_id){     //so for every image in results check if relevant images are there. (j to iterate through every relevant image to find matches)
                count++;
            }
        }
    }
    return static_cast<float>(count) / 5 ;  //casting so we wont make an integer dividing
}


// float calculate_ap10(std::vector<Searchresult>& result,std::vector<ImageData>& relevant_imgs){
//     int sum = 0;
//     for(int i = 0 ; i < 10; i ++){
        
//     }

// }