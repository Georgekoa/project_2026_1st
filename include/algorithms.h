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




struct IVFFlatIndex{     //actual IVFFlat database 
    std::vector<std::vector<float>> centers;      // kmeans centers (256 floats this time instead of the 128 of prev kmeans - 256 floats per histogram)
    std::vector<std::vector<int>> inverted_lists;  //inverted_lists.size() is the # of centers. and each cell has another vector of ints that stores the indexes of the actual histogram that belong to current center
};
std::vector<std::vector<float>> create_histogram_database(std::vector<ImageData>& database);
IVFFlatIndex build_ivff_index(std::vector<ImageData>& database, int centers_count);   
std::vector<int> find_nearest_centers(std::vector<float>& query, IVFFlatIndex& index, int nprobe);
std::vector<Searchresult> ivfflat_search(std::vector<float>& query, std::string& query_image_id, std::vector<ImageData>& database, IVFFlatIndex& index, int nprobe, int total_results);


struct IVFPQIndex{
    std::vector<std::vector<float>> centers;      
    std::vector<std::vector<int>> inverted_lists;    //both are the same fields as IVFFlat
    std::vector<std::vector<std::vector<float>>> codebooks;  //16 codebooks -> containing 256 centers of 16 dimensions
    std::vector<std::vector<int>> compressed_codes;  //16 ints to represent each image
};

IVFPQIndex build_ivfpq_index(std::vector<ImageData>& database, int centers_count, int M, int nbits);
std::vector<Searchresult> ivfpq_search(std::vector<float>& query,std::string& query_image_id,std::vector<ImageData>& database, IVFPQIndex& index, int nprobe, int total_results);
#endif