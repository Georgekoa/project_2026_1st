#ifndef EVALUATIONS_H
#define EVALUATIONS_H
#include <vector>
float calculate_recall(std::vector<Searchresult>& result,std::vector<ImageData>& relevant_imgs, int mode);
float calculate_ap10(std::vector<Searchresult>& result,std::vector<ImageData>& relevant_imgs);

#endif