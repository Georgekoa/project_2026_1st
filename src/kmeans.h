#ifndef KMEANS_H
#define KMEANS_H
#include <vector>

std::vector<std::vector<float>> initialize_centers(std::vector<std::vector<float>>& master_vector, int center_count);
float calculate_distance(std::vector<float>& point_a , std::vector<float>& point_b, int dimensions);
int find_nearest_center(std::vector<float> descriptor, std::vector<std::vector<float>> centers);
#endif