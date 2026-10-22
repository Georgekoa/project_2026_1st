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

std::vector<int> assign_centers(std::vector<std::vector<float>> master_vector, std::vector<std::vector<float>> centers){
    std::vector<int> assigned_centers;
    for (int i = 0 ; i < master_vector.size(); i++){
       int current_center_index = find_nearest_center(master_vector[i],centers);
        assigned_centers.push_back(current_center_index ) ;
    }
    return assigned_centers;
}

//centers.size =  # of centers
void update_centers(std::vector<int>& assigned_index, std::vector<std::vector<float>>& master_vector, std::vector<std::vector<float>>& centers, int dimensions){   //dimensions will be 128 for sift descriptors, assigned_index.size = master_vector = # of descs
    std::vector<std::vector<float>> sums(centers.size(), std::vector<float>(dimensions, 0.0f));   //a vector of vectors again to store for each center the sum of their corresponding descriptors (in different dimensions)
    std::vector<int> counts(centers.size(),0);  //the count of total descriptors assigned to each center
    for(int i = 0 ; i < master_vector.size() ; i++){
        counts[assigned_index[i]]+=1;   //at the center's position in counts that corresponds to i incerement by 1
        for(int j = 0 ; j < dimensions ; j ++){
            sums[assigned_index[i]][j] += master_vector[i][j]; 
        }

    }
    for (int i=0 ; i < counts.size() ; i++){  //counts.size is the same as centers.size 
        for(int j = 0 ; j < dimensions ; j ++){
            if (counts[i] == 0) { //if no one uses this center then we leave it as is.
                continue; 
            }
            centers[i][j]=sums[i][j]/counts[i];
        }
    }

}


std::vector<std::vector<float>> kmeans(std::vector<std::vector<float>>& master_vector, int dimensions, int kmeans_max_iterations, int center_count){  //it will return the centers vector of vectors
    std::cout<< "test"<<"\n";
    std::vector<std::vector<float>> centers = initialize_centers(master_vector,center_count);   
    std::cout<< "test2"<<"\n";
    std::vector<int> assigned_centers = assign_centers(master_vector, centers);
    std::cout<< "test3"<<"\n";
    std::vector<int> old_assignments = assigned_centers;
    std::cout<< "test4"<<"\n";
    for(int i = 0 ; i < kmeans_max_iterations; i ++){
        std::cout<< "test"<< i << "\n";
        update_centers(assigned_centers,master_vector, centers, dimensions);
        assigned_centers = assign_centers(master_vector, centers);
        if(old_assignments == assigned_centers){
            break;          //optimal state, no change in centers
        }
        old_assignments = assigned_centers;         //old will be used in the next iteration.
    }
    return centers;
}