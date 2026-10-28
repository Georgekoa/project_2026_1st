#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include "functions.h"
struct Searchresult {
    std::string image_id; //path
    float distance;   //its distance to the image in hand
};

bool compare_distance(Searchresult& a, Searchresult& b);
std::vector<Searchresult> exact_search(ImageData& query, std::vector<ImageData>& database, int total_results);

class LSHIndex {
private:
    int k;          
    int L;          
    double w;       
    int dimensions; 
    int table_size; 
    std::vector<ImageData>* db_ptr;
    std::vector<std::vector<std::vector<float>>> v; // Τυχαία διανύσματα N(0,1)
    std::vector<std::vector<double>> t;             // Μετατοπίσεις στο [0, w)
    std::vector<uint32_t> r;                        // Ακέραιοι συντελεστές για το ID(x)

    struct BucketEntry {
        int image_index;
        uint32_t query_id;
    };

    std::vector<std::vector<std::vector<BucketEntry>>> tables;

    uint32_t compute_id(const std::vector<float>& vec, int table_idx) const;

public:
    LSHIndex(std::vector<ImageData>& database, int k_val = 4, int L_val = 5, double w_val = 0.05, int seed = 214);
    std::vector<Searchresult> query(std::vector<float>& query_histogram, int total_results = 10);
};

#endif