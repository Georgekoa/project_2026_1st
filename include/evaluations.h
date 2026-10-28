#ifndef EVALUATIONS_H
#define EVALUATIONS_H
#include <vector>
float calculate_recall(std::vector<Searchresult>& result,std::vector<ImageData>& relevant_imgs, int mode);
float calculate_ap10(std::vector<Searchresult>& result,std::vector<ImageData>& relevant_imgs);
std::vector<ImageData> relevant_imgs(std::vector<ImageData>& database, ImageData& image);
float calculate_map10(std::vector<ImageData>& database, std::string algorithm);
float calculate_query_time(std::vector<ImageData>& database, std::string algorithm);
#endif