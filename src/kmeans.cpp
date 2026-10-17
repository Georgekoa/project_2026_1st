#include <iostream>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 
#include <random>
#include <vector>
#include <algorithm>

std::vector<std::vector<float>> initiliaze_centers(std::vector<std::vector<float>>& master_vector){
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