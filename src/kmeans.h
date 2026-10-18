#ifndef KMEANS_H
#define KMEANS_H
#include <vector>

std::vector<std::vector<float>> initialize_centers(std::vector<std::vector<float>>& master_vector);
float calculate_distance(std::vector<float>& point_a , std::vector<float>& point_b, int dimensions);
#endif