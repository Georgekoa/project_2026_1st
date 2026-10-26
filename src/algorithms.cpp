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


std::vector<Searchresult> exact_search(std::vector<float>& histogram, std::vector<ImageData>& database, int total_results){
    std::vector<Searchresult> results; //a vector to store id and disatance in each cell.
    
   
    
    for(int i = 0 ; i < database.size(); i ++){   //database.size() corresponds to the total number of images we are going to proccess
        Searchresult curr_image;
        curr_image.distance = calculate_distance(histogram, database[i].histogram, 256);
        curr_image.image_id = database[i].image_id;
        results.push_back(curr_image);
        
    }
    
    std::vector<Searchresult> leaderboard;
    std::sort(results.begin(), results.end(), compare_distance);
    for(int j = 0 ; j < total_results; j++){
        leaderboard.push_back(results[j]);
    }   
    return leaderboard;
}


bool compare_distance(Searchresult& a, Searchresult& b){   //compare the diatance of 2 searchresult objects.
    return a.distance < b.distance;
}


