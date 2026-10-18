#include <iostream>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 
#include <random>
#include <vector>
#include <algorithm>
#include <limits>

std::vector<std::vector<float>> initialize_centers(std::vector<std::vector<float>>& master_vector, int center_count){ 
    std::vector<int> indexes(master_vector.size());  //initialze a vector of intergers where we keep the ~200k indexes

    for (int i = 0; i < indexes.size(); i++){
        indexes[i] = i;
    }

    std::mt19937 gen(214);  //our selected seed
    std::shuffle(indexes.begin(), indexes.end(), gen); //we suffle the vector of indexes

    std::vector<std::vector<float>> centers;

    for (int i = 0; i < center_count; i++){
        centers.push_back(master_vector[indexes[i]]);   //and we push back to our centers vector the actual rows (vectors) of our master vector that corresponds to our random 256 indexes.
    }
    return centers;
}

float calculate_distance(std::vector<float>& point_a , std::vector<float>& point_b, int dimensions){    //dimensions is probably going to be used only for testing, as the normal dimensions count is 128
    float sum = 0; 
    for(int i = 0 ; i < dimensions ; i++){   //if dimensions are 128 then we iterate to 127
        sum += (point_a[i]-point_b[i])*(point_a[i]-point_b[i]);
    }
    return sum;   //we return sum altough the sum here is not exactly the distance, but rather distance^2 , because it works either way the same way.
}

int find_nearest_center(std::vector<float> descriptor, std::vector<std::vector<float>> centers){
    float min = std::numeric_limits<float>::infinity();   
    int min_index = 0 ; 
    for(int i = 0 ; i < centers.size(); i++){                         //for each center calcualte the best center for the received descriptor and return it.
        float distance = calculate_distance(descriptor, centers[i],descriptor.size());
        if(distance < min){
            min = distance ;
            min_index = i ;  //keep the index of where is the best (until now) center so we can return it from the centers vector of vectors
        }
    }
    return min_index; 
}