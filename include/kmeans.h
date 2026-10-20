#ifndef KMEANS_H
#define KMEANS_H
#include <vector>

std::vector<std::vector<float>> initialize_centers(std::vector<std::vector<float>>& master_vector, int center_count);
float calculate_distance(std::vector<float>& point_a , std::vector<float>& point_b, int dimensions);
int find_nearest_center(std::vector<float> descriptor, std::vector<std::vector<float>> centers);
std::vector<int> assign_centers(std::vector<std::vector<float>> master_vector, std::vector<std::vector<float>> centers );   //vector of indexes of centers 
void update_centers(std::vector<int>& assigned_index, std::vector<std::vector<float>>& master_vector, std::vector<std::vector<float>>& centers, int dimensions);  //assigned index is a vector with size # of descriptors which has an int inside pointing us to which centers its assigned
std::vector<std::vector<float>> kmeans(std::vector<std::vector<float>>& master_vector, int dimensions, int kmeans_max_iterations,int center_count); //puts the pieces all together
#endif