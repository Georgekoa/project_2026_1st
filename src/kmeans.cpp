#include <iostream>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 
#include <random>
#include <vector>
#include <algorithm>

std::vector<std::vector<float>> initialize_centers(std::vector<std::vector<float>>& master_vector){ 
    std::vector<int> indixes(master_vector.size());  //initialze a vector of intergers where we keep the ~200k indexes

    for (int i = 0; i < indixes.size(); i++){
        indixes[i] = i;
    }

    std::mt19937 gen(214);  //our selected seed
    std::shuffle(indixes.begin(), indixes.end(), gen); //we suffle the vector of indexes

    std::vector<std::vector<float>> centers;

    for (int i = 0; i < 256; i++){
        centers.push_back(master_vector[indixes[i]]);   //and we push back to our centers vector the actual rows (vectors) of our master vector that corresponds to our random 256 indexes.
    }
    return centers;
}

float calculate_distance(std::vector<float>& point_a , std::vector<float>& point_b, int dimensions){    //dimensions is probably going to be user only for testing, as the normal dimensions count is 128
    float sum = 0; 
    for(int i = 0 ; i < dimensions ; i++){   //if dimensions are 128 then we iterate to 127
        sum += (point_a[i]-point_b[i])*(point_a[i]-point_b[i]);
    }
    return sum;   //we return sum altough the sum here is not exactly the distance, but rather distance^2 , because it works either way the same way.
}