#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include "functions.h"
struct Searchresult {
    std::string image_id; //path
    float distance;   //its distance to the image in hand
};

bool compare_distance(Searchresult& a, Searchresult& b);
std::vector<Searchresult> exact_search(std::vector<float>& histogram, std::vector<ImageData>& database, int total_results);
#endif 